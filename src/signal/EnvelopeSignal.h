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

        // Calculate phase from beat position
        float totalBeatPhase = snapshot.beatPhase + static_cast<float>(snapshot.beatInBar);
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

private:
    std::vector<ControlPoint> points_;
    float beatDuration_;
    float amplitude_ = 1.0f;
    float phaseOffset_ = 0.0f;
    CurveType curveType_ = CurveType::Linear;
    bool oneShot_ = false;
    bool looping_ = true;
};
