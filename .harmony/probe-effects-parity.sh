#!/bin/bash
# probe-effects-parity.sh -- s-rta-0926 live witness for the clip-effects -> layer-effects FBO aliasing bug
# (ms-white-diagnosis-2.md finding B): ONE per-clip effect + ONE layer effect, opacity 1.0, no transform,
# rendered all-zero RGBA before the fix (4fca2c5). Hardened s-rta-0926 xfade lane (parity-diagnosis.md
# section 8): every render_frame response is checked, captures go into a FRESH dir, 6 frames per variant,
# V5 (1 clip + 2 layer) and V6 (3 clip + 1 layer) rows, and a PARITY-TO-REFERENCE row per variant (the
# clip+layer frame must equal the same effects run as one clip chain). Rows/thresholds: the docstring of
# .harmony/probe-effects-parity.py. Pixel-decoded (never file hashes).
#
# s-rta-0926 cleanup lane: added a G1 row for the GLOBAL, single-image EffectChain
# (src/effects/EffectChain.cpp) via /api/load_image + /api/set_effect_chain -- V1/V5/V6 above never touch
# that code path. Runs first, before any load_composition. See the .py docstring's G1 section, including
# PARITY-DOCUMENTS-GAP: /api/set_effect_chain has no dryWet field today, so the mid-chain dry/wet fix's
# own numeric assertion can't be driven live without an API change (out of this lane's fence).
#
# Screen-safe: open -g (never plain open / foreground exec), no screen capture, no Output window, no synthetic
# input; graceful quit, pkill only if still running after 30 s. REFUSES if Audio-DNA is already running.
# The caller holds /tmp/audiodna-live.lock (now enforced -- see PROBE RIG GATE below).
#
# usage: probe-effects-parity.sh [out-base]
#   PARITY_APP        app bundle to launch (default: <root>/${PARITY_BUILD_DIR:-build}/AudioDNA_artefacts/Release/Audio-DNA.app)
#   PARITY_PY         python with PIL+numpy+requests (default: <root>/.venv, else the main checkout's .venv)
# Every run captures into a FRESH dir: mktemp -d "<out-base>/parity.XXXXXX" (out-base default /tmp).
# PROBE RIG GATE (probehygiene2, s-rta-0926b): refuses (exit 64) unless
# /tmp/audiodna-live.lock/owner exists; if AUDIODNA_LOCK_OWNER is set, it must match the owner
# file's first field. Every pgrep/pkill below is replaced by adna_pids/adna_running/adna_kill
# (defined right after the lock gate), which filter on `ps -o ucomm=` -- the KERNEL's real
# exec-time process name, set from the actual binary that was exec'd, NOT from argv[0] -- being
# exactly "Audio-DNA". Verified both directions: (1) a build's linker/compiler command line whose
# -o argument is ".../MacOS/Audio-DNA" has REAL ucomm clang/ld, never matched (the old `pgrep -f`
# substring match DID match it -- that was the bug); (2) a process that spoofs argv[0] via
# `exec -a .../MacOS/Audio-DNA <cmd>` still has the REAL exec'd command's ucomm (e.g. "sleep"), also
# never matched -- `pgrep -x` alone is NOT enough here, since macOS pgrep without -f still matches
# on the (spoofable) comm/argv[0] field, not on ucomm.
set -u
# --- live-lock gate: refuse unless the caller holds /tmp/audiodna-live.lock (rig rule) ---
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held -- mkdir /tmp/audiodna-live.lock && echo \"<lane> \$\$ \$(date +%s)\" > $LOCK_OWNER_FILE first"; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
fi
# adna_pids: PIDs whose REAL kernel process name (ucomm, set at exec() time from the actual binary
# run -- immune to argv[0] spoofing) is exactly "Audio-DNA". adna_running/adna_kill build on it.
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
adna_kill() { local p; p="$(adna_pids)"; [ -n "$p" ] && kill $p 2>/dev/null; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${PARITY_APP:-$ROOT/${PARITY_BUILD_DIR:-build}/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${PARITY_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
MEDIA="$ROOT/media"; [ -f "$MEDIA/P16_01_baseline.png" ] || MEDIA="$MAIN/media"
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set PARITY_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests (set PARITY_PY)"; exit 64; }
[ -f "$MEDIA/P16_01_baseline.png" ] || { echo "REFUSE: media/P16_01_baseline.png not found"; exit 64; }
adna_running && { echo "REFUSE: Audio-DNA already running"; exit 64; }
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/parity.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" "$APP"
UP=0; for _ in $(seq 1 40); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-effects-parity.py" "$ROOT" "$OUT" "$MEDIA"; RC=$?
else echo "FAIL  app never answered /api/health"; fi
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do adna_running || break; sleep 1; done
adna_running && { adna_kill; sleep 2; }
if adna_running; then echo "FAIL  app still running"; RC=1; else echo "PASS  app terminated"; fi
echo; [ "$RC" -eq 0 ] && echo "PROBE-EFFECTS-PARITY GREEN" || echo "PROBE-EFFECTS-PARITY RED"
exit "$RC"
