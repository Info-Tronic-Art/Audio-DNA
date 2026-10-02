// SCRATCH (architect, plan-bt2): C3 RED on main's REAL AudioEngine, NO device opened (every real device denied through
// the TEST-ONLY ADNA_AUDIO_DENY_DEVICES the engine reads at construction). Witnesses: JUCE's explicit-settings XML and
// the manager's change messages -- setSourceMode must be a pure flag flip.
#include <catch2/catch_test_macros.hpp>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_events/juce_events.h>
#include "audio/AudioEngine.h"
#include "audio/CoreAudioDeviceInfo.h"
#include <cstdlib>

namespace {
struct Counter : juce::ChangeListener { int n = 0; void changeListenerCallback(juce::ChangeBroadcaster*) override { ++n; } };

}

TEST_CASE("XC3 setSourceMode never touches the device manager (no explicit-settings XML, no change message)", "[c3]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    juce::StringArray all;
    for (const auto& d : enumerateCoreAudioDevices())
    {
        if (d.inputName.isNotEmpty()) all.addIfNotAlreadyThere(d.inputName);
        if (d.outputName.isNotEmpty()) all.addIfNotAlreadyThere(d.outputName);
    }
    ::setenv("ADNA_AUDIO_DENY_DEVICES", all.joinIntoString(";").toRawUTF8(), 1);
    {
        // SAFETY: prove BEFORE constructing the engine that the guard would list NOTHING (no device can open, no TCC).
        std::unique_ptr<juce::AudioIODeviceType> ca(juce::AudioIODeviceType::createAudioIODeviceType_CoreAudio());
        REQUIRE(ca != nullptr);
        GuardedDeviceType g(std::move(ca), &enumerateCoreAudioDevices, devguard::productionConfig());
        g.scanForDevices();
        INFO("inputs " << g.getDeviceNames(true).joinIntoString("|") << " outputs " << g.getDeviceNames(false).joinIntoString("|"));
        REQUIRE(g.getDeviceNames(true).isEmpty());
        REQUIRE(g.getDeviceNames(false).isEmpty());
    }
    RingBuffer<float> ring(8192);
    {
        AudioEngine engine(ring);
        ::unsetenv("ADNA_AUDIO_DENY_DEVICES");
        REQUIRE(engine.getDeviceManager().getCurrentAudioDevice() == nullptr);   // nothing opened, nothing to prompt for
        engine.getDeviceManager().dispatchPendingMessages();
        Counter c;
        engine.getDeviceManager().addChangeListener(&c);
        CHECK(engine.getDeviceManager().createStateXml() == nullptr);
        engine.setSourceMode(AudioEngine::SourceMode::MicInput);
        engine.getDeviceManager().dispatchPendingMessages();
        INFO("xml after mic: " << (engine.getDeviceManager().createStateXml() != nullptr ? "SET" : "null") << ", change messages " << c.n);
        CHECK(engine.getDeviceManager().createStateXml() == nullptr);
        CHECK(c.n == 0);
        engine.setSourceMode(AudioEngine::SourceMode::File);
        engine.getDeviceManager().dispatchPendingMessages();
        engine.setSourceMode(AudioEngine::SourceMode::MicInput);
        engine.getDeviceManager().dispatchPendingMessages();
        CHECK(c.n == 0);
        engine.getDeviceManager().removeChangeListener(&c);
    }
}
