#include "AudioCallback.h"

AudioCallback::AudioCallback(RingBuffer<float>& ringBuffer)
    : ringBuffer_(ringBuffer)
{
}

void AudioCallback::audioDeviceIOCallbackWithContext(
    const float* const* inputChannelData,
    int numInputChannels,
    float* const* outputChannelData,
    int numOutputChannels,
    int numSamples,
    const juce::AudioIODeviceCallbackContext&)
{
    // We don't use inputChannelData — audio comes from the transport source
    // which writes directly to outputChannelData. We just capture output for analysis.
    juce::ignoreUnused(inputChannelData, numInputChannels);

    if (numOutputChannels == 0 || numSamples == 0)
        return;

    // Mono downmix from output channels
    auto* dest = monoBuffer_.data();
    const int samplesToProcess = std::min(numSamples, static_cast<int>(monoBuffer_.size()));

    if (numOutputChannels == 1)
    {
        std::memcpy(dest, outputChannelData[0],
                    static_cast<size_t>(samplesToProcess) * sizeof(float));
    }
    else
    {
        const float gain = 1.0f / static_cast<float>(numOutputChannels);
        for (int i = 0; i < samplesToProcess; ++i)
        {
            float sum = 0.0f;
            for (int ch = 0; ch < numOutputChannels; ++ch)
                sum += outputChannelData[ch][i];
            dest[i] = sum * gain;
        }
    }

    ringBuffer_.push(dest, static_cast<size_t>(samplesToProcess));
}

void AudioCallback::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    monoBuffer_.resize(static_cast<size_t>(device->getCurrentBufferSizeSamples()), 0.0f);
}

void AudioCallback::audioDeviceStopped()
{
}
