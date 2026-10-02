#!/bin/bash
# probe-tsan.sh -- lane tsan (s-rta-1002; plan .harmony/.reports/s-rta-0930/plan-tsan.md T8, ruling ruling-tsan.md
# amendment 12 + FINAL GATES G3). The ThreadSanitizer APP sweep: launches a TSan build of Audio-DNA once per SPEC item,
# drives one REST scenario (.harmony/probe-tsan.py a|b|c|d|e), quits it gracefully and keeps every TSan report
# (log_path) per launch; .harmony/probe-tsan-analyze.py then keys the reports (classes APP / UNATTRIBUTED /
# JUCE-SYSTEM, families A-E). Adapted from the s-rta-0929b / s-rta-0930 harness
# (.harmony/.reports/s-rta-0930/tsan-harness/run_batch.sh + mkfix.sh). The unit gate is .harmony/probe-tsan-unit.sh.
#
# usage: probe-tsan.sh <out-dir> "<N:scen@arm> <N:scen@arm> ..."      e.g. "1:a@main 2:a@lane 3:d@lane"
#   TSAN_APP_<arm>  the TSan app bundle of that arm, from its own cmake build dir -- never a copied / re-signed bundle
#                   (e.g. TSAN_APP_lane=<tree>/build-tsan/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app)
#   TSAN_MEDIA      fixture dir (default <out-dir>/media; made here with ffmpeg if v1080_00.mp4 is missing)
#   TSAN_HISTORY    TSan history_size (default 4; G3.3 re-runs an UNATTRIBUTED report with 7)
#   TSAN_PY         python with requests + pyobjc Quartz (default <tree>/.venv, else the main checkout's .venv)
# Output: <out-dir>/runs/tsan-<N>/{tsan.<pid>, app-err.log, scen.log, state.json, health-*.json, quit.txt, ...},
# <out-dir>/launches.tsv (one row per launch: arm + validity fields), <out-dir>/batch-<stamp>.log.
# TSAN_OPTIONS (identical on every arm, G3): halt_on_error=0:abort_on_error=0:exitcode=0:report_signal_unsafe=0:
# history_size=<TSAN_HISTORY>:log_path=<run>/tsan.
# Validity per launch (G3.4; a failed launch is re-run, never counted): health up with Audio-DNA on 7070, the scenario
# exited 0, alive at the end, graceful quit (no kill), 0 Output-named windows, built-in audio pre-check passed; per
# batch: 0 UserNotificationCenter windows >= 15 s after the last quit, 0 new Audio-DNA .ips. A d launch whose final
# /api/state carries the lane witnesses must show render_pending_fired >= 1 AND render_autopilot_advances >= 1.
#
# Screen-safe: open -g only (never plain open / foreground exec), no screen capture, NEVER an Output window (the
# scenarios never touch an output; outwins is checked), no synthetic input; graceful quit, kill only after 30 s.
# Every REST request is a fresh connection (Connection: close). REFUSES if Audio-DNA is already running or 7070 / 8080
# has a listener.
# PROBE RIG GATE (probehygiene2, s-rta-0926b; cloned from probe-crossfade.sh): refuses (exit 64) unless
# /tmp/audiodna-live.lock/owner exists; if AUDIODNA_LOCK_OWNER is set, it must match the owner file's first field.
# adna_pids / adna_running / adna_kill filter on `ps -o ucomm=` (the kernel's exec-time name, immune to argv[0]
# spoofing) being exactly "Audio-DNA".
set -u
# --- live-lock gate: refuse unless the caller holds /tmp/audiodna-live.lock (rig rule) ---
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held -- mkdir /tmp/audiodna-live.lock && echo \"<lane> \$\$ \$(date +%s)\" > $LOCK_OWNER_FILE first"; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
fi
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
adna_kill() { local p; p="$(adna_pids)"; [ -n "$p" ] && kill $p 2>/dev/null; }

ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
[ $# -eq 2 ] || { echo "usage: probe-tsan.sh <out-dir> \"<N:scen@arm> ...\""; exit 64; }
OUT="$1"; SPEC="$2"
PY="${TSAN_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
[ -n "$PY" ] && "$PY" -c 'import requests, Quartz' 2>/dev/null || { echo "REFUSE: no python with requests + Quartz (set TSAN_PY)"; exit 64; }
HIST="${TSAN_HISTORY:-4}"
mkdir -p "$OUT/runs" || exit 64
MEDIA="${TSAN_MEDIA:-$OUT/media}"; export TSAN_MEDIA="$MEDIA"
# Validate the whole SPEC before any launch.
for item in $SPEC; do
  N=${item%%:*}; R=${item#*:}; SC=${R%%@*}; ARM=${R#*@}
  case "$SC" in a|b|c|d|e) ;; *) echo "REFUSE: bad scenario in '$item' (a|b|c|d|e)"; exit 64;; esac
  [ "$ARM" != "$R" ] && [ -n "$ARM" ] || { echo "REFUSE: '$item' has no @arm"; exit 64; }
  V="TSAN_APP_$ARM"; APP="${!V:-}"
  [ -d "$APP" ] || { echo "REFUSE: $V is not an app bundle ('$APP')"; exit 64; }
  otool -L "$APP/Contents/MacOS/Audio-DNA" 2>/dev/null | grep -q 'libclang_rt.tsan' || { echo "REFUSE: $APP is not a TSan build (no libclang_rt.tsan)"; exit 64; }
  [ -e "$OUT/runs/tsan-$N" ] && { echo "REFUSE: $OUT/runs/tsan-$N exists (use a new N)"; exit 64; }
