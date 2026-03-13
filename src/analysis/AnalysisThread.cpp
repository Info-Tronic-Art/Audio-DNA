#include "AnalysisThread.h"
#include <cmath>
#include <algorithm>

AnalysisThread::AnalysisThread(RingBuffer<float>& ringBuffer)
    : juce::Thread("AnalysisThread"),
      ringBuffer_(ringBuffer)
{
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

        // Compute RMS and peak over current buffer
        float sumSq = 0.0f;
        float peak = 0.0f;
        const int count = std::min(samplesInBuffer_, kBlockSize);

        for (int i = 0; i < count; ++i)
        {
            float s = analysisBuffer_[static_cast<size_t>(i)];
            sumSq += s * s;
            float absS = std::fabs(s);
            if (absS > peak)
                peak = absS;
        }

        float rms = (count > 0) ? std::sqrt(sumSq / static_cast<float>(count)) : 0.0f;
        currentRMS_.store(rms, std::memory_order_relaxed);
        currentPeak_.store(peak, std::memory_order_relaxed);

        // Copy recent samples for waveform display
        int wfCount = std::min(count, kWaveformBufferSize);
        int offset = count - wfCount;
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
