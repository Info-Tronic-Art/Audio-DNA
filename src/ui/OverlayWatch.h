#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

// s-rta-0928b idlepaint (Pitfall 57): the screen rects of every JUCE-drawn thing that can sit ABOVE a native-layer widget
// (NativeLayerHost) inside the main window. A native NSView is above ALL JUCE content of its window, so an in-peer
// overlay that crosses one must hand that widget back to JUCE painting while it is up.
// Tracked: the explicit overlays (addOverlay: the binding / MIDI-learn overlays), every child of `root` that passes
// isRootOverlay (a juce::TooltipWindow, or a child added after the baseline -- a ClipCell drag image), every child of the
// top-level window that passes isWindowOverlay (a PopupMenu shown withParentComponent(getTopLevelComponent()) and its
// shadow). Desktop windows (ComboBox popups, dialogs, other drag images) are above every native view and need nothing.
// Event-driven (ComponentListener): nothing runs at idle; clients hear overlaysChanged only when the list changed.
// Message thread only.
class OverlayWatch final : private juce::ComponentListener
{
public:
    struct Client
    {
        virtual ~Client() = default;
        virtual void overlaysChanged(const std::vector<juce::Rectangle<int>>& screenRects) = 0;
    };

    explicit OverlayWatch(juce::Component& root);   // takes the BASELINE of root's children now
    ~OverlayWatch() override;

    void addOverlay(juce::Component* c);            // explicit (full-window when active)
    void addClient(Client* c);
    void removeClient(Client* c);

    // The screen bounds of every tracked overlay that is visible up to the top, in tracking order. Computed fresh.
    std::vector<juce::Rectangle<int>> visibleOverlayScreenRects() const;

    // Pure rules (tests/test_overlay_watch.cpp).
    static bool isRootOverlay(const juce::Component& child,
                              const std::vector<juce::Component::SafePointer<juce::Component>>& baseline);
    static bool isWindowOverlay(const juce::Component& child, const juce::Component& content);
    static bool intersectsAny(juce::Rectangle<int> widgetScreen, const std::vector<juce::Rectangle<int>>& overlayScreens);

private:
    void componentChildrenChanged(juce::Component&) override;
    void componentParentHierarchyChanged(juce::Component&) override;
    void componentVisibilityChanged(juce::Component&) override;
    void componentMovedOrResized(juce::Component&, bool, bool) override;
    void componentBeingDeleted(juce::Component&) override;

    void attachToTopLevel();
    void rescan();                                  // rebuild the tracked set, then recompute
    void recompute();                               // notify clients when the rect list changed

    juce::Component& root_;
    juce::Component::SafePointer<juce::Component> topLevel_;
    std::vector<juce::Component::SafePointer<juce::Component>> baseline_;
    std::vector<juce::Component::SafePointer<juce::Component>> explicit_;
    std::vector<juce::Component::SafePointer<juce::Component>> tracked_;   // explicit_ + discovered, each listened to
    std::vector<Client*> clients_;
    std::vector<juce::Rectangle<int>> last_;

    JUCE_DECLARE_NON_COPYABLE(OverlayWatch)
};
