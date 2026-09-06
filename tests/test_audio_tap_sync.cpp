// test_audio_tap_sync -- s168 step 2: T1, D10.3's "headless alignment
// proof" (s167-performance-log-and-routines.md section 2 D10.3, section 5
// Build order row 2). Drives CombinedCallback (src/audio/CombinedCallback.h)
// directly with a FakeAudioIODevice -- no real audio device, no device
// thread; everything here runs synchronously on the test's own thread, so
// there is no real concurrency between the "audio thread" work (push()) and
// the "message thread" work (start()/stop()/prepare()) in this file. That
// is deliberate: T1 proves the SYNC CONTRACT (D10.2), not thread-safety
// under real concurrency (see the builder report's RISKS for the one
// documented gap that follows from this).
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "audio/CombinedCallback.h"
#include "audio/AudioCallback.h"
#include "audio/RingBuffer.h"
#include "recording/AudioTap.h"
#include "recording/Take.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

using Catch::Approx;

namespace
{
    // Minimal AudioIODevice fake. CombinedCallback::audioDeviceAboutToStart
    // only ever reads getCurrentSampleRate/getCurrentBufferSizeSamples/
    // getActiveOutputChannels from it (see that file) -- the rest of
    // juce::AudioIODevice's pure virtuals are trivial stand-ins purely so
    // this class isn't abstract; none of them are ever called by anything
    // this test exercises (the per-block callback itself takes no device
    // pointer at all).
    class FakeAudioIODevice : public juce::AudioIODevice
    {
    public:
        FakeAudioIODevice(double rate, int bufferSize, int channels)
            : juce::AudioIODevice("FakeDevice", "Fake"),
              rate_(rate), bufferSize_(bufferSize), channels_(channels)
        {}

        juce::StringArray getOutputChannelNames() override { return {}; }
        juce::StringArray getInputChannelNames() override { return {}; }
        juce::Array<double> getAvailableSampleRates() override { return { rate_ }; }
        juce::Array<int> getAvailableBufferSizes() override { return { bufferSize_ }; }
        int getDefaultBufferSize() override { return bufferSize_; }
        juce::String open(const juce::BigInteger&, const juce::BigInteger&, double, int) override { return {}; }
        void close() override {}
        bool isOpen() override { return true; }
        void start(juce::AudioIODeviceCallback*) override {}
        void stop() override {}
        bool isPlaying() override { return false; }
        juce::String getLastError() override { return {}; }
        int getCurrentBufferSizeSamples() override { return bufferSize_; }
        double getCurrentSampleRate() override { return rate_; }
        int getCurrentBitDepth() override { return 16; }
        juce::BigInteger getActiveOutputChannels() const override
        {
            juce::BigInteger b; b.setRange(0, channels_, true); return b;
        }
        juce::BigInteger getActiveInputChannels() const override
        {
            juce::BigInteger b; b.setRange(0, channels_, true); return b;
        }
        int getOutputLatencyInSamples() override { return 0; }
        int getInputLatencyInSamples() override { return 0; }

    private:
        double rate_;
        int bufferSize_;
        int channels_;
    };

    // RAII temp file in the system temp dir -- deleted whether the test
    // passes or fails.
    struct TempWavFile
    {
        juce::File file;
        explicit TempWavFile(const juce::String& tag)
            : file(juce::File::getSpecialLocation(juce::File::tempDirectory)
                       .getChildFile("audiodna_test_" + tag + "_"
                                     + juce::String(juce::Random::getSystemRandom().nextInt64()) + ".wav"))
        {}
        ~TempWavFile() { file.deleteFile(); }
    };

    struct TempFolder
    {
        juce::File dir;
        explicit TempFolder(const juce::String& tag)
            : dir(juce::File::getSpecialLocation(juce::File::tempDirectory)
                      .getChildFile("audiodna_test_" + tag + "_"
                                    + juce::String(juce::Random::getSystemRandom().nextInt64()) + ".adna-take"))
        {
            dir.createDirectory();
        }
        ~TempFolder() { dir.deleteRecursively(); }
    };

