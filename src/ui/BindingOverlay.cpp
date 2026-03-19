#include "BindingOverlay.h"

BindingOverlay::BindingOverlay(BindingManager& bindingManager, Composition& composition)
    : bindingManager_(bindingManager), composition_(composition)
{
    setInterceptsMouseClicks(true, false);
    setWantsKeyboardFocus(true);
}

BindingOverlay::~BindingOverlay()
{
    if (auto* topLevel = getTopLevelComponent())
        topLevel->removeKeyListener(this);
}

void BindingOverlay::enterBindingMode()
{
    active_ = true;
    waitingForKey_ = false;
    selectedTargetIndex_ = -1;
    bindingManager_.setBindingMode(true);
    setVisible(true);
    toFront(true);
    grabKeyboardFocus();

    if (auto* topLevel = getTopLevelComponent())
        topLevel->addKeyListener(this);

    repaint();
}

void BindingOverlay::exitBindingMode()
{
    active_ = false;
    waitingForKey_ = false;
    selectedTargetIndex_ = -1;
    bindingManager_.setBindingMode(false);
    setVisible(false);

    if (auto* topLevel = getTopLevelComponent())
        topLevel->removeKeyListener(this);

    if (onBindingModeExit)
        onBindingModeExit();
}

void BindingOverlay::setBindableTargets(const std::vector<BindableTarget>& targets)
{
    targets_ = targets;
    if (active_)
        repaint();
}

void BindingOverlay::paint(juce::Graphics& g)
{
    if (!active_) return;

    // Semi-transparent dark overlay
    g.setColour(juce::Colour(0xcc000000));
    g.fillRect(getLocalBounds());

    // Title bar
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    g.setFont(juce::Font(juce::FontOptions(18.0f).withStyle("Bold")));

    juce::String title = waitingForKey_
        ? "Press a key to bind..."
        : "Keyboard Binding Mode — Click a target, then press a key";
    g.drawText(title, getLocalBounds().removeFromTop(40), juce::Justification::centred);

    // Escape hint
    g.setFont(juce::Font(juce::FontOptions(12.0f)));
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.drawText("Press Escape to exit", getLocalBounds().removeFromTop(60).removeFromBottom(18),
               juce::Justification::centred);

    // Draw bindable targets
    for (int i = 0; i < static_cast<int>(targets_.size()); ++i)
    {
        const auto& t = targets_[static_cast<size_t>(i)];
        bool isSelected = (i == selectedTargetIndex_);

        // Target box
        auto r = t.bounds;
        if (isSelected)
        {
            g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.4f));
            g.fillRect(r);
            g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
            g.drawRect(r, 2);
        }
        else
        {
            g.setColour(juce::Colour(AudioDNALookAndFeel::kSurfaceLight).withAlpha(0.5f));
            g.fillRect(r);
            g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.6f));
            g.drawRect(r, 1);
        }

        // Label
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.drawText(t.label, r.reduced(2), juce::Justification::centred, true);

        // Show existing binding if any
        auto* existing = findExistingBinding(t);
        if (existing)
        {
            auto bindLabel = getKeyDescription(*existing);
            g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta));
            g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
            auto tagBounds = r.removeFromBottom(14);
            g.fillRect(tagBounds.reduced(2, 0));
            g.setColour(juce::Colours::black);
            g.drawText(bindLabel, tagBounds.reduced(2, 0), juce::Justification::centred, true);
        }
    }
}

void BindingOverlay::resized()
{
}

void BindingOverlay::mouseDown(const juce::MouseEvent& event)
{
    if (!active_) return;

    int idx = hitTestTarget(event.position.toInt());
    if (idx >= 0)
    {
        selectedTargetIndex_ = idx;
        waitingForKey_ = true;
        repaint();
    }
    else
    {
        // Click outside any target — deselect
        selectedTargetIndex_ = -1;
        waitingForKey_ = false;
        repaint();
    }
}

