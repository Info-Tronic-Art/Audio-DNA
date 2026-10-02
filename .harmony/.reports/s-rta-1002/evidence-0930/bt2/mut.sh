#!/bin/bash
SBX=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/bt2
ROOT=/Users/boriskarpman/projects/RealTimeAudio
BASE=$(python3 - <<'PY'
import json
cc=json.load(open('/Users/boriskarpman/projects/RealTimeAudio/build/compile_commands.json'))
for e in cc:
    if e['file'].endswith('tests/test_device_policy.cpp'):
        print(e.get('command') or ' '.join(e['arguments']))
PY
)
SRC="$ROOT/tests/test_device_policy.cpp"
OBJ="$ROOT/build/tests/CMakeFiles/test_device_policy.dir"
run_mutant() {  # name file python-replace-old python-replace-new
  local name="$1" file="$2"
  rm -rf $SBX/m_$name && mkdir -p $SBX/m_$name/audio && cp $SBX/proto/audio/* $SBX/m_$name/audio/
  python3 - "$SBX/m_$name/audio/$file" "$3" "$4" <<'PY'
import sys
p,old,new=sys.argv[1],sys.argv[2],sys.argv[3]
s=open(p).read(); assert old in s, ("anchor missing", old); open(p,'w').write(s.replace(old,new,1))
PY
  cd "$ROOT/build/tests"
  for f in DevicePolicy.cpp DeviceGuard.cpp; do
    CMD="${BASE//$SRC/$SBX/m_$name/audio/$f}"
    CMD=$(echo "$CMD" | sed -E "s# -o [^ ]+# -o $SBX/m_$name/$f.o#; s# -MF [^ ]+##; s# -MD##; s# -MT [^ ]+##; s#-I$ROOT/src #-I$SBX/m_$name -I$ROOT/src #")
    eval "$CMD" 2>/dev/null || { echo "$name: compile error"; return; }
  done
  local res=""
  for main in test_device_policy.cpp x_bt2_red.cpp x_bt2_new.cpp; do
    /usr/bin/c++ -O3 -arch arm64 $SBX/p3/$main.o $SBX/m_$name/DevicePolicy.cpp.o $SBX/p3/CoreAudioDeviceInfo.cpp.o $SBX/m_$name/DeviceGuard.cpp.o \
      "$OBJ/__/_deps/juce-src/modules/juce_core/juce_core_CompilationTime.cpp.o" "$OBJ/__/_deps/juce-src/modules/juce_core/juce_core.mm.o" \
      "$OBJ/__/_deps/juce-src/modules/juce_events/juce_events.mm.o" "$OBJ/__/_deps/juce-src/modules/juce_audio_basics/juce_audio_basics.mm.o" \
      "$OBJ/__/_deps/juce-src/modules/juce_audio_devices/juce_audio_devices.mm.o" -o $SBX/m_$name/run \
      $ROOT/build/_deps/catch2-build/src/libCatch2Main.a $ROOT/build/_deps/catch2-build/src/libCatch2.a \
      -framework Cocoa -framework Foundation -framework IOKit -framework Security -framework Accelerate -framework CoreAudio -framework CoreMIDI -framework AudioToolbox
    local failed; failed=$($SBX/m_$name/run -r junit 2>/dev/null | python3 -c "
import sys, xml.etree.ElementTree as ET
t = ET.fromstring(sys.stdin.read())
bad = set()
for tc in t.iter('testcase'):
    if tc.find('failure') is not None or tc.find('error') is not None:
        bad.add(tc.get('name').split(' ')[0])
print(' '.join(sorted(bad)))")
    res="$res$failed"
  done
  echo "$name: failing -> $(echo $res | tr ' ' '\n' | sort -u | tr '\n' ' ')"
}
run_mutant noAdoptInput DevicePolicy.cpp 'return lists.inputs.isEmpty() ? Reapply::None : Reapply::AdoptInput;' 'return Reapply::None;'
run_mutant noInputLost DevicePolicy.cpp 'return lists.inputs.contains(openedInput) ? Reapply::None : Reapply::InputLost;' 'return Reapply::None;'
run_mutant switchWorking DevicePolicy.cpp 'return lists.inputs.contains(openedInput) ? Reapply::None : Reapply::InputLost;' 'return lists.inputs.size() > 1 ? Reapply::AdoptInput : (lists.inputs.contains(openedInput) ? Reapply::None : Reapply::InputLost);'
run_mutant noRestore DeviceGuard.cpp 'if (haveDevice && manager_.getCurrentAudioDevice() == nullptr)' 'if (false)'
run_mutant noLaunchSeq DeviceGuard.cpp 'lastAttemptSeq_ = guarded->lastScan().seq;   // AFTER' 'juce::ignoreUnused(guarded);   // AFTER'
run_mutant noSettle DeviceGuard.cpp '    startTimer(settleMs_);   // restarts: re-evaluated after the LAST change message
}' '    timerCallback();
}'
