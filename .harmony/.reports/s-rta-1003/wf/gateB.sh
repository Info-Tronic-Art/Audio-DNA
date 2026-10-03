#!/bin/bash
# Harmony's pre-merge gates, part B: ASAN-LIVE's failing arm (the wiring-line mutant), then B6 perf (quiet machine)
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a868a537-8200-4641-a63f-f3029b3cf7ee/scratchpad
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b
OUT=$SP/gate; L=$OUT/live; mkdir -p $L
MAIN0=$SP/apps/pre-bf9b.app; LANEAPP=$WT/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app; ASANAPP=$WT/build-asan-app/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app
ts(){ date '+%H:%M:%S'; }
export LANE=harmony-bf9b; . $SP/lib/lock.sh
echo "$(ts) MU3: build the wiring-line mutant into build-asan-app (the builder's script; source restored at once)"
bash $SP/bf9b-fix-FIX-4/mu3_build.sh > $OUT/mu3-build.out 2>&1
grep -E 'MU3 build rc|git diff --quiet' $OUT/mu3-build.out
acquire_lock || { echo "$(ts) BLOCKED (lock)"; exit 3; }
echo "$(ts) >>> asan-mu3"; ASAN_LIVE_APP=$ASANAPP bash $WT/.harmony/probe-asan-live.sh $L/asan-mu3 > $L/asan-mu3.log 2>&1; rc=$?; echo "$(ts) <<< asan-mu3 rc=$rc"
grep -E '^PROBE-ASAN-LIVE|^RED ' $L/asan-mu3.log | cut -c1-220 | tail -3
release_lock
echo "$(ts) restore: rebuild the ASan app from the restored source"
cmake --build $WT/build-asan-app --target AudioDNA -j6 > $OUT/asan-restore.log 2>&1; rc=$?; echo "restore build rc=$rc; sha256 $(shasum -a 256 $ASANAPP/Contents/MacOS/Audio-DNA | cut -c1-16)"
git -C $WT diff --quiet -- src tests; rc=$?; echo "git diff --quiet -- src tests rc=$rc; status dirty lines: $(git -C $WT status --short | grep -v build-asan-app | wc -l | tr -d ' ')"
acquire_lock || { echo "$(ts) BLOCKED (lock)"; exit 3; }
echo "$(ts) >>> asan-fh2 (the restored app must be GREEN again)"; ASAN_LIVE_APP=$ASANAPP bash $WT/.harmony/probe-asan-live.sh $L/asan-fh2 > $L/asan-fh2.log 2>&1; rc=$?; echo "$(ts) <<< asan-fh2 rc=$rc"
grep -E '^PROBE-ASAN-LIVE' $L/asan-fh2.log | tail -1
release_lock
echo "$(ts) B6 perf: selftest, then the gate run under the quiet lock"
bash $WT/.harmony/probe-boxes-perf.sh --selftest > $OUT/perf-selftest.log 2>&1; rc=$?; echo "perf selftest rc=$rc | $(tail -1 $OUT/perf-selftest.log | cut -c1-160)"
ps -Ao pcpu=,etime=,comm= | sort -rn | head -5 | cut -c1-130
acquire_quiet_lock || { echo "$(ts) B6 BLOCKED (no quiet lock)"; exit 3; }
echo "$(ts) >>> b6"; PERF_FH_APP=$LANEAPP PERF_MAIN_APP=$MAIN0 bash $WT/.harmony/probe-boxes-perf.sh $L/b6 > $L/b6.log 2>&1; rc=$?; echo "$(ts) <<< b6 rc=$rc"
grep -E '^B6\(|^PROBE-BOXES-PERF' $L/b6.log | cut -c1-330
ps -Ao pcpu=,etime=,comm= | sort -rn | head -4 | cut -c1-130
echo "$(ts) windows after: $(outwins); adna: [$(adna | tr '\n' ' ')]"
release_lock
sleep 16; /Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print('UserNotificationCenter windows (OptionAll):', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
echo "$(ts) GATE B DONE"
