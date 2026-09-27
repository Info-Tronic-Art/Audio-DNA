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
// The app never opens an output by itself: only the Output menu, the TopBar button and Cmd+F call openDisplay.

#include <juce_gui_basics/juce_gui_basics.h>
#include "output/OutputMenuModel.h"
#include "output/SharedFrameSet.h"
#include "ui/OutputWindow.h"
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

    // Destroys every window synchronously (the shutdown law); idempotent; the destructor calls it too.
    void shutdown();

    std::function<void(int liveCount)> onLiveCountChanged;

    static DisplayInfo toDisplayInfo(const juce::Displays::Display& d);

private:
    struct Live
    {
        std::unique_ptr<OutputWindow> window;
        DisplayInfo target;   // the display it was opened on, matched exactly against the current display list
    };

    static std::vector<DisplayInfo> currentDisplays();
    std::vector<bool> liveFlags(const std::vector<DisplayInfo>& displays) const;
    void closeWindow(const OutputWindow* window);        // the window's own close request
    void closeLive(size_t index);                        // hide + deferred destroy, no notification
    void destroyLater(std::unique_ptr<OutputWindow> window);
    void changed();                                      // live count -> renderer, state, onLiveCountChanged
    void rebuildState();
    void handleAsyncUpdate() override;                   // drains the graveyard

    SharedFrameSet& frames_;
    Renderer& renderer_;
    std::vector<Live> live_;
    std::vector<std::unique_ptr<OutputWindow>> graveyard_;
    bool shutDown_ = false;

    mutable std::mutex stateMutex_;   // UI/HTTP only -- never on a GL frame path
    juce::var stateSnapshot_;

    JUCE_DECLARE_NON_COPYABLE(OutputManager)
};
} // namespace output
