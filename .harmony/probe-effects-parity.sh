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
# The caller holds /tmp/audiodna-live.lock.
#
# usage: probe-effects-parity.sh [out-base]
#   PARITY_APP        app bundle to launch (default: <root>/${PARITY_BUILD_DIR:-build}/AudioDNA_artefacts/Release/Audio-DNA.app)
#   PARITY_PY         python with PIL+numpy+requests (default: <root>/.venv, else the main checkout's .venv)
# Every run captures into a FRESH dir: mktemp -d "<out-base>/parity.XXXXXX" (out-base default /tmp).
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${PARITY_APP:-$ROOT/${PARITY_BUILD_DIR:-build}/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${PARITY_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
MEDIA="$ROOT/media"; [ -f "$MEDIA/P16_01_baseline.png" ] || MEDIA="$MAIN/media"
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set PARITY_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests (set PARITY_PY)"; exit 64; }
[ -f "$MEDIA/P16_01_baseline.png" ] || { echo "REFUSE: media/P16_01_baseline.png not found"; exit 64; }
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: Audio-DNA already running"; exit 64; }
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/parity.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" "$APP"
UP=0; for _ in $(seq 1 40); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-effects-parity.py" "$ROOT" "$OUT" "$MEDIA"; RC=$?
else echo "FAIL  app never answered /api/health"; fi
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { pkill -f 'MacOS/Audio-DN[A]'; sleep 2; }
if pgrep -f 'MacOS/Audio-DN[A]' >/dev/null; then echo "FAIL  app still running"; RC=1; else echo "PASS  app terminated"; fi
echo; [ "$RC" -eq 0 ] && echo "PROBE-EFFECTS-PARITY GREEN" || echo "PROBE-EFFECTS-PARITY RED"
exit "$RC"