    // D10.3 step 3: the recorder's two simulated stamp reads per click --
    // one exact (as if read at the instant), one late (as if the
    // message-thread stamp landed one block after the block containing the
    // click). `blockSize` is the device block size in effect when this
    // click was captured (needed post-restart, when it differs from the
    // suite's nominal block size).
    struct ClickPoint
    {
        uint64_t exactSample;
        uint64_t lateSample;
        int blockSize;
    };

    // Reads back a WAV file and returns the frame indices where |x| > 0.5
    // on channel 0 (D10.3 step 5's impulse locator).
    std::vector<uint64_t> findImpulses(const juce::File& wavFile)
    {
        juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatReader> reader(
            format.createReaderFor(new juce::FileInputStream(wavFile), true));
        REQUIRE(reader != nullptr);

        const int numFrames = static_cast<int>(reader->lengthInSamples);
        juce::AudioBuffer<float> buffer(static_cast<int>(reader->numChannels), numFrames);
        reader->read(&buffer, 0, numFrames, 0, true, true);

        std::vector<uint64_t> impulses;
        const float* ch0 = buffer.getReadPointer(0);
        for (int i = 0; i < numFrames; ++i)
            if (std::fabs(ch0[i]) > 0.5f)
                impulses.push_back(static_cast<uint64_t>(i));
        return impulses;
    }

