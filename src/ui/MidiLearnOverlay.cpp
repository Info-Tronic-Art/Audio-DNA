#include "MidiLearnOverlay.h"

MidiLearnOverlay::MidiLearnOverlay(BindingManager& bindingManager, Composition& composition)
    : bindingManager_(bindingManager), composition_(composition)
{
    setInterceptsMouseClicks(true, false);
    setWantsKeyboardFocus(true);
}

MidiLearnOverlay::~MidiLearnOverlay()
{
    stopListening();
    if (auto* topLevel = getTopLevelComponent())
        topLevel->removeKeyListener(this);
}

void MidiLearnOverlay::enterLearnMode(juce::AudioDeviceManager* deviceManager)
{
    deviceManager_ = deviceManager;
    active_ = true;
    waitingForMidi_ = false;
    selectedTargetIndex_ = -1;
    lastMidiMessage_ = "";
    bindingManager_.setBindingMode(true);
    setVisible(true);
    toFront(true);
    grabKeyboardFocus();

    if (auto* topLevel = getTopLevelComponent())
        topLevel->addKeyListener(this);

    startListening();
    repaint();
}

void MidiLearnOverlay::exitLearnMode()
{
    active_ = false;
    waitingForMidi_ = false;
    selectedTargetIndex_ = -1;
    bindingManager_.setBindingMode(false);
    stopListening();
    setVisible(false);

    if (auto* topLevel = getTopLevelComponent())
        topLevel->removeKeyListener(this);

    if (onLearnModeExit)
        onLearnModeExit();
}

void MidiLearnOverlay::startListening()
{
    if (!deviceManager_) return;

    auto midiInputs = juce::MidiInput::getAvailableDevices();
    for (const auto& device : midiInputs)
    {
        if (!deviceManager_->isMidiInputDeviceEnabled(device.identifier))
            deviceManager_->setMidiInputDeviceEnabled(device.identifier, true);
        deviceManager_->addMidiInputDeviceCallback(device.identifier, this);
    }
}

void MidiLearnOverlay::stopListening()
{
    if (!deviceManager_) return;

    auto midiInputs = juce::MidiInput::getAvailableDevices();
    for (const auto& device : midiInputs)
        deviceManager_->removeMidiInputDeviceCallback(device.identifier, this);
}

void MidiLearnOverlay::setBindableTargets(const std::vector<BindingOverlay::BindableTarget>& targets)
{
    targets_ = targets;
    if (active_)
        repaint();
}

void MidiLearnOverlay::paint(juce::Graphics& g)
{
    if (!active_) return;

    // Semi-transparent dark overlay with magenta tint
    g.setColour(juce::Colour(0xcc100010));
    g.fillRect(getLocalBounds());

    // Title
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta));
    g.setFont(juce::Font(juce::FontOptions(18.0f).withStyle("Bold")));

    juce::String title = waitingForMidi_
        ? "Send a MIDI note or CC..."
        : "MIDI Learn Mode — Click a target, then send MIDI";
    g.drawText(title, getLocalBounds().removeFromTop(40), juce::Justification::centred);

    // Status / escape hint
    g.setFont(juce::Font(juce::FontOptions(12.0f)));
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    juce::String hint = "Press Escape to exit";
    if (lastMidiMessage_.isNotEmpty())
        hint += "  |  Last: " + lastMidiMessage_;
    g.drawText(hint, getLocalBounds().removeFromTop(60).removeFromBottom(18),
               juce::Justification::centred);

    // Draw bindable targets
    for (int i = 0; i < static_cast<int>(targets_.size()); ++i)
    {
        const auto& t = targets_[static_cast<size_t>(i)];
        bool isSelected = (i == selectedTargetIndex_);

        auto r = t.bounds;
        if (isSelected)
        {
            g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.4f));
            g.fillRect(r);
            g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta));
            g.drawRect(r, 2);
        }
        else
        {
            g.setColour(juce::Colour(AudioDNALookAndFeel::kSurfaceLight).withAlpha(0.5f));
            g.fillRect(r);
            g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.6f));
            g.drawRect(r, 1);
        }

        // Label
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.drawText(t.label, r.reduced(2), juce::Justification::centred, true);

        // Show existing MIDI binding if any
        auto* existing = findExistingMidiBinding(t);
        if (existing)
        {
            juce::String bindLabel;
            if (existing->inputType == Binding::InputType::MidiNote)
                bindLabel = "Note " + juce::String(existing->midiNote);
            else if (existing->inputType == Binding::InputType::MidiCC)
                bindLabel = "CC " + juce::String(existing->midiCC);

            g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
            g.setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
            auto tagBounds = r.removeFromBottom(14);
            g.fillRect(tagBounds.reduced(2, 0));
            g.setColour(juce::Colours::black);
            g.drawText(bindLabel, tagBounds.reduced(2, 0), juce::Justification::centred, true);
        }
    }
}

void MidiLearnOverlay::resized()
{
}

