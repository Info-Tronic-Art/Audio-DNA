// R13 lane A -- T1: AnalysisResampler (device-rate -> internal 48 kHz).
// Headless: no real audio device. T1.6 exercises AudioCallback's rate-cell
// wiring via a FakeAudioIODevice copied from test_audio_tap_sync.cpp:38-77
// (CombinedCallback::audioDeviceAboutToStart's needs are the same minimal
// subset — getCurrentSampleRate/getCurrentBufferSizeSamples/getActiveOutputChannels).
#include <catch2/catch_test_macros.hpp>
#include "analysis/AnalysisResampler.h"
#include "analysis/FFTProcessor.h"
#include "audio/AudioCallback.h"
#include "audio/RingBuffer.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace
{
    constexpr double kPi = 3.14159265358979323846;

    // Minimal AudioIODevice fake -- same minimal subset AudioCallback's
    // audioDeviceAboutToStart reads (getCurrentSampleRate/
    // getCurrentBufferSizeSamples). Copied from test_audio_tap_sync.cpp.
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

    // Deterministic LCG for a repeatable test tone/burst source -- avoids
    // relying on any global RNG state.
    float lcgNoise(uint32_t& state)
    {
        state = state * 1664525u + 1013904223u;
        return (static_cast<float>(state) / 4294967295.0f) * 2.0f - 1.0f;
    }

    std::vector<float> generateSineAtRate(float freqHz, float amplitude, size_t numSamples, double rate)
    {
        std::vector<float> out(numSamples);
        for (size_t i = 0; i < numSamples; ++i)
            out[i] = amplitude * std::sin(2.0f * static_cast<float>(kPi) * freqHz
                                           * static_cast<float>(i) / static_cast<float>(rate));
        return out;
    }

    // Push `src` through the resampler in bounded chunks, draining fully
    // after every push (mirrors how AnalysisThread's ring buffer is fed and
    // drained). Returns the resampled (48 kHz) stream.
    std::vector<float> resampleAll(AnalysisResampler& r, const std::vector<float>& src, RingBuffer<float>& ring)
    {
        std::vector<float> out;
        out.reserve(src.size());
        std::array<float, 512> hop{};
        size_t offset = 0;
        const size_t pushChunk = 8192;
        while (offset < src.size())
        {
            size_t chunk = std::min(pushChunk, src.size() - offset);
            size_t pushed = ring.push(src.data() + offset, chunk);
            offset += pushed;
            while (r.pullHop(ring, hop.data(), 512))
                out.insert(out.end(), hop.begin(), hop.end());
        }
        while (r.pullHop(ring, hop.data(), 512))
            out.insert(out.end(), hop.begin(), hop.end());
        return out;
    }

    double rmsOfTail(const std::vector<float>& v, size_t tailCount)
    {
        size_t base = v.size() > tailCount ? v.size() - tailCount : 0;
        if (base >= v.size())
            return 0.0;
        double sumSq = 0.0;
        for (size_t i = base; i < v.size(); ++i)
            sumSq += static_cast<double>(v[i]) * static_cast<double>(v[i]);
        return std::sqrt(sumSq / static_cast<double>(v.size() - base));
    }
}

// =============================================================================
// T1.1 -- bypass bit-exact at 48 kHz
// =============================================================================

TEST_CASE("T1.1: bypass is bit-exact at 48kHz", "[resampler]")
{
    AnalysisResampler r;
    r.setSourceRate(48000.0);
    REQUIRE(r.isBypass());

    RingBuffer<float> ring(1 << 14);
    std::array<float, 512> hop{};
    uint32_t rng = 42;

    for (int h = 0; h < 10; ++h)
    {
        std::vector<float> in(512);
        for (auto& s : in)
            s = lcgNoise(rng);

        REQUIRE(ring.push(in.data(), in.size()) == in.size());
        REQUIRE(r.pullHop(ring, hop.data(), 512));
        for (size_t i = 0; i < 512; ++i)
            REQUIRE(hop[i] == in[i]);
    }
}

// =============================================================================
// T1.2 -- sample-count accounting / no drift
// =============================================================================

TEST_CASE("T1.2: sample-count accounting / no drift", "[resampler]")
{
    for (double rate : { 8000.0, 16000.0, 22050.0, 32000.0, 44100.0, 88200.0, 96000.0, 192000.0 })
    {
        AnalysisResampler r;
        r.setSourceRate(rate);

        const double durationSec = 20.0;
        const size_t totalSamples = static_cast<size_t>(rate * durationSec);
        std::vector<float> src = generateSineAtRate(300.0f, 0.3f, totalSamples, rate);

        RingBuffer<float> ring(1 << 15);
        std::array<float, 512> hop{};
        int hops = 0;
        size_t offset = 0;
        const size_t pushChunk = 8192;
        while (offset < src.size())
        {
            size_t chunk = std::min(pushChunk, src.size() - offset);
            size_t pushed = ring.push(src.data() + offset, chunk);
            offset += pushed;
            while (r.pullHop(ring, hop.data(), 512))
                ++hops;
        }
        while (r.pullHop(ring, hop.data(), 512))
            ++hops;

        double expectedSamplesEquivalent = static_cast<double>(totalSamples) * 48000.0 / rate;
        double actualSamplesEquivalent = static_cast<double>(hops) * 512.0;

        INFO("rate=" << rate << " hops=" << hops);
        REQUIRE(std::fabs(actualSamplesEquivalent - expectedSamplesEquivalent) <= 2.0 * 512.0);
    }
}

// =============================================================================
// T1.3 -- spectral fidelity
// =============================================================================

