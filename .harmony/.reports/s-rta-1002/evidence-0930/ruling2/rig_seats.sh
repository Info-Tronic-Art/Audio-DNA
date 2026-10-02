#!/bin/bash
# SCRATCH (architect ruling-bt2-seats). Usage: rig_seats.sh <tag> <protoDir|MAIN> <TU.cpp>...
# Compiles each TU with test_device_policy's compile command; with a protoDir, -I<protoDir> goes FIRST (one tree:
# the TU and the objects under test both see that tree's headers -- AM5); links each TU to that tree's objects.
set -e
R2=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/ruling2
ROOT=/Users/boriskarpman/projects/RealTimeAudio
TAG=$1; PROTO=$2; shift 2
OUT=$R2/out_$TAG; mkdir -p $OUT; rm -f $OUT/*.o
BASE=$(python3 - <<'PY'
import json
cc=json.load(open('/Users/boriskarpman/projects/RealTimeAudio/build/compile_commands.json'))
for e in cc:
    if e['file'].endswith('tests/test_device_policy.cpp'):
        print(e.get('command') or ' '.join(e['arguments']))
PY
)
SRC="$ROOT/tests/test_device_policy.cpp"
cc1() {  # $1 source  $2 object
  local CMD="${BASE//$SRC/$1}"
  CMD=$(echo "$CMD" | sed -E "s# -o [^ ]+# -o $2#; s# -MF [^ ]+##; s# -MD##; s# -MT [^ ]+##")
  if [ "$PROTO" != MAIN ]; then
    CMD=$(echo "$CMD" | sed -E "s#-I$ROOT/src #-I$PROTO -I$ROOT/src #")
    echo "$CMD" | grep -q -- "-I$PROTO -I$ROOT/src" || { echo "include swap failed"; exit 3; }
  fi
  (cd "$ROOT/build/tests" && eval "$CMD -I$R2")
}
OBJ="$ROOT/build/tests/CMakeFiles/test_device_policy.dir"
if [ "$PROTO" = MAIN ]; then
  UNDER=("$OBJ/__/src/audio/DevicePolicy.cpp.o" "$OBJ/__/src/audio/CoreAudioDeviceInfo.cpp.o" "$OBJ/__/src/audio/DeviceGuard.cpp.o")
else
  for f in DevicePolicy DeviceGuard CoreAudioDeviceInfo; do cc1 "$PROTO/audio/$f.cpp" "$OUT/$f.o"; done
  UNDER=("$OUT/DevicePolicy.o" "$OUT/CoreAudioDeviceInfo.o" "$OUT/DeviceGuard.o")
fi
LIBS=("$OBJ/__/_deps/juce-src/modules/juce_core/juce_core_CompilationTime.cpp.o" "$OBJ/__/_deps/juce-src/modules/juce_core/juce_core.mm.o" "$OBJ/__/_deps/juce-src/modules/juce_events/juce_events.mm.o" "$OBJ/__/_deps/juce-src/modules/juce_audio_basics/juce_audio_basics.mm.o" "$OBJ/__/_deps/juce-src/modules/juce_audio_devices/juce_audio_devices.mm.o")
FW="-framework Cocoa -framework Foundation -framework IOKit -framework Security -framework Accelerate -framework CoreAudio -framework CoreMIDI -framework AudioToolbox"
C2="$ROOT/build/_deps/catch2-build/src/libCatch2Main.a $ROOT/build/_deps/catch2-build/src/libCatch2.a"
for tu in "$@"; do
  b=$(basename "$tu" .cpp)
  cc1 "$tu" "$OUT/$b.o"
  /usr/bin/c++ -O3 -arch arm64 "$OUT/$b.o" "${UNDER[@]}" "${LIBS[@]}" -o "$OUT/run_$b" $C2 $FW
done
echo "BUILT $TAG"
