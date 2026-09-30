#!/bin/bash
# s-rta-0930 gop2: merge + Harmony gates G1-G7 (ruling-gop2 FINAL GATE LIST) + forward-identity extras.
M=/Users/boriskarpman/projects/RealTimeAudio; H=$M/.harmony; W=$M/.claude/worktrees/gop2
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad
O=$S/gate/gop2; mkdir -p $O
APP=$M/build/AudioDNA_artefacts/Release/Audio-DNA.app; PRE=$S/apps/main-655d232.app; REF=$O/w10ref; PY=$M/.venv/bin/python
export LANE=harmony-gate; export LOCK_LIB=$S/lib/lock.sh
. $LOCK_LIB
export VIDEO_FIXTURES=$O/fixtures; mkdir -p $VIDEO_FIXTURES
sm() { grep -E '^(FAIL) |PY [0-9]+ PASS|PASS [0-9]+ / FAIL|[0-9]+ PASS / [0-9]+ FAIL|PROBE-[A-Z0-9-]+|AB-VERDICT|^(PASS|FAIL)  [a-z0-9_]+\[' "$1" | tail -${2:-4} | cut -c1-230; }
run() { n=$1; shift; echo "### $n $(date +%T) load=$(sysctl -n vm.loadavg)"; env "$@" > $O/$n.log 2>&1; echo "rc=$? :: $(sm $O/$n.log 3 | tr '\n' ' ')"; quit_app >/dev/null; }
ab() { n=$1; shift; echo "### AB $n $(date +%T) load=$(sysctl -n vm.loadavg)"; ENV_A="${6:-}" ENV_B="${7:-}" bash $H/probe-vupload-ab.sh "$1" "$2" "$3" "$4" "$5" $O/ab-$n > $O/ab-$n.log 2>&1; echo "rc=$? :: $(grep -E 'PASS|FAIL|INFO|median|verdict' $O/ab-$n/summary.txt 2>/dev/null | cut -c1-200 | tr '\n' '|')"; }
unc() { $PY -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"; }
echo "== START $(date '+%F %T') audio: $(system_profiler SPAudioDataType | grep -A6 'Default Input Device: Yes' | grep -i transport | tr -s ' ')"
echo "burners: $(ps -Ao pcpu=,comm= | sort -rn | head -3 | tr '\n' ';')"
echo "== PRE sha $(shasum -a 256 $PRE/Contents/MacOS/Audio-DNA | cut -c1-16) (main 655d232)"
echo "== merge $(date +%T)"
git -C $M merge --no-ff -q -m "merge(s-rta-0930): lane/gop2 - GC7 in-place PREFETCH window (a window counts the slots the clock frees while its run decodes the lead-in from the index's real keyframe, at the measured decode time; unchanged when the lead-in is under a frame); R3 reverse store gate also opens after a seek that landed on a demuxer-key packet (at or after its pts); VFR residual documented by design; probe-vupload-ab u8 rules per lane-arm cap + --selftest + burner taint

Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>" lane/gop2 || { echo "MERGE CONFLICT:"; git -C $M diff --name-only --diff-filter=U; git -C $M merge --abort; exit 2; }
git -C $M log --oneline -1 | cut -c1-70
echo "code diff merge vs lane head (src tests docs CMakeLists CLAUDE.md probes): [$(git -C $M diff --stat 48dd81b HEAD -- src tests docs CMakeLists.txt CLAUDE.md .harmony/probe-vupload-ab.sh .harmony/probe-vupload-ab.py | tail -1)] (empty = identical)"
cmake -S $M -B $M/build > $O/cfg.log 2>&1 && cmake --build $M/build -j8 > $O/build.log 2>&1; echo "build rc $? $(date +%T)"
echo "== G1 ctest $(date +%T)"; ctest --test-dir $M/build -j1 > $O/ctest.log 2>&1; grep -E 'tests passed|Total Test' $O/ctest.log
echo "== G2 TSan (lane build-lane-tsan = same C++ as the merge) $(date +%T)"
for t in test_gop_cache_store test_video_decode_trace test_gop_cache; do
  TSAN_OPTIONS=halt_on_error=0:abort_on_error=0:exitcode=66 $W/build-lane-tsan/tests/$t > $O/g2-$t.log 2>&1; rc=$?
  echo "G2 $t rc=$rc tsan_reports=$(grep -c 'WARNING: ThreadSanitizer' $O/g2-$t.log) :: $(grep -E 'All tests passed|test cases.*failed' $O/g2-$t.log | tail -1)"
done
echo "== G3 $(date +%T)"; $PY $H/probe-vupload-ab.py --selftest 2>&1 | tail -2
$PY $H/probe-vupload-ab.py $H/.reports/s-rta-0930/evidence-0929b/ab-ab256/ab.tsv > $O/g3-replay.txt 2>&1; echo "replay cap0.0 lines: $(grep -c 'cap 0.0' $O/g3-replay.txt)"; diff <(grep -E '^(PASS|FAIL)' $O/g3-replay.txt | grep 'cap 256') <(grep -E '^(PASS|FAIL)' $H/.reports/s-rta-0930/evidence-0929b/ab-ab256/summary.txt | grep 'cap 256') > /dev/null && echo "G3 cap-256 verdicts identical to summary.txt" || echo "G3 cap-256 verdicts DIFFER (see g3-replay.txt)"
echo "== GREEN arm sha $(shasum -a 256 $APP/Contents/MacOS/Audio-DNA | cut -c1-16) $(date +%T)"
ab g4-ab256 $PRE $APP 5 $H/probe-vupload.sh u8_reverse_column_1080x4 ADNA_GOPCACHE_BUDGET_MB=256 ADNA_GOPCACHE_BUDGET_MB=256
ab g5-uncapped $PRE $APP 5 $H/probe-vupload.sh u7_reverse_pingpong,u8_reverse_column_1080x4,u9_reverse_cache_drop,u11_reverse_column_return
ab g6-u10-512 $PRE $APP 5 $H/probe-vupload.sh u10_reverse_4k ADNA_GOPCACHE_BUDGET_MB=512 ADNA_GOPCACHE_BUDGET_MB=512
ab g6-u10-info $PRE $APP 3 $H/probe-vupload.sh u10_reverse_4k
ab g6b-192 $PRE $APP 3 $H/probe-vupload.sh u8_reverse_column_1080x4 ADNA_GOPCACHE_BUDGET_MB=192 ADNA_GOPCACHE_BUDGET_MB=192
ab g6b-128 $PRE $APP 3 $H/probe-vupload.sh u8_reverse_column_1080x4 ADNA_GOPCACHE_BUDGET_MB=128 ADNA_GOPCACHE_BUDGET_MB=128
acquire_quiet_lock || exit 70
trap 'quit_app >/dev/null; release_lock' EXIT
echo "== G5b identity $(date +%T)"
run g5b-uncapped VIDEO_APP=$APP bash $H/probe-vupload.sh $O/g5b-u u12_reverse_pixel_identity
run g5b-256 VIDEO_APP=$APP VIDEO_ENV=ADNA_GOPCACHE_BUDGET_MB=256 bash $H/probe-vupload.sh $O/g5b-256 u12_reverse_pixel_identity
run g5b-128 VIDEO_APP=$APP VIDEO_ENV=ADNA_GOPCACHE_BUDGET_MB=128 bash $H/probe-vupload.sh $O/g5b-128 u12_reverse_pixel_identity
echo "== G7 forward smoke + identity extras $(date +%T)"
rm -rf $REF; mkdir -p $REF
run ref-w10 VIDEO_REF_WRITE=$REF VIDEO_APP=$PRE bash $H/probe-video.sh $O/ref-w10 w10_pixel_identity; echo "refs $(ls $REF | wc -l)"
run w10-all bash $H/probe-video-w10-all.sh $REF $O/w10
run g7-video bash $H/probe-video.sh $O/g7-video
run g7-u4b bash $H/probe-vupload.sh $O/g7-u4b u4b_idle_ring_trim
run g7-u6 bash $H/probe-vupload.sh $O/g7-u6 u6_crossfade_video
run media-open bash $H/probe-media-open.sh $O/media-open
run seq-vram bash $H/probe-seq-vram.sh $O/seq-vram
quit_app >/dev/null; release_lock; trap - EXIT
echo "outwins: $(outwins)"; sleep 16; echo "UNC windows (OptionAll): $(unc)"
echo "new Audio-DNA ips since 15:00: $(find ~/Library/Logs/DiagnosticReports -name 'Audio-DNA*' -newermt '2026-09-30 15:00' | wc -l)"
echo "### GATE DONE $(date '+%F %T')"
