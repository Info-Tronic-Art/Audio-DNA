#include "output/OutputTargets.h"

namespace output
{
namespace
{
enum class Rung { Exact, SameSize, SameOrigin, Main };

bool fits(Rung rung, const DisplayInfo& saved, const DisplayInfo& c)
{
    switch (rung)
    {
        case Rung::Exact:      return saved == c;
        case Rung::SameSize:   return saved.w == c.w && saved.h == c.h && saved.scale == c.scale;
        case Rung::SameOrigin: return saved.x == c.x && saved.y == c.y;
        case Rung::Main:       return saved.isMain && c.isMain;
    }
    return false;
}

const std::vector<Rung>& rungs(MatchRule rule)
{
    static const std::vector<Rung> reopen { Rung::Exact, Rung::SameSize, Rung::Main };
    static const std::vector<Rung> track { Rung::Exact, Rung::SameSize, Rung::SameOrigin, Rung::Main };
    return rule == MatchRule::Track ? track : reopen;
}

bool blocked(const std::vector<bool>& mask, size_t i) { return i < mask.size() && mask[i]; }

std::optional<int> firstFit(Rung rung, const DisplayInfo& saved, const std::vector<DisplayInfo>& current,
                            const std::vector<bool>& excluded)
{
    for (size_t i = 0; i < current.size(); ++i)
        if (!blocked(excluded, i) && fits(rung, saved, current[i]))
            return static_cast<int>(i);
    return std::nullopt;
}

bool contains(const std::vector<DisplayInfo>& v, const DisplayInfo& d)
{
    for (const auto& x : v)
        if (x == d)
            return true;
    return false;
}
} // namespace

std::optional<int> matchDisplay(const DisplayInfo& saved, const std::vector<DisplayInfo>& current,
                                const std::vector<bool>& used, MatchRule rule)
{
    for (const Rung rung : rungs(rule))
        if (const auto i = firstFit(rung, saved, current, used))
            return i;
    return std::nullopt;
}

OutputDiff diffOutputs(const std::vector<DisplayInfo>& live, const std::vector<DisplayInfo>& interrupted,
                       const std::vector<DisplayInfo>& current, const std::vector<DisplayInfo>& previous)
{
    OutputDiff diff;
    std::vector<bool> used(current.size(), false);

    // Loose rungs only ever consider a display that is new or changed since the last reconcile.
    std::vector<bool> unchanged(current.size(), false);
    for (size_t i = 0; i < current.size(); ++i)
        unchanged[i] = contains(previous, current[i]);

    auto matchAll = [&](const std::vector<DisplayInfo>& targets, MatchRule rule, std::vector<int>& matchOf)
    {
        matchOf.assign(targets.size(), -1);
        for (const Rung rung : rungs(rule))   // rung by rung over ALL targets: an exact match is never stolen
        {
            for (size_t t = 0; t < targets.size(); ++t)
            {
                if (matchOf[t] >= 0)
                    continue;
                std::vector<bool> excluded = used;
                if (rung != Rung::Exact)
                    for (size_t i = 0; i < current.size(); ++i)
                        excluded[i] = excluded[i] || unchanged[i];
                if (const auto i = firstFit(rung, targets[t], current, excluded))
                {
                    matchOf[t] = *i;
                    used[static_cast<size_t>(*i)] = true;
                }
            }
        }
    };

    std::vector<int> liveMatch, interruptedMatch;
    matchAll(live, MatchRule::Track, liveMatch);
    for (size_t t = 0; t < live.size(); ++t)
    {
        if (liveMatch[t] < 0)
            diff.toClose.push_back(static_cast<int>(t));
        else if (current[static_cast<size_t>(liveMatch[t])] != live[t])
            diff.toRebound.push_back({ static_cast<int>(t), liveMatch[t] });
    }

    matchAll(interrupted, MatchRule::Reopen, interruptedMatch);
    for (size_t t = 0; t < interrupted.size(); ++t)
        if (interruptedMatch[t] >= 0)
            diff.toOpen.push_back({ static_cast<int>(t), interruptedMatch[t] });
    return diff;
}

bool sameTargets(const std::vector<DisplayInfo>& a, const std::vector<DisplayInfo>& b)
{
    if (a.size() != b.size())
        return false;
    std::vector<bool> taken(b.size(), false);
    for (const auto& x : a)
    {
        bool found = false;
        for (size_t i = 0; i < b.size() && !found; ++i)
            if (!taken[i] && b[i] == x)
                taken[i] = found = true;
        if (!found)
            return false;
    }
    return true;
}

juce::var wantedToVar(const std::vector<DisplayInfo>& targets)
{
    juce::Array<juce::var> arr;
    for (const auto& t : targets)
    {
        auto* o = new juce::DynamicObject();
        o->setProperty("x", t.x);
        o->setProperty("y", t.y);
        o->setProperty("w", t.w);
        o->setProperty("h", t.h);
        o->setProperty("scale", t.scale);
        o->setProperty("main", t.isMain);
        arr.add(juce::var(o));
    }
    auto* root = new juce::DynamicObject();
    root->setProperty("version", 1);
    root->setProperty("targets", juce::var(arr));
    return juce::var(root);
}

std::vector<DisplayInfo> wantedFromVar(const juce::var& v)
{
    std::vector<DisplayInfo> out;
    const auto* obj = v.getDynamicObject();
    if (obj == nullptr)
        return out;
    const auto* arr = obj->getProperty("targets").getArray();
    if (arr == nullptr)
        return out;
    for (const auto& e : *arr)
    {
        const auto* t = e.getDynamicObject();
        if (t == nullptr)
            continue;
        DisplayInfo d;
        d.x = static_cast<int>(t->getProperty("x"));
        d.y = static_cast<int>(t->getProperty("y"));
        d.w = static_cast<int>(t->getProperty("w"));
        d.h = static_cast<int>(t->getProperty("h"));
        d.scale = t->hasProperty("scale") ? static_cast<double>(t->getProperty("scale")) : 1.0;
        d.isMain = static_cast<bool>(t->getProperty("main"));
        if (d.w > 0 && d.h > 0 && d.scale > 0.0)
            out.push_back(d);
    }
    return out;
}
} // namespace output