    // Runs D10.3's full T1 script at a given rate/block size: arm at block
    // 7; a 3-blocks-elapsed/1-delivered driver drop at block 40->41 (a
    // 2*blockSize-sample gap); a block-size-only device restart at block 80
    // (D10.1: not a rate/channel change, so no new segment); a 3-block
    // forced disk stall at block 120 that must not lose frames. Then proves
    // the sync contract (D10.2) against the resulting audio.wav.
    void runSyncSuite(double rate, int blockSize, int restartBlockSize, const juce::String& label)
    {
        INFO(label.toStdString());

        RingBuffer<float> ring(8192);
        AudioCallback analysis(ring);
        juce::AudioSourcePlayer player;
        CombinedCallback combined(player, analysis);
        combined.useInputForAnalysis.store(true);   // mic-mode path -- click train fed via inputChannelData

        FakeAudioIODevice device(rate, blockSize, 2);
        combined.audioDeviceAboutToStart(&device);

        TempWavFile wav(label);

        constexpr uint64_t kClickPeriod = 24000;   // one impulse every 0.5 s @ 48 kHz (D10.3 step 1)
        constexpr int kArmBlock = 7;
        constexpr int kDropBlock = 41;    // the block immediately after the 3-elapsed/1-delivered drop
        constexpr int kRestartBlock = 80;
        constexpr int kStallBlock = 120;
        constexpr int kTotalBlocks = 150;

        uint64_t mirrorDelivered = 0;     // this test's own prediction of CombinedCallback::getDeliveredSamples()
        uint64_t hostTimeNs = 1'000'000'000ULL;
        int currentBlockSize = blockSize;
        uint64_t firstSampleExpected = 0;
        bool armed = false;

        std::vector<ClickPoint> points;
        uint64_t expectedGapSample = 0;
        uint32_t expectedGapSize = 0;

        std::unique_ptr<FakeAudioIODevice> restartedDevice;   // must outlive the restart (D10.1's stop/restart)

        for (int b = 0; b < kTotalBlocks; ++b)
        {
            if (b == kRestartBlock)
            {
                // D10.1: device stop/restart, block size only -- no rate or
                // channel change, so the tap keeps running (no new segment).
                combined.audioDeviceStopped();
                currentBlockSize = restartBlockSize;
                restartedDevice = std::make_unique<FakeAudioIODevice>(rate, currentBlockSize, 2);
                combined.audioDeviceAboutToStart(restartedDevice.get());
            }

            const int numSamples = currentBlockSize;

            if (b == kArmBlock)
            {
                REQUIRE(combined.tap().start(wav.file));
                armed = true;
                firstSampleExpected = mirrorDelivered;   // D10.3 step 2: firstSample == 7*blockSize
            }

            uint32_t gapFramesThisBlock = 0;
            uint64_t deltaNs;
            if (b == kDropBlock)
            {
                // hostTimeNs advances as if 3 blocks elapsed; only 1 block's
                // worth of samples is actually delivered this callback.
                deltaNs = static_cast<uint64_t>((3.0 * static_cast<double>(numSamples) / rate) * 1.0e9);
                gapFramesThisBlock = static_cast<uint32_t>(2 * numSamples);   // expected(3n) - delivered(n) = 2n
                expectedGapSample = mirrorDelivered;
                expectedGapSize = gapFramesThisBlock;
            }
            else
            {
                deltaNs = static_cast<uint64_t>((static_cast<double>(numSamples) / rate) * 1.0e9);
            }
            hostTimeNs += deltaNs;

            // The real signal this block represents starts AFTER any
            // gap-filled silence (D10.1: the tap inserts the missing frames
            // ahead of the block's real data).
            const uint64_t contentStart = mirrorDelivered + gapFramesThisBlock;

            std::vector<float> inL(static_cast<size_t>(numSamples)), inR(static_cast<size_t>(numSamples));
            for (int i = 0; i < numSamples; ++i)
            {
                const uint64_t idx = contentStart + static_cast<uint64_t>(i);
                const float v = (idx % kClickPeriod == 0) ? 1.0f : 0.0f;
                inL[static_cast<size_t>(i)] = v;
                inR[static_cast<size_t>(i)] = v;

                if (armed && idx % kClickPeriod == 0 && idx >= firstSampleExpected)
                {
                    const uint64_t deliveredAfterBlock = mirrorDelivered + gapFramesThisBlock + static_cast<uint64_t>(numSamples);
                    points.push_back({ idx, deliveredAfterBlock, numSamples });
                }
            }

            std::vector<float> outL(static_cast<size_t>(numSamples), 0.0f), outR(static_cast<size_t>(numSamples), 0.0f);
            const float* inPtrs[2] = { inL.data(), inR.data() };
            float* outPtrs[2] = { outL.data(), outR.data() };

            juce::AudioIODeviceCallbackContext ctx;
            ctx.hostTimeNs = &hostTimeNs;

            if (b == kStallBlock)
                combined.tap().debugForceNextWritesToFail(3);   // D10.3 step 4: a disk stall shorter than the FIFO -- must not lose frames

            combined.audioDeviceIOCallbackWithContext(inPtrs, 2, outPtrs, 2, numSamples, ctx);

            mirrorDelivered += gapFramesThisBlock + static_cast<uint64_t>(numSamples);
            REQUIRE(combined.getDeliveredSamples() == mirrorDelivered);
        }

        combined.tap().stop();

        REQUIRE(combined.tap().firstSample() == firstSampleExpected);
        REQUIRE(combined.tap().droppedFrames() == 0);              // the stall was shorter than the retry buffer -- nothing lost
        REQUIRE(combined.tap().gapDetectionSupported());           // hostTimeNs was always non-null in this run (R14's other branch)
        REQUIRE_FALSE(combined.tap().unreliableFrom().has_value());

        std::pair<uint64_t, uint32_t> gap;
        REQUIRE(combined.tap().popGap(gap));                        // exactly one marker
        REQUIRE(gap.first == expectedGapSample);
        REQUIRE(gap.second == expectedGapSize);
        REQUIRE_FALSE(combined.tap().popGap(gap));

        const auto impulses = findImpulses(wav.file);
        REQUIRE(impulses.size() == points.size());

        const uint64_t firstSample = combined.tap().firstSample();
        for (size_t i = 0; i < points.size(); ++i)
        {
            const uint64_t w = impulses[i];
            REQUIRE(w + firstSample == points[i].exactSample);      // 0 samples tolerance (exact)

            const uint64_t late = points[i].lateSample;
            REQUIRE(late >= w + firstSample);
            REQUIRE(late - (w + firstSample) <= static_cast<uint64_t>(points[i].blockSize));   // <= one device block (late)
        }

        juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatReader> reader(
            format.createReaderFor(new juce::FileInputStream(wav.file), true));
        REQUIRE(reader != nullptr);
        REQUIRE(static_cast<uint64_t>(reader->lengthInSamples) == combined.getDeliveredSamples() - firstSample);
    }
}

