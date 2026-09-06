#include "recording/AudioTap.h"
#include <algorithm>
#include <cstring>
#include <thread>

namespace
{
    // Sets `flag` true for the lifetime of the guard, false on every exit
    // path (RAII -- push() has several early returns) -- see
    // audioThreadBusy_'s own comment in AudioTap.h and stopInternal() below
    // for the race this closes. seq_cst on both sides: this runs at most
    // once per audio block, the cost is one uncontended store either way,
    // and seq_cst removes any doubt about the ordering against
    // stopInternal()'s acquire load without having to reason about a
    // weaker pairing.
    struct ScopedBusyFlag
    {
        std::atomic<bool>& flag;
        explicit ScopedBusyFlag(std::atomic<bool>& f) : flag(f) { flag.store(true, std::memory_order_seq_cst); }
        ~ScopedBusyFlag() { flag.store(false, std::memory_order_seq_cst); }
        ScopedBusyFlag(const ScopedBusyFlag&) = delete;
        ScopedBusyFlag& operator=(const ScopedBusyFlag&) = delete;
    };
}

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
    silenceDebtFrames_ = 0;
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
    // s168 review fix (stop()/push() UAF race): busy for the whole call, on
    // every return path (RAII) -- see audioThreadBusy_'s comment in
    // AudioTap.h and stopInternal() below. Wait-free (a store on entry, a
    // store on exit, no CAS/loop/lock): the ONLY thing this adds to the
    // audio-thread path is two uncontended atomic stores.
    ScopedBusyFlag busy(audioThreadBusy_);

#if defined(AUDIODNA_AUDIOTAP_TEST_HOOKS)
    // TEST ONLY -- see debugArmPushBlockForTest() in AudioTap.h. Deliberately
    // placed right after the busy guard above (not before it): this is the
    // exact "in flight, busy == true" window stopInternal()'s wait loop is
    // built to wait out.
    if (debugBlockPushForTest_.exchange(false, std::memory_order_acq_rel))
    {
        debugPushIsBlockedForTest_.store(true, std::memory_order_release);
        while (!debugReleasePushForTest_.load(std::memory_order_acquire))
            std::this_thread::yield();
        debugPushIsBlockedForTest_.store(false, std::memory_order_relaxed);
        debugReleasePushForTest_.store(false, std::memory_order_relaxed);
    }
#endif

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

    // Fewer channels than the writer was created with -- pad the missing
    // trailing channels with silence via a scratch array pre-sized in
    // prepare() (no allocation on the audio thread). channels_ > 0 is
    // guaranteed here (checked above), so chans == 0 always lands here too:
    // this is the deliberate path for "no analyzable buffer this block"
    // (ch == nullptr, chans == 0, s168 review's "listened == nullptr"
    // case, CombinedCallback.h) -- the loop below is simply empty when
    // chans == 0, so ch is never dereferenced.
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

    // s168 review fix: pay down any outstanding "must become silence" debt
    // (spillIntoPending's genuine-overrun branch, below) BEFORE this call's
    // data is considered -- the debt is strictly older (the tail of an
    // earlier block that could not be buffered) than whatever this call is
    // about to do.
    if (silenceDebtFrames_ > 0)
        queueSilenceDebt();

    if (pendingFrames_ == 0 && silenceDebtFrames_ == 0)
    {
        if (tryRealWrite(data, numSamples))
            return;
    }

    // Either older pending data (or unpaid silence debt) still hasn't
    // drained -- must preserve frame order -- cannot let this block's data
    // overtake it -- or the direct write just failed/was forced to fail:
    // spill the whole block.
    spillIntoPending(data, numSamples);
}

void AudioTap::flushPendingNonBlocking()
{
    if (pendingFrames_ <= 0) return;
    if (tryRealWrite(pendingChannelPtrs_.data(), pendingFrames_))
        pendingFrames_ = 0;
    // else: leave it queued -- the next push() call retries.
}

