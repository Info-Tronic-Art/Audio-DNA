#!/bin/bash
M=/Users/boriskarpman/projects/RealTimeAudio; H=$M/.harmony; WG=$M/.claude/worktrees/rta0929b-gopcache
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/bd9a1c96-8ac1-42b0-a088-3d8fb87be738/scratchpad; G=$S/gate; O=$G/final; mkdir -p $O
APP=$M/build/AudioDNA_artefacts/Release/Audio-DNA.app; PRE=$G/pre-gop/Audio-DNA.app; REF=$G/w10ref; PY=$M/.venv/bin/python
LANE=harmony-gate
. $S/lib/lock.sh
sm() { grep -E '^(FAIL) |PY [0-9]+ PASS|PASS [0-9]+ / FAIL|[0-9]+ PASS / [0-9]+ FAIL|PROBE-[A-Z0-9-]+|AB-VERDICT|^(PASS|FAIL)  [a-z0-9_]+\[' "$1" | tail -${2:-4} | cut -c1-230; }
run() { n=$1; shift; echo "### $n $(date +%T) load=$(sysctl -n vm.loadavg)"; env "$@" > $O/$n.log 2>&1; echo "rc=$? :: $(sm $O/$n.log 3 | tr '\n' ' ')"; quit_app >/dev/null; }
ab() { n=$1; shift; echo "### AB $n $(date +%T)"; LOCK_LIB=$S/lib/lock.sh ENV_A="${6:-}" ENV_B="${7:-}" bash $H/probe-vupload-ab.sh "$1" "$2" "$3" "$4" "$5" $O/ab-$n > $O/ab-$n.log 2>&1; echo "rc=$? :: $(grep -E 'PASS|FAIL|median|verdict' $O/ab-$n.log | tail -8 | cut -c1-200 | tr '\n' '|')"; }
system_profiler SPAudioDataType | grep -A6 'Default Input Device: Yes' | grep -i transport
cmake -S $M -B $M/build > $O/cfg.log 2>&1 && cmake --build $M/build -j8 > $O/build.log 2>&1; echo "build rc $?"
echo "== ctest $(date +%T)"; ctest --test-dir $M/build -j1 2>&1 | grep -E 'tests passed|Total Test'
echo "== GREEN $(date +%T) sha $(shasum -a 256 $APP/Contents/MacOS/Audio-DNA | cut -c1-16)"
ab ab7 $PRE $APP 5 $H/probe-vupload.sh u7_reverse_pingpong,u8_reverse_column_1080x4,u9_reverse_cache_drop,u10_reverse_4k,u11_reverse_column_return,u12_reverse_pixel_identity
ab ab256 $PRE $APP 5 $H/probe-vupload.sh u8_reverse_column_1080x4 "" ADNA_GOPCACHE_BUDGET_MB=256
ab abw $PRE $APP 5 $H/probe-video.sh w1c_column_trigger_1080x4,w2c_steady_4kx4_blit,w6b_retrigger_mid_fade
acquire_quiet_lock || exit 70
trap 'quit_app >/dev/null; release_lock' EXIT
run w10-all bash $H/probe-video-w10-all.sh $REF $O/w10
run u12-client VIDEO_ENV=ADNA_VIDEO_FORCE_FALLBACK=client bash $H/probe-vupload.sh $O/u12c u12_reverse_pixel_identity
run u12-malloc VIDEO_ENV=ADNA_VIDEO_FORCE_FALLBACK=malloc bash $H/probe-vupload.sh $O/u12m u12_reverse_pixel_identity
run vupload bash $H/probe-vupload.sh $O/vupload
run video bash $H/probe-video.sh $O/video
run btguard BTGUARD_APP=$APP bash $H/probe-btguard.sh $O/btguard
run async-load bash $H/probe-async-load.sh $O/async-load
run outputs bash $H/probe-outputs.sh $O/outputs
run routine-display bash $H/probe-routine-display.sh $O/routine-display
run beatclock bash $H/probe-beatclock.sh $O/beatclock
run render-state RSTATE_APP=$APP bash $H/probe-render-state.sh $O
run crossfade XFADE_APP=$APP bash $H/probe-crossfade.sh $O
run effects-parity PARITY_BUILD_DIR=build bash $H/probe-effects-parity.sh
run manual-bpm MANUALBPM_BUILD_DIR=build bash $H/probe-manual-bpm.sh
run resync RESYNC_BUILD_DIR=build bash $H/probe-resync.sh
run downbeat DOWNBEAT_BUILD_DIR=build bash $H/probe-downbeat-level.sh
run routines ROUTINES_BUILD_DIR=build ROUTINES_RECORD_PAUSE=1.8 bash $H/probe-routines.sh
run mastersignal MS_BUILD_DIR=build bash $H/probe-mastersignal.sh
run decktabs DECKTABS_PY=$PY bash $H/probe-deck-tabs.sh $O/decktabs
run canvas bash $H/probe-canvas.sh $O/canvas
run deckclock bash $H/probe-deck-clock.sh $O/deckclock
run fitmode bash $H/probe-fitmode.sh $O/fitmode
run step3 STEP3_BUILD_DIR=build bash $H/probe-step3.sh
run tempo-start TEMPOSTART_APP=$APP TEMPOSTART_WITNESS_RUNS=20 bash $H/probe-tempo-start.sh $O/tempo-start
run image-load bash $H/probe-image-load.sh $O/image-load
run capture bash $H/probe-capture.sh $O/capture
run finalize-loop FINLOOP_BUILD_DIR=build bash $H/probe-finalize-loop.sh
run onset-render ONSET_BUILD_DIR=build bash $H/probe-onset-render.sh
run seq-vram bash $H/probe-seq-vram.sh $O/seq-vram
run media-open bash $H/probe-media-open.sh $O/media-open
run idle-paint IDLEPAINT_LAUNCHES=5 bash $H/probe-idle-paint.sh $O/idle-paint
echo "### tier1 $(date +%T)"; start_app $APP $O/tier1 test
AUDIODNA_NO_SPAWN=1 $PY -m pytest $M/tests/visual/test_sources.py $M/tests/visual/test_effects.py $M/tests/visual/test_audio_reactivity.py $M/tests/visual/test_time_sweep.py $M/tests/visual/test_performance.py -q > $O/tier1.log 2>&1; echo "tier1 rc $? :: $(tail -1 $O/tier1.log)"
quit_app >/dev/null; release_lock; trap - EXIT
outwins; sleep 16
$PY -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print('UNC windows (OptionAll)', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
echo "new Audio-DNA ips since 08:00: $(find ~/Library/Logs/DiagnosticReports -name 'Audio-DNA*' -newermt '2026-09-30 08:00' | wc -l)"
echo "### FINAL DONE $(date +%T)"
