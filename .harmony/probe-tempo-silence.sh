#!/bin/bash
# probe-tempo-silence.sh — does a TAPPED tempo keep the bars moving with no audio at all? s167.
#
# WHY THIS EXISTS
# Boris, live: "when no beat is detected, everything timed to the beat freezes -- silence between
# tracks stops all modulation dead." The bar counters only advanced on a real detected onset, and
# aubio gates onsets off below -70dB. Since every oscillator folds beatPhase + beatInBar +
# 4*barCount into its cycle, that made the freeze DURATION-DEPENDENT: <=1-beat cycles ran fine,
# while 2/4/8-beat cycles retraced a fixed fraction of their shape forever. His ruling: keep
# running from the last detected or TAPPED tempo. This proves the tapped half, end to end.
#
# >>> IT MUST RUN IN PRODUCTION MODE. NOT --test-mode. <<<
# This cost a full gate run to learn. In --test-mode the AnalysisThread is NEVER STARTED
# (MainComponent: `if (!testMode_) { ...startThread... }`) and the TestServer owns the FeatureBus
# instead, so /api/bpm reads a snapshot nothing ever writes and reports zeros forever no matter
# what the tracker is doing. A --test-mode run of this probe is structurally incapable of
# observing BPMTracker and will report a FALSE FAILURE.
#
# Note port 7070 (ApiServer) is unconditional -- it exists in production mode. 8080 does not.
set -u
OUT="${1:-/tmp/audiodna-tempo-probe}"; mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APP="$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA"
A='http://127.0.0.1:7070'
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
bpmfield(){ curl -s --max-time 3 "$A/api/bpm" | tr -d ' \n' | sed -n "s/.*\"$1\":\([0-9.]*\).*/\1/p"; }

pgrep -f 'MacOS/Audio-DNA' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running."; exit 64; }
[ -x "$APP" ] || { echo "REFUSE: no built app at $APP"; exit 64; }

"$APP" > "$OUT/app.log" 2>&1 &   # PRODUCTION MODE -- deliberately no --test-mode
PID=$!
for i in $(seq 1 45); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: 7070 never came up"; kill $PID 2>/dev/null; exit 1; }

B0=$(bpmfield bpm)
[ "${B0:-0}" = "0.0" ] && ok "cold start in a quiet room reports no tempo (bpm=0)" || echo "NOTE  bpm=$B0 at start (room not silent?) -- the test below still holds"

curl -s --max-time 5 -X POST "$A/api/set_bpm" -H 'Content-Type: application/json' -d '{"bpm":120.0}' >/dev/null
sleep 1
B1=$(bpmfield bpm)
[ "$B1" = "120.0" ] && ok "a tapped tempo takes effect (bpm=120)" || no "tapped tempo did not take: bpm=$B1"

C0=$(bpmfield barCount); sleep 8; C1=$(bpmfield barCount)
D=$(( ${C1:-0} - ${C0:-0} ))
# 120 BPM = 2s per bar, so 8s = 4 bars. Allow 3-5 for scheduling slop; 0 is the bug this guards.
if [ "$D" -ge 3 ] && [ "$D" -le 5 ]; then
  ok "BARS KEEP RUNNING WITH NO AUDIO: barCount advanced by $D in 8s (expect ~4 at 120 BPM)"
elif [ "$D" -eq 0 ]; then
  no "THE FREEZE IS BACK: barCount did not move in 8s. This is the exact regression this guards."
else
  no "barCount advanced by $D in 8s -- expected ~4 at 120 BPM. Wrong rate is as bad as frozen."
fi

osascript -e "tell application \"System Events\" to tell (first process whose unix id is $PID) to quit" >/dev/null 2>&1
sleep 2; kill -0 "$PID" 2>/dev/null && osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 20); do kill -0 "$PID" 2>/dev/null || break; sleep 1; done
pgrep -f 'MacOS/Audio-DNA' >/dev/null && no "APP STILL RUNNING -- screen-safety breach" || ok "app quit gracefully"
W=$("$ROOT/.venv/bin/python" -c "
import Quartz
wl=Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName',''))]))" 2>/dev/null)
[ "$W" = "0" ] && ok "0 Audio-DNA windows in the FULL window list" || no "$W window(s) still on screen"
echo; echo "$PASS PASS / $FAIL FAIL"
[ "$FAIL" -eq 0 ] || exit 1
