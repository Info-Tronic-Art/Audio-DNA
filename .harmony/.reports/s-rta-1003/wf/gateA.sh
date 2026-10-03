#!/bin/bash
# Harmony's pre-merge gates, part A: section 5 G-0, G-1, B1, B2, B3, B3b, B4 (no app), then the live batch K / H1 / U1 / ASAN-LIVE
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a868a537-8200-4641-a63f-f3029b3cf7ee/scratchpad
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b
OUT=$SP/gate; L=$OUT/live; mkdir -p $L
MAIN0=$SP/apps/pre-bf9b.app; LANEAPP=$WT/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app; ASANAPP=$WT/build-asan-app/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app
ts(){ date '+%H:%M:%S'; }
echo "$(ts) G-0 FH=$(git -C $WT rev-parse --short HEAD) status: $(git -C $WT status --short | grep -v 'build-asan-app' | wc -l | tr -d ' ') dirty"
for a in $MAIN0 $LANEAPP $ASANAPP; do echo "  sha256 $(shasum -a 256 $a/Contents/MacOS/Audio-DNA | cut -c1-16) $a"; done
echo "$(ts) G-1 safety grep (every hit listed)"
for f in probe-boxes.sh probe-milkdrop.sh probe-ui-files-rename.sh probe-asan-live.sh probe-boxes-perf.sh probe-quit-ours.sh; do grep -nE 'osascript.*Audio-DNA|pkill|killall|adna_kill' $WT/.harmony/$f | cut -c1-170 | sed "s|^|  $f:|"; done
echo "$(ts) B1"; cmake --build $WT/build-lane -j8 > $OUT/b1.log 2>&1; echo "B1 rc=$? ($(grep -c 'error:' $OUT/b1.log) error lines)"
echo "$(ts) B2"; until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done
ctest --test-dir $WT/build-lane --output-on-failure > $OUT/b2.log 2>&1; echo "B2 rc=$?"; rm -rf /tmp/audiodna-ctest.lock
grep -E 'tests passed|Total Test time' $OUT/b2.log
ctest --test-dir $WT/build-lane -N > $OUT/b2-names.log 2>&1
for n in 'AS1' 'AS2' 'AS3' 'AS3b' 'AS4' 'AS5' 'AS6' 'AS7' 'N1' 'T6f' 'T6g' 'T6h' 'T6i' 'T6j' "G1'" 'G2' 'G3' 'BF14' 'M-t' 'M-a' 'M-c' 'B4h' 'B4i' 'B4j'; do printf '%s=%s ' "$n" "$(grep -c -- "$n" $OUT/b2-names.log)"; done; echo
echo "$(ts) B3"; bash $WT/.harmony/probe-tsan-unit.sh > $OUT/b3.log 2>&1; echo "B3 rc=$? | $(grep -E 'tests passed' $OUT/b3.log | tail -1) | $(grep -E 'finds' $OUT/b3.log | tail -1) | TSan warnings $(grep -c 'WARNING: ThreadSanitizer' $OUT/b3.log)"
echo "$(ts) B3b"; bash $WT/.harmony/probe-asan-unit.sh > $OUT/b3b.log 2>&1; echo "B3b rc=$? | $(tail -1 $OUT/b3b.log)"
echo "$(ts) B4 docs grep hits: $(git -C $WT grep -n -i -E 'strip badge|source-deck badge|badge, dots|tab dot|shows a dot' HEAD -- docs/claude CLAUDE.md .harmony/APP-INVENTORY.md tests/CMakeLists.txt | wc -l | tr -d ' ') ; CLAUDE.md $(wc -c < $WT/CLAUDE.md) B"
echo "$(ts) UNIT PART DONE -- live batch"
export LANE=harmony-bf9b; . $SP/lib/lock.sh
acquire_lock || { echo "$(ts) LIVE BATCH BLOCKED (lock)"; exit 3; }
step(){ local name=$1; shift; echo "$(ts) >>> $name"; "$@" > $L/$name.log 2>&1; echo "$(ts) <<< $name rc=$?"; }
step k-main0 env BOXES_APP=$MAIN0 bash $WT/.harmony/probe-boxes.sh $L/k-main0
grep -E '^PROBE-BOXES|PY [0-9]+ PASS' $L/k-main0.log | tail -6
TAKE=$(grep -o 'recorded take .*' $L/k-main0.log | tail -1 | sed 's/recorded take //' | awk '{print $1}'); echo "  take: $TAKE"
step k-fh env BOXES_OLD_TAKE="$TAKE" BOXES_APP=$LANEAPP bash $WT/.harmony/probe-boxes.sh $L/k-fh
grep -E '^PROBE-BOXES|PY [0-9]+ PASS|rows run|EXPECTED_ROWS' $L/k-fh.log | tail -6
step h1-fh env MILKDROP_APP=$LANEAPP bash $WT/.harmony/probe-milkdrop.sh $L/h1-fh
grep -E 'm9b_deck_switch_live|^PROBE-MILKDROP|PY [0-9]+ PASS' $L/h1-fh.log | cut -c1-200 | tail -6
step h1-main0 env MILKDROP_MODE=pre MILKDROP_APP=$MAIN0 bash $WT/.harmony/probe-milkdrop.sh $L/h1-main0 m9b_deck_switch_live
grep -E 'm9b_deck_switch_live|^PROBE-MILKDROP|PY [0-9]+ PASS' $L/h1-main0.log | cut -c1-200 | tail -6
step u1-fh env UIFR_APP=$LANEAPP bash $WT/.harmony/probe-ui-files-rename.sh $L/u1-fh --shots
grep -E 'PASS / |FAIL  |GREEN|RED' $L/u1-fh.log | cut -c1-200 | tail -6
step u1-main0 env UIFR_APP=$MAIN0 bash $WT/.harmony/probe-ui-files-rename.sh $L/u1-main0 --shots
grep -E 'PASS / |FAIL  |GREEN|RED' $L/u1-main0.log | cut -c1-200 | tail -8
step asan-fh env ASAN_LIVE_APP=$ASANAPP bash $WT/.harmony/probe-asan-live.sh $L/asan-fh
grep -E '^PROBE-ASAN-LIVE' $L/asan-fh.log | tail -2
echo "$(ts) windows after: $(outwins); adna: [$(adna | tr '\n' ' ')]"
release_lock
sleep 16; /Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print('UserNotificationCenter windows (OptionAll):', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
echo "$(ts) GATE A DONE"
