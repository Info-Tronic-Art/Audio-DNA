#pragma once
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "AudioCallback.h"
#include "CombinedCallback.h"
#include "RingBuffer.h"
#include "recording/AudioTap.h"

// Owns AudioDeviceManager, AudioTransportSource, format readers.
// Provides transport controls and feeds AudioCallback for analysis.
class AudioEngine : public juce::ChangeListener
{
public:
    AudioEngine(RingBuffer<float>& ringBuffer);
    ~AudioEngine() override;

    // Load an audio file for analysis (no playback output)
    bool loadFile(const juce::File& file);
    void play();   // Start reading file into analysis pipeline
    void pause();
    void stop();
    bool isPlaying() const;

    juce::AudioDeviceManager& getDeviceManager() { return deviceManager_; }
    juce::AudioTransportSource& getTransportSource() { return transportSource_; }

    // ChangeListener — transport state changes
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    std::function<void(bool isPlaying)> onTransportStateChanged;
    std::function<void(const juce::String& message)> onError;

    bool hasAudioDevice() const;
    juce::String getDeviceStatus() const;

    // Actual sample rate of the running output device, or 0.0 if none.
    double getCurrentSampleRate() const;

    // Audio source mode
    enum class SourceMode { File, MicInput };
    void setSourceMode(SourceMode mode);
    SourceMode getSourceMode() const { return sourceMode_; }

    void setInputGain(float gain) { combinedCallback_.inputGain.store(gain, std::memory_order_relaxed); }
    float getInputLevel() const { return combinedCallback_.inputLevel.load(std::memory_order_relaxed); }

    // D10.1/D1: the take's timebase origin (delivered-sample counter),
    // independent of the analysis ring buffer -- RecorderClock reads this
    // for `sample` (step 3 wiring). The tap itself, for step 3's recorder
    // to arm/disarm/drain gaps from.
    uint64_t getDeliveredSamples() const { return combinedCallback_.getDeliveredSamples(); }
    AudioTap& getAudioTap() { return combinedCallback_.tap(); }

private:
    juce::AudioDeviceManager deviceManager_;
    juce::AudioFormatManager formatManager_;
    juce::AudioSourcePlayer sourcePlayer_;
    juce::AudioTransportSource transportSource_;

    std::unique_ptr<juce::AudioFormatReaderSource> readerSource_;
    juce::TimeSliceThread readAheadThread_{"AudioReadAhead"};
    AudioCallback audioCallback_;

    // Mixes transport output/mic input, feeds the analysis ring buffer AND
    // (D10.1, s168 step 2) the AudioTap second fan-out. Lifted to
    // src/audio/CombinedCallback.h for testability (T1 constructs one
    // directly, without a real device).
    CombinedCallback combinedCallback_;
    SourceMode sourceMode_ = SourceMode::File;
};
