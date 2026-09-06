#include "recording/AudioTap.h"
#include <algorithm>
#include <cstring>

AudioTap::AudioTap() {}

AudioTap::~AudioTap()
{
    stopInternal();
    if (flushThread_.isThreadRunning())
        flushThread_.stopThread(2000);
}

void AudioTap::prepare(double deviceRate, int channels, int maxBlockSize)
{
    const int clampedChannels = std::max(0, channels);
    const int clampedMaxBlock = std::max(1, maxBlockSize);

    const bool wasPrepared = (rate_ > 0.0 || channels_ > 0);
    const bool rateOrChannelsChanged = wasPrepared && (deviceRate != rate_ || clampedChannels != channels_);

    // D10.1: "device stop/restart -- the counter continues" -- a restart
    // (even a benign block-size-only one) is NOT a dropout. Reset the
    // gap-detection baseline unconditionally on every prepare() so the
    // first post-restart block never gets compared against a hostTimeNs
    // reading from before the (possibly real-wall-clock-costing) pause --
    // safe to touch this push()-thread field here because JUCE only calls
    // audioDeviceAboutToStart while the callback thread is quiesced (the
    // same guarantee AudioCallback/AudioSourcePlayer already lean on for
    // their own audioDeviceAboutToStart resizing).
    haveLastHostTime_ = false;

    if (rateOrChannelsChanged && (running_.load(std::memory_order_relaxed) || armed_.load(std::memory_order_relaxed)))
    {
        // D10.1: a genuine rate/channel change mid-take needs a new segment
        // ("audio-2.wav") -- that bookkeeping (the take's segment list) is
        // Take-level, step 3's job, NOT this packet's TARGET FILES. This
        // tap's own contract stays simple and safe: stop cleanly rather
        // than keep writing at a channel/rate the open file's header no
        // longer matches. See the builder report's DEVIATIONS -- this path
        // is deliberately NOT what T1 exercises (its "restart" fault
        // injection changes block size only, not rate/channels).
        stopInternal();
    }

    const bool needRealloc = rateOrChannelsChanged || !wasPrepared;

    rate_ = deviceRate;
    channels_ = clampedChannels;

    if (needRealloc)
    {
        // ~2 s of retry headroom -- D10.3 step 4's forced-failure window is
        // 3 blocks, far under this; sized here (message thread) so push()
        // never allocates.
        pendingCapacityFrames_ = static_cast<int>(rate_ > 0.0 ? rate_ * 2.0 : 96000.0);
        pendingStorage_.assign(static_cast<size_t>(channels_),
                                std::vector<float>(static_cast<size_t>(pendingCapacityFrames_), 0.0f));
        pendingChannelPtrs_.resize(static_cast<size_t>(channels_));
        for (int c = 0; c < channels_; ++c)
            pendingChannelPtrs_[static_cast<size_t>(c)] = pendingStorage_[static_cast<size_t>(c)].data();
        pendingFrames_ = 0;
        scratchChannelPtrs_.resize(static_cast<size_t>(channels_));
    }

    if (needRealloc || clampedMaxBlock > maxBlock_)
    {
        maxBlock_ = std::max(maxBlock_, clampedMaxBlock);
        silenceStorage_.assign(static_cast<size_t>(channels_),
                                std::vector<float>(static_cast<size_t>(maxBlock_), 0.0f));
        silenceChannelPtrs_.resize(static_cast<size_t>(channels_));
        for (int c = 0; c < channels_; ++c)
            silenceChannelPtrs_[static_cast<size_t>(c)] = silenceStorage_[static_cast<size_t>(c)].data();
    }
    else
    {
        maxBlock_ = clampedMaxBlock;
    }

    if (!flushThread_.isThreadRunning())
        flushThread_.startThread(juce::Thread::Priority::normal);
}

bool AudioTap::start(const juce::File& wavFile)
{
    stopInternal();   // in case a previous take on this tap is still open

    if (freeSpaceBytes(wavFile.getParentDirectory()) < kMinFreeBytes)
        return false;   // R15 mechanism: refuse rather than start a take that will hit the FIFO-overrun path almost immediately

    if (!wavFile.getParentDirectory().exists())
        wavFile.getParentDirectory().createDirectory();

    auto stream = std::make_unique<juce::FileOutputStream>(wavFile);
    if (!stream->openedOk())
        return false;

    auto* writer = wavFormat_.createWriterFor(stream.get(), rate_, static_cast<unsigned int>(channels_), 16, {}, 0);
    if (writer == nullptr)
        return false;
    stream.release();   // the writer now owns the stream (AudioRecordingDemo pattern, G29)

    // D10.1's 8-second FIFO -- numSamplesToBuffer is FRAMES, not
    // frames*channels (confirmed against JUCE's own Buffer ctor).
    threadedWriter_ = std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(
        writer, flushThread_, static_cast<int>(rate_ * 8.0));
    activeWriter_.store(threadedWriter_.get(), std::memory_order_relaxed);

    hasUnreliableFrom_.store(false, std::memory_order_relaxed);
    unreliableFromSample_.store(0, std::memory_order_relaxed);
    droppedFrames_.store(0, std::memory_order_relaxed);
    framesWritten_.store(0, std::memory_order_relaxed);
    gapDetectionSupported_.store(true, std::memory_order_relaxed);
    pendingFrames_ = 0;
    haveLastHostTime_ = false;

    // Release-paired with push()'s armed_.exchange(acquire): everything
    // written above (activeWriter_, the counter resets) becomes visible to
    // the audio thread the moment it observes armed_ == true.
    armed_.store(true, std::memory_order_release);
    return true;
}

