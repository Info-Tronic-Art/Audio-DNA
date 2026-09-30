#!/bin/bash
M=/Users/boriskarpman/projects/RealTimeAudio; H=$M/.harmony; W=$M/.claude/worktrees/rta0929-vupload
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad; G=$S/gate; O=$G/final; mkdir -p $O
APP=$M/build/AudioDNA_artefacts/Release/Audio-DNA.app; PRE=$G/pre-vu/Audio-DNA.app; REF=$G/w10ref
LANE=harmony-gate
. $S/lib/lock.sh
sm() { grep -E '^(FAIL) |PY [0-9]+ PASS|PASS [0-9]+ / FAIL|[0-9]+ PASS / [0-9]+ FAIL|PROBE-[A-Z0-9-]+ (GREEN|RED)' "$1" | tail -${2:-4} | cut -c1-230; }
rm -rf $G/pre-vu $REF; mkdir -p $G/pre-vu $REF; cp -R $APP $PRE
echo "== PRE app sha $(shasum -a 256 $PRE/Contents/MacOS/Audio-DNA | cut -c1-16) head $(git -C $M log --oneline -1 | cut -c1-8) $(date +%T)"
acquire_quiet_lock || exit 70; uptime
echo "== RED: lane rows on the pre-merge app"
VIDEO_APP=$PRE bash $W/.harmony/probe-vupload.sh $G/red-vu > $G/red-vu.log 2>&1; echo "vupload rc $?"; quit_app >/dev/null; sm $G/red-vu.log 6
VIDEO_APP=$PRE bash $W/.harmony/probe-video.sh $G/red-w1c w1c_column_trigger_1080x4 > $G/red-w1c.log 2>&1; echo "w1c rc $?"; quit_app >/dev/null; sm $G/red-w1c.log 4
VIDEO_APP=$PRE bash $W/.harmony/probe-video-w10-all.sh $REF > $G/red-w10.log 2>&1; echo "w10-all (no refs yet) rc $?"; quit_app >/dev/null; sm $G/red-w10.log 3
echo "== w10 references from the pre-merge app"
VIDEO_REF_WRITE=$REF VIDEO_APP=$PRE bash $W/.harmony/probe-video.sh $G/ref-w10 w10_pixel_identity > $G/ref-w10.log 2>&1; echo "ref rc $? files $(ls $REF | wc -l)"; quit_app >/dev/null
release_lock
echo "== merge $(date +%T)"
git -C $M merge --no-ff -q -m "merge(s-rta-0929): lane/vupload - video upload: a demand-adaptive per-frame video upload budget (column-trigger 4 x 1080p 105 -> 119.7 fps, peak callback 6.55 -> ~1-2 ms), IOSurface ring slots + one GPU blit per new frame (4 x 4K 85-90 -> 100-104 fps, pixel-identical on blit / client / malloc x 5 formats), the shown frame's slot held across a context loss (hold_no_texture 463 -> 0), idle players' slots purged (-284..-318 MB), reverse geometry from the writer look-ahead; GL-thread QoS tried and reverted (VU15: heavier trigger round-trip tail); probe-vupload, probe-video w1c / w1d / w2c / w10, w10-all wrapper

Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>" lane/vupload || { echo MERGE CONFLICT; git -C $M merge --abort; exit 2; }
git -C $M log --oneline -1 | cut -c1-60
cmake -S $M -B $M/build > $G/cfg-vu.log 2>&1 && cmake --build $M/build -j6 > $G/build-vu.log 2>&1; echo "build rc $?"
echo "== ctest $(date +%T)"; ctest --test-dir $M/build -j1 2>&1 | grep -E 'tests passed|Total Test'
echo "== GREEN + FINAL battery $(date +%T) sha $(shasum -a 256 $APP/Contents/MacOS/Audio-DNA | cut -c1-16)"
run() { n=$1; shift; echo "### $n $(date +%T) load=$(sysctl -n vm.loadavg)"; env "$@" > $O/$n.log 2>&1; echo "rc=$? :: $(sm $O/$n.log 2 | tr '\n' ' ')"; }
acquire_quiet_lock || exit 70
trap 'quit_app >/dev/null; release_lock' EXIT
run vupload bash $H/probe-vupload.sh $O/vupload
run video bash $H/probe-video.sh $O/video
run w10-all bash $H/probe-video-w10-all.sh $REF
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
run decktabs DECKTABS_PY=$M/.venv/bin/python bash $H/probe-deck-tabs.sh $O/decktabs
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
quit_app >/dev/null; release_lock; trap - EXIT
echo "### probes done $(date +%T)"; outwins
for f in test_sources test_effects test_audio_reactivity test_time_sweep test_performance; do
  TESTDIR=$M/tests/visual FILES=$f bash $G/tier1/fractals.sh $APP final-$f 2>&1 | grep -E "rc=|passed|failed" | tail -2
done
$M/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
print('UNC windows', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
echo "### FINAL DONE $(date +%T)"
