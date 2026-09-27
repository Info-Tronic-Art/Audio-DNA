#pragma once

// OutputTargets: which connected display is which across a display change, and the machine's WANTED output set
// (s-rta-0927 outputs-c3 = plan5 slice C3, .harmony/.reports/s-rta-0926b/plan5-final.md sections 5 and 9).
// Pure: juce_core only -- no window, no GL, no Desktop -- unit-tested headless (tests/test_output_plan.cpp).
// OutputManager::reconcile() runs diffOutputs() on every display change; "Restore Last Outputs" runs matchDisplay()
// on the saved set; settings.json "outputs" is wantedToVar()/wantedFromVar().

#include <juce_core/juce_core.h>
#include <optional>
#include <vector>

namespace output
{
// A display as the output code identifies it: Displays::Display::totalArea (logical points), scale and isMain.
// JUCE's Display has no id and no name, so a display is recognised by this fingerprint.
struct DisplayInfo
{
    int x = 0, y = 0, w = 0, h = 0;
    double scale = 1.0;
    bool isMain = false;
};

inline bool operator==(const DisplayInfo& a, const DisplayInfo& b) noexcept
{
    return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h && a.scale == b.scale && a.isMain == b.isMain;
}

inline bool operator!=(const DisplayInfo& a, const DisplayInfo& b) noexcept { return !(a == b); }

// How a saved display is looked for among the current ones (the first rung that finds an unused display wins):
//   Reopen -- a target with no window (a hot-plug-interrupted one, or the saved set): exact (x,y,w,h,scale,main)
//             -> same (w,h,scale) at another position -> saved.isMain && current.isMain.
//   Track  -- a LIVE output: exact -> same (w,h,scale) moved -> the same display after a MODE change (same
//             top-left, other w/h/scale) -> main. A live output follows its display through a resolution/scale
//             change; a closed one never reopens on a display it only shares a position with.
enum class MatchRule { Reopen, Track };

// The index of `current` that is `saved`, or nullopt. `used[i]` (may be shorter than `current`) excludes a display
// already matched: with two identical displays, position wins, then the first unused one.
std::optional<int> matchDisplay(const DisplayInfo& saved, const std::vector<DisplayInfo>& current,
                                const std::vector<bool>& used, MatchRule rule = MatchRule::Reopen);

// What one reconcile does. `from` indexes `live` (toClose, toRebound) or `interrupted` (toOpen); `display` indexes
// `current`.
struct OutputDiff
{
    struct Pair { int from = 0; int display = 0; };
    std::vector<int> toClose;       // a live output whose display is gone: close the window, keep its target
    std::vector<Pair> toRebound;    // a live output whose display changed (moved / new mode / main flag): follow it
    std::vector<Pair> toOpen;       // an interrupted target whose display is back (plan5 Q6): open it again
    bool empty() const { return toClose.empty() && toRebound.empty() && toOpen.empty(); }
};

// live = the live outputs' targets, interrupted = targets closed by a hot-plug, current = the display list now,
// previous = the display list the last reconcile saw. Every live output first keeps an UNCHANGED display (exact);
// only then are the loose rungs tried, and only against displays that are new or changed since `previous` -- a
// display that did not change was not "the one that moved", so an output never jumps onto an untouched display
// and an interrupted target never reopens on one just because some other display changed. A live output is
// matched with MatchRule::Track, an interrupted target with MatchRule::Reopen, after every live output (a display
// that already shows an output is never opened twice). Never opens anything but an interrupted target.
OutputDiff diffOutputs(const std::vector<DisplayInfo>& live, const std::vector<DisplayInfo>& interrupted,
                       const std::vector<DisplayInfo>& current, const std::vector<DisplayInfo>& previous);

// The same set of targets, in any order (the settings file is rewritten only when this is false).
bool sameTargets(const std::vector<DisplayInfo>& a, const std::vector<DisplayInfo>& b);

// The WANTED set settings.json "outputs" holds: every live target, every interrupted one, then every saved target
// Restore Last Outputs has not opened yet -- each once. The file's key is replaced whole on a write, so a saved target
// left out here would be lost from disk by a partial Restore, an unrelated output change or All Outputs Off.
std::vector<DisplayInfo> wantedSet(const std::vector<DisplayInfo>& live, const std::vector<DisplayInfo>& interrupted,
                                   const std::vector<DisplayInfo>& saved);

// settings.json "outputs": {"version": 1, "targets": [{"x", "y", "w", "h", "scale", "main"}, ...]}.
juce::var wantedToVar(const std::vector<DisplayInfo>& targets);
// Unknown keys are ignored; an entry without a positive w and h is skipped; anything else reads as no targets.
std::vector<DisplayInfo> wantedFromVar(const juce::var& v);
} // namespace output