TEST_CASE("AudioTap sync -- D10.3 T1 at 48 kHz / 512-sample blocks", "[audiotap][sync]")
{
    runSyncSuite(48000.0, 512, 256, "48k");
}

TEST_CASE("AudioTap sync -- D10.3 T1 at 44.1 kHz / 128-sample blocks (tolerance scales)", "[audiotap][sync]")
{
    runSyncSuite(44100.0, 128, 64, "44k1");
}

TEST_CASE("AudioTap R14 -- hostTimeNs null disables gap detection, and the take says so", "[audiotap][r14]")
{
    RingBuffer<float> ring(8192);
    AudioCallback analysis(ring);
    juce::AudioSourcePlayer player;
    CombinedCallback combined(player, analysis);
    combined.useInputForAnalysis.store(true);

    constexpr double rate = 48000.0;
    constexpr int blockSize = 512;
    FakeAudioIODevice device(rate, blockSize, 2);
    combined.audioDeviceAboutToStart(&device);

    TempWavFile wav("r14null");
    REQUIRE(combined.tap().start(wav.file));
    REQUIRE(combined.tap().gapDetectionSupported());   // optimistic default (R14: ASSUMED non-null on CoreAudio) until proven otherwise

    for (int b = 0; b < 20; ++b)
    {
        std::vector<float> inL(blockSize, 0.1f), inR(blockSize, 0.1f);
        std::vector<float> outL(static_cast<size_t>(blockSize), 0.0f), outR(static_cast<size_t>(blockSize), 0.0f);
        const float* inPtrs[2] = { inL.data(), inR.data() };
        float* outPtrs[2] = { outL.data(), outR.data() };

        juce::AudioIODeviceCallbackContext ctx;   // hostTimeNs defaults to nullptr -- R14's "if the host doesn't provide this"
        combined.audioDeviceIOCallbackWithContext(inPtrs, 2, outPtrs, 2, blockSize, ctx);
    }

    combined.tap().stop();

    REQUIRE_FALSE(combined.tap().gapDetectionSupported());   // R14: latched off for the whole take -- it must say so
    REQUIRE(combined.tap().droppedFrames() == 0);

    std::pair<uint64_t, uint32_t> gap;
    REQUIRE_FALSE(combined.tap().popGap(gap));   // never attempted without hostTimeNs -- no markers, ever

    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(
        format.createReaderFor(new juce::FileInputStream(wav.file), true));
    REQUIRE(reader != nullptr);
    REQUIRE(static_cast<uint64_t>(reader->lengthInSamples)
            == combined.getDeliveredSamples() - combined.tap().firstSample());
}

