#!/bin/bash
# MERGE 1 of lane one-save, after the merge commit: build main, serial ctest, GREEN live rows on MAIN's merged app, his folders again.
R=/Users/boriskarpman/projects/RealTimeAudio; SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/31b4c846-ad01-4801-82d9-569a43efdd3b/scratchpad; O=$SP/merge1; PYQ=$R/.venv/bin/python; APP=$R/build/AudioDNA_artefacts/Release/Audio-DNA.app
SET="$HOME/Library/Audio-DNA/settings.json"; HIS="$HOME/Library/AudioDNA"
echo "$(date '+%F %T') MERGE-1 GREEN arm start; main head $(git -C $R rev-parse --short HEAD); app sha before build $(shasum -a 256 $APP/Contents/MacOS/Audio-DNA | cut -c1-16)"
cmake --build $R/build -j 8 > $O/build.log 2>&1
rc=$?
echo "main build rc=$rc; compiled objects: $(grep -c 'Building CXX\|Building OBJCXX' $O/build.log); errors: $(grep -c ' error[: ]' $O/build.log); app sha after $(shasum -a 256 $APP/Contents/MacOS/Audio-DNA | cut -c1-16)"
[ $rc -ne 0 ] && { tail -5 $O/build.log | cut -c1-200; exit 1; }
ctest --test-dir $R/build -N > $O/ctestN.log 2>&1; tail -1 $O/ctestN.log
bash $SP/lib/ctest-mutex.sh $R/build > $O/ctest-full.log 2>&1
rc=$?
echo "full SERIAL ctest rc=$rc"; grep -E 'tests passed|tests failed|Total Test time|ctest rc=|\*\*\*Failed|MUTEX' $O/ctest-full.log | head -10
LANE=h-m1
. $SP/lib/lock.sh
acquire_lock || { echo "LIVE ROWS NOT RUN: no lock"; exit 1; }
ONESAVE_APP=$APP ONESAVE_QPY=$PYQ bash $R/.harmony/probe-one-save.sh $O/runs os_l13,os_l14,os_l14b > $O/probe-main.log 2>&1
rc=$?
echo "[merged main: OS-L13, L14, L14b] rc=$rc: $(grep -E '^(PASS|FAIL)  OS-|^PROBE-ONE-SAVE|^REFUSE' $O/probe-main.log | cut -c1-150)"
BOXES_APP=$APP bash $R/.harmony/probe-boxes.sh $O/runs k7_old_show > $O/k7-main.log 2>&1
rc=$?
echo "[merged main: K7] rc=$rc: $(grep -E 'k7_old_show save: the saved file|^PROBE' $O/k7-main.log | cut -c1-200)"
release_lock
sleep 16
$PYQ -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
a=[w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName', ''))]
u=[w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName', ''))]
print('16 s after the last quit: audio-dna windows %d, Output-named %d, UserNotificationCenter windows %d' % (len(a), len([w for w in a if 'Output' in str(w.get('kCGWindowName', ''))]), len(u)))"
echo "adna: [$(adna | tr '\n' ' ')]; lock present: $(ls -d /tmp/audiodna-live.lock 2>/dev/null | wc -l | tr -d ' ')"
echo "settings sha: $(shasum -a 256 "$SET" | cut -d' ' -f1)"
echo "his show sha: $(shasum -a 256 "$HIS/compositions/test with harry.json" | cut -d' ' -f1); compositions holds: $(ls "$HIS/compositions" | tr '\n' ','); Decks holds $(ls "$HIS/Decks" | wc -l | tr -d ' ') files; decks sha set: $(cat "$HIS"/Decks/*.json | shasum -a 256 | cut -c1-16)"
echo "$(date '+%F %T') MERGE-1 GREEN arm end"
