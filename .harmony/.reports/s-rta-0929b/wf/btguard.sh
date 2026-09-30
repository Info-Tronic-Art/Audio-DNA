#!/bin/bash
M=/Users/boriskarpman/projects/RealTimeAudio; H=$M/.harmony; W=$M/.claude/worktrees/rta0929b-btguard
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/bd9a1c96-8ac1-42b0-a088-3d8fb87be738/scratchpad; G=$S/gate; O=$G/btg; mkdir -p $O
APP=$M/build/AudioDNA_artefacts/Release/Audio-DNA.app; PRE=$S/apps/main-c8730a61.app
LANE=harmony-gate
. $S/lib/lock.sh
sm() { grep -E '^(FAIL) |PY [0-9]+ PASS|PASS [0-9]+ / FAIL|[0-9]+ PASS / [0-9]+ FAIL|PROBE-[A-Z0-9-]+' "$1" | tail -${2:-4} | cut -c1-230; }
run() { n=$1; shift; echo "### $n $(date +%T) load=$(sysctl -n vm.loadavg)"; env "$@" > $O/$n.log 2>&1; echo "rc=$? :: $(sm $O/$n.log 3 | tr '\n' ' ')"; quit_app >/dev/null; }
system_profiler SPAudioDataType | grep -A6 'Default Input Device: Yes' | grep -i transport
echo "== RED on pre-merge app copy sha $(shasum -a 256 $PRE/Contents/MacOS/Audio-DNA | cut -c1-16) $(date +%T)"
acquire_quiet_lock || exit 70
run red-btguard BTGUARD_APP=$PRE bash $W/.harmony/probe-btguard.sh $O/red
release_lock
echo "== merge $(date +%T)"
git -C $M merge --no-ff -q -m "merge(s-rta-0929b): lane/btguard - Audio-DNA never opens a Bluetooth / BLE / AirPlay / wireless-Continuity audio device (GuardedAudioDeviceManager wraps JUCE's CoreAudio type: hidden from lists, never the default, refused on open; aggregates with such a member denied; a transport read error fails closed); falls back to the onboard mic / built-in or wired output; persistent plain notice when no allowed mic / device; device-gone reconciler (coalesced); ends the startup crash when Bluetooth earbuds are the macOS default (JUCE 8.0.4 CoreAudio overflow); test_device_policy (36 cases), probe-btguard

Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>" lane/btguard || { echo MERGE CONFLICT; git -C $M merge --abort; exit 2; }
git -C $M log --oneline -1 | cut -c1-70
cmake -S $M -B $M/build > $O/cfg.log 2>&1 && cmake --build $M/build -j8 > $O/build.log 2>&1; echo "build rc $?"; tail -1 $O/build.log | cut -c1-150
echo "== ctest $(date +%T)"; ctest --test-dir $M/build -j1 2>&1 | grep -E 'tests passed|Total Test'
echo "== GREEN $(date +%T) sha $(shasum -a 256 $APP/Contents/MacOS/Audio-DNA | cut -c1-16)"
acquire_quiet_lock || exit 70
trap 'quit_app >/dev/null; release_lock' EXIT
run green-btguard BTGUARD_APP=$APP BTGUARD_SHOTS=1 bash $H/probe-btguard.sh $O/green
run manual-bpm MANUALBPM_BUILD_DIR=build bash $H/probe-manual-bpm.sh
run resync RESYNC_BUILD_DIR=build bash $H/probe-resync.sh
run downbeat DOWNBEAT_BUILD_DIR=build bash $H/probe-downbeat-level.sh
run step3 STEP3_BUILD_DIR=build bash $H/probe-step3.sh
run onset-render ONSET_BUILD_DIR=build bash $H/probe-onset-render.sh
outwins
sleep 16
/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print('UNC windows (OptionAll)', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
echo "new Audio-DNA ips since 08:00: $(find ~/Library/Logs/DiagnosticReports -name 'Audio-DNA*' -newermt '2026-09-30 08:00' | wc -l)"
echo "== done $(date +%T)"
