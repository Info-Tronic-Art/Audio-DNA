#!/bin/bash
# s-rta-1002 tsan: merge + Harmony gates G1-G6 (ruling-tsan.md FINAL GATES, adoption H1-H13). Compact lines to $SUM.
# usage: tsan-gate.sh <T0 sha> [skip-merge]
M=/Users/boriskarpman/projects/RealTimeAudio; H=$M/.harmony; W=$M/.claude/worktrees/tsan
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/267d1ba0-b658-4610-9f47-ef72f4b49872/scratchpad
O=$S/gate/tsan; mkdir -p $O; SUM=$O/SUMMARY.txt; T0=$1
PRE=$S/apps/pre-d4e81bd/Audio-DNA.app; APP=$M/build/AudioDNA_artefacts/Release/Audio-DNA.app
TMAIN=$S/build-tsan-main/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app; TLANEB=$S/build-tsan-lane; TLANE=$TLANEB/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app
PY=$M/.venv/bin/python; export ADNA_DEPS_DIR=$M/build/_deps
LANE=harmony-gate; . $S/lib/lock.sh
say() { echo "$*" | tee -a $SUM; }
ctl() { until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done; "$@"; local rc=$?; rm -rf /tmp/audiodna-ctest.lock; return $rc; }
unc() { $PY -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"; }
say "== START $(date '+%F %T') burners: $(ps -Ao pcpu=,comm= | sort -rn | head -3 | sed 's#.*/##' | tr '\n' ';')"
LANEN=$(ctest --test-dir $W/build-lane -N | tail -1 | grep -oE '[0-9]+'); MAINN=$(ctest --test-dir $M/build -N | tail -1 | grep -oE '[0-9]+')
EXP=$(( MAINN + LANEN - 1055 )); say "counts: main $MAINN, lane $LANEN (base 1055) -> expected merged $EXP"
BASE=$(git -C $M rev-parse HEAD)
if [ "$2" != skip-merge ]; then
  git -C $M merge --no-ff -q -F $S/wf/tsan-merge-msg.txt lane/tsan || { say "MERGE CONFLICT: $(git -C $M diff --name-only --diff-filter=U | tr '\n' ' ')"; git -C $M merge --abort; exit 2; }
fi
say "merged $(git -C $M log --oneline -1 | cut -c1-60) base $BASE; code diff vs lane head over lane-only paths: [$(git -C $M diff --stat lane/tsan HEAD -- src/model src/render src/core src/connect src/media tests/test_layer_runtime_race.cpp | tail -1)]"
cmake -S $M -B $M/build > $O/cfg.log 2>&1 && cmake --build $M/build -j6 > $O/build.log 2>&1; say "G1 build rc $? $(date +%T)"
ctl ctest --test-dir $M/build -j1 > $O/ctest.log 2>&1; say "G1 $(grep -E 'tests passed' $O/ctest.log) | ctest -N $(ctest --test-dir $M/build -N | tail -1) | expected $EXP"
grep -A12 'tests FAILED' $O/ctest.log | head -14 | tee -a $SUM
for t in test_layer_runtime_race test_manual_scalar_race test_layer_runtime test_clip_transport_sync test_relaxed test_log_line_lint test_render_thread_lint test_shared_field_types; do b=$(find $M/build/tests -maxdepth 2 -name $t -type f -perm -u+x | head -1); say "  G1 target $t: $( [ -n "$b" ] && $b --list-tests 2>/dev/null | grep -oE '^[0-9]+ test cases|^[0-9]+ matching test cases' | head -1 || echo MISSING)"; done
say "== G2 RED arm (T0 $T0) $(date +%T)"
git -C $M worktree add --detach $S/wt-t0 $T0 > /dev/null 2>&1; bash $S/wt-t0/.harmony/probe-tsan-unit.sh $S/build-tsan-red > $O/g2-red.log 2>&1; say "G2 RED rc=$? :: $(grep -E 'tests passed|tests failed' $O/g2-red.log | tail -1)"
grep -E '^ *[0-9]+/[0-9]+ Test' $O/g2-red.log | sed 's/  */ /g' | cut -c1-150 | tee -a $SUM
say "G2 RED races naming src/: $(grep -c 'WARNING: ThreadSanitizer: data race' $O/g2-red.log) warnings; src frames $(grep -cE '/src/[A-Za-z/]+\.(h|cpp)' $O/g2-red.log)"
git -C $M worktree remove --force $S/wt-t0 > /dev/null 2>&1
say "== G2 GREEN arm (merged) $(date +%T)"
bash $H/probe-tsan-unit.sh $TLANEB > $O/g2-green.log 2>&1; say "G2 GREEN rc=$? :: $(grep -E 'tests passed|tests failed|found' $O/g2-green.log | tail -2 | tr '\n' ' ') warnings=$(grep -c 'WARNING: ThreadSanitizer' $O/g2-green.log)"
cmake --build $TLANEB --target AudioDNA -j6 > $O/tsan-lane-app.log 2>&1; say "TSan lane app build rc $? ($(otool -L $TLANE/Contents/MacOS/Audio-DNA | grep -c tsan) tsan dylib)"
say "== G3 sweep $(date +%T)"
export TSAN_APP_main=$TMAIN TSAN_APP_lane=$TLANE; G3=$O/g3; mkdir -p $G3; n=0
for round in 1 2 3; do SPEC=""; for sc in a b c d; do for arm in main lane; do n=$((n+1)); SPEC="$SPEC $n:$sc@$arm"; done; done
  acquire_lock || exit 70; bash $H/probe-tsan.sh $G3 "$SPEC" > $O/g3-round$round.log 2>&1; say "G3 round $round rc=$? $(date +%T)"; quit_app >/dev/null; release_lock; done
for round in 1 2; do SPEC=""; for arm in main lane; do n=$((n+1)); SPEC="$SPEC $n:e@$arm"; done
  acquire_lock || exit 70; bash $H/probe-tsan.sh $G3 "$SPEC" > $O/g3-e$round.log 2>&1; say "G3 e round $round rc=$? $(date +%T)"; quit_app >/dev/null; release_lock; done
$PY $H/probe-tsan-analyze.py $G3 --repo $M --src main=git:$BASE --src lane=$M --json $O/g3.json > $O/g3-analyze.txt 2>&1; say "G3 analyze rc=$? (full: $O/g3-analyze.txt)"
say "G3 launches.tsv rows: $(tail -n +2 $G3/launches.tsv 2>/dev/null | wc -l)"
say "== G4 parity $(date +%T)"
bash $S/gatetools/g4-parity.sh $PRE $APP $O/g4 > $O/g4.log 2>&1; say "G4 rc=$? :: $(grep -E '^G4:' $O/g4.log | tail -1)"; grep -E 'PRE-EXISTING|MISSING' $O/g4.log | head -8 | tee -a $SUM
say "== G6 perf $(date +%T) burners: $(ps -Ao pcpu=,comm= | sort -rn | head -3 | sed 's#.*/##' | tr '\n' ';')"
PROBE_DIR=$H bash $S/gatetools/g6-perf.sh $PRE $APP $O/g6 5 > $O/g6.log 2>&1; say "G6 rc=$? :: $(grep -E '^G6:' $O/g6.log | tail -1)"
say "outwins: $(outwins)"; sleep 16; say "UNC windows: $(unc); new Audio-DNA ips today: $(find ~/Library/Logs/DiagnosticReports -name 'Audio-DNA*' -newermt "$(date '+%Y-%m-%d') 00:00" | wc -l)"
say "### GATE DONE $(date '+%F %T')"