void MidiLearnOverlay::mouseDown(const juce::MouseEvent& event)
{
    if (!active_) return;

    int idx = hitTestTarget(event.position.toInt());
    if (idx >= 0)
    {
        selectedTargetIndex_ = idx;
        waitingForMidi_ = true;
        repaint();
    }
    else
    {
        selectedTargetIndex_ = -1;
        waitingForMidi_ = false;
        repaint();
    }
}

bool MidiLearnOverlay::keyPressed(const juce::KeyPress& key, juce::Component* /*originatingComponent*/)
{
    if (!active_) return false;

    if (key.isKeyCode(juce::KeyPress::escapeKey))
    {
        exitLearnMode();
        return true;
    }

    return true; // Consume all keys while in learn mode
}

void MidiLearnOverlay::handleIncomingMidiMessage(juce::MidiInput* /*source*/,
                                                  const juce::MidiMessage& message)
{
    // This is called on the MIDI thread — post to message thread
    int channel = message.getChannel();

    if (message.isNoteOn())
    {
        int note = message.getNoteNumber();
        int velocity = message.getVelocity();
        juce::String desc = "Note " + juce::String(note) + " vel=" + juce::String(velocity)
                            + " ch=" + juce::String(channel);

        juce::MessageManager::callAsync([this, note, channel, desc]()
        {
            lastMidiMessage_ = desc;

            if (waitingForMidi_ && selectedTargetIndex_ >= 0
                && selectedTargetIndex_ < static_cast<int>(targets_.size()))
            {
                const auto& target = targets_[static_cast<size_t>(selectedTargetIndex_)];

                // Remove any existing binding for this MIDI note
                for (int i = bindingManager_.getNumBindings() - 1; i >= 0; --i)
                {
                    auto* existing = bindingManager_.getBindingAt(i);
                    if (existing && existing->inputType == Binding::InputType::MidiNote
                        && existing->midiNote == note)
                    {
                        bindingManager_.removeBinding(existing->id);
                    }
                }

                Binding b;
                b.inputType = Binding::InputType::MidiNote;
                b.midiNote = note;
                b.midiChannel = channel;
                b.action = target.action;
                b.targetLayerIndex = target.layerIndex;
                b.targetColumn = target.column;
                b.targetDeckIndex = target.deckIndex;
                b.targetEffectIndex = target.effectIndex;
                b.targetMacroIndex = target.macroIndex;

                bindingManager_.addBinding(b);

                waitingForMidi_ = false;
                selectedTargetIndex_ = -1;
            }
            repaint();
        });
    }
    else if (message.isController())
    {
        int cc = message.getControllerNumber();
        int value = message.getControllerValue();
        juce::String desc = "CC " + juce::String(cc) + " val=" + juce::String(value)
                            + " ch=" + juce::String(channel);

        juce::MessageManager::callAsync([this, cc, channel, desc]()
        {
            lastMidiMessage_ = desc;

            if (waitingForMidi_ && selectedTargetIndex_ >= 0
                && selectedTargetIndex_ < static_cast<int>(targets_.size()))
            {
                const auto& target = targets_[static_cast<size_t>(selectedTargetIndex_)];

                // Remove any existing binding for this CC
                for (int i = bindingManager_.getNumBindings() - 1; i >= 0; --i)
                {
                    auto* existing = bindingManager_.getBindingAt(i);
                    if (existing && existing->inputType == Binding::InputType::MidiCC
                        && existing->midiCC == cc)
                    {
                        bindingManager_.removeBinding(existing->id);
                    }
                }

                Binding b;
                b.inputType = Binding::InputType::MidiCC;
                b.midiCC = cc;
                b.midiChannel = channel;
                b.action = target.action;
                b.targetLayerIndex = target.layerIndex;
                b.targetColumn = target.column;
                b.targetDeckIndex = target.deckIndex;
                b.targetEffectIndex = target.effectIndex;
                b.targetMacroIndex = target.macroIndex;

                bindingManager_.addBinding(b);

                waitingForMidi_ = false;
                selectedTargetIndex_ = -1;
            }
            repaint();
        });
    }
}

int MidiLearnOverlay::hitTestTarget(juce::Point<int> pos) const
{
    for (int i = 0; i < static_cast<int>(targets_.size()); ++i)
    {
        if (targets_[static_cast<size_t>(i)].bounds.contains(pos))
            return i;
    }
    return -1;
}

const Binding* MidiLearnOverlay::findExistingMidiBinding(
    const BindingOverlay::BindableTarget& target) const
{
    for (int i = 0; i < bindingManager_.getNumBindings(); ++i)
    {
        auto* b = const_cast<BindingManager&>(bindingManager_).getBindingAt(i);
        if (b && b->action == target.action
            && (b->inputType == Binding::InputType::MidiNote
                || b->inputType == Binding::InputType::MidiCC))
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
                    matches = true;
                    break;
            }
            if (matches) return b;
        }
    }
    return nullptr;
}
