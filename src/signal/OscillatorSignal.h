#pragma once
#include "signal/Signal.h"
#include <cmath>

// OscillatorSignal: BPM-locked waveform generator.
// Wave shapes: Sine, Saw Up, Saw Down, Triangle, Square.
// Beat duration determines period relative to detected BPM.
class OscillatorSignal : public Signal
{
public:
    enum class WaveShape : uint8_t
    {
        Sine, SawUp, SawDown, Triangle, Square
    };

    // beatDuration: how many beats per cycle (0.25, 0.5, 1, 2, 4, 8)
    OscillatorSignal(const std::string& name, WaveShape shape = WaveShape::Sine,
                     float beatDuration = 1.0f)
        : Signal(name, Type::Oscillator, Category::Modulation)
        , shape_(shape)
        , beatDuration_(beatDuration) {}

    float getValue(const FeatureSnapshot& snapshot) const override
    {
        // Phase is derived from beatPhase scaled by duration
        // beatPhase is [0,1) per beat. We want [0,1) per cycle of beatDuration beats.
        // Use barPhase for multi-beat durations, or derive from beatPhase + beatInBar.
        float totalBeatPhase = snapshot.beatPhase + static_cast<float>(snapshot.beatInBar);
        // totalBeatPhase is in [0, 4) over one bar
        float cyclePhase = std::fmod(totalBeatPhase / beatDuration_, 1.0f);
        if (cyclePhase < 0.0f) cyclePhase += 1.0f;

        // Add phase offset
        cyclePhase = std::fmod(cyclePhase + phaseOffset_, 1.0f);

        float value = 0.0f;
        switch (shape_)
        {
            case WaveShape::Sine:
                value = 0.5f + 0.5f * std::sin(cyclePhase * 2.0f * 3.14159265f);
                break;
            case WaveShape::SawUp:
                value = cyclePhase;
                break;
            case WaveShape::SawDown:
                value = 1.0f - cyclePhase;
                break;
            case WaveShape::Triangle:
                value = (cyclePhase < 0.5f) ? (cyclePhase * 2.0f) : (2.0f - cyclePhase * 2.0f);
                break;
            case WaveShape::Square:
                value = (cyclePhase < 0.5f) ? 1.0f : 0.0f;
                break;
        }

        return value * amplitude_;
    }

    // Settings
    WaveShape getShape() const { return shape_; }
    void setShape(WaveShape s) { shape_ = s; }

    float getBeatDuration() const { return beatDuration_; }
    void setBeatDuration(float d) { beatDuration_ = d; }

    float getAmplitude() const { return amplitude_; }
    void setAmplitude(float a) { amplitude_ = a; }

    float getPhaseOffset() const { return phaseOffset_; }
    void setPhaseOffset(float p) { phaseOffset_ = p; } // [0, 1]

private:
    WaveShape shape_;
    float beatDuration_;
    float amplitude_ = 1.0f;
    float phaseOffset_ = 0.0f;
};
