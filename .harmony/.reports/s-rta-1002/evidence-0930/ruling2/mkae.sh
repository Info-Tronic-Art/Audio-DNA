#!/bin/bash
set -e
R2=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/ruling2
AE=/Users/boriskarpman/projects/RealTimeAudio/src/audio/AudioEngine.cpp
sed -n '138,141p;150,153p' "$AE"
sed '138,141d;150,153d' "$AE" > "$R2/AE_c3.cpp"
# a C3 regression GUARDED by getCurrentAudioDevice() (invisible to AE1's deny-everything engine)
awk '{print} /Enable input channels/ && !done {getline; print; print "        if (deviceManager_.getCurrentAudioDevice() != nullptr) { auto s = deviceManager_.getAudioDeviceSetup(); s.inputChannels.setRange(0, 2, true); deviceManager_.setAudioDeviceSetup(s, true); }"; done=1}' "$R2/AE_c3.cpp" > "$R2/AE_guarded.cpp"
grep -n "setSourceMode" -A 22 "$R2/AE_guarded.cpp" | sed -n '1,24p'