TEST_CASE("AudioTap + Take -- .adna-take folder save/load completion", "[audiotap][take]")
{
    RingBuffer<float> ring(8192);
    AudioCallback analysis(ring);
    juce::AudioSourcePlayer player;
    CombinedCallback combined(player, analysis);
    combined.useInputForAnalysis.store(true);

    constexpr double rate = 48000.0;
    constexpr int blockSize = 512;
    FakeAudioIODevice device(rate, blockSize, 2);
    combined.audioDeviceAboutToStart(&device);

    TempFolder folder("takefolder");
    REQUIRE(combined.tap().start(folder.dir.getChildFile("audio.wav")));

    for (int b = 0; b < 10; ++b)
    {
        std::vector<float> inL(blockSize, 0.2f), inR(blockSize, 0.2f);
        std::vector<float> outL(static_cast<size_t>(blockSize), 0.0f), outR(static_cast<size_t>(blockSize), 0.0f);
        const float* inPtrs[2] = { inL.data(), inR.data() };
        float* outPtrs[2] = { outL.data(), outR.data() };

        uint64_t hostTimeNs = static_cast<uint64_t>(b) * static_cast<uint64_t>((static_cast<double>(blockSize) / rate) * 1.0e9);
        juce::AudioIODeviceCallbackContext ctx;
        ctx.hostTimeNs = &hostTimeNs;
        combined.audioDeviceIOCallbackWithContext(inPtrs, 2, outPtrs, 2, blockSize, ctx);
    }

    const uint64_t firstSample = combined.tap().firstSample();
    const uint64_t frames = combined.tap().framesWritten();
    const bool gapDetection = combined.tap().gapDetectionSupported();
    combined.tap().stop();

    // Take::save/load already take a directory as the real contract (Lane
    // A, s168 step 1) -- this proves it end to end against a REAL AudioTap
    // output, not synthetic data: audio.wav sits beside take.json, and the
    // AudioRef round-trips through both.
    Take take;
    take.audio.mode = "input";
    take.audio.gapDetection = gapDetection;
    AudioRef::Segment seg;
    seg.file = "audio.wav";
    seg.firstSample = firstSample;
    seg.frames = frames;
    seg.rate = rate;
    seg.channels = 2;
    take.audio.segments.push_back(seg);

    REQUIRE(take.save(folder.dir));
    REQUIRE(folder.dir.getChildFile("audio.wav").existsAsFile());
    REQUIRE(folder.dir.getChildFile("take.json").existsAsFile());

    LoadStats stats;
    auto loaded = Take::load(folder.dir, stats);
    REQUIRE(loaded.has_value());
    REQUIRE_FALSE(stats.refused);
    REQUIRE(loaded->audio.mode == "input");
    REQUIRE(loaded->audio.gapDetection == gapDetection);
    REQUIRE(loaded->audio.segments.size() == 1);
    REQUIRE(loaded->audio.segments[0].file == "audio.wav");
    REQUIRE(loaded->audio.segments[0].firstSample == firstSample);
    REQUIRE(loaded->audio.segments[0].frames == frames);
    REQUIRE(loaded->audio.segments[0].rate == Approx(rate));
    REQUIRE(loaded->audio.segments[0].channels == 2);

    // The arithmetic identity (success criteria), proven against the real
    // file this round-trip now points at.
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(
        format.createReaderFor(new juce::FileInputStream(folder.dir.getChildFile(loaded->audio.segments[0].file)), true));
    REQUIRE(reader != nullptr);
    REQUIRE(static_cast<uint64_t>(reader->lengthInSamples) == frames);
}

// ============================================================================
// s168 review (review-lane-s2), blocking issue 2: spillIntoPending's
// genuine-overrun branch (accepted < numSamples) must insert SILENCE for
// the lost span, not just count it -- D10.1's FIFO-overrun bullet and the
// function's own header comment both say so; pre-fix, neither was true
// (the lost content was never replaced with anything, so the eventual WAV
// fell short of deliveredSamples_ - firstSample_). T1's own stall (3
// blocks) is deliberately far under pendingCapacityFrames_ (~2s of audio,
// AudioTap::prepare()) and never exercises this branch at all.
// ============================================================================