uint32_t AudioTap::push(const float* const* ch, int chans, int numSamples,
                          uint64_t deliveredBefore,
                          const juce::AudioIODeviceCallbackContext& context)
{
    // Unconditional RMW (cheap, uncontended, no lock) rather than a relaxed
    // peek-then-exchange -- a relaxed peek could miss a same-instant
    // release-store from start() on the message thread and silently skip
    // the very block that should have captured firstSample.
    if (armed_.exchange(false, std::memory_order_acquire))
    {
        firstSample_.store(deliveredBefore, std::memory_order_relaxed);
        running_.store(true, std::memory_order_relaxed);
        // Never compare hostTimeNs across the arm boundary (D10.1's
        // sample-exact start -- whatever happened before arming is not
        // this take's problem).
        haveLastHostTime_ = false;
    }

    if (!running_.load(std::memory_order_relaxed))
        return 0;   // called every block regardless of recording state (D10's "second fan-out"); no-op when idle

    uint32_t gapFrames = 0;
    const bool haveHostTime = (context.hostTimeNs != nullptr);
    if (!haveHostTime)
    {
        // R14: latches false permanently for this take -- see header.
        gapDetectionSupported_.store(false, std::memory_order_relaxed);
    }

    if (haveHostTime && gapDetectionSupported_.load(std::memory_order_relaxed))
    {
        if (haveLastHostTime_ && rate_ > 0.0)
        {
            const double deltaSeconds = static_cast<double>(*context.hostTimeNs - lastHostTimeNs_) * 1.0e-9;
            const double expected = deltaSeconds * rate_;
            const double over = expected - static_cast<double>(numSamples);
            if (over > static_cast<double>(numSamples) * 0.5)
            {
                gapFrames = static_cast<uint32_t>(over + 0.5);
                writeSilenceFrames(gapFrames);
                pushGapMarker(deliveredBefore, gapFrames);
            }
        }
        lastHostTimeNs_ = *context.hostTimeNs;
        haveLastHostTime_ = true;
    }

    writeFrames(ch, chans, numSamples);
    return gapFrames;
}

void AudioTap::writeFrames(const float* const* ch, int chans, int numSamples)
{
    if (numSamples <= 0 || channels_ <= 0) return;
    framesWritten_.fetch_add(static_cast<uint64_t>(numSamples), std::memory_order_relaxed);

    if (chans >= channels_)
    {
        // Extra channels beyond channels_ (if any) are simply not read.
        writeBlockRetrying(ch, numSamples);
        return;
    }

    // Defensive, not expected in practice (see header/report ASSUMPTION):
    // fewer channels than the writer was created with. Pad the missing
    // trailing channels with silence via a scratch array pre-sized in
    // prepare() -- no allocation on the audio thread.
    for (int c = 0; c < chans; ++c)
        scratchChannelPtrs_[static_cast<size_t>(c)] = ch[c];
    for (int c = chans; c < channels_; ++c)
        scratchChannelPtrs_[static_cast<size_t>(c)] = silenceChannelPtrs_[0];
    writeBlockRetrying(scratchChannelPtrs_.data(), numSamples);
}

void AudioTap::writeSilenceFrames(uint32_t numFrames)
{
    if (numFrames == 0 || channels_ <= 0) return;
    framesWritten_.fetch_add(static_cast<uint64_t>(numFrames), std::memory_order_relaxed);

    uint32_t remaining = numFrames;
    while (remaining > 0)
    {
        const int chunk = static_cast<int>(std::min<uint32_t>(remaining, static_cast<uint32_t>(maxBlock_)));
        writeBlockRetrying(silenceChannelPtrs_.data(), chunk);
        remaining -= static_cast<uint32_t>(chunk);
    }
}

void AudioTap::writeBlockRetrying(const float* const* data, int numSamples)
{
    if (numSamples <= 0) return;

    if (pendingFrames_ > 0)
        flushPendingNonBlocking();

    if (pendingFrames_ == 0)
    {
        if (tryRealWrite(data, numSamples))
            return;
    }

    // Either older pending data still hasn't drained (must preserve
    // frame order -- cannot let this block's data overtake it), or the
    // direct write just failed/was forced to fail: spill the whole block.
    spillIntoPending(data, numSamples);
}

void AudioTap::flushPendingNonBlocking()
{
    if (pendingFrames_ <= 0) return;
    if (tryRealWrite(pendingChannelPtrs_.data(), pendingFrames_))
        pendingFrames_ = 0;
    // else: leave it queued -- the next push() call retries.
}

