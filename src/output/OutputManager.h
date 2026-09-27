#pragma once

// OutputManager: the output windows, one per display, as many as the machine has connected -- the main screen
// included (s-rta-0927 outputs-c2 = plan5 slice C2, .harmony/.reports/s-rta-0926b/plan5-final.md 5, 6, 8.4).
// Message thread only. The TRUTH is the live windows: the Output menu, the TopBar "Outputs" button and
// /api/state.outputs.displays are views of them (populateMenu / stateVar, both built from buildOutputMenu), so
// the two doors cannot disagree and nothing else holds a second "which output is on" state.
//
// Each window is an OutputWindow (normal level, never key, presents output::SharedFrameSet -- Pitfall 40); the
// manager only ever shows one through OutputWindow::openOnDisplay (bounds before visible). A closed window is
// hidden at once and destroyed on the next message-loop turn (never inside one of its own callbacks).
//
// Hot-plug (s-rta-0927 outputs-c3 = plan5 C3, section 9): reconcile() -- ONE idempotent function -- matches the
// live outputs to the display list (output::diffOutputs): a vanished display closes its output and keeps its target
// as INTERRUPTED; a moved display / a display in a new mode keeps its output (the window follows); an interrupted
// target whose display is back reopens by itself (Q6). Two triggers: every live window's parentSizeChanged()
// (JUCE's screen-change dispatch) schedules it for the next message-loop turn, and pollDisplays() -- called on EVERY
// 30 Hz UI timer tick -- runs it when the display list differs from the last one seen (a cached read; no window
// needed). After every reconcile the menu ticks, the TopBar count and /api/state.outputs(.displays) are rebuilt.
//
// The WANTED set (live + interrupted + the saved targets not yet restored, output::wantedSet) is saved in
// settings.json "outputs" through AppSettings, on a change only, never per tick: a manual open or close decides for
// its display (the target leaves the saved set; a close also leaves the wanted set), a hot-plug close keeps it, and a
// saved target Restore has not opened stays in the file until it is restored or decided for; quitting writes
// nothing. The app NEVER opens an output by itself at launch (Q1): the saved set is only loaded (SAVED targets), and
// only the "Restore Last Outputs" menu action opens them. The only openers are the Output menu, the TopBar button,
// Cmd+F, Restore Last Outputs, and a hot-plug reopen of a target that was live this session.

#include <juce_gui_basics/juce_gui_basics.h>
#include "output/OutputMenuModel.h"
#include "output/OutputTargets.h"
#include "output/SharedFrameSet.h"
#include "ui/OutputWindow.h"
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

class Renderer;

namespace output
{
class OutputManager : private juce::AsyncUpdater
{
public:
    OutputManager(SharedFrameSet& frames, Renderer& renderer);
    ~OutputManager() override;

    // displayIndex = the index into juce::Desktop::getInstance().getDisplays().displays (out of range: no-op).
    void toggleDisplay(int displayIndex);
    void openDisplay(int displayIndex);
    void closeDisplay(int displayIndex);
    void closeAll();   // every live window, whichever display it is on

    bool isLive(int displayIndex) const;
    int liveCount() const { return static_cast<int>(live_.size()); }
    int mainDisplayIndex() const;   // the isMain display (Cmd+F); 0 if none is flagged; -1 with no display

    // The ONE item list (Output menu + TopBar button): one tickable item per display, then "All Outputs Off".
    void populateMenu(juce::PopupMenu& menu) const;

    // /api/state.outputs.displays: [{index, x, y, w, h, scale, main, live, label}], rebuilt on the message
    // thread after every change; HTTP threads read a copy (never Desktop::getDisplays() off the message thread).
    juce::var stateVar() const;

    // Destroys every window synchronously (the shutdown law); idempotent; the destructor calls it too. Writes no
    // settings: quitting with outputs on keeps them as the set "Restore Last Outputs" brings back.
    void shutdown();

