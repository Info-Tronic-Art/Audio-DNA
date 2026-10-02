// test_audio_engine_devices -- s-rta-0930 bt2 (.harmony/.reports/s-rta-0930/plan-bt2.md I2 + ruling-bt2-seats.md AM16):
// the REAL AudioEngine and its device open, headless, with NO device opened.
//   AE1  every real device is denied through the TEST-ONLY env (the "*" deny-all token + every enumerated name), a
//        SAFETY precheck proves the guard lists nothing BEFORE the engine is built (nothing can open, no TCC prompt),
//        then setSourceMode must send no change message and write no explicit settings (one device open per launch).
//   AE2  [lint] setSourceMode's body names no device manager / reconciler (also catches a re-open GUARDED by
//        getCurrentAudioDevice(), which AE1's deny-everything engine cannot see).
#include <catch2/catch_test_macros.hpp>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_events/juce_events.h>
#include "audio/AudioEngine.h"
#include "audio/CoreAudioDeviceInfo.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#ifndef AUDIODNA_SRC_DIR
#error "AUDIODNA_SRC_DIR must point at src/"
#endif

namespace
{
struct Counter : juce::ChangeListener
{
    int n = 0;
    void changeListenerCallback(juce::ChangeBroadcaster*) override { ++n; }
};

std::string readFile(const std::string& p)
{
    std::ifstream f(p);
    std::stringstream s;
    s << f.rdbuf();
    return s.str();
}

std::string stripLineComments(const std::string& s)
{
    std::string out;
    std::istringstream in(s);
    std::string line;
    while (std::getline(in, line))
    {
        const auto c = line.find("//");
        out += (c == std::string::npos ? line : line.substr(0, c)) + "\n";
    }
    return out;
}

// The body of the first definition `sig` ... `{ ... }` (brace-matched), or "" when absent.
std::string bodyOf(const std::string& src, const std::string& sig)
{
    const auto at = src.find(sig);
    if (at == std::string::npos)
        return {};
    const auto open = src.find('{', at);
    if (open == std::string::npos)
        return {};
    int depth = 0;
    for (size_t i = open; i < src.size(); ++i)
    {
        if (src[i] == '{')
            ++depth;
        else if (src[i] == '}' && --depth == 0)
            return src.substr(open, i - open + 1);
    }
    return {};
}
}

#if JUCE_MAC
TEST_CASE("AE1 setSourceMode never touches the device manager (no explicit-settings XML, no change message)",
          "[audio_engine_devices]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    juce::StringArray all("*");   // TEST-ONLY deny-all: a device plugged in after the precheck is hidden too
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
#endif

TEST_CASE("AE2 [lint] AudioEngine::setSourceMode is an atomic flag flip: its body names no device manager / reconciler",
          "[audio_engine_devices][lint]")
{
    const std::string path = std::string(AUDIODNA_SRC_DIR) + "/audio/AudioEngine.cpp";
    const auto src = stripLineComments(readFile(path));
    const auto body = bodyOf(src, "void AudioEngine::setSourceMode(");
    INFO(path);
    REQUIRE_FALSE(body.empty());   // the function is found: a rename never makes this lint vacuous
    CHECK(body.find("deviceManager_") == std::string::npos);
    CHECK(body.find("deviceReconciler_") == std::string::npos);
}
