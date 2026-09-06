#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <atomic>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

// AudioTap -- s167 D10.1: the take's SECOND, INDEPENDENT consumer off the
// device callback. It is fed the exact same buffer the analysis pipeline
// is fed, at the same point in CombinedCallback (src/audio/CombinedCallback.h)
// -- never a second reader of the analysis ring buffer (G26: that one stays
// single-producer/single-consumer, untouched; R12's silent overflow there
// can never desynchronise a take).
//
// Threading contract (D10, section 4 of the spec):
//   prepare() / start() / stop()  -- message thread. Every allocation this
//                                     class ever makes happens here (the
//                                     pending-frames spill buffer and the
//                                     silence scratch buffer are sized once
//                                     in prepare(); start()/stop() do real,
//                                     blocking file I/O, which is fine off
//                                     the audio thread).
//   push()                        -- AUDIO THREAD. No allocation, no lock,
//                                     no blocking I/O, no logging. See the
//                                     builder report's function-by-function
//                                     breakdown of why each call inside it
//                                     is RT-safe.
//   popGap()                      -- message thread (the recorder drains
//                                     gap markers there); backed by a
//                                     juce::AbstractFifo so the audio-thread
//                                     producer side never blocks either.
//
// R12 is explicitly NOT this class's problem (D10.2 #4): a stall on the
// analysis ring buffer is invisible here by construction, because this tap
// never touches that buffer.
class AudioTap
{
public:
    AudioTap();
    ~AudioTap();

    AudioTap(const AudioTap&) = delete;
    AudioTap& operator=(const AudioTap&) = delete;

    // Message thread (CombinedCallback::audioDeviceAboutToStart). Sizes and
    // allocates every fixed-capacity buffer push() will ever touch, and
    // starts the background flush thread (idempotent -- safe to call again
    // on a device restart). `channels` is the tap's own best information
    // about how many channels push() will be given (CombinedCallback reads
    // it from the device; see that file for the mic/file-mode ASSUMPTION
    // this rests on). Calling this again with a DIFFERENT rate or channel
    // count while a take is running stops that take (D10.1's multi-segment
    // "audio-2.wav" bookkeeping is Take-level, step 3's job -- NOT built
    // here, see the builder report's DEVIATIONS); a block-size-only change
    // (the common device-restart case) leaves an active recording running
    // untouched. EVERY call (including a benign restart) resets the
    // gap-detection hostTimeNs baseline -- D10.1: "the counter continues",
    // a stop/restart is not itself a dropout, however much real wall-clock
    // time the pause actually cost.
    void prepare(double deviceRate, int channels, int maxBlockSize);

    // Message thread. Creates the WAV writer and arms the tap; the NEXT
    // push() call captures firstSample (D10.1's sample-exact start).
    // Returns false (nothing armed) if the writer could not be created, or
    // if R15's free-space floor is not met (see freeSpaceBytes/kMinFreeBytes
    // below) -- the caller surfaces the failure; this packet's chosen home
    // for R15's MECHANISM is here, in the tap, not in RecordPanel (out of
    // scope this step) -- see the builder report for why.
    bool start(const juce::File& wavFile);

    // AUDIO THREAD, RT-safe. `ch`/`chans` must be the SAME buffer and
    // channel count fed to the analysis callback this block (D10.1's "same
    // buffers ... at the same point"); `numSamples` is this block's sample
    // count; `deliveredBefore` is the CALLER's OWN delivered-sample counter
    // read BEFORE this block (CombinedCallback owns that counter, not this
    // class -- D1's threading contract). Returns the number of EXTRA
    // (gap-filled) frames written ahead of the block's real data, so the
    // caller advances ITS OWN counter by `numSamples + the return value`
    // (D10.1: "deliveredSamples_ advances by the same amount"). Returns 0,
    // touching nothing else, when the tap is neither armed nor running --
    // CombinedCallback calls this unconditionally every block, the same
    // way it always feeds the analysis callback.
    uint32_t push(const float* const* ch, int chans, int numSamples,
                  uint64_t deliveredBefore,
                  const juce::AudioIODeviceCallbackContext& context);

    // Message thread. Synchronously drains any still-pending (retried)
    // frames into the real writer, then destroys it (ThreadedWriter's
    // destructor blocks until its own FIFO is flushed to disk -- fine here,
    // off the audio thread) and disarms.
    void stop();

    uint64_t firstSample() const noexcept { return firstSample_.load(std::memory_order_relaxed); }
    uint64_t framesWritten() const noexcept { return framesWritten_.load(std::memory_order_relaxed); }
    uint64_t droppedFrames() const noexcept { return droppedFrames_.load(std::memory_order_relaxed); }
    bool isRunning() const noexcept { return running_.load(std::memory_order_relaxed); }