bool BindingOverlay::keyPressed(const juce::KeyPress& key, juce::Component* /*originatingComponent*/)
{
    if (!active_) return false;

    // Escape exits binding mode
    if (key.isKeyCode(juce::KeyPress::escapeKey))
    {
        exitBindingMode();
        return true;
    }

    // If waiting for a key and a target is selected, create the binding
    if (waitingForKey_ && selectedTargetIndex_ >= 0
        && selectedTargetIndex_ < static_cast<int>(targets_.size()))
    {
        const auto& target = targets_[static_cast<size_t>(selectedTargetIndex_)];
        auto mod = key.getModifiers();

        // Don't bind modifier keys alone
        int keyCode = key.getKeyCode();
        if (keyCode == juce::KeyPress::leftKey || keyCode == juce::KeyPress::rightKey
            || keyCode == juce::KeyPress::upKey || keyCode == juce::KeyPress::downKey)
        {
            // Allow arrow keys
        }

        // Remove any existing binding for this exact key combo
        for (int i = bindingManager_.getNumBindings() - 1; i >= 0; --i)
        {
            auto* existing = bindingManager_.getBindingAt(i);
            if (existing && existing->inputType == Binding::InputType::Keyboard
                && existing->keyCode == keyCode
                && existing->keyModShift == mod.isShiftDown()
                && existing->keyModCmd == mod.isCommandDown()
                && existing->keyModAlt == mod.isAltDown())
            {
                bindingManager_.removeBinding(existing->id);
            }
        }

        // Create new binding
        Binding b;
        b.inputType = Binding::InputType::Keyboard;
        b.keyCode = keyCode;
        b.keyModShift = mod.isShiftDown();
        b.keyModCmd = mod.isCommandDown();
        b.keyModAlt = mod.isAltDown();
        b.action = target.action;
        b.targetLayerIndex = target.layerIndex;
        b.targetColumn = target.column;
        b.targetDeckIndex = target.deckIndex;
        b.targetEffectIndex = target.effectIndex;
        b.targetMacroIndex = target.macroIndex;

        bindingManager_.addBinding(b);

        // Reset to pick another target
        waitingForKey_ = false;
        selectedTargetIndex_ = -1;
        repaint();
        return true;
    }

    return true; // Consume all keys while in binding mode
}

int BindingOverlay::hitTestTarget(juce::Point<int> pos) const
{
    for (int i = 0; i < static_cast<int>(targets_.size()); ++i)
    {
        if (targets_[static_cast<size_t>(i)].bounds.contains(pos))
            return i;
    }
    return -1;
}

juce::String BindingOverlay::getKeyDescription(const Binding& b)
{
    if (b.inputType == Binding::InputType::MidiNote)
        return "MIDI " + juce::String(b.midiNote);
    if (b.inputType == Binding::InputType::MidiCC)
        return "CC " + juce::String(b.midiCC);

    juce::String desc;
    if (b.keyModCmd) desc += "Cmd+";
    if (b.keyModShift) desc += "Shift+";
    if (b.keyModAlt) desc += "Alt+";

    // Convert key code to readable string
    if (b.keyCode >= 'A' && b.keyCode <= 'Z')
        desc += juce::String::charToString(static_cast<juce::juce_wchar>(b.keyCode));
    else if (b.keyCode >= '0' && b.keyCode <= '9')
        desc += juce::String::charToString(static_cast<juce::juce_wchar>(b.keyCode));
    else if (b.keyCode == juce::KeyPress::spaceKey)
        desc += "Space";
    else if (b.keyCode == juce::KeyPress::returnKey)
        desc += "Enter";
    else if (b.keyCode == juce::KeyPress::tabKey)
        desc += "Tab";
    else if (b.keyCode == juce::KeyPress::leftKey)
        desc += "Left";
    else if (b.keyCode == juce::KeyPress::rightKey)
        desc += "Right";
    else if (b.keyCode == juce::KeyPress::upKey)
        desc += "Up";
    else if (b.keyCode == juce::KeyPress::downKey)
        desc += "Down";
    else
        desc += juce::String(b.keyCode);

    return desc;
}

const Binding* BindingOverlay::findExistingBinding(const BindableTarget& target) const
{
    for (int i = 0; i < bindingManager_.getNumBindings(); ++i)
    {
        auto* b = const_cast<BindingManager&>(bindingManager_).getBindingAt(i);
        if (b && b->action == target.action)
        {
            bool matches = true;
            switch (target.action)
            {
                case Binding::Action::TriggerClip:
                    matches = (b->targetLayerIndex == target.layerIndex
                               && b->targetColumn == target.column);
                    break;
                case Binding::Action::TriggerColumn:
                    matches = (b->targetColumn == target.column);
                    break;
                case Binding::Action::ToggleLayerBypass:
                case Binding::Action::ToggleLayerSolo:
                case Binding::Action::ToggleLayerMute:
                case Binding::Action::ToggleLayerAutopilot:
                case Binding::Action::ToggleLayerVisible:
                case Binding::Action::LayerTransport:
                    matches = (b->targetLayerIndex == target.layerIndex);
                    break;
                case Binding::Action::ToggleEffectBypass:
                    matches = (b->targetEffectIndex == target.effectIndex);
                    break;
                case Binding::Action::AdjustMacro:
                    matches = (b->targetMacroIndex == target.macroIndex);
                    break;
                case Binding::Action::SwitchDeck:
                    matches = (b->targetDeckIndex == target.deckIndex);
                    break;
                default:
                    matches = true; // Global actions (TapTempo, etc.)
                    break;
            }
            if (matches) return b;
        }
    }
    return nullptr;
}
