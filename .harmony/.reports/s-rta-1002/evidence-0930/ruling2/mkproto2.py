import re,sys
d=sys.argv[1]+'/proto/audio/'
def sub(path, old, new, count=1):
    s=open(path).read()
    if s.count(old)!=count:
        raise SystemExit(f"{path}: expected {count} of {old[:60]!r}, found {s.count(old)}")
    s=s.replace(old,new)
    open(path,'w').write(s)
# --- DevicePolicy.h: the table gains devicePlaying + DeviceStopped
sub(d+'DevicePolicy.h','enum class Reapply { None, NoDevice, AdoptInput, InputLost };\nReapply reconcile(bool haveDevice, const juce::String& openedInput, const Lists&);',
    'enum class Reapply { None, NoDevice, AdoptInput, InputLost, DeviceStopped };\nReapply reconcile(bool haveDevice, bool devicePlaying, const juce::String& openedInput, const Lists&);')
# --- DevicePolicy.cpp: deny-all token (TEST-ONLY), the table, the codes
sub(d+'DevicePolicy.cpp','    if ((d.inputName.isNotEmpty() && cfg.testDeniedNames.contains(d.inputName))',
    '    if (cfg.testDeniedNames.contains("*")\n        || (d.inputName.isNotEmpty() && cfg.testDeniedNames.contains(d.inputName))')
s=open(d+'DevicePolicy.cpp').read()
i=s.index('Reapply reconcile(bool haveDevice'); j=s.index('juce::String toString(Reapply r)')
s=s[:i]+'''Reapply reconcile(bool haveDevice, bool devicePlaying, const juce::String& openedInput, const Lists& lists)
{
    const bool anyAllowed = !(lists.inputs.isEmpty() && lists.outputs.isEmpty());
    if (!haveDevice)
        return anyAllowed ? Reapply::NoDevice : Reapply::None;
    if (openedInput.isNotEmpty() && !lists.inputs.contains(openedInput))
        return Reapply::InputLost;
    if (!devicePlaying)
        return anyAllowed ? Reapply::DeviceStopped : Reapply::None;
    if (openedInput.isEmpty() && !lists.inputs.isEmpty())
        return Reapply::AdoptInput;
    return Reapply::None;
}

'''+s[j:]
k=s.index('juce::String toString(Reapply r)'); e=s.index('return "none";\n}',k)+len('return "none";\n}')
s=s[:k]+'''juce::String toString(Reapply r)
{
    switch (r)
    {
        case Reapply::NoDevice:      return "no-device";
        case Reapply::AdoptInput:    return "adopt-input";
        case Reapply::InputLost:     return "input-lost";
        case Reapply::DeviceStopped: return "device-stopped";
        case Reapply::None:          break;
    }
    return {};
}'''+s[e:]
open(d+'DevicePolicy.cpp','w').write(s)
# --- DeviceGuard.h: new members + accessors
sub(d+'DeviceGuard.h','    int reapplies() const noexcept { return reapplies_; }',
    '    int reapplies() const noexcept { return reapplies_; }\n    audiodna::devpolicy::Reapply lastAction() const noexcept { return lastAction_; }\n    // The input a re-apply lost and replaced by ANOTHER input ("" otherwise); updated whenever a re-apply changes the input.\n    const juce::String& lostInput() const noexcept { return lostInput_; }')
sub(d+'DeviceGuard.h','    uint64_t lastAttemptSeq_ = 0;   // the device scan the last open / re-apply ran on',
    '    uint64_t lastAttemptSeq_ = 0;   // the device scan the last open / re-apply ran on\n    juce::StringArray noInputOn_;   // the allowed inputs a re-apply could not open: AdoptInput waits for a different list\n    juce::String lostInput_;\n    audiodna::devpolicy::Reapply lastAction_ = audiodna::devpolicy::Reapply::None;')
# --- DeviceGuard.cpp: timerCallback
g=open(d+'DeviceGuard.cpp').read()
old_eval='''    const auto& scan = guarded->lastScan();
    const bool haveDevice = manager_.getCurrentAudioDevice() != nullptr;
    const auto previous = manager_.getAudioDeviceSetup();
    const auto action = dp::reconcile(haveDevice, previous.inputDeviceName, scan.lists);
    if (action == dp::Reapply::None)
        return;
'''
new_eval='''    const auto& scan = guarded->lastScan();
    if (!noInputOn_.isEmpty() && noInputOn_ != scan.lists.inputs)
        noInputOn_.clear();   // the allowed inputs changed: an adoption may try again
    auto* device = manager_.getCurrentAudioDevice();
    const bool haveDevice = device != nullptr;
    const auto previous = manager_.getAudioDeviceSetup();
    const auto action = dp::reconcile(haveDevice, haveDevice && device->isPlaying(), previous.inputDeviceName, scan.lists);
    if (action == dp::Reapply::None)
        return;
    if (action == dp::Reapply::AdoptInput && noInputOn_ == scan.lists.inputs)
        return;   // no listed input could be opened: wait for a DIFFERENT list of allowed inputs
'''
assert g.count(old_eval)==1; g=g.replace(old_eval,new_eval)
old_inc='''    lastReapplyMs_ = now;
    ++reapplies_;
'''
new_inc='''    lastReapplyMs_ = now;
    ++reapplies_;
    lastAction_ = action;
'''
assert g.count(old_inc)==1; g=g.replace(old_inc,new_inc)
old_tail='''    if (onReapplied)
        onReapplied(error);
}'''
new_tail='''    const bool nowDevice = manager_.getCurrentAudioDevice() != nullptr;
    const auto nowInput = nowDevice ? manager_.getAudioDeviceSetup().inputDeviceName : juce::String();
    noInputOn_ = (nowDevice && nowInput.isEmpty() && !lists.inputs.isEmpty()) ? lists.inputs : juce::StringArray();
    if (nowInput != previous.inputDeviceName)
        lostInput_ = ((action == dp::Reapply::InputLost || action == dp::Reapply::DeviceStopped)
                      && previous.inputDeviceName.isNotEmpty() && nowInput.isNotEmpty())
                         ? previous.inputDeviceName : juce::String();
    if (onReapplied)
        onReapplied(error);
}'''
assert g.count(old_tail)==1; g=g.replace(old_tail,new_tail)
open(d+'DeviceGuard.cpp','w').write(g)
print("proto2 written")