    // R14: latches to false the first time `context.hostTimeNs` is null
    // while running, and never returns to true for this take -- once a gap
    // could have gone undetected, the take must say so for its whole
    // duration (audio.gapDetection:false), not flicker back to claiming
    // reliability. True until the first such observation (optimistic
    // default matches D10.1's "ASSUMED non-null on CoreAudio").
    bool gapDetectionSupported() const noexcept { return gapDetectionSupported_.load(std::memory_order_relaxed); }

    // D10.1's FIFO-overrun path: set once a gap/stall genuinely exceeded
    // this tap's retry capacity and silence had to stand in permanently
    // (never for the short, retried stalls T1 proves are lossless).
    std::optional<uint64_t> unreliableFrom() const;

    // Drained by the recorder on the message thread (D10's small lock-free
    // queue for gap markers). Returns false when empty.
    bool popGap(std::pair<uint64_t, uint32_t>& out);

    // R15 mechanism (packet's choice -- the UI refusal/warning call itself
    // is RecordPanel's job, step 4, out of scope here). Bytes free on the
    // volume containing `folder`, or -1 if it could not be determined.
    static int64_t freeSpaceBytes(const juce::File& folder);
    static constexpr int64_t kMinFreeBytes = 2LL * 1024 * 1024 * 1024;   // R15: 2 GB

#if defined(AUDIODNA_AUDIOTAP_TEST_HOOKS)
    // TEST ONLY -- compiled in only when the test target defines
    // AUDIODNA_AUDIOTAP_TEST_HOOKS (never the AudioDNA app; see
    // tests/CMakeLists.txt). Forces the next `n` disk-write attempts to
    // report failure without touching the real writer at all, so T1 can
    // prove the retry/spill path deterministically -- JUCE's
    // AudioFormatWriter::ThreadedWriter::write() is a concrete, non-virtual
    // method and cannot be mocked, and driving a REAL disk stall of a known
    // duration from a unit test would be a real-time race. See the builder
    // report's "HOW T1 SIMULATES ... THE STALL" section.
    void debugForceNextWritesToFail(int n) { forcedFailuresRemaining_ = n; }
#endif

private:
    void writeFrames(const float* const* ch, int chans, int numSamples);
    void writeSilenceFrames(uint32_t numFrames);
    void writeBlockRetrying(const float* const* data, int numSamples);
    void flushPendingNonBlocking();
    void spillIntoPending(const float* const* data, int numSamples);
    bool tryRealWrite(const float* const* data, int numSamples);
    void pushGapMarker(uint64_t sample, uint32_t n);
    void stopInternal();   // shared by stop() and prepare()'s rate/channel-change path

    juce::TimeSliceThread flushThread_{ "AudioTapWriter" };
    juce::WavAudioFormat wavFormat_;
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter_;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter_{ nullptr };

    double rate_ = 0.0;
    int channels_ = 0;
    int maxBlock_ = 0;

    std::atomic<bool> armed_{ false };
    std::atomic<bool> running_{ false };
    std::atomic<uint64_t> firstSample_{ 0 };
    std::atomic<uint64_t> framesWritten_{ 0 };
    std::atomic<uint64_t> droppedFrames_{ 0 };
    std::atomic<bool> gapDetectionSupported_{ true };
    std::atomic<bool> hasUnreliableFrom_{ false };
    std::atomic<uint64_t> unreliableFromSample_{ 0 };

    // Audio-thread-only (single producer -- CombinedCallback calls push()
    // from one thread only, so these need no atomics of their own).
    bool haveLastHostTime_ = false;
    uint64_t lastHostTimeNs_ = 0;

    // Fixed-capacity spill buffer for "must not lose frames" on a disk
    // stall shorter than this capacity (D10.3 step 4) -- allocated once in
    // prepare(), never resized on the audio thread. Interleaved
    // [frame][channel] would need per-channel pointers rebuilt every push;
    // stored per-channel contiguous instead so pendingChannelPtrs_ stays
    // valid without rebuilding.
    std::vector<std::vector<float>> pendingStorage_;   // one vector per channel
    std::vector<float*> pendingChannelPtrs_;
    int pendingCapacityFrames_ = 0;
    int pendingFrames_ = 0;   // frames currently held, oldest-first

    // All-zero scratch buffer for gap-fill (never written to after
    // prepare() -- always zero, so no need to re-zero per gap).
    std::vector<std::vector<float>> silenceStorage_;
    std::vector<const float*> silenceChannelPtrs_;

    // Pre-sized (prepare()) scratch for the defensive chans < channels_
    // pad-with-silence path in writeFrames() -- never expected to fire in
    // practice (ASSUMPTION: CombinedCallback keeps chans == channels_), but
    // must not allocate on the audio thread if it ever does.
    std::vector<const float*> scratchChannelPtrs_;

#if defined(AUDIODNA_AUDIOTAP_TEST_HOOKS)
    int forcedFailuresRemaining_ = 0;
#endif

    juce::AbstractFifo gapFifo_{ 64 };
    std::vector<std::pair<uint64_t, uint32_t>> gapStorage_{ 64 };
};