done

mkfix() {   # the sweep fixtures (no app involved): 16 x 1080p + 4 x 4K H.264 copies, 4 PNGs
  local F; F="$(command -v ffmpeg || echo /opt/homebrew/bin/ffmpeg)"
  [ -x "$F" ] || { echo "REFUSE: no ffmpeg for the fixtures"; return 1; }
  mkdir -p "$MEDIA" || return 1
  "$F" -y -loglevel error -f lavfi -i testsrc2=s=1920x1080:r=30:d=6 -g 60 -c:v libx264 -pix_fmt yuv420p -preset ultrafast "$MEDIA/v1080.mp4" || return 1
  "$F" -y -loglevel error -f lavfi -i testsrc2=s=3840x2160:r=30:d=4 -g 60 -c:v libx264 -pix_fmt yuv420p -preset ultrafast "$MEDIA/v4k.mp4" || return 1
  for k in $(seq 0 15); do cp -c "$MEDIA/v1080.mp4" "$MEDIA/v1080_$(printf %02d $k).mp4" || return 1; done
  for k in 0 1 2 3; do cp -c "$MEDIA/v4k.mp4" "$MEDIA/v4k_$k.mp4" || return 1; done
  for k in 0 1 2 3; do "$F" -y -loglevel error -f lavfi -i "testsrc2=s=1920x1080:r=1:d=1,hue=h=$((k*70))" -frames:v 1 "$MEDIA/img_$k.png" || return 1; done
}
[ -f "$MEDIA/v1080_15.mp4" ] && [ -f "$MEDIA/v4k_3.mp4" ] && [ -f "$MEDIA/img_3.png" ] || { echo "fixtures -> $MEDIA"; mkfix || exit 64; }

