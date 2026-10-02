#!/bin/bash
# probe-idle-paint.sh -- s-rta-0928b idlepaint (.harmony/.reports/s-rta-0928b/plan-idlepaint.md section 3.2 + HARMONY
# ADOPTION I1-I8). Live witness that the message thread is quiet at idle: JUCE 8's mac peer redraws the UNION of every
# rect repainted since the last vblank, so four always-animating widgets at opposite window edges used to repaint the
# whole window 30 times a second (Pitfall 57). Rows, fixtures and thresholds: the docstring of .harmony/probe-idle-paint.py
# and .harmony/probe-idle-paint.json.
#
# Clone of .harmony/probe-seq-vram.sh's header (live-lock gate / refuse / port check / venv discovery / fresh out dir /
# final quit) -- but the .py OWNS launching: every gate row is a >= 5-launch arm and the identity rows alternate two
# bundles. Screen-safe: open -g only (never plain open), window-only captures by Quartz window id (never full screen),
# no Output window, no synthetic input; graceful quit, kill only if still running after 30 s. REFUSES if Audio-DNA is
# already running. The caller holds /tmp/audiodna-live.lock (PROBE RIG GATE below).
#
# usage: probe-idle-paint.sh [out-base] [row,row,...]
#   IDLEPAINT_APP         the app under test (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   IDLEPAINT_APP_BEFORE  the pre-change app for the identity rows v1 / v3 (default: the MAIN checkout's build)
#   IDLEPAINT_PY          python with PIL + numpy + requests + pyobjc Quartz (default: <root>/.venv, else the main .venv)
# Every run writes into a FRESH dir: mktemp -d "<out-base>/idlepaint.XXXXXX" (out-base default /tmp).
# PROBE RIG GATE: refuses (exit 64) unless /tmp/audiodna-live.lock/owner exists; if AUDIODNA_LOCK_OWNER is set, it must
# match the owner file's first field. Process matching by the kernel's ucomm (adna_pids), never pgrep -f.
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
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${IDLEPAINT_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
APPB="${IDLEPAINT_APP_BEFORE:-$MAIN/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${IDLEPAINT_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set IDLEPAINT_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests, Quartz' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests+Quartz (set IDLEPAINT_PY)"; exit 64; }
adna_running && { echo "REFUSE: Audio-DNA already running"; exit 64; }
for p in 7070 8080; do
  lsof -nP -iTCP:$p -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $p already has a listener: $(lsof -nP -iTCP:$p -sTCP:LISTEN | tail -n +2 | awk '{print $1" "$2}' | head -2 | tr '\n' ' ')"; exit 64; }
done
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/idlepaint.XXXXXX")" || exit 64
echo "app: $APP"; echo "app before: $APPB"; echo "out: $OUT"; echo "load: $(sysctl -n vm.loadavg)"
IDLEPAINT_APP="$APP" IDLEPAINT_APP_BEFORE="$APPB" "$PY" "$ROOT/.harmony/probe-idle-paint.py" "$ROOT" "$OUT" "${2:-}"; RC=$?
if adna_running; then
  osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
  for _ in $(seq 1 30); do adna_running || break; sleep 1; done
  adna_running && { adna_kill; sleep 2; }
fi
if adna_running; then echo "FAIL  app still running"; RC=1; else echo "PASS  app terminated"; fi
rm -rf "$OUT/media"
echo; [ "$RC" -eq 0 ] && echo "PROBE-IDLE-PAINT GREEN" || echo "PROBE-IDLE-PAINT RED"
exit "$RC"
