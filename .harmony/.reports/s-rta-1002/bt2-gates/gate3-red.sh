#!/bin/bash
# s-rta-1002 bt2 GATE-3 -- unit RED on the lane's BASE (ruling-bt2-seats.md FINAL GATE LIST; AM5 one-tree method).
# A clean worktree of the base sha is made under SCRATCH; its src/ (the code under test) is NOT touched. The lane's
# main-API cases are extracted VERBATIM (extract_cases.py) into the base's tests/test_device_policy.cpp (the lane's
# fixture prelude + the cases) and tests/test_audio_engine_devices.cpp (AE1 / AE2), and the lane's
# test_audio_engine_devices target block is inserted after the base's test_device_policy block. A normal cmake build
# of those two targets in SCRATCH compiles the TUs and the code under test from that ONE tree (never mixed -- E5).
# Per-case verdicts come from `-r junit` (Catch2 wraps long names in --list-tests). The worktree and its build dir are
# removed at the end.
#
# usage: gate3-red.sh <lane-worktree> <base-sha> <scratch-dir> <deps-dir>
#   deps-dir = a build tree's _deps with juce-src / httplib-src / catch2-src / syphon-src (read-only use)
# Predicted (pre-registered): FAIL {R6 R7 R8 R13 R14 R15 R16 AE1 AE2}; PASS {R9 M5c M5r R17}.
set -euo pipefail
LANE=$1; BASE=$2; S=$3; D=$4
HERE="$(cd "$(dirname "$0")" && pwd)"
WT=$S/wt-base; BD=$S/build-base
TDP_IDS="R6 R7 R8 R9 R13 M5c M5r R14 R15 R16 R17"
AE_IDS="AE1 AE2"
PRED_FAIL="AE1 AE2 R13 R14 R15 R16 R6 R7 R8"
PRED_PASS="M5c M5r R17 R9"

echo "=== GATE-3 $(date '+%F %T')  lane $(git -C "$LANE" rev-parse --short HEAD)  base $BASE"
git -C "$LANE" worktree add --detach "$WT" "$BASE" >/dev/null 2>&1
trap 'git -C "$LANE" worktree remove --force "$WT" 2>/dev/null || true; rm -rf "$BD"; echo "cleanup: worktree + build dir removed"' EXIT

python3 "$HERE/extract_cases.py" "$LANE/tests/test_device_policy.cpp" "$WT/tests/test_device_policy.cpp" $TDP_IDS
python3 "$HERE/extract_cases.py" "$LANE/tests/test_audio_engine_devices.cpp" "$WT/tests/test_audio_engine_devices.cpp" $AE_IDS
# verbatim proof: every extracted case (and the prelude) in the base-tree TU == the lane file's
for f in test_device_policy test_audio_engine_devices; do
  ids=$TDP_IDS; [ $f = test_audio_engine_devices ] && ids=$AE_IDS
  python3 "$HERE/extract_cases.py" "$LANE/tests/$f.cpp" --dump "$S/g3-lane-$f" $ids >/dev/null
  python3 "$HERE/extract_cases.py" "$WT/tests/$f.cpp" --dump "$S/g3-tu-$f" $ids >/dev/null
  if diff -r "$S/g3-lane-$f" "$S/g3-tu-$f" >/dev/null; then echo "diff -r lane vs TU ($f): empty"; else echo "diff -r lane vs TU ($f): NOT EMPTY"; exit 3; fi
  rm -rf "$S/g3-lane-$f" "$S/g3-tu-$f"
done
# the lane's target block, inserted after the base's test_device_policy block
python3 - "$LANE/tests/CMakeLists.txt" "$WT/tests/CMakeLists.txt" <<'PY'
import sys
lane, base = open(sys.argv[1]).read(), open(sys.argv[2]).read()
a = lane.index('# --- test_audio_engine_devices')
b = lane.index('catch_discover_tests(test_audio_engine_devices)') + len('catch_discover_tests(test_audio_engine_devices)\n')
anchor = 'catch_discover_tests(test_device_policy)\n'
assert base.count(anchor) == 1 and 'test_audio_engine_devices' not in base
open(sys.argv[2], 'w').write(base.replace(anchor, anchor + '\n' + lane[a:b]))
print('inserted the lane target block (%d lines)' % lane[a:b].count('\n'))
PY
echo "base src/ untouched: git diff --stat -- src = '$(git -C "$WT" diff --stat -- src)'"
echo "base tree changes (tests only):"; git -C "$WT" status --short

cmake -S "$WT" -B "$BD" -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
  -DFETCHCONTENT_SOURCE_DIR_JUCE=$D/juce-src -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=$D/httplib-src \
  -DFETCHCONTENT_SOURCE_DIR_CATCH2=$D/catch2-src -DFETCHCONTENT_SOURCE_DIR_SYPHON=$D/syphon-src >"$S/g3-configure.log" 2>&1
cmake --build "$BD" --target test_device_policy test_audio_engine_devices -j3 >"$S/g3-build.log" 2>&1
echo "built: $(ls "$BD/tests/test_device_policy" "$BD/tests/test_audio_engine_devices" | tr '\n' ' ')"

verdicts() {  # verdicts <binary>: "<ID> PASS|FAIL <failed assertions>" per case, from -r junit
  { "$1" -r junit 2>/dev/null || true; } | python3 -c '
import sys, xml.etree.ElementTree as ET
for tc in ET.fromstring(sys.stdin.read()).iter("testcase"):
    f = len(tc.findall("failure")) + len(tc.findall("error"))
    print(tc.get("name").split(" ")[0], "FAIL" if f else "PASS", f)'
}
{ verdicts "$BD/tests/test_device_policy"; verdicts "$BD/tests/test_audio_engine_devices"; } | sort > "$S/g3-verdicts.txt"
echo "--- per-case verdicts (failed-assertion count) on base $BASE:"; cat "$S/g3-verdicts.txt"
echo "--- raw Catch2 summaries:"
{ "$BD/tests/test_device_policy" 2>&1 || true; } | tail -3
{ "$BD/tests/test_audio_engine_devices" 2>&1 || true; } | tail -3
GOT_FAIL=$(awk '$2=="FAIL"{print $1}' "$S/g3-verdicts.txt" | sort | tr '\n' ' ' | sed 's/ $//')
GOT_PASS=$(awk '$2=="PASS"{print $1}' "$S/g3-verdicts.txt" | sort | tr '\n' ' ' | sed 's/ $//')
echo "FAIL set: {$GOT_FAIL}   predicted {$PRED_FAIL}"
echo "PASS set: {$GOT_PASS}   predicted {$PRED_PASS}"
if [ "$GOT_FAIL" = "$PRED_FAIL" ] && [ "$GOT_PASS" = "$PRED_PASS" ]; then echo "GATE-3: AS PREDICTED"; else echo "GATE-3: MISMATCH"; exit 1; fi