void AudioTap::spillIntoPending(const float* const* data, int numSamples)
{
    if (numSamples <= 0 || channels_ <= 0) return;

    const int freeCapacity = pendingCapacityFrames_ - pendingFrames_;
    const int accepted = std::min(numSamples, std::max(0, freeCapacity));

    for (int c = 0; c < channels_; ++c)
        std::memcpy(pendingStorage_[static_cast<size_t>(c)].data() + pendingFrames_,
                    data[c], static_cast<size_t>(accepted) * sizeof(float));
    pendingFrames_ += accepted;

    if (accepted < numSamples)
    {
        // D10.1's FIFO-overrun path: the stall outlasted even this tap's
        // own retry headroom (untested by T1 -- its forced-failure window
        // is deliberately shorter than pendingCapacityFrames_). The
        // frame-count invariant (framesWritten_ == deliveredSamples -
        // firstSample) was already preserved by the caller incrementing
        // framesWritten_ before this call; what's lost here is CONTENT,
        // counted and flagged rather than silently dropped.
        const uint32_t lost = static_cast<uint32_t>(numSamples - accepted);
        droppedFrames_.fetch_add(lost, std::memory_order_relaxed);
        if (!hasUnreliableFrom_.exchange(true, std::memory_order_relaxed))
        {
            const uint64_t framesBeforeThisBlock =
                framesWritten_.load(std::memory_order_relaxed) - static_cast<uint64_t>(numSamples);
            unreliableFromSample_.store(
                firstSample_.load(std::memory_order_relaxed) + framesBeforeThisBlock + static_cast<uint64_t>(accepted),
                std::memory_order_relaxed);
        }
    }
}

bool AudioTap::tryRealWrite(const float* const* data, int numSamples)
{
#if defined(AUDIODNA_AUDIOTAP_TEST_HOOKS)
    if (forcedFailuresRemaining_ > 0)
    {
        --forcedFailuresRemaining_;
        return false;
    }
#endif
    auto* w = activeWriter_.load(std::memory_order_relaxed);
    return w != nullptr && w->write(data, numSamples);
}

void AudioTap::pushGapMarker(uint64_t sample, uint32_t n)
{
    int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
    gapFifo_.prepareToWrite(1, start1, size1, start2, size2);
    if (size1 > 0)
        gapStorage_[static_cast<size_t>(start1)] = { sample, n };
    else if (size2 > 0)
        gapStorage_[static_cast<size_t>(start2)] = { sample, n };
    gapFifo_.finishedWrite(size1 + size2);
    // A full 64-entry backlog (size1==size2==0) drops the marker -- the
    // recorder's live gap list is a convenience feed; droppedFrames_ /
    // unreliableFrom() remain the authoritative signal either way.
}

bool AudioTap::popGap(std::pair<uint64_t, uint32_t>& out)
{
    int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
    gapFifo_.prepareToRead(1, start1, size1, start2, size2);
    if (size1 > 0)
    {
        out = gapStorage_[static_cast<size_t>(start1)];
        gapFifo_.finishedRead(size1);
        return true;
    }
    if (size2 > 0)
    {
        out = gapStorage_[static_cast<size_t>(start2)];
        gapFifo_.finishedRead(size2);
        return true;
    }
    return false;
}

void AudioTap::stop()
{
    stopInternal();
}

void AudioTap::stopInternal()
{
    // Null this FIRST so any push() call that loads it fresh sees the tap
    // as unwritable before the writer is torn down.
    //
    // KNOWN LIMITATION (see builder report RISKS): this does not by itself
    // make stop() safe to call concurrently with an in-flight push() on
    // another (the real audio) thread -- a push() that already loaded a
    // non-null activeWriter_ before this store could still be inside
    // ThreadedWriter::write() while the destructor below runs. T1 never
    // exercises this (its single-threaded FakeDevice harness never calls
    // stop() concurrently with push()); step 3's caller is ASSUMED to
    // serialize AudioTap::stop() against the audio thread the same way
    // AudioEngine::stop()/loadFile() already lean on AudioTransportSource's
    // own internal locking for the equivalent transport-swap race.
    activeWriter_.store(nullptr, std::memory_order_release);

    if (threadedWriter_)
    {
        // Flush our own still-pending (retried) frames first -- synchronous
        // and blocking is fine here, off the audio thread.
        while (pendingFrames_ > 0)
        {
            if (threadedWriter_->write(pendingChannelPtrs_.data(), pendingFrames_))
                pendingFrames_ = 0;
            else
                break;   // the real writer's own FIFO is still full -- give up rather than spin forever
        }
        threadedWriter_.reset();   // destructor flushes to disk (blocking, message thread -- fine)
    }

    armed_.store(false, std::memory_order_relaxed);
    running_.store(false, std::memory_order_relaxed);
}

std::optional<uint64_t> AudioTap::unreliableFrom() const
{
    if (!hasUnreliableFrom_.load(std::memory_order_relaxed))
        return std::nullopt;
    return unreliableFromSample_.load(std::memory_order_relaxed);
}

int64_t AudioTap::freeSpaceBytes(const juce::File& folder)
{
    return static_cast<int64_t>(folder.getBytesFreeOnVolume());
}
