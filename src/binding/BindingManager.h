#pragma once
#include "binding/Binding.h"
#include <vector>
#include <functional>
#include <unordered_map>

// BindingManager: stores all bindings and processes input events.
// When an input matches a binding, the associated action callback is invoked.
class BindingManager
{
public:
    BindingManager() = default;

    // Add a binding. Returns its ID.
    uint32_t addBinding(const Binding& binding);

    // Remove a binding by ID.
    bool removeBinding(uint32_t bindingId);

    // Get binding by ID.
    Binding* getBinding(uint32_t bindingId);
    const Binding* getBinding(uint32_t bindingId) const;

    // Get all bindings
    int getNumBindings() const { return static_cast<int>(bindings_.size()); }
    Binding* getBindingAt(int index);

    // Clear all bindings
    void clearAll();

    // Process keyboard input. Returns true if a binding was triggered.
    bool processKeyDown(int keyCode, bool shift, bool cmd, bool alt);
    bool processKeyUp(int keyCode, bool shift, bool cmd, bool alt);

    // Process MIDI input.
    bool processMidiNoteOn(int channel, int note, int velocity);
    bool processMidiNoteOff(int channel, int note);
    bool processMidiCC(int channel, int cc, int value);

    // Action callback: called when a binding is triggered.
    // Parameters: binding, triggerValue (1.0 for note-on/key-down, 0.0 for off, 0-1 for CC)
    using ActionCallback = std::function<void(const Binding& binding, float value)>;
    void setActionCallback(ActionCallback callback) { actionCallback_ = std::move(callback); }

    // Binding mode: when active, next input creates a binding instead of triggering an action.
    bool isBindingMode() const { return bindingMode_; }
    void setBindingMode(bool enabled) { bindingMode_ = enabled; }

    // In binding mode, this callback is called with the input event details.
    using BindingCaptureCallback = std::function<void(Binding::InputType type, int keyOrNote, int cc,
                                                       bool shift, bool cmd, bool alt)>;
    void setBindingCaptureCallback(BindingCaptureCallback cb) { captureCallback_ = std::move(cb); }

    // Track relative CC accumulated values (for Relative mode encoders)
    float getRelativeCCValue(int channel, int cc) const;

private:
    std::vector<Binding> bindings_;
    uint32_t nextId_ = 1;
    bool bindingMode_ = false;
    ActionCallback actionCallback_;
    BindingCaptureCallback captureCallback_;

    // Relative CC accumulated values: key = (channel << 8) | cc
    std::unordered_map<int, float> relativeCCValues_;
};
