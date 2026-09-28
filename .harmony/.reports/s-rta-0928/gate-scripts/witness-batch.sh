#!/bin/bash
# Harmony tempo witness batch: N relaunch runs of (health, +2 s, set_bpm 120, record, 1.5 s, stop) under ONE lock hold.
# Usage: witness-batch.sh APP LABEL N OUTDIR    Tally: start bpm 0 count, unknown bar grid (startBeatInBar < 0) count.
APP="$1"; LBL="$2"; N="${3:-20}"; OUT="$4"; A='http://127.0.0.1:7070'; T="$HOME/Documents/Audio-DNA/Takes"
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad
LANE=harmony . $S/lib/lock.sh
mkdir -p "$OUT"; : > "$OUT/tally.txt"
acquire_lock || exit 70
trap 'quit_app >/dev/null; release_lock' EXIT
echo "WITNESS $LBL start $(date '+%F %T') load=$(sysctl -n vm.loadavg) app=$APP"
for i in $(seq -w 1 $N); do
  NAME="hw-$LBL-$i"; R="$OUT/run-$i"; mkdir -p "$R"
  [ -n "$(adna)" ] && { echo "REFUSE: Audio-DNA already running"; exit 64; }
  open -g --stdout "$R/app.out" --stderr "$R/app.err" "$APP"
  for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 -H 'Connection: close' $A/api/health)" ] && break; sleep 1; done
  sleep 2
  curl -s -X POST -H 'Connection: close' -H 'Content-Type: application/json' -d '{"bpm":120}' $A/api/set_bpm >/dev/null
  curl -s -X POST -H 'Connection: close' -H 'Content-Type: application/json' -d "{\"name\":\"$NAME\",\"audio\":false}" $A/api/perf/record >/dev/null
  sleep 1.5
  curl -s -X POST -H 'Connection: close' -H 'Content-Type: application/json' -d '{}' $A/api/perf/stop >/dev/null
  sleep 1
  F=$(ls -d "$T/$NAME"*/take.json 2>/dev/null | head -1)
  [ -n "$F" ] && cp "$F" "$R/take-final.json"
  quit_app > "$R/quit.log"
  python3 - "$R/take-final.json" "$i" >> "$OUT/tally.txt" <<'PY'
import json,sys
try:
    d=json.load(open(sys.argv[1])); tm=d.get('tempoMap') or []; m=d.get('meta',{})
    b0=tm[0].get('bpm') if tm else None; sbb=m.get('startBeatInBar')
    print(f"run {sys.argv[2]} start_bpm={b0} startBeatInBar={sbb} anchors={len(tm)} first={json.dumps(tm[:2])[:160]}")
except Exception as e:
    print(f"run {sys.argv[2]} NO-TAKE {e}")
PY
  for d in "$T/$NAME"*; do [ -d "$d" ] && rm -rf "$d"; done
done
python3 - "$OUT/tally.txt" "$LBL" <<'PY'
import re,sys
L=open(sys.argv[1]).read().splitlines()
n=len(L); b0=sum(1 for l in L if re.search(r'start_bpm=(0(\.0)?|None)\b',l)); ug=sum(1 for l in L if re.search(r'startBeatInBar=(-\d|None)',l)); nt=sum(1 for l in L if 'NO-TAKE' in l)
print(f"WITNESS-TALLY {sys.argv[2]}: runs={n} start_bpm0={b0} unknown_bar_grid={ug} no_take={nt}")
PY
echo "WITNESS $LBL end $(date '+%F %T') load=$(sysctl -n vm.loadavg)"; outwins