unc_all() { "$PY" -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName', ''))]))"; }
outwins() { "$PY" -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
a = [w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName', ''))]
print(len([w for w in a if 'Output' in str(w.get('kCGWindowName', ''))]), len(a))"; }
audio_check() {  # 0 = the default input AND output devices are built-in
  local d nd nb
  d=$(system_profiler SPAudioDataType 2>/dev/null | grep -B2 -A6 Default)
  nd=$(echo "$d" | grep -c "Default .*Device: Yes")
  nb=$(echo "$d" | grep -i "Transport:" | grep -vc "Built-in")
  echo "$(date +%T) audio: defaults=$nd non-builtin-transport=$nb $(echo "$d" | grep -E 'Default|Transport' | tr -s ' ' | tr '\n' ';')"
  [ "$nd" -ge 2 ] && [ "$nb" -eq 0 ]
}
quit_graceful() {  # prints "graceful=yes|no"
  osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
  for _ in $(seq 1 30); do adna_running || break; sleep 1; done
  if adna_running; then adna_kill; sleep 3; echo "graceful=no (killed after 30 s)"; else echo "graceful=yes"; fi
}

STAMP="$(date +%Y%m%d-%H%M%S)"; BLOG="$OUT/batch-$STAMP.log"
exec > >(tee -a "$BLOG") 2>&1
echo "=== probe-tsan batch $STAMP start $(date '+%F %T')  history_size=$HIST  media=$MEDIA"
ls ~/Library/Logs/DiagnosticReports 2>/dev/null | grep -i "^Audio-DNA" | sort > "$OUT/ips-before-$STAMP.txt"
echo "UNC windows (OptionAll) before: $(unc_all)"
adna_running && { echo "REFUSE: Audio-DNA already running"; exit 64; }
INVALID=0
for item in $SPEC; do
  N=${item%%:*}; R=${item#*:}; SC=${R%%@*}; ARM=${R#*@}
  V="TSAN_APP_$ARM"; APP="${!V}"
  RD="$OUT/runs/tsan-$N"; mkdir -p "$RD"
  echo "--- tsan-$N scenario $SC arm $ARM  $(date +%T)  app $APP"
  if ! audio_check > "$RD/audio-precheck.txt"; then
    cat "$RD/audio-precheck.txt"; echo "STOP: the default audio device is not built-in before tsan-$N (no launch)"
    printf 'tsan\t%s\t%s\t%s\tSKIPPED-AUDIO\n' "$N" "$SC" "$ARM" >> "$OUT/launches.tsv"; INVALID=1; break
  fi
  adna_running && { echo "REFUSE: Audio-DNA running before tsan-$N"; INVALID=1; break; }
  BAD=0; for p in 7070 8080; do lsof -nP -iTCP:$p -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: $p has a listener"; BAD=1; }; done
  [ $BAD = 1 ] && { INVALID=1; break; }
  T0=$(date +%s)
  open -g --env "TSAN_OPTIONS=halt_on_error=0:abort_on_error=0:exitcode=0:report_signal_unsafe=0:history_size=$HIST:log_path=$RD/tsan" \
       --stdout "$RD/app-out.log" --stderr "$RD/app-err.log" "$APP"
  H=""; GONE=0
  for i in $(seq 1 150); do
    H=$(curl -s --max-time 3 -H 'Connection: close' "$A/api/health"); [ -n "$H" ] && break
    adna_running || { [ $i -gt 3 ] && { GONE=1; break; }; }; sleep 1
  done
  T1=$(date +%s); echo "health after $((T1-T0)) s gone=$GONE: ${H:0:120}"; echo "$H" > "$RD/health-start.json"
  sleep 2
  L7070=$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)
  echo "7070 listener: $L7070; pids: $(adna_pids | tr '\n' ' '); Output-named / Audio-DNA windows: $(outwins)"
  SRC=na
  if [ -n "$H" ] && [ "$L7070" = "Audio-DNA" ]; then
    perl -e 'alarm 240; exec @ARGV' "$PY" "$ROOT/.harmony/probe-tsan.py" "$SC" "$RD" </dev/null > "$RD/scen.out" 2>&1; SRC=$?
    echo "scenario rc=$SRC"
  else echo "no scenario (the app is not up)"; fi
  ALIVE=$(adna_running && echo yes || echo no)
  HEND=$(curl -s --max-time 4 -H 'Connection: close' "$A/api/health"); echo "$HEND" > "$RD/health-end.json"
  OW=$(outwins); OWN=${OW%% *}
  echo "alive at end: $ALIVE; Output-named / Audio-DNA windows: $OW; health end: ${HEND:0:100}"
  Q=$(quit_graceful); echo "$Q" | tee "$RD/quit.txt"
  echo "Output-named / Audio-DNA windows after quit: $(outwins)"
  NREP=$(ls "$RD" | grep -cE '^tsan\.'); NW=$(cat "$RD"/tsan.* 2>/dev/null | grep -c 'WARNING: ThreadSanitizer')
  CNT=$("$PY" -c "
import json, sys
try: j = json.load(open('$RD/state.json'))
except Exception: print('na\tna\tna'); sys.exit(0)
print('\t'.join(str(j.get(k, 'na')) for k in ('render_pending_fired', 'render_autopilot_advances', 'render_tuple_adopts')))")
  IFS=$'\t' read -r PF AA TA <<< "$CNT"
  VALID=yes; WHY=""
  [ -n "$H" ] && [ "$L7070" = "Audio-DNA" ] || { VALID=no; WHY="$WHY no-health"; }
  [ "$SRC" = "0" ] || { VALID=no; WHY="$WHY scenario-rc=$SRC"; }
  [ "$ALIVE" = yes ] || { VALID=no; WHY="$WHY died"; }
  [ "$Q" = "graceful=yes" ] || { VALID=no; WHY="$WHY not-graceful"; }
  [ "$OWN" = "0" ] || { VALID=no; WHY="$WHY output-window"; }
  if [ "$SC" = d ] && [ "$PF" != na ]; then   # the lane witnesses exist: G3.4's render-writer bar
    [ "$PF" -ge 1 ] 2>/dev/null && [ "$AA" -ge 1 ] 2>/dev/null || { VALID=no; WHY="$WHY d-counters(pending=$PF,advances=$AA)"; }
  fi
  [ "$VALID" = yes ] || INVALID=1
  echo "VALIDITY tsan-$N $SC@$ARM: $VALID${WHY:+ (${WHY# })}  warnings=$NW render_pending_fired=$PF render_autopilot_advances=$AA render_tuple_adopts=$TA"
  printf 'tsan\t%s\t%s\t%s\thealthS=%s\tgone=%s\talive_end=%s\t%s\tsanfiles=%s\twarnings=%s\toutput_named=%s\tvalid=%s\tpending_fired=%s\tautopilot_advances=%s\ttuple_adopts=%s\n' \
    "$N" "$SC" "$ARM" "$((T1-T0))" "$GONE" "$ALIVE" "$Q" "$NREP" "$NW" "$OWN" "$VALID" "$PF" "$AA" "$TA" >> "$OUT/launches.tsv"
  sleep 3
done
echo "last quit $(date +%T)"; sleep 16
UNC=$(unc_all); echo "UNC windows (OptionAll) >= 15 s after the last quit: $UNC"
ls ~/Library/Logs/DiagnosticReports 2>/dev/null | grep -i "^Audio-DNA" | sort > "$OUT/ips-after-$STAMP.txt"
NEWIPS=$(comm -13 "$OUT/ips-before-$STAMP.txt" "$OUT/ips-after-$STAMP.txt" | tr '\n' ' ')
echo "new Audio-DNA .ips: ${NEWIPS:-none}"
[ "$UNC" = "0" ] && [ -z "$NEWIPS" ] || INVALID=1
echo "=== probe-tsan batch $STAMP done $(date '+%F %T')  batch-valid=$([ $INVALID = 0 ] && echo yes || echo no)"
[ $INVALID = 0 ]
