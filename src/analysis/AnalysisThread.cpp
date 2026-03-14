#include "AnalysisThread.h"
#include <chrono>
#include "FFTProcessor.h"
#include "SpectralFeatures.h"
#include "OnsetDetector.h"
#include "BPMTracker.h"
#include "MFCCExtractor.h"
#include "ChromaExtractor.h"
#include "KeyDetector.h"
#include "LoudnessAnalyzer.h"
#include "StructuralDetector.h"
#include "PitchTracker.h"
#include <cmath>
#include <algorithm>

AnalysisThread::AnalysisThread(RingBuffer<float>& ringBuffer)
    : juce::Thread("AnalysisThread"),
      ringBuffer_(ringBuffer)
{
    // Pre-allocate all analysis modules
    fftProcessor_       = std::make_unique<FFTProcessor>();
    spectralFeatures_   = std::make_unique<SpectralFeatures>(
                              FFTProcessor::kNumBins, static_cast<float>(kSampleRate), FFTProcessor::kFFTSize);
    onsetDetector_      = std::make_unique<OnsetDetector>(kHopSize, 1024, kSampleRate);
    bpmTracker_         = std::make_unique<BPMTracker>(kHopSize, 1024, kSampleRate);
    mfccExtractor_      = std::make_unique<MFCCExtractor>(
                              FFTProcessor::kNumBins, static_cast<float>(kSampleRate), FFTProcessor::kFFTSize);
    chromaExtractor_    = std::make_unique<ChromaExtractor>(
                              FFTProcessor::kNumBins, static_cast<float>(kSampleRate), FFTProcessor::kFFTSize);
    keyDetector_        = std::make_unique<KeyDetector>();
    loudnessAnalyzer_   = std::make_unique<LoudnessAnalyzer>(kSampleRate);
    structuralDetector_ = std::make_unique<StructuralDetector>(
                              static_cast<float>(kSampleRate), kHopSize);
    pitchTracker_       = std::make_unique<PitchTracker>(kHopSize, FFTProcessor::kFFTSize, kSampleRate);

    // Init transient density tracking
    onsetHistory_.fill(false);
    hopsPerSecond_ = static_cast<float>(kSampleRate) / static_cast<float>(kHopSize);
}

AnalysisThread::~AnalysisThread()
{
    stopThread(1000);
}

