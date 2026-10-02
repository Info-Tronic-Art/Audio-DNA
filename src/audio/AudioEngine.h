#pragma once
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <atomic>
#include <mutex>
#include "AudioCallback.h"
#include "CombinedCallback.h"
#include "DeviceGuard.h"
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
    // s-rta-0929b btguard: true when the open device has an input (a wired mic or the onboard mic). The device policy
    // (DeviceGuard.h) never opens a Bluetooth / wireless device, so with only such an input the app runs output-only.
    bool hasInputDevice() const;
    // The input device's name and rate ("MacBook Pro Microphone @ 48000Hz"), or the plain-words no-input / no-device text.
    juce::String getDeviceStatus() const;

    // s-rta-0929b btguard: Ok = an input is open; NoInput = output-only (no allowed input); NoDevice = nothing allowed.
    // s-rta-0930 bt2: MicReplaced = a re-apply replaced a lost mic by another (lostInput() names it, openInput() the new one).
    enum class DeviceState { Ok, NoInput, NoDevice, MicReplaced };
    DeviceState getDeviceState() const;
    static DeviceState deviceStateFor(bool haveDevice, const juce::String& openInput, const juce::String& lostInput);
    juce::String openInput() const;   // the open device's input name ("" when none)
    juce::String lostInput() const;   // the mic the last input-changing re-apply lost and replaced ("" otherwise)
    // Message thread: the device manager changed (open / close / device list). MainComponent refreshes its indicator.
    std::function<void()> onDeviceStateChanged;
    // s-rta-0930 bt2 (message thread): after each automatic device re-apply (DeviceReconciler) -- "" = no error.
    std::function<void(const juce::String& error)> onDevicesReapplied;

    // Actual sample rate of the running output device, or 0.0 if none.
    double getCurrentSampleRate() const;

    // R13: the device rate cell (release-stored by AudioCallback on
    // audioDeviceAboutToStart), read (acquire) by the analysis thread's
    // resampler. 0.0 = no device / not started yet.
    const std::atomic<double>& sourceSampleRateCell() const noexcept { return audioCallback_.sampleRateCell(); }

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

#if AUDIODNA_TEST_SERVER
    // s-rta-0929 asyncload (TEST-ONLY): the audio-callback witnesses (CombinedCallback / AudioCallback, plan 5.7).
    double takeAudioGapMaxMs() { return combinedCallback_.takeGapMaxMs(); }
    uint64_t audioCallbacks() const { return combinedCallback_.callbacks(); }
    int audioPeriodSamples() const { return combinedCallback_.periodSamples(); }
    uint64_t ringOverruns() const { return audioCallback_.ringOverruns(); }
    // s-rta-0929b btguard (TEST-ONLY): GET /api/debug/audio_devices -- the last device scan, the opened devices, the
    // state, `opens` (device starts since launch) and the reconciler's re-applies. Built on the MESSAGE thread at every
    // device change (publishDeviceStatus), read on the HTTP thread as a mutex-guarded copy: never the manager itself.
    juce::var deviceStatusVar() const;
    // s-rta-0930 bt2 (TEST-ONLY, message thread): POST /api/debug/audio_deny replaces the denied device names and runs
    // the guard's device-list-change path (the plug / unplug stand-in); POST /api/debug/audio_stop stops the open device
    // as JUCE's combiner does when its input dies (the manager keeps it).
    void debugSetDeniedDevices(const juce::StringArray& names);
    void debugStopDevice();
#endif

private:
    // s-rta-0929b btguard: JUCE's manager with its CoreAudio type wrapped by the no-wireless device policy.
    GuardedAudioDeviceManager deviceManager_{ devguard::productionConfig() };
    // BG4 / bt2: re-applies the policy when the app has no allowed device / input or lost its mic (DeviceGuard.h).
    DeviceReconciler deviceReconciler_{ deviceManager_, 2, 2 };
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
#if AUDIODNA_TEST_SERVER
    void publishDeviceStatus();                     // message thread
    mutable std::mutex deviceStatusMutex_;          // message thread (publish) <-> HTTP thread (read) only
    juce::var deviceStatus_;                        // guarded by deviceStatusMutex_; never mutated after publish
#endif
};