TEST_CASE("AudioTap genuine FIFO overrun -- a stall longer than the retry buffer inserts silence, keeps the frame count honest, and is surfaced", "[audiotap][overrun]")
{
    RingBuffer<float> ring(8192);
    AudioCallback analysis(ring);
    juce::AudioSourcePlayer player;
    CombinedCallback combined(player, analysis);
    combined.useInputForAnalysis.store(true);

    constexpr double rate = 48000.0;
    constexpr int blockSize = 512;
    FakeAudioIODevice device(rate, blockSize, 2);
    combined.audioDeviceAboutToStart(&device);

    TempWavFile wav("overrun");
    REQUIRE(combined.tap().start(wav.file));

    // pendingCapacityFrames_ is sized to ~2s of audio in AudioTap::prepare()
    // (rate * 2.0 when rate > 0) -- 96000 frames at 48 kHz. 300 consecutive
    // forced write failures (300 * 512 = 153600 samples) guarantees the
    // buffer fills completely partway through and stays full for the rest
    // of the stall, so the genuine-overrun branch fires repeatedly, unlike
    // T1's own 3-block stall. kStallBlocks == kForcedFailures exactly: this
    // test's own trace (see the report) shows every one of the 300 blocks
    // consumes exactly one forced-write attempt (pendingFrames_ never
    // drains to 0 mid-stall, so writeBlockRetrying never gets a second
    // tryRealWrite attempt in the same block).
    constexpr int kForcedFailures = 300;
    constexpr int kStallBlocks = 300;
    constexpr uint64_t kApproxCapacityFrames = static_cast<uint64_t>(rate * 2.0);   // AudioTap::prepare()'s own formula

    uint64_t hostTimeNs = 1'000'000'000ULL;
    uint64_t mirrorDelivered = 0;
    constexpr float kSignalValue = 0.3f;   // plain, non-zero, non-silent -- so the dropped span reads back unambiguously as zero, not leftover content

    combined.tap().debugForceNextWritesToFail(kForcedFailures);

    for (int b = 0; b < kStallBlocks; ++b)
    {
        std::vector<float> inL(blockSize, kSignalValue), inR(blockSize, kSignalValue);
        std::vector<float> outL(static_cast<size_t>(blockSize), 0.0f), outR(static_cast<size_t>(blockSize), 0.0f);
        const float* inPtrs[2] = { inL.data(), inR.data() };
        float* outPtrs[2] = { outL.data(), outR.data() };

        hostTimeNs += static_cast<uint64_t>((static_cast<double>(blockSize) / rate) * 1.0e9);
        juce::AudioIODeviceCallbackContext ctx;
        ctx.hostTimeNs = &hostTimeNs;

        combined.audioDeviceIOCallbackWithContext(inPtrs, 2, outPtrs, 2, blockSize, ctx);
        mirrorDelivered += static_cast<uint64_t>(blockSize);
        REQUIRE(combined.getDeliveredSamples() == mirrorDelivered);
    }

    // Stop IMMEDIATELY after the stall, with no recovery blocks -- forces
    // stopInternal()'s own silence-debt drain (the s168 fix's other new
    // code path) to be what pays down the debt, not ordinary push()-driven
    // recovery.
    combined.tap().stop();

    // Surfaced: genuinely lost, not silently dropped.
    REQUIRE(combined.tap().droppedFrames() > 0);
    REQUIRE(combined.tap().unreliableFrom().has_value());
    // Exact, not approximate: the first overrun always lands the instant
    // pendingStorage_ fills to capacity (spillIntoPending's own accounting
    // -- framesBeforeThisBlock + accepted == pendingCapacityFrames_ at that
    // instant), and firstSample() == 0 here (armed on the very first push).
    REQUIRE(*combined.tap().unreliableFrom() == kApproxCapacityFrames);

    // The frame-count invariant (D10.1/D10.3) must still hold even though
    // content was lost -- this is the assertion that fails against the
    // pre-fix code, which left a genuine hole (a file short of
    // deliveredSamples_ - firstSample_) instead of silence.
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(
        format.createReaderFor(new juce::FileInputStream(wav.file), true));
    REQUIRE(reader != nullptr);
    const uint64_t firstSample = combined.tap().firstSample();
    REQUIRE(static_cast<uint64_t>(reader->lengthInSamples) == combined.getDeliveredSamples() - firstSample);

    // The dropped span itself must actually BE silence (zeros), not
    // leftover/garbage content: probe the frames starting exactly at
    // unreliableFrom() and confirm they read back near-zero, distinct from
    // the constant 0.3f signal used everywhere else in this run.
    const uint64_t unreliableFrame = *combined.tap().unreliableFrom() - firstSample;
    const int probeFrames = std::min(static_cast<int>(reader->lengthInSamples - static_cast<int64_t>(unreliableFrame)), 256);
    REQUIRE(probeFrames > 0);
    juce::AudioBuffer<float> probe(2, probeFrames);
    reader->read(&probe, 0, probeFrames, static_cast<int64_t>(unreliableFrame), true, true);
    const float* p0 = probe.getReadPointer(0);
    float maxAbs = 0.0f;
    for (int i = 0; i < probeFrames; ++i)
        maxAbs = std::max(maxAbs, std::fabs(p0[i]));
    REQUIRE(maxAbs < 0.001f);
}

