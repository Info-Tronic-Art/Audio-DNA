#include "OutputWindow.h"
#include "render/GLThreadQos.h"
#include <iostream>

using namespace juce::gl;

// ============================================================
// OutputWindow::Presenter -- one blit per refresh, never blocks
// ============================================================

void OutputWindow::Presenter::newOpenGLContextCreated()
{
    // JUCE set a swap interval of 1 just before this call (juce_OpenGLContext.cpp). All of the app's GL
    // contexts render on ONE shared thread: a blocking swap here would stall the main render. Each output
    // context is paced by its own display's display link instead.
    context_.setSwapInterval(0);
    raiseRenderThreadQos();   // s-rta-0929 vupload P2 / VU11: that shared thread at QoS USER_INTERACTIVE from every context
}

void OutputWindow::Presenter::renderOpenGL()
{
    GLint fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
    auto* component = context_.getTargetComponent();
    const double scale = context_.getRenderingScale();   // Retina: physical pixels
    const int w = component != nullptr ? juce::roundToInt(component->getWidth() * scale) : 0;
    const int h = component != nullptr ? juce::roundToInt(component->getHeight() * scale) : 0;
    output::presentSharedFrame(frames_, state_, static_cast<unsigned int>(fbo), w, h);
}

void OutputWindow::Presenter::openGLContextClosing()
{
    state_.release();   // this context's textures/FBOs and its retains on the shared surfaces
}

// ============================================================
// OutputWindow
// ============================================================

OutputWindow::OutputWindow(output::SharedFrameSet& frames)
    : DocumentWindow("Audio-DNA Output",
                     juce::Colours::black,
                     0), // No title bar buttons
      presenter_(frames, glContext_)
{
    setUsingNativeTitleBar(false);
    setTitleBarHeight(0);
    // No shadow window around a display-sized window. This also re-adds the window to the desktop with
    // getDesktopWindowStyleFlags() -- the override below -- so the peer is (re)created with
    // windowIgnoresKeyPresses before the window is ever shown.
    setDropShadowEnabled(false);
    // Checked in every build type (a jassert is a Release no-op): a window without the flag could take the
    // keyboard, so a peer that lacks it is logged and rebuilt with the override's flags.
    if (auto* peer = getPeer();
        peer == nullptr || (peer->getStyleFlags() & juce::ComponentPeer::windowIgnoresKeyPresses) == 0)
    {
        std::cerr << "[OutputWindow] the peer lacks windowIgnoresKeyPresses -- re-adding the window to the desktop"
                  << std::endl;
        addToDesktop();
    }
    jassert(getPeer() != nullptr
            && (getPeer()->getStyleFlags() & juce::ComponentPeer::windowIgnoresKeyPresses) != 0);

    // Add the output component directly as a child (not via content component,
    // which can leave gaps). We manage its bounds in resized().
    Component::addAndMakeVisible(outputComponent_);
    setWantsKeyboardFocus(false);

    glContext_.setOpenGLVersionRequired(juce::OpenGLContext::openGL4_1);
    glContext_.setRenderer(&presenter_);
    glContext_.setContinuousRepainting(true);   // display-link paced on macOS
    glContext_.setComponentPaintingEnabled(false);
    glContext_.attachTo(outputComponent_);
}

OutputWindow::~OutputWindow()
{
    detachGL();
}

int OutputWindow::getDesktopWindowStyleFlags() const
{
    // The output window can NEVER become the key window: TopLevelWindow::visibilityChanged() skips its
    // toFront(true) for this flag, and the mac peer's canBecomeKeyWindow() returns false. Clicking on it
    // changes nothing; every shortcut keeps going to the app.
    return DocumentWindow::getDesktopWindowStyleFlags() | juce::ComponentPeer::windowIgnoresKeyPresses;
}

void OutputWindow::detachGL()
{
    glContext_.detach();
}

void OutputWindow::closeButtonPressed()
{
    // Never hide-and-forget: the owner destroys the window.
    if (onCloseRequested)
        onCloseRequested();
}

void OutputWindow::parentSizeChanged()
{
    DocumentWindow::parentSizeChanged();
    if (onDisplaysChanged)
        onDisplaysChanged();
}

void OutputWindow::resized()
{
    DocumentWindow::resized();
    outputComponent_.setBounds(getLocalBounds());
}

void OutputWindow::openOnDisplay(const juce::Displays::Display& display)
{
    auto area = display.totalArea;

    // Cover the display with a borderless window at NORMAL window level.
    // Do NOT call setAlwaysOnTop(true): JUCE maps it to
    // NSFloatingWindowLevel and never sets an NSWindow Spaces-participation
    // bit, so macOS falls back to the level-derived default — a non-normal
    // level means Transient, i.e. the window floats onto EVERY desktop Space
    // as an opaque black overlay. At normal level the default is Managed, so
    // the window stays pinned to its own Space. Native macOS fullscreen is
    // still avoided — it creates a new Space and the GL context transition
    // can fail. Set bounds before showing: DocumentWindow's constrainer
    // floors a fresh window at 128x128 and the GL context only attaches
    // once the window is visible, so showing first would build the context
    // at 128x128 and immediately resize it — bounds-first attaches once,
    // already at display size.
    setBounds(area);
    setVisible(true);
    toFront(false);   // ordered front, never made key (windowIgnoresKeyPresses)

    // Ensure the output component fills the window
    outputComponent_.setBounds(getLocalBounds());
}
