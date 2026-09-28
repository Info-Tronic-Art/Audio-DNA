#!/bin/bash
# Harmony FINAL battery on main (after all four merges). One lock hold for the probes, one per Tier-1 run.
M=/Users/boriskarpman/projects/RealTimeAudio; H=$M/.harmony; APP=$M/build/AudioDNA_artefacts/Release/Audio-DNA.app
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad; O=$S/gate/final; mkdir -p $O
LANE=harmony . $S/lib/lock.sh
run() { n=$1; shift; echo "### $n $(date +%T) load=$(sysctl -n vm.loadavg)"; env "$@" > $O/$n.log 2>&1; echo "rc=$? :: $(grep -E 'PASS [0-9]+ / FAIL|PASS / .* FAIL|GREEN|RED' $O/$n.log | tail -2 | tr '\n' ' ')"; }
wait_quiet; acquire_lock || exit 70
trap 'quit_app >/dev/null; release_lock' EXIT
echo "FINAL start $(date '+%F %T') head=$(git -C $M log --oneline -1 | cut -c1-8)"
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
quit_app >/dev/null; release_lock; trap - EXIT
echo "### probes done $(date +%T)"; outwins
for f in test_sources test_effects test_audio_reactivity test_time_sweep test_performance test_fractals; do
  TESTDIR=$M/tests/visual FILES=$f bash $S/gate/tier1/fractals.sh $APP final-$f 2>&1 | grep -E "rc=|^START|^END"
done
echo "### FINAL DONE $(date +%T)"