// ============================================================================
// s168 review (review-lane-s2), narrower gap: `listened == nullptr` (mic
// mode with no input channels open this callback) used to skip the whole
// analysis+tap block, including audioTap_.push() -- deliveredSamples_ still
// advanced by numSamples (D10.1: unconditional, every callback), but
// AudioTap::framesWritten() never did for that span. Fix: CombinedCallback
// now calls push() unconditionally; AudioTap's existing chans < channels_
// pad-with-silence path (writeFrames) turns chans == 0 into a full block of
// silence.
// ============================================================================

TEST_CASE("CombinedCallback: a block with no analyzable buffer (listened == nullptr) still advances AudioTap in lockstep with deliveredSamples_", "[audiotap][combinedcallback]")
{
    RingBuffer<float> ring(8192);
    AudioCallback analysis(ring);
    juce::AudioSourcePlayer player;
    CombinedCallback combined(player, analysis);
    combined.useInputForAnalysis.store(true);   // mic mode -- listened comes from inputChannelData

    constexpr double rate = 48000.0;
    constexpr int blockSize = 512;
    FakeAudioIODevice device(rate, blockSize, 2);
    combined.audioDeviceAboutToStart(&device);

    TempWavFile wav("nullbuf");
    REQUIRE(combined.tap().start(wav.file));

    uint64_t hostTimeNs = 1'000'000'000ULL;
    uint64_t mirrorDelivered = 0;

    auto pushBlock = [&](bool withInput)
    {
        std::vector<float> outL(static_cast<size_t>(blockSize), 0.0f), outR(static_cast<size_t>(blockSize), 0.0f);
        float* outPtrs[2] = { outL.data(), outR.data() };

        hostTimeNs += static_cast<uint64_t>((static_cast<double>(blockSize) / rate) * 1.0e9);
        juce::AudioIODeviceCallbackContext ctx;
        ctx.hostTimeNs = &hostTimeNs;

        if (withInput)
        {
            std::vector<float> inL(blockSize, 0.4f), inR(blockSize, 0.4f);
            const float* inPtrs[2] = { inL.data(), inR.data() };
            combined.audioDeviceIOCallbackWithContext(inPtrs, 2, outPtrs, 2, blockSize, ctx);
        }
        else
        {
            // The exact scenario the s168 review flagged: mic mode with no
            // input channels open this callback -- CombinedCallback's own
            // "if (numInputChannels > 0 && inputChannelData != nullptr)"
            // guard leaves listened == nullptr / listenedChans == 0.
            combined.audioDeviceIOCallbackWithContext(nullptr, 0, outPtrs, 2, blockSize, ctx);
        }
        mirrorDelivered += static_cast<uint64_t>(blockSize);
    };

    for (int b = 0; b < 5; ++b) pushBlock(true);
    for (int b = 0; b < 3; ++b) pushBlock(false);   // the no-buffer span
    for (int b = 0; b < 5; ++b) pushBlock(true);

    combined.tap().stop();

    // The defining assertion: without the fix, deliveredSamples_ would
    // outrun AudioTap::framesWritten() by exactly the 3 no-buffer blocks
    // (1536 samples) -- pre-fix, push() was never called for them at all,
    // so framesWritten_ never advanced for that span.
    REQUIRE(combined.getDeliveredSamples() == mirrorDelivered);
    const uint64_t firstSample = combined.tap().firstSample();
    REQUIRE(combined.tap().framesWritten() == mirrorDelivered - firstSample);

    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(
        format.createReaderFor(new juce::FileInputStream(wav.file), true));
    REQUIRE(reader != nullptr);
    REQUIRE(static_cast<uint64_t>(reader->lengthInSamples) == combined.getDeliveredSamples() - firstSample);

    // The no-buffer span itself must be silence in the file (not garbage,
    // not a hole): frames [5*blockSize, 8*blockSize) relative to
    // firstSample (0 here -- armed on the very first, withInput==true push).
    REQUIRE(firstSample == 0);
    const int64_t noBufferStart = 5 * blockSize;
    juce::AudioBuffer<float> probe(2, 3 * blockSize);
    reader->read(&probe, 0, 3 * blockSize, noBufferStart, true, true);
    const float* p0 = probe.getReadPointer(0);
    float maxAbs = 0.0f;
    for (int i = 0; i < 3 * blockSize; ++i)
        maxAbs = std::max(maxAbs, std::fabs(p0[i]));
    REQUIRE(maxAbs < 0.0001f);
}