void AnalysisThread::run()
{
    std::array<float, kHopSize> hopBuffer{};

    while (!threadShouldExit())
    {
        auto available = ringBuffer_.availableToRead();

        if (available < static_cast<size_t>(kHopSize))
        {
            sleep(1);
            continue;
        }

        auto read = ringBuffer_.pop(hopBuffer.data(), kHopSize);
        if (read == 0)
            continue;

        // Shift analysis buffer left by hop, append new samples
        if (samplesInBuffer_ >= kBlockSize)
        {
            std::memmove(analysisBuffer_.data(),
                         analysisBuffer_.data() + kHopSize,
                         static_cast<size_t>(kBlockSize - kHopSize) * sizeof(float));
            samplesInBuffer_ = kBlockSize - kHopSize;
        }

        std::memcpy(analysisBuffer_.data() + samplesInBuffer_,
                     hopBuffer.data(),
                     read * sizeof(float));
        samplesInBuffer_ += static_cast<int>(read);

        totalSamplesProcessed_ += read;

        // Need a full block before running FFT-based analysis
        if (samplesInBuffer_ < kBlockSize)
            continue;

        // === PIPELINE START ===
        auto pipelineStart = std::chrono::high_resolution_clock::now();
        auto stageStart = pipelineStart;
        auto stageEnd = pipelineStart;

        // Acquire write buffer from FeatureBus
        FeatureSnapshot* snap = featureBus_.acquireWrite();

        // --- 1. Raw time-domain: RMS, peak ---
        float sumSq = 0.0f;
        float peak = 0.0f;
        for (int i = 0; i < kBlockSize; ++i)
        {
            float s = analysisBuffer_[static_cast<size_t>(i)];
            sumSq += s * s;
            float absS = std::fabs(s);
            if (absS > peak)
                peak = absS;
        }

        float rms = std::sqrt(sumSq / static_cast<float>(kBlockSize));
        snap->rms = rms;
        snap->peak = peak;
        snap->rmsDB = (rms > 1e-10f) ? 20.0f * std::log10(rms) : -100.0f;

        // Backward-compatible atomic accessors
        currentRMS_.store(rms, std::memory_order_relaxed);
        currentPeak_.store(peak, std::memory_order_relaxed);
        stageEnd = std::chrono::high_resolution_clock::now();
        stageTimesUs_[0] += std::chrono::duration<double, std::micro>(stageEnd - stageStart).count();
        stageStart = stageEnd;

        // --- 2. FFT → magnitude spectrum ---
        fftProcessor_->process(analysisBuffer_.data(), kBlockSize);
        const float* mag = fftProcessor_->magnitudeSpectrum();

        stageEnd = std::chrono::high_resolution_clock::now();
        stageTimesUs_[1] += std::chrono::duration<double, std::micro>(stageEnd - stageStart).count();
        stageStart = stageEnd;

        // --- 3. Spectral features ---
        spectralFeatures_->process(mag);
        snap->spectralCentroid = spectralFeatures_->centroid();
        snap->spectralFlux     = spectralFeatures_->flux();
        snap->spectralFlatness = spectralFeatures_->flatness();
        snap->spectralRolloff  = spectralFeatures_->rolloff();
        std::memcpy(snap->bandEnergies, spectralFeatures_->bandEnergies(),
                     sizeof(snap->bandEnergies));

        stageEnd = std::chrono::high_resolution_clock::now();
        stageTimesUs_[2] += std::chrono::duration<double, std::micro>(stageEnd - stageStart).count();
        stageStart = stageEnd;

        // --- 4. Onset detection ---
        onsetDetector_->process(hopBuffer.data());
        snap->onsetDetected = onsetDetector_->onsetDetected();
        snap->onsetStrength = onsetDetector_->onsetStrength();

        stageEnd = std::chrono::high_resolution_clock::now();
        stageTimesUs_[3] += std::chrono::duration<double, std::micro>(stageEnd - stageStart).count();
        stageStart = stageEnd;

        // --- 5. BPM tracking + beat phase ---
        bpmTracker_->process(hopBuffer.data());
        snap->bpm       = bpmTracker_->bpm();
        snap->beatPhase = bpmTracker_->beatPhase();

        stageEnd = std::chrono::high_resolution_clock::now();
        stageTimesUs_[4] += std::chrono::duration<double, std::micro>(stageEnd - stageStart).count();
        stageStart = stageEnd;

        // --- 6. MFCC ---
        mfccExtractor_->process(mag);
        std::memcpy(snap->mfccs, mfccExtractor_->mfccs(), sizeof(snap->mfccs));

        stageEnd = std::chrono::high_resolution_clock::now();
        stageTimesUs_[5] += std::chrono::duration<double, std::micro>(stageEnd - stageStart).count();
        stageStart = stageEnd;

        // --- 7. Chroma + HCDF ---
        chromaExtractor_->process(mag);
        std::memcpy(snap->chromagram, chromaExtractor_->chromagram(),
                     sizeof(snap->chromagram));
        snap->harmonicChangeDetection = chromaExtractor_->hcdf();

        stageEnd = std::chrono::high_resolution_clock::now();
        stageTimesUs_[6] += std::chrono::duration<double, std::micro>(stageEnd - stageStart).count();
        stageStart = stageEnd;

        // --- 8. Key detection ---
        keyDetector_->process(chromaExtractor_->chromagram());
        snap->detectedKey = keyDetector_->detectedKey();
        snap->keyIsMajor  = keyDetector_->isMajor();

        stageEnd = std::chrono::high_resolution_clock::now();
        stageTimesUs_[7] += std::chrono::duration<double, std::micro>(stageEnd - stageStart).count();
        stageStart = stageEnd;

        // --- 9. Pitch detection ---
        pitchTracker_->process(hopBuffer.data());
        snap->dominantPitch  = pitchTracker_->dominantPitch();
        snap->pitchConfidence = pitchTracker_->confidence();

        stageEnd = std::chrono::high_resolution_clock::now();
        stageTimesUs_[8] += std::chrono::duration<double, std::micro>(stageEnd - stageStart).count();
        stageStart = stageEnd;

        // --- 10. Loudness (LUFS, dynamic range) ---
        loudnessAnalyzer_->process(hopBuffer.data(), static_cast<int>(read));
        snap->lufs         = loudnessAnalyzer_->lufs();
        snap->dynamicRange = loudnessAnalyzer_->dynamicRange();

        stageEnd = std::chrono::high_resolution_clock::now();
        stageTimesUs_[9] += std::chrono::duration<double, std::micro>(stageEnd - stageStart).count();
        stageStart = stageEnd;

        // --- 11. Structural detection ---
        // (needs transient density, compute it first)

        // --- 12. Transient density: sliding window onset counter ---
        // Remove the oldest onset from count, add the newest
        if (onsetHistory_[static_cast<size_t>(onsetHistoryPos_)])
            --onsetCount_;
        onsetHistory_[static_cast<size_t>(onsetHistoryPos_)] = snap->onsetDetected;
        if (snap->onsetDetected)
            ++onsetCount_;
        onsetHistoryPos_ = (onsetHistoryPos_ + 1) % kOnsetWindowSize;

        float windowDurationSec = static_cast<float>(kOnsetWindowSize) / hopsPerSecond_;
        snap->transientDensity = static_cast<float>(onsetCount_) / windowDurationSec;

        // Now run structural detection with transient density
        structuralDetector_->process(rms, spectralFeatures_->flux(),
                                      snap->transientDensity);
        snap->structuralState = structuralDetector_->structuralState();

        stageEnd = std::chrono::high_resolution_clock::now();
        stageTimesUs_[10] += std::chrono::duration<double, std::micro>(stageEnd - stageStart).count();
        stageStart = stageEnd;

        // --- Timing ---
        snap->timestamp = totalSamplesProcessed_;
        snap->wallClockSeconds = static_cast<double>(totalSamplesProcessed_)
                                 / static_cast<double>(kSampleRate);

        // === PUBLISH ===
        featureBus_.publishWrite();

        // CPU load: time spent as fraction of hop period
        auto pipelineEnd = std::chrono::high_resolution_clock::now();
        double elapsedUs = std::chrono::duration<double, std::micro>(pipelineEnd - pipelineStart).count();
        double hopPeriodUs = 1e6 * static_cast<double>(kHopSize) / static_cast<double>(kSampleRate);
        float load = static_cast<float>(elapsedUs / hopPeriodUs) * 100.0f;
        // EMA smoothing
        float prev = cpuLoad_.load(std::memory_order_relaxed);
        cpuLoad_.store(prev + 0.1f * (load - prev), std::memory_order_relaxed);

        // Periodic per-stage profiling output
        if (++profileFrameCount_ >= kProfileInterval)
        {
            static const char* stageNames[] = {
                "RMS/Peak", "FFT", "Spectral", "Onset", "BPM",
                "MFCC", "Chroma", "Key", "Pitch", "Loudness",
                "Structural"
            };
            double total = 0.0;
            std::cerr << "[Analysis Profile] Per-stage avg (us) over " << kProfileInterval << " hops:" << std::endl;
            for (int s = 0; s < 11; ++s)
            {
                double avg = stageTimesUs_[static_cast<size_t>(s)] / kProfileInterval;
                total += avg;
                std::cerr << "  " << stageNames[s] << ": " << static_cast<int>(avg + 0.5) << " us" << std::endl;
            }
            std::cerr << "  TOTAL: " << static_cast<int>(total + 0.5) << " us ("
                      << static_cast<int>(total / hopPeriodUs * 100.0 + 0.5) << "% of hop period)" << std::endl;

            stageTimesUs_.fill(0.0);
            profileFrameCount_ = 0;
        }

        // Copy recent samples for waveform display
        int wfCount = std::min(kBlockSize, kWaveformBufferSize);
        int offset = kBlockSize - wfCount;
        std::memcpy(const_cast<float*>(waveformBuffer_.data()),
                     analysisBuffer_.data() + offset,
                     static_cast<size_t>(wfCount) * sizeof(float));
        waveformSampleCount_.store(wfCount, std::memory_order_release);
    }
}

void AnalysisThread::getWaveformSamples(float* dest, int& count) const
{
    count = waveformSampleCount_.load(std::memory_order_acquire);
    if (count > 0)
        std::memcpy(dest, waveformBuffer_.data(), static_cast<size_t>(count) * sizeof(float));
}
