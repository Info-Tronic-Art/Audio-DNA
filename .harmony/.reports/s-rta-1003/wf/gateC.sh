#!/bin/bash
# Harmony's merge + B8: merge lane/bf9b into main, build main, ctest, TSan unit, K batch on the merged app
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a868a537-8200-4641-a63f-f3029b3cf7ee/scratchpad
P=/Users/boriskarpman/projects/RealTimeAudio
OUT=$SP/gate; L=$OUT/live; mkdir -p $L
MAINAPP=$P/build/AudioDNA_artefacts/Release/Audio-DNA.app
ts(){ date '+%H:%M:%S'; }
FH=$(git -C $P rev-parse --short lane/bf9b); PRE=$(git -C $P rev-parse --short main)
echo "$(ts) pre-merge main $PRE, lane head $FH; main dirty tracked files: $(git -C $P status --short | grep -v '^??' | wc -l | tr -d ' ')"
git -C $P merge --no-ff --no-edit lane/bf9b -m "merge(s-rta-1003): lane/bf9b - decks are boxes of clips: the layers are ONE shared playing stack, a deck is a box of clips, a deck switch changes only the grid (never what plays); Persistent and the deck fade removed; retired decks keep a playing clip alive; no source-deck badge, no tab dot, no Undo Remove button, no event texts (Boris 2026-10-03); the freed-memory read after a layer-stack move fixed inside setLayer / setClip + one hook function (ASan unit gate + live ASan row); Undo of Add / Duplicate / Load Deck retires a playing deck; 33 probes quit only their own app; MilkDrop keeps playing across deck switches (m9b); Pitfall 67

Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>" > $OUT/merge.log 2>&1; rc=$?
echo "merge rc=$rc"; tail -3 $OUT/merge.log | cut -c1-200
[ $rc -ne 0 ] && { echo "MERGE FAILED -- stopping (git -C $P merge --abort to undo)"; git -C $P status --short | head -10; exit 1; }
echo "merged head $(git -C $P rev-parse --short HEAD); conflict markers: $(git -C $P grep -l -E '^(<<<<<<<|>>>>>>>) ' -- src tests docs CLAUDE.md 2>/dev/null | wc -l | tr -d ' ')"
echo "FM-8 diff FH..merged over src tests cmake CMakeLists.txt + probe files: $(git -C $P diff --stat $FH HEAD -- src tests cmake CMakeLists.txt '.harmony/*.sh' '.harmony/*.py' '.harmony/*.json' | wc -l | tr -d ' ') line(s)"
export LANE=harmony-bf9b; . $SP/lib/lock.sh
acquire_lock || { echo "$(ts) BLOCKED (lock) before the main build"; exit 3; }
echo "$(ts) B8-B1 configure + build main"
cmake -S $P -B $P/build > $OUT/b8-configure.log 2>&1; rc=$?; echo "configure rc=$rc | $(grep -i 'libprojectM' $OUT/b8-configure.log | tail -1 | cut -c1-120)"
cmake --build $P/build --config Release -j8 > $OUT/b8-build.log 2>&1; rc=$?; echo "B8-B1 build rc=$rc ($(grep -c 'error:' $OUT/b8-build.log) error lines; $(grep -c 'Building CXX' $OUT/b8-build.log) objects)"
[ $rc -ne 0 ] && { release_lock; echo "BUILD FAILED"; exit 1; }
echo "  merged app sha256 $(shasum -a 256 $MAINAPP/Contents/MacOS/Audio-DNA | cut -c1-16)"
echo "$(ts) B8-B2 ctest"; until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done
ctest --test-dir $P/build --output-on-failure > $OUT/b8-ctest.log 2>&1; rc=$?; rm -rf /tmp/audiodna-ctest.lock; echo "B8-B2 rc=$rc | $(grep -E 'tests passed' $OUT/b8-ctest.log)"
echo "$(ts) B8-K on the merged app"
TAKE=$(grep -o 'recorded take .*' $L/k-main0.log | tail -1 | sed 's/recorded take //' | awk '{print $1}')
BOXES_OLD_TAKE="$TAKE" BOXES_APP=$MAINAPP bash $P/.harmony/probe-boxes.sh $L/k-merged > $L/k-merged.log 2>&1; rc=$?; echo "B8-K rc=$rc (bar: 3)"
grep -E '^PROBE-BOXES|PY [0-9]+ PASS|rows run' $L/k-merged.log | tail -4
echo "$(ts) windows after: $(outwins); adna: [$(adna | tr '\n' ' ')]"
release_lock
echo "$(ts) B8-B3 TSan unit on main"
bash $P/.harmony/probe-tsan-unit.sh > $OUT/b8-tsan.log 2>&1; rc=$?; echo "B8-B3 rc=$rc | $(grep -E 'tests passed' $OUT/b8-tsan.log | tail -1) | $(grep -E 'finds' $OUT/b8-tsan.log | tail -1) | TSan warnings $(grep -c 'WARNING: ThreadSanitizer' $OUT/b8-tsan.log)"
sleep 5; /Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print('UserNotificationCenter windows (OptionAll):', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
echo "$(ts) GATE C DONE"
