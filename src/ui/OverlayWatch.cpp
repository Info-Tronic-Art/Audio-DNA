#include "ui/OverlayWatch.h"
#include <algorithm>

namespace
{
bool contains(const std::vector<juce::Component::SafePointer<juce::Component>>& v, const juce::Component* c)
{
    return std::any_of(v.begin(), v.end(), [c](const auto& p) { return p.getComponent() == c; });
}

// isShowing() without the peer test: the component and every parent up to the top are visible. In the app the top is
// the (visible) main window, so this is isShowing(); headless tests have no peer.
bool visibleToTop(const juce::Component& c)
{
    for (auto* p = &c; p != nullptr; p = p->getParentComponent())
        if (!p->isVisible())
            return false;
    return true;
}
}

OverlayWatch::OverlayWatch(juce::Component& root) : root_(root)
{
    for (auto* child : root_.getChildren())
        baseline_.emplace_back(child);
    root_.addComponentListener(this);
    attachToTopLevel();
    rescan();
}

OverlayWatch::~OverlayWatch()
{
    for (auto& t : tracked_)
        if (auto* c = t.getComponent())
            c->removeComponentListener(this);
    if (auto* top = topLevel_.getComponent(); top != nullptr && top != &root_)
        top->removeComponentListener(this);
    root_.removeComponentListener(this);
}

void OverlayWatch::addOverlay(juce::Component* c)
{
    if (c == nullptr || contains(explicit_, c))
        return;
    explicit_.emplace_back(c);
    rescan();
}

void OverlayWatch::addClient(Client* c)
{
    if (c != nullptr && std::find(clients_.begin(), clients_.end(), c) == clients_.end())
        clients_.push_back(c);
}

void OverlayWatch::removeClient(Client* c)
{
    clients_.erase(std::remove(clients_.begin(), clients_.end(), c), clients_.end());
}

std::vector<juce::Rectangle<int>> OverlayWatch::visibleOverlayScreenRects() const
{
    // Still inside our window: JUCE sends no componentChildrenChanged when a child that is not showing is removed, so
    // a tracked overlay may have left without a rescan.
    const auto* top = root_.getTopLevelComponent();
    std::vector<juce::Rectangle<int>> r;
    for (const auto& t : tracked_)
        if (auto* c = t.getComponent(); c != nullptr && c->getTopLevelComponent() == top && visibleToTop(*c)
                                        && !c->getBounds().isEmpty())
            r.push_back(c->getScreenBounds());
    return r;
}

bool OverlayWatch::isRootOverlay(const juce::Component& child,
                                 const std::vector<juce::Component::SafePointer<juce::Component>>& baseline)
{
    if (dynamic_cast<const juce::TooltipWindow*>(&child) != nullptr)
        return true;
    return !contains(baseline, &child);
}

bool OverlayWatch::isWindowOverlay(const juce::Component& child, const juce::Component& content)
{
    if (&child == &content)
        return false;
    return dynamic_cast<const juce::ResizableCornerComponent*>(&child) == nullptr
        && dynamic_cast<const juce::ResizableBorderComponent*>(&child) == nullptr;
}

bool OverlayWatch::intersectsAny(juce::Rectangle<int> widgetScreen, const std::vector<juce::Rectangle<int>>& overlayScreens)
{
    return std::any_of(overlayScreens.begin(), overlayScreens.end(),
                       [widgetScreen](const juce::Rectangle<int>& r) { return r.intersects(widgetScreen); });
}

void OverlayWatch::componentChildrenChanged(juce::Component& c)
{
    if (&c == &root_ || &c == topLevel_.getComponent())
        rescan();
}

void OverlayWatch::componentParentHierarchyChanged(juce::Component& c)
{
    if (&c == &root_)
    {
        attachToTopLevel();
        rescan();
    }
}

void OverlayWatch::componentVisibilityChanged(juce::Component&) { recompute(); }
void OverlayWatch::componentMovedOrResized(juce::Component&, bool, bool) { recompute(); }

void OverlayWatch::componentBeingDeleted(juce::Component& c)
{
    c.removeComponentListener(this);
    if (&c == topLevel_.getComponent())
        topLevel_ = nullptr;
    tracked_.erase(std::remove_if(tracked_.begin(), tracked_.end(),
                                  [&c](const auto& p) { return p.getComponent() == &c || p.getComponent() == nullptr; }),
                   tracked_.end());
    recompute();
}

void OverlayWatch::attachToTopLevel()
{
    auto* top = root_.getTopLevelComponent();
    if (top == topLevel_.getComponent())
        return;
    if (auto* old = topLevel_.getComponent(); old != nullptr && old != &root_)
        old->removeComponentListener(this);
    topLevel_ = top;
    if (top != nullptr && top != &root_)
        top->addComponentListener(this);
}

void OverlayWatch::rescan()
{
    std::vector<juce::Component::SafePointer<juce::Component>> next;
    for (auto& e : explicit_)
        if (e != nullptr && !contains(next, e.getComponent()))
            next.push_back(e);
    for (auto* child : root_.getChildren())
        if (isRootOverlay(*child, baseline_) && !contains(next, child))
            next.emplace_back(child);
    if (auto* top = topLevel_.getComponent(); top != nullptr && top != &root_)
        for (auto* child : top->getChildren())
            if (isWindowOverlay(*child, root_) && !contains(next, child))
                next.emplace_back(child);

    for (auto& t : tracked_)
        if (auto* c = t.getComponent(); c != nullptr && !contains(next, c))
            c->removeComponentListener(this);
    for (auto& n : next)
        if (auto* c = n.getComponent(); c != nullptr && !contains(tracked_, c))
            c->addComponentListener(this);
    tracked_ = std::move(next);
    recompute();
}

void OverlayWatch::recompute()
{
    auto now = visibleOverlayScreenRects();
    if (now == last_)
        return;
    last_ = std::move(now);
    const auto clients = clients_;   // a client may remove itself
    for (auto* cl : clients)
        cl->overlaysChanged(last_);
}
