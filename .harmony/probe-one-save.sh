#!/bin/bash
# probe-one-save.sh -- lane one-save (s-rta-1004b; ruling-one-save.md section 5 "LIVE ROWS"): ONE SAVE -- the show
# file carries "version": 2, is written by one verified writer, and any existing file that is not a version-2 show is
# copied to backups/ before it is written over. RUN ONLY BY HARMONY: a builder never launches the app.
# Rows, fixtures and RED arms: the docstring of .harmony/probe-one-save.py (REST on 7070, Python stdlib only). Stage S1
# rows: os_l13, os_l14, os_l14b (gate G-OS1) and os_l21 (= M-1, the full-disk run; needs ONESAVE_FULLDISK). Later stages
# add their rows to the .py and re-pin EXPECTED_ROWS.
# Self-test WITHOUT the app (a stub server; every row green on the ruled behaviour, red on its mutant):
#   bash .harmony/probe-one-save-selftest.sh
#
# TEST MODE launch (`open -g ... --args --test-mode`; the /api/debug/* routes are test-mode only) with a SCRATCH
# settings file in the run's out dir (AUDIODNA_SETTINGS_FILE). AUDIODNA_LIBRARY_DIR is passed too, but NOTHING in the
# app reads it before stage S4b: until then the launched app still READS the user's library -- it lists
# ~/Library/AudioDNA/compositions and parses every ~/Library/AudioDNA/decks/*.json (CompDecksBrowser::scanForFiles /
# isV2DeckFile) at launch and after every successful save. It never WRITES there: this probe names neither
# ~/Library/AudioDNA nor ~/Library/Audio-DNA and points no route at them. The guard on his files is Harmony's
# checksums (G-OS4-0), not this probe.
# Screen-safe: open -g (never plain open / foreground exec), no screen capture of any kind, no Output window, no
# synthetic input, no window of the quit flow. QUITS ONLY THE APP IT LAUNCHED (quit_ours, .harmony/probe-quit-ours.sh):
# graceful quit, kill of that one pid only if still running after 30 s. REFUSES if any Audio-DNA is already running
# (it may be the user's). The caller holds /tmp/audiodna-live.lock (PROBE RIG GATE below).
#
# A row prints exactly "PASS  OS-L<n>: <facts>" or "FAIL  OS-L<n>: <facts>" (os_l21 without its disk: "INFO  OS-L21:
# ..."). After the batch: no Audio-DNA process, 0 on-screen Audio-DNA / UserNotificationCenter windows (Quartz count).
# Final line: "PROBE-ONE-SAVE GREEN" (exit 0) only when no row FAILED, the app answered, quit cleanly and left no
# window; "PROBE-ONE-SAVE RED" (exit 1 / 64) otherwise. ROW-COUNT PIN: a FULL run (no row list) must print exactly
# EXPECTED_ROWS "--- <row>" headers and the .py must register that many.
# usage: probe-one-save.sh [out-base] [row,row,...]
#   ONESAVE_APP       app bundle to launch (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   ONESAVE_PY        python3 for the rows (stdlib only; default: python3 on PATH)
#   ONESAVE_QPY       python with the Quartz module for the window count (default: <root>/.venv, else the main
#                     checkout's .venv)
#   ONESAVE_FULLDISK  os_l21: the mount point of the 2 MB disk image Harmony made (hdiutil ... -nobrowse -mountpoint)
# Every run writes into a FRESH dir: mktemp -d "<out-base>/onesave.XXXXXX" (out-base default /tmp); the app's stdout /
# stderr go to out.log / err.log there.
# PROBE RIG GATE: refuses (exit 64) unless /tmp/audiodna-live.lock/owner exists; if AUDIODNA_LOCK_OWNER is set, it
# must match the owner file's first field. Process matching by the kernel's ucomm (adna_pids), never pgrep -f.
set -u
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held -- mkdir /tmp/audiodna-live.lock && echo \"<lane> \$\$ \$(date +%s)\" > $LOCK_OWNER_FILE first"; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
fi
EXPECTED_ROWS=4   # S1: os_l13, os_l14, os_l14b, os_l21. A new row re-pins this number.
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
. "$ROOT/.harmony/probe-quit-ours.sh"   # refuse_foreign_start / record_ourpid / quit_ours: quit ONLY the app this run launched
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${ONESAVE_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${ONESAVE_PY:-python3}"
QPY="${ONESAVE_QPY:-}"
if [ -z "$QPY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { QPY="$c"; break; }; done; fi
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set ONESAVE_APP)"; exit 64; }
"$PY" -c 'import sys; sys.exit(0 if sys.version_info >= (3, 8) else 1)' 2>/dev/null || { echo "REFUSE: no python3 >= 3.8 (set ONESAVE_PY)"; exit 64; }
refuse_foreign_start || exit 64
[ -n "$QPY" ] && "$QPY" -c 'import Quartz' 2>/dev/null || { echo "REFUSE: no python with Quartz for the window count (set ONESAVE_QPY)"; exit 64; }
for PORT in 7070 8080; do
  lsof -nP -iTCP:$PORT -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $PORT already has a listener: $(lsof -nP -iTCP:$PORT -sTCP:LISTEN | tail -n +2 | awk '{print $1" "$2}' | head -2 | tr '\n' ' ')"; exit 64; }
done
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/onesave.XXXXXX")" || exit 64
ROWS="${2:-}"
mkdir -p "$OUT/library"
echo "app: $APP"; echo "out: $OUT"
win_count() {   # on-screen windows owned by Audio-DNA or UserNotificationCenter (Quartz); "?" = could not count
  "$QPY" -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
print(len([w for w in wl if str(w.get('kCGWindowOwnerName', '')) in ('Audio-DNA', 'UserNotificationCenter')]))" 2>/dev/null || echo "?"
}
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" --env "AUDIODNA_SETTINGS_FILE=$OUT/settings.json" --env "AUDIODNA_LIBRARY_DIR=$OUT/library" "$APP" --args --test-mode
record_ourpid; echo "ours: pid ${OURPID:-none}"
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 -H 'Connection: close' "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
if [ "$UP" -eq 1 ] && [ "$L7070" != "Audio-DNA" ]; then echo "FAIL  port 7070 is answered by '$L7070', not Audio-DNA"; UP=0; fi
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-one-save.py" "$ROOT" "$OUT" "$ROWS" | tee "$OUT/py.log"; RC=${PIPESTATUS[0]}
else echo "FAIL  app never answered /api/health"; fi
quit_ours || RC=1
if ours_running; then echo "FAIL  app still running (pid $OURPID)"; RC=1; else echo "PASS  app terminated"; fi
if [ -n "$(adna_pids)" ]; then echo "FAIL  an Audio-DNA process is running after the batch (pid $(adna_pids | tr '\n' ' '))"; RC=1; else echo "PASS  no Audio-DNA process after the batch"; fi
sleep 1
W="$(win_count)"
if [ "$W" = "0" ]; then echo "PASS  0 on-screen Audio-DNA / UserNotificationCenter windows"; else echo "FAIL  on-screen Audio-DNA / UserNotificationCenter windows after the batch: $W"; RC=1; fi
NRUN="$(grep -c '^--- ' "$OUT/py.log" 2>/dev/null)"; NRUN="${NRUN:-0}"
NREG="$(sed -n 's/^PY-ROWS registered \([0-9][0-9]*\)$/\1/p' "$OUT/py.log" 2>/dev/null | tail -1)"
if [ -z "$ROWS" ]; then
  if [ "$NRUN" -eq "$EXPECTED_ROWS" ] && [ "${NREG:-0}" -eq "$EXPECTED_ROWS" ]; then echo "PASS  rows run $NRUN == EXPECTED_ROWS $EXPECTED_ROWS (registered ${NREG:-?})"
  else echo "FAIL  rows run $NRUN, registered ${NREG:-?} != EXPECTED_ROWS $EXPECTED_ROWS (full run)"; RC=1; fi
else echo "      rows run $NRUN of $EXPECTED_ROWS (row list given: no row-count pin)"; fi
echo
if [ "$RC" -eq 0 ]; then echo "PROBE-ONE-SAVE GREEN"; else echo "PROBE-ONE-SAVE RED"; fi
exit "$RC"
