#include "recording/TempoMap.h"
#include <algorithm>

juce::var TempoAnchor::toVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("t", t);
    obj->setProperty("beat", beat);
    obj->setProperty("sample", static_cast<juce::int64>(sample));
    obj->setProperty("bpm", static_cast<double>(bpm));
    obj->setProperty("why", juce::String(why));
    return juce::var(obj);
}

TempoAnchor TempoAnchor::fromVar(const juce::var& v)
{
    TempoAnchor anc;
    if (auto* obj = v.getDynamicObject())
    {
        anc.t = static_cast<double>(obj->getProperty("t"));
        anc.beat = static_cast<double>(obj->getProperty("beat"));
        anc.sample = static_cast<uint64_t>(static_cast<juce::int64>(obj->getProperty("sample")));
        anc.bpm = static_cast<float>(static_cast<double>(obj->getProperty("bpm")));
        anc.why = obj->getProperty("why").toString().toStdString();
    }
    return anc;
}

void TempoMap::append(TempoAnchor anchor)
{
    auto it = std::upper_bound(a.begin(), a.end(), anchor,
        [](const TempoAnchor& x, const TempoAnchor& y) { return x.t < y.t; });
    a.insert(it, std::move(anchor));
}

namespace
{
    // Index of the last anchor with anchor.t <= t (or 0 if t is before the
    // first anchor -- extrapolation then uses the first segment's slope).
    size_t bracketByTime(const std::vector<TempoAnchor>& a, double t)
    {
        size_t lo = 0;
        for (size_t i = 0; i < a.size(); ++i)
        {
            if (a[i].t <= t) lo = i;
            else break;
        }
        return lo;
    }

    size_t bracketByBeat(const std::vector<TempoAnchor>& a, double beat)
    {
        size_t lo = 0;
        for (size_t i = 0; i < a.size(); ++i)
        {
            if (a[i].beat <= beat) lo = i;
            else break;
        }
        return lo;
    }
}

double TempoMap::beatAt(double t) const
{
    if (a.empty()) return 0.0;
    const size_t lo = bracketByTime(a, t);
    const TempoAnchor& anc = a[lo];
    if (anc.bpm <= 0.0f) return anc.beat;   // unmetered segment: beat does not advance (D1)
    return anc.beat + (t - anc.t) * (static_cast<double>(anc.bpm) / 60.0);
}

double TempoMap::tAt(double beat) const
{
    if (a.empty()) return 0.0;
    const size_t lo = bracketByBeat(a, beat);
    const TempoAnchor& anc = a[lo];
    if (anc.bpm <= 0.0f) return anc.t;      // not invertible inside an unmetered segment
    return anc.t + (beat - anc.beat) * (60.0 / static_cast<double>(anc.bpm));
}

uint64_t TempoMap::sampleAt(double t) const
{
    if (a.empty()) return 0;
    const size_t lo = bracketByTime(a, t);
    const TempoAnchor& anc = a[lo];

    // Rate from the segment this anchor starts (anc -> next), or from the
    // PRECEDING segment if this is the last anchor -- holds the last known
    // rate rather than freezing to a flat line beyond the take's end.
    double rate = 0.0;
    if (lo + 1 < a.size())
    {
        const TempoAnchor& next = a[lo + 1];
        const double dt = next.t - anc.t;
        if (dt > 1e-9)
            rate = (static_cast<double>(next.sample) - static_cast<double>(anc.sample)) / dt;
    }
    else if (lo > 0)
    {
        const TempoAnchor& prev = a[lo - 1];
        const double dt = anc.t - prev.t;
        if (dt > 1e-9)
            rate = (static_cast<double>(anc.sample) - static_cast<double>(prev.sample)) / dt;
    }

    const double sample = static_cast<double>(anc.sample) + (t - anc.t) * rate;
    return sample > 0.0 ? static_cast<uint64_t>(sample) : 0;
}

juce::var TempoMap::toVar() const
{
    juce::Array<juce::var> arr;
    for (const auto& anc : a)
        arr.add(anc.toVar());
    return arr;
}

TempoMap TempoMap::fromVar(const juce::var& v)
{
    TempoMap m;
    if (auto* arr = v.getArray())
        for (const auto& e : *arr)
            m.a.push_back(TempoAnchor::fromVar(e));
    return m;
}