void AudioTap::queueSilenceDebt()
{
    // Writes as much of the outstanding silence debt as currently fits into
    // pendingStorage_, in maxBlock_-sized chunks (silenceStorage_ is only
    // ever sized to maxBlock_ per channel -- see prepare()). Audio-thread,
    // RT-safe: a bounded number of memcpys into fixed-capacity storage,
    // same as spillIntoPending itself; no allocation.
    while (silenceDebtFrames_ > 0)
    {
        const int freeCapacity = pendingCapacityFrames_ - pendingFrames_;
        if (freeCapacity <= 0) break;   // no room yet -- next call retries

        const int chunk = std::min({ silenceDebtFrames_, freeCapacity, maxBlock_ });
        for (int c = 0; c < channels_; ++c)
            std::memcpy(pendingStorage_[static_cast<size_t>(c)].data() + pendingFrames_,
                        silenceStorage_[static_cast<size_t>(c)].data(),
                        static_cast<size_t>(chunk) * sizeof(float));
        pendingFrames_ += chunk;
        silenceDebtFrames_ -= chunk;
    }
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
        // own retry headroom (T1's forced-failure window is deliberately
        // shorter than pendingCapacityFrames_; the genuine-overrun test
        // drives a longer one). The frame-count invariant (framesWritten_
        // == deliveredSamples - firstSample) is preserved NOT by writing
        // this content -- it is genuinely, unrecoverably lost; there is no
        // room -- but by recording a debt of that many SILENCE frames in
        // silenceDebtFrames_, which queueSilenceDebt() (writeBlockRetrying,
        // above) writes into this same buffer, in order, ahead of any
        // later block's real data, the moment room exists again (and which
        // stopInternal() also drains directly if stop() lands mid-overrun).
        // Content is lost; the frame count and the take's timeline are not.
        const uint32_t lost = static_cast<uint32_t>(numSamples - accepted);
        droppedFrames_.fetch_add(lost, std::memory_order_relaxed);
        silenceDebtFrames_ += static_cast<int>(lost);
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
    // seq_cst, paired with stopInternal()'s seq_cst null-store: together
    // with audioThreadBusy_ (also seq_cst), this guarantees any push() call
    // that could still observe a non-null activeWriter_ after
    // stopInternal() has nulled it has already finished doing so (and
    // therefore any call into w->write() below) before stopInternal()
    // destroys the writer -- see stopInternal()'s own comment.
    auto* w = activeWriter_.load(std::memory_order_seq_cst);
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
    // Null this FIRST (seq_cst -- see tryRealWrite()'s matching load) so
    // any push() call that loads it fresh sees the tap as unwritable before
    // the writer is torn down below.
    activeWriter_.store(nullptr, std::memory_order_seq_cst);

    // s168 review fix (stop()/push() UAF race): wait out a push() call that
    // may ALREADY be inside tryRealWrite()/w->write() at this exact
    // instant, holding a copy of the (now stale) OLD non-null pointer read
    // before the store above took effect. Without this wait,
    // threadedWriter_.reset() below could destroy the ThreadedWriter out
    // from under a write() call still running on the real audio thread --
    // the exact race the s168 review flagged.
    //
    // Unbounded on purpose, not a defended-against hang: push()'s entire
    // call graph is RT-safe by construction -- no lock, no allocation, no
    // blocking I/O, only a bounded number of memcpys into fixed-capacity
    // storage sized once in prepare() (see the class-level threading
    // contract and every push()-reachable function's own comment) -- so
    // this flag is provably cleared again within one push() call's bounded
    // duration. Blocking here is explicitly fine per this class's own
    // threading contract (stop() runs on the message thread, off the audio
    // thread); the audio thread itself never waits on anything in return --
    // it only ever performs the wait-free store/load pair in
    // ScopedBusyFlag (AudioTap.cpp, anonymous namespace).
    while (audioThreadBusy_.load(std::memory_order_seq_cst))
        std::this_thread::yield();

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

        // s168 review fix (spillIntoPending's genuine-overrun branch, D10.1's
        // FIFO-overrun bullet): any outstanding "must still become silence"
        // debt that hadn't drained yet must still land in the file now, or
        // the WAV would end short of deliveredSamples_ - firstSample_ even
        // though droppedFrames_/unreliableFrom() already explain why.
        while (silenceDebtFrames_ > 0)
        {
            const int chunk = std::min(silenceDebtFrames_, maxBlock_);
            if (threadedWriter_->write(silenceChannelPtrs_.data(), chunk))
                silenceDebtFrames_ -= chunk;
            else
                break;   // still stalled -- give up rather than spin forever; droppedFrames_ already says why
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