    // ---- plan5 C3: hot-plug, persistence, restore ----
    // Reads the saved set from `settingsFile` (settings.json "outputs") as SAVED targets and remembers the file for
    // later writes. Opens NOTHING (Q1). Call once, at startup, before any output is opened.
    void attachSettings(const juce::File& settingsFile);
    // The "Restore Last Outputs" menu action (the ONLY caller): opens every saved target whose display is connected
    // and has no output yet (matchDisplay, MatchRule::Reopen); the others stay saved.
    void restoreLast();
    // How many outputs restoreLast() would open now (the menu item is enabled iff > 0).
    int restorableCount() const;
    // Every UI timer tick (30 Hz): reconcile iff the display list differs from the last one seen. No allocation and
    // no work beyond that comparison while nothing changes.
    void pollDisplays();
    // Coalesced: reconcile on the next message-loop turn (the OutputWindow::parentSizeChanged hook).
    void scheduleReconcile();
    // Idempotent: match the live outputs to the current displays, close / follow / reopen (see the header comment).
    void reconcile();

    // TEST MODE (8080 only): counters for the probes -- {poll_enabled, poll_ticks, reconciles, settings_writes,
    // restore_calls, interrupted, saved, restorable}. Atomics: readable from an HTTP thread.
    juce::var statsVar() const;
    void setPollEnabled(bool enabled);   // TEST MODE (8080 set_output_poll): the poll A/B of probe-outputs o_poll_idle

    std::function<void(int liveCount)> onLiveCountChanged;

    static DisplayInfo toDisplayInfo(const juce::Displays::Display& d);

private:
    struct Live
    {
        std::unique_ptr<OutputWindow> window;
        DisplayInfo target;   // the display it is on, matched exactly against the current display list (updated
                              // by every reconcile when that display moves or changes mode)
    };

    static std::vector<DisplayInfo> currentDisplays();
    std::vector<bool> liveFlags(const std::vector<DisplayInfo>& displays) const;
    std::vector<OutputDiff::Pair> restorePlan() const;   // {saved index, display index} restoreLast() would open
    bool openWindow(int displayIndex);                   // a window on that display (if none yet); no notification
    void closeWindow(const OutputWindow* window);        // the window's own close request
    void closeLive(size_t index);                        // hide + deferred destroy, no notification
    void destroyLater(std::unique_ptr<OutputWindow> window);
    void forgetInterrupted(const DisplayInfo& target);   // a manual open/close decides for that display
    void forgetSaved(const DisplayInfo& target);         // ditto, for the saved set
    bool reconcileIfDisplaysChanged();                   // the poll's comparison; true if it reconciled
    void changed();                                      // live count -> renderer, state, onLiveCountChanged
    void rebuildState();
    void persistWanted();                                // settings.json "outputs" iff the wanted set changed
    void handleAsyncUpdate() override;                   // drains the graveyard; runs a scheduled reconcile

    SharedFrameSet& frames_;
    Renderer& renderer_;
    std::vector<Live> live_;
    std::vector<std::unique_ptr<OutputWindow>> graveyard_;
    bool shutDown_ = false;

    std::vector<DisplayInfo> lastSeen_;       // the display list the last reconcile (or the constructor) saw
    std::vector<DisplayInfo> interrupted_;    // closed by a hot-plug this session; reopen when their display returns
    std::vector<DisplayInfo> saved_;          // the saved set loaded at startup; opened only by restoreLast()
    std::vector<DisplayInfo> lastWanted_;     // the wanted set as last written (attachSettings: the loaded set)
    juce::File settingsFile_;                 // empty until attachSettings(): nothing is written
    bool reconcilePending_ = false;

    std::atomic<bool> pollEnabled_ { true };
    std::atomic<juce::int64> pollTicks_ { 0 }, reconciles_ { 0 }, settingsWrites_ { 0 }, restoreCalls_ { 0 };
    std::atomic<int> interruptedCount_ { 0 }, savedCount_ { 0 }, restorable_ { 0 };

    mutable std::mutex stateMutex_;   // UI/HTTP only -- never on a GL frame path
    juce::var stateSnapshot_;

    JUCE_DECLARE_NON_COPYABLE(OutputManager)
};
} // namespace output
