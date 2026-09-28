#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>
#include "output/SharedFrameSet.h"
#include "output/OutputPresenter.h"

// OutputWindow: a borderless window covering one display that shows the COMPOSITION -- the main
// Renderer's canvas, copied once per frame into shared IOSurface frames (output::SharedFrameSet), which
// this window's own GL context presents letter/pillar-boxed (output::presentSharedFrame). No shader, no
// effect chain, no GL object shared with any other context (s-rta-0927 outputs-c1 = plan5 slice C1,
// .harmony/.reports/s-rta-0926b/plan5-final.md 8.3).
//
// The screen-safety law (asserted in source by tests/test_output_law.cpp):
//   - NORMAL window level: never always-on-top (that is the black-overlay-on-every-Space bug), never kiosk,
//     never native fullscreen;
//   - it can NEVER become the key window (ComponentPeer::windowIgnoresKeyPresses): the keyboard never
//     leaves the app, even while this window covers it -- there is no key handling here at all;
//   - bounds are set before it is shown.
class OutputWindow : public juce::DocumentWindow
{
public:
    explicit OutputWindow(output::SharedFrameSet& frames);
    ~OutputWindow() override;

    // The window's peer is created with windowIgnoresKeyPresses (plus DocumentWindow's own flags).
    int getDesktopWindowStyleFlags() const override;

    void closeButtonPressed() override;
    void resized() override;

    // Cover `display` (its totalArea) and show the window, without taking focus.
    void openOnDisplay(const juce::Displays::Display& display);

    // Stop this window's GL rendering (the shutdown law); idempotent. The destructor calls it too.
    void detachGL();

    // Asked to close (by the OS or a future UI): the owner destroys the window.
    std::function<void()> onCloseRequested;

    // The display list changed: JUCE calls parentSizeChanged() on every window after Displays::refresh() (the
    // screen-change notification). The owner only SCHEDULES its reconcile here -- a window is never destroyed
    // inside one of its own callbacks (plan5 C3 section 9, s-rta-0927 outputs-c3).
    void parentSizeChanged() override;
    std::function<void()> onDisplaysChanged;

private:
    // Presents the shared frames: ONE blit per refresh of this window's display (display-link paced).
    class Presenter : public juce::OpenGLRenderer
    {
    public:
        Presenter(output::SharedFrameSet& frames, juce::OpenGLContext& context) : frames_(frames), context_(context) {}
        void newOpenGLContextCreated() override;
        void renderOpenGL() override;
        void openGLContextClosing() override;

    private:
        output::SharedFrameSet& frames_;
        juce::OpenGLContext& context_;
        output::PresenterGLState state_;
    };

    // The GL rendering surface
    class OutputComponent : public juce::Component
    {
    public:
        OutputComponent() = default;
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colours::black);
        }
    };

    OutputComponent outputComponent_;
    juce::OpenGLContext glContext_;
    Presenter presenter_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OutputWindow)
};