TEST_CASE("T1.3: spectral fidelity after resampling", "[resampler]")
{
    for (double rate : { 8000.0, 16000.0, 22050.0, 32000.0, 44100.0, 88200.0, 96000.0, 192000.0 })
    {
        AnalysisResampler r;
        r.setSourceRate(rate);

        const double durationSec = 1.0;
        const size_t totalSamples = static_cast<size_t>(rate * durationSec);
        std::vector<float> src = generateSineAtRate(1000.0f, 0.5f, totalSamples, rate);

        RingBuffer<float> ring(1 << 15);
        std::vector<float> resampled = resampleAll(r, src, ring);

        REQUIRE(resampled.size() >= 2048);
        size_t base = resampled.size() - 2048;

        FFTProcessor fft;
        std::array<float, 2048> block{};
        for (int i = 0; i < 2048; ++i)
            block[static_cast<size_t>(i)] = resampled[base + static_cast<size_t>(i)];
        fft.process(block.data(), 2048);
        const float* mag = fft.magnitudeSpectrum();

        int peakBin = 0;
        float peakVal = 0.0f;
        for (int b = 0; b < FFTProcessor::kNumBins; ++b)
        {
            if (mag[static_cast<size_t>(b)] > peakVal)
            {
                peakVal = mag[static_cast<size_t>(b)];
                peakBin = b;
            }
        }
        INFO("rate=" << rate << " peakBin=" << peakBin);
        REQUIRE(std::abs(peakBin - 43) <= 1);

        double sumSq = 0.0;
        for (float s : block)
            sumSq += static_cast<double>(s) * static_cast<double>(s);
        double blockRms = std::sqrt(sumSq / static_cast<double>(block.size()));
        INFO("rate=" << rate << " rms=" << blockRms);
        REQUIRE(std::fabs(blockRms - 0.3536) <= 0.03 * 0.3536 + 0.01);
    }
}

// =============================================================================
// T1.4 -- anti-alias prefilter (ratio > 1: device above 48 kHz)
// =============================================================================

TEST_CASE("T1.4: anti-alias prefilter attenuates above-Nyquist content", "[resampler]")
{
    const double rate = 96000.0;
    const double durationSec = 0.5;
    const size_t totalSamples = static_cast<size_t>(rate * durationSec);
    const float amplitude = 0.5f;
    const double inputRms = static_cast<double>(amplitude) / std::sqrt(2.0);

    AnalysisResampler r40;
    r40.setSourceRate(rate);
    std::vector<float> src40 = generateSineAtRate(40000.0f, amplitude, totalSamples, rate);
    RingBuffer<float> ring40(1 << 15);
    std::vector<float> out40 = resampleAll(r40, src40, ring40);
    double outRms40 = rmsOfTail(out40, 4096);

    // Fail-first (pre-R13, no prefilter): the 40kHz tone folds into the
    // 48kHz band at roughly full amplitude instead of vanishing.
    INFO("40kHz outRms=" << outRms40 << " inputRms=" << inputRms);
    REQUIRE(outRms40 <= 0.15 * inputRms);

    AnalysisResampler r10;
    r10.setSourceRate(rate);
    std::vector<float> src10 = generateSineAtRate(10000.0f, amplitude, totalSamples, rate);
    RingBuffer<float> ring10(1 << 15);
    std::vector<float> out10 = resampleAll(r10, src10, ring10);
    double outRms10 = rmsOfTail(out10, 4096);

    INFO("10kHz outRms=" << outRms10 << " inputRms=" << inputRms);
    REQUIRE(std::fabs(outRms10 - inputRms) <= 0.03 * inputRms);
}

// =============================================================================
// T1.5 -- rate switch mid-stream (run under ASan via apply_sanitizers)
// =============================================================================

TEST_CASE("T1.5: rate switch mid-stream produces no assertion failures", "[resampler]")
{
    AnalysisResampler r;
    RingBuffer<float> ring(1 << 15);
    std::array<float, 512> hop{};

    auto feedSegment = [&](double rate, double durationSec) {
        r.setSourceRate(rate);
        const size_t totalSamples = static_cast<size_t>(rate * durationSec);
        std::vector<float> sig = generateSineAtRate(500.0f, 0.4f, totalSamples, rate);

        int hopCount = 0;
        size_t offset = 0;
        const size_t pushChunk = 4096;
        while (offset < sig.size())
        {
            size_t chunk = std::min(pushChunk, sig.size() - offset);
            size_t pushed = ring.push(sig.data() + offset, chunk);
            offset += pushed;
            while (r.pullHop(ring, hop.data(), 512))
                ++hopCount;
        }
        return hopCount;
    };

    int hops16 = feedSegment(16000.0, 2.0);
    int expected16 = static_cast<int>(2.0 * 48000.0 / 512.0);
    REQUIRE(std::abs(hops16 - expected16) <= 2);

    int hops48 = feedSegment(48000.0, 2.0);
    int expected48 = static_cast<int>(2.0 * 48000.0 / 512.0);
    REQUIRE(std::abs(hops48 - expected48) <= 2);
}

// =============================================================================
// T1.6 -- AudioCallback rate-cell wiring
// =============================================================================

TEST_CASE("T1.6: AudioCallback rate cell reflects audioDeviceAboutToStart", "[resampler][audiocallback]")
{
    RingBuffer<float> ring(1024);
    AudioCallback cb(ring);

    FakeAudioIODevice fake16k(16000.0, 128, 2);
    cb.audioDeviceAboutToStart(&fake16k);
    REQUIRE(cb.sampleRateCell().load() == 16000.0);

    FakeAudioIODevice fake48k(48000.0, 128, 2);
    cb.audioDeviceAboutToStart(&fake48k);
    REQUIRE(cb.sampleRateCell().load() == 48000.0);
}
