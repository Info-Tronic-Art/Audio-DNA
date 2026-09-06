#pragma once
#include "signal/Signal.h"
#include <vector>
#include <algorithm>
#include <cmath>

// EnvelopeSignal: custom curve with control points, BPM-locked or free-running.
// Each control point has a position [0,1] and value [0,1].
// The envelope interpolates between points.
class EnvelopeSignal : public Signal
{
public:
    struct ControlPoint
    {
        float position = 0.0f; // [0, 1] along the cycle
        float value = 0.0f;    // [0, 1] output value
    };

    enum class CurveType : uint8_t { Linear, Exponential, SCurve };

    EnvelopeSignal(const std::string& name, float beatDuration = 4.0f)
        : Signal(name, Type::Envelope, Category::Modulation)
        , beatDuration_(beatDuration)
    {
        // Default: linear ramp up then down
        points_.push_back({0.0f, 0.0f});
        points_.push_back({0.5f, 1.0f});
        points_.push_back({1.0f, 0.0f});
    }

    float getValue(const FeatureSnapshot& snapshot) const override
    {
        if (points_.size() < 2) return 0.0f;

        // Calculate phase from beat position.
        // S166-L5a: same fold-across-bars fix as OscillatorSignal::getValue
        // (see its comment for the full rationale) -- extends the phase
        // across bars so beatDuration_ > 4 (the default here is 4.0, the
        // boundary case that happened to still work) completes a full cycle
        // instead of stalling partway through.
        //
        // Which bar count feeds that fold is the same per-instance choice
        // as OscillatorSignal (S168, resetPhaseOnStructural_): false
        // (default) reads FeatureSnapshot::totalBarCount, so a real
        // structural drop mid-gesture can no longer yank this envelope's
        // shape backward; true reads FeatureSnapshot::barCount, reproducing
        // the original S166-L5a jump-on-drop trade-off. CORRECTION: this
        // comment previously claimed EnvelopeSignal "already received the
        // same fold-across-bars fix as OscillatorSignal" -- it had NOT; it
        // read barCount unconditionally with no switch at all until S168.
        float barsElapsed = resetPhaseOnStructural_
            ? static_cast<float>(snapshot.barCount)
            : static_cast<float>(snapshot.totalBarCount);
        float totalBeatPhase = snapshot.beatPhase + static_cast<float>(snapshot.beatInBar)
                             + 4.0f * barsElapsed;
        float cyclePhase = std::fmod(totalBeatPhase / beatDuration_, 1.0f);
        if (cyclePhase < 0.0f) cyclePhase += 1.0f;

        cyclePhase = std::fmod(cyclePhase + phaseOffset_, 1.0f);

        // One-shot: clamp to end after one cycle
        if (oneShot_ && !looping_)
        {
            cyclePhase = std::min(cyclePhase, 1.0f);
        }

        // Find surrounding control points
        const ControlPoint* before = &points_.front();
        const ControlPoint* after = &points_.back();

        for (size_t i = 0; i < points_.size() - 1; ++i)
        {
            if (cyclePhase >= points_[i].position && cyclePhase <= points_[i + 1].position)
            {
                before = &points_[i];
                after = &points_[i + 1];
                break;
            }
        }

        // Interpolate
        float range = after->position - before->position;
        float t = (range > 1e-6f) ? (cyclePhase - before->position) / range : 0.0f;
        t = std::clamp(t, 0.0f, 1.0f);

        // Apply curve type to interpolation factor
        switch (curveType_)
        {
            case CurveType::Exponential:
                t = t * t;
                break;
            case CurveType::SCurve:
                t = t * t * (3.0f - 2.0f * t);
                break;
            default:
                break;
        }

        float value = before->value + t * (after->value - before->value);
        return value * amplitude_;
    }

    // Control point management
    const std::vector<ControlPoint>& getPoints() const { return points_; }
    void setPoints(const std::vector<ControlPoint>& pts)
    {
        points_ = pts;
        std::sort(points_.begin(), points_.end(),
            [](const ControlPoint& a, const ControlPoint& b) {
                return a.position < b.position;
            });
    }

    void addPoint(float position, float value)
    {
        points_.push_back({position, value});
        std::sort(points_.begin(), points_.end(),
            [](const ControlPoint& a, const ControlPoint& b) {
                return a.position < b.position;
            });
    }

    // Settings
    float getBeatDuration() const { return beatDuration_; }
    void setBeatDuration(float d) { beatDuration_ = d; }
    float getAmplitude() const { return amplitude_; }
    void setAmplitude(float a) { amplitude_ = a; }
    float getPhaseOffset() const { return phaseOffset_; }
    void setPhaseOffset(float p) { phaseOffset_ = p; }
    CurveType getCurveType() const { return curveType_; }
    void setCurveType(CurveType t) { curveType_ = t; }
    bool isOneShot() const { return oneShot_; }
    void setOneShot(bool o) { oneShot_ = o; }
    bool isLooping() const { return looping_; }
    void setLooping(bool l) { looping_ = l; }

    // S168: false (default) = phase folds across FeatureSnapshot::totalBarCount
    // (never jumps backward on a structural reset); true = the original
    // S166-L5a behaviour, folding across FeatureSnapshot::barCount (jumps on
    // a real drop/breakdown transition). Runtime-only -- not currently
    // serialized, since no EnvelopeSignal field is (see s168 report).
    bool getResetPhaseOnStructural() const { return resetPhaseOnStructural_; }
    void setResetPhaseOnStructural(bool r) { resetPhaseOnStructural_ = r; }

private:
    std::vector<ControlPoint> points_;
    float beatDuration_;
    float amplitude_ = 1.0f;
    float phaseOffset_ = 0.0f;
    CurveType curveType_ = CurveType::Linear;
    bool oneShot_ = false;
    bool looping_ = true;
    bool resetPhaseOnStructural_ = false; // S168, default false = flow-through
};
