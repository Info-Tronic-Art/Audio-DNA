#!/bin/bash
echo "ARCHIVED RECORD (R-N1, s-rta-1003): this script quits Audio-DNA by name -- never run or source it; use .harmony/probe-quit-ours.sh" >&2; exit 64
# stop-witness.sh APPBUNDLE OUTDIR -- launch (open -g), run stop-witness.py, quit. Caller holds the live lock.
APP="$1"; OUT="$2"; mkdir -p "$OUT"
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a07fe8e4-258f-47f0-88b1-97bfb5129bb6/scratchpad/routines-followup
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_156f9cfa-a36-1
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
[ -n "$(adna_pids)" ] && { echo "REFUSE: Audio-DNA already running"; exit 64; }
sed "s#@ROOT@#$WT#g" $WT/.harmony/probe-routines.json > "$OUT/fixture.json"
TAKE="$(ls -dt $HOME/Documents/Audio-DNA/Takes/probe-routines-*.adna-take | head -1)"
NAME="stopwitness-$(date +%Y%m%d-%H%M%S)"
echo "(app: $APP)"; echo "(take: $TAKE)"
open -g --stdout "$OUT/app-out.log" --stderr "$OUT/app-err.log" "$APP"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 http://127.0.0.1:7070/api/health 2>/dev/null)" ] && break; sleep 1; done
if [ -z "$(curl -s --max-time 2 http://127.0.0.1:7070/api/health 2>/dev/null)" ]; then echo "FAIL: health never came up (a dialog? STOP)"; else
  python3 $SP/stop-witness.py "$OUT/fixture.json" "$TAKE" "$NAME"
fi
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do [ -z "$(adna_pids)" ] && break; sleep 1; done
if [ -n "$(adna_pids)" ]; then echo "pkill (last resort)"; kill $(adna_pids) 2>/dev/null; sleep 2; fi
[ -z "$(adna_pids)" ] && echo "(app terminated)" || echo "APP STILL RUNNING"
TF="$HOME/Documents/Audio-DNA/Takes/$NAME.adna-take"
if [ -d "$TF" ]; then cp "$TF/take.json" "$OUT/witness-take.json" 2>/dev/null; rm -rf "$TF"; echo "(witness take copied to $OUT and its folder removed)"; fi
