#!/bin/bash
# Harmony's pre-merge gates on the FINAL lane head (after FIX-5): unit gates, then the live rows that the delta can touch
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a868a537-8200-4641-a63f-f3029b3cf7ee/scratchpad
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b
OUT=$SP/gate/a2; L=$OUT/live; mkdir -p $L
LANEAPP=$WT/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app; ASANAPP=$WT/build-asan-app/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app
ts(){ date '+%H:%M:%S'; }
echo "$(ts) FH2=$(git -C $WT rev-parse --short HEAD); dirty tracked: $(git -C $WT status --short | grep -v 'build-asan-app' | wc -l | tr -d ' ')"
echo "delta 4137f60..HEAD src files: $(git -C $WT diff --name-only 4137f60 HEAD -- src | tr '\n' ' ')"
echo "delta tests/probes: $(git -C $WT diff --name-only 4137f60 HEAD -- tests .harmony/*.sh .harmony/*.py | tr '\n' ' ')"
echo "  sha256 lane app $(shasum -a 256 $LANEAPP/Contents/MacOS/Audio-DNA | cut -c1-16); asan app $(shasum -a 256 $ASANAPP/Contents/MacOS/Audio-DNA | cut -c1-16)"
cmake --build $WT/build-lane -j8 > $OUT/b1.log 2>&1; rc=$?; echo "$(ts) B1 rc=$rc ($(grep -c 'Building CXX' $OUT/b1.log) objects compiled)"
until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done
ctest --test-dir $WT/build-lane --output-on-failure > $OUT/b2.log 2>&1; rc=$?; rm -rf /tmp/audiodna-ctest.lock; echo "$(ts) B2 rc=$rc | $(grep -E 'tests passed' $OUT/b2.log)"
bash $WT/.harmony/probe-tsan-unit.sh > $OUT/b3.log 2>&1; rc=$?; echo "$(ts) B3 rc=$rc | $(grep -E 'tests passed' $OUT/b3.log | tail -1) | TSan warnings $(grep -c 'WARNING: ThreadSanitizer' $OUT/b3.log)"
bash $WT/.harmony/probe-asan-unit.sh > $OUT/b3b.log 2>&1; rc=$?; echo "$(ts) B3b rc=$rc | $(tail -1 $OUT/b3b.log)"
echo "$(ts) B4 docs grep hits: $(git -C $WT grep -n -i -E 'strip badge|source-deck badge|badge, dots|tab dot|shows a dot' HEAD -- docs/claude CLAUDE.md .harmony/APP-INVENTORY.md tests/CMakeLists.txt | wc -l | tr -d ' '); BORIS_DECISIONS stale words: $(git -C $WT grep -c -i -E 'strip badge names|tab dots|yellow note' HEAD -- BORIS_DECISIONS.md | cut -d: -f3); CLAUDE.md $(wc -c < $WT/CLAUDE.md) B"
export LANE=harmony-bf9b; . $SP/lib/lock.sh
acquire_lock || { echo "$(ts) LIVE BLOCKED (lock)"; exit 3; }
TAKE=$(grep -o 'recorded take .*' $SP/gate/live/k-main0.log | tail -1 | sed 's/recorded take //' | awk '{print $1}')
BOXES_OLD_TAKE="$TAKE" BOXES_APP=$LANEAPP bash $WT/.harmony/probe-boxes.sh $L/k-fh2 > $L/k-fh2.log 2>&1; rc=$?; echo "$(ts) K FH2 rc=$rc (bar 3)"
grep -E '^PROBE-BOXES|PY [0-9]+ PASS|rows run' $L/k-fh2.log | tail -3
MILKDROP_APP=$LANEAPP bash $WT/.harmony/probe-milkdrop.sh $L/h1-fh2 > $L/h1-fh2.log 2>&1; rc=$?; echo "$(ts) H1 FH2 rc=$rc | $(grep -c '^PASS  m9b_deck_switch_live' $L/h1-fh2.log) m9b PASS lines | $(grep -E 'PY [0-9]+ PASS' $L/h1-fh2.log | tail -1) | $(grep -E '^PROBE-MILKDROP' $L/h1-fh2.log | tail -1)"
UIFR_APP=$LANEAPP bash $WT/.harmony/probe-ui-files-rename.sh $L/u1-fh2 --shots > $L/u1-fh2.log 2>&1; rc=$?; echo "$(ts) U1 FH2 rc=$rc | $(grep -E 'PASS / ' $L/u1-fh2.log | tail -1 | cut -c1-40)"
ASAN_LIVE_APP=$ASANAPP bash $WT/.harmony/probe-asan-live.sh $L/asan-fh2 > $L/asan-fh2.log 2>&1; rc=$?; echo "$(ts) ASAN-LIVE FH2 rc=$rc | $(grep -E '^PROBE-ASAN-LIVE' $L/asan-fh2.log | tail -1)"
echo "$(ts) windows after: $(outwins); adna: [$(adna | tr '\n' ' ')]"
release_lock
echo "$(ts) F5 live rows (the builder's driver, lane app with the lever): Layer tab after Remove Deck / Undo"
bash $SP/bf9b-fix-FIX-5/f5run.sh green > $OUT/f5-live.log 2>&1; rc=$?; echo "F5 rc=$rc | $(grep -E '^F5-LIVE|STARTED|QUIT' $OUT/f5-live.log | tr '\n' ' ' | cut -c1-220)"
grep -E '^FAIL' $OUT/f5-live.log | head -5
sleep 2; /Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print('UserNotificationCenter windows (OptionAll):', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]), '| Audio-DNA windows:', len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName',''))]))"
echo "$(ts) GATE A2 DONE"
