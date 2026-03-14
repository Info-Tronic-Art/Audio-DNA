#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "analysis/SpectralFeatures.h"
#include <array>
#include <cmath>
#include <vector>

using Catch::Approx;

static constexpr int kFFTSize = 2048;
static constexpr int kNumBins = kFFTSize / 2 + 1;  // 1025
static constexpr float kSampleRate = 48000.0f;

// Generate magnitude spectrum for a pure sine wave at a given frequency.
// The energy concentrates in the bin closest to the frequency.
static std::vector<float> sineMagnitudeSpectrum(float freqHz, float amplitude = 1.0f)
{
    std::vector<float> mag(kNumBins, 0.0f);
    int bin = static_cast<int>(std::round(freqHz * kFFTSize / kSampleRate));
    if (bin >= 0 && bin < kNumBins)
        mag[static_cast<size_t>(bin)] = amplitude;
    return mag;
}

// Generate flat (white noise-like) magnitude spectrum.
static std::vector<float> flatMagnitudeSpectrum(float amplitude = 1.0f)
{
    std::vector<float> mag(kNumBins, amplitude);
    mag[0] = 0.0f;  // skip DC
    return mag;
}

TEST_CASE("SpectralFeatures centroid — pure sine", "[spectral]")
{
    SpectralFeatures sf(kNumBins, kSampleRate, kFFTSize);

    SECTION("440 Hz sine → centroid near 440 Hz")
    {
        auto mag = sineMagnitudeSpectrum(440.0f);
        sf.process(mag.data());

        // Centroid should be very close to 440 Hz (within one bin width ~23.4 Hz)
        float binWidth = kSampleRate / kFFTSize;
        REQUIRE(sf.centroid() == Approx(440.0f).margin(binWidth));
    }

    SECTION("1000 Hz sine → centroid near 1000 Hz")
    {
        auto mag = sineMagnitudeSpectrum(1000.0f);
        sf.process(mag.data());

        float binWidth = kSampleRate / kFFTSize;
        REQUIRE(sf.centroid() == Approx(1000.0f).margin(binWidth));
    }

    SECTION("high frequency sine → high centroid")
    {
        auto mag = sineMagnitudeSpectrum(8000.0f);
        sf.process(mag.data());

        REQUIRE(sf.centroid() > 7000.0f);
    }
}

TEST_CASE("SpectralFeatures centroid — flat spectrum", "[spectral]")
{
    SpectralFeatures sf(kNumBins, kSampleRate, kFFTSize);
    auto mag = flatMagnitudeSpectrum();
    sf.process(mag.data());

    // Flat spectrum → centroid should be near the midpoint of frequency range
    // For uniform magnitudes, centroid = mean of all bin frequencies
    // = sum(k * fs/N, k=1..1024) / 1024 ≈ fs/2 * (1025/2048) ≈ Nyquist/2
    float nyquist = kSampleRate / 2.0f;
    REQUIRE(sf.centroid() > nyquist * 0.4f);
    REQUIRE(sf.centroid() < nyquist * 0.6f);
}

TEST_CASE("SpectralFeatures flux — identical frames give zero flux", "[spectral]")
{
    SpectralFeatures sf(kNumBins, kSampleRate, kFFTSize);
    auto mag = sineMagnitudeSpectrum(440.0f);

    // First frame establishes baseline (no previous frame)
    sf.process(mag.data());

    // Second identical frame → zero flux
    sf.process(mag.data());
    REQUIRE(sf.flux() == Approx(0.0f).margin(1e-6f));
}

TEST_CASE("SpectralFeatures flux — changing frames give nonzero flux", "[spectral]")
{
    SpectralFeatures sf(kNumBins, kSampleRate, kFFTSize);

    auto mag1 = sineMagnitudeSpectrum(440.0f);
    sf.process(mag1.data());

    auto mag2 = sineMagnitudeSpectrum(1000.0f);
    sf.process(mag2.data());

    REQUIRE(sf.flux() > 0.0f);
}

TEST_CASE("SpectralFeatures flatness — peaked spectrum is tonal (low flatness)", "[spectral]")
{
    SpectralFeatures sf(kNumBins, kSampleRate, kFFTSize);

    // Create a spectrum with one dominant peak plus low-level noise floor.
    // Pure single-bin spectrum doesn't test flatness well because the impl
    // skips zero-power bins — we need multiple nonzero bins with unequal power.
    std::vector<float> mag(kNumBins, 0.001f);  // noise floor
    mag[0] = 0.0f;  // skip DC
    int bin440 = static_cast<int>(std::round(440.0f * kFFTSize / kSampleRate));
    mag[static_cast<size_t>(bin440)] = 10.0f;  // dominant peak
    sf.process(mag.data());

    // Peaked spectrum → low flatness (geometric mean << arithmetic mean)
    REQUIRE(sf.flatness() < 0.1f);
}

TEST_CASE("SpectralFeatures flatness — flat spectrum is noisy (high flatness)", "[spectral]")
{
    SpectralFeatures sf(kNumBins, kSampleRate, kFFTSize);
    auto mag = flatMagnitudeSpectrum(1.0f);
    sf.process(mag.data());

    // Uniform magnitudes → flatness ≈ 1.0
    REQUIRE(sf.flatness() > 0.9f);
}

TEST_CASE("SpectralFeatures rolloff — low frequency content", "[spectral]")
{
    SpectralFeatures sf(kNumBins, kSampleRate, kFFTSize);
    auto mag = sineMagnitudeSpectrum(200.0f);
    sf.process(mag.data());

    // All energy at 200 Hz → rolloff should be at or near 200 Hz
    float binWidth = kSampleRate / kFFTSize;
    REQUIRE(sf.rolloff() == Approx(200.0f).margin(binWidth));
}

TEST_CASE("SpectralFeatures band energies — bass sine activates bass band", "[spectral]")
{
    SpectralFeatures sf(kNumBins, kSampleRate, kFFTSize);

    // 100 Hz → Bass band (60-250 Hz), index 1
    auto mag = sineMagnitudeSpectrum(100.0f, 10.0f);
    sf.process(mag.data());

    const float* bands = sf.bandEnergies();

    // Bass band should have highest energy
    float bassEnergy = bands[1];
    REQUIRE(bassEnergy > 0.0f);

    // Other bands should be zero or very small
    for (int i = 0; i < SpectralFeatures::kNumBands; ++i)
    {
        if (i != 1)
            REQUIRE(bands[i] < bassEnergy);
    }
}

TEST_CASE("SpectralFeatures band energies — high frequency sine activates brilliance", "[spectral]")
{
    SpectralFeatures sf(kNumBins, kSampleRate, kFFTSize);

    // 10000 Hz → Brilliance band (6k-20k Hz), index 6
    auto mag = sineMagnitudeSpectrum(10000.0f, 10.0f);
    sf.process(mag.data());

    const float* bands = sf.bandEnergies();
    float brilliance = bands[6];
    REQUIRE(brilliance > 0.0f);

    for (int i = 0; i < SpectralFeatures::kNumBands; ++i)
    {
        if (i != 6)
            REQUIRE(bands[i] < brilliance);
    }
}

TEST_CASE("SpectralFeatures silent input", "[spectral]")
{
    SpectralFeatures sf(kNumBins, kSampleRate, kFFTSize);
    std::vector<float> silence(kNumBins, 0.0f);
    sf.process(silence.data());

    REQUIRE(sf.centroid() == Approx(0.0f));
    REQUIRE(sf.flux() == Approx(0.0f));
    REQUIRE(sf.flatness() == Approx(0.0f));
}