// ============================================================================
// s168 review (review-lane-s2), the priority blocking issue: a UAF race
// between stopInternal() (message thread) and an in-flight push() (audio
// thread) -- stopInternal() could destroy the ThreadedWriter while a
// push() call already past its activeWriter_ load was still inside
// w->write(). Deterministic reproduction via the test-only pause hooks
// (AudioTap.h's debugArmPushBlockForTest/debugReleaseBlockedPush/
// debugWaitUntilPushBlocked) rather than real thread timing (which cannot
// be made to race reliably): a background thread's push() call is held
// "in flight" (busy == true, paused mid-call, past the point where a real
// push() would have already loaded activeWriter_) while the main thread
// calls stop() concurrently.
// ============================================================================

TEST_CASE("AudioTap::stop() waits out an in-flight push() before destroying the writer (s168 review: UAF race)", "[audiotap][concurrency]")
{
    RingBuffer<float> ring(8192);
    AudioCallback analysis(ring);
    juce::AudioSourcePlayer player;
    CombinedCallback combined(player, analysis);
    combined.useInputForAnalysis.store(true);

    constexpr double rate = 48000.0;
    constexpr int blockSize = 512;
    FakeAudioIODevice device(rate, blockSize, 2);
    combined.audioDeviceAboutToStart(&device);

    TempWavFile wav("uafrace");
    REQUIRE(combined.tap().start(wav.file));

    // One real push() first, off the test hook, so the tap is armed/running
    // and has a real writer for stop() to (eventually) tear down.
    std::vector<float> inL(blockSize, 0.2f), inR(blockSize, 0.2f);
    std::vector<float> outL(static_cast<size_t>(blockSize), 0.0f), outR(static_cast<size_t>(blockSize), 0.0f);
    const float* inPtrs[2] = { inL.data(), inR.data() };
    float* outPtrs[2] = { outL.data(), outR.data() };
    uint64_t hostTimeNs = 1'000'000'000ULL;
    juce::AudioIODeviceCallbackContext ctx0;
    ctx0.hostTimeNs = &hostTimeNs;
    combined.audioDeviceIOCallbackWithContext(inPtrs, 2, outPtrs, 2, blockSize, ctx0);

    combined.tap().debugArmPushBlockForTest();

    std::atomic<bool> pushReturned{ false };
    std::thread pushThread([&]()
    {
        hostTimeNs += static_cast<uint64_t>((static_cast<double>(blockSize) / rate) * 1.0e9);
        juce::AudioIODeviceCallbackContext ctx;
        ctx.hostTimeNs = &hostTimeNs;
        combined.audioDeviceIOCallbackWithContext(inPtrs, 2, outPtrs, 2, blockSize, ctx);
        pushReturned.store(true, std::memory_order_release);
    });

    REQUIRE(combined.tap().debugWaitUntilPushBlocked());   // push() is now paused, busy == true

    std::atomic<bool> stopReturned{ false };
    std::thread stopThread([&]()
    {
        combined.tap().stop();
        stopReturned.store(true, std::memory_order_release);
    });

    // stop() must NOT return while push() is still held -- give it a
    // generous window and assert it has genuinely not finished. THIS is
    // the assertion that fails against the pre-fix code: without the
    // busy-wait, stop() would race ahead and return almost immediately,
    // regardless of the paused push() call.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    REQUIRE_FALSE(stopReturned.load(std::memory_order_acquire));
    REQUIRE_FALSE(pushReturned.load(std::memory_order_acquire));

    combined.tap().debugReleaseBlockedPush();
    pushThread.join();

    stopThread.join();
    REQUIRE(stopReturned.load(std::memory_order_acquire));
}
