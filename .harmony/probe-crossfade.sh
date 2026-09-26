#!/bin/bash
# probe-crossfade.sh -- s-rta-0926 xfade lane. Live witness for the clip-to-clip crossfade bug (a Dissolve
# between two clips that BOTH have clip effects held the outgoing clip for the whole transition, then
# hard-cut) and for the rest of its bug class: a texture held by the compositor while a later pass writes
# the same FBO (10 cases: 6 crossfades a-f, 4 single-state g-j). Cases, fixtures and thresholds:
# .harmony/probe-crossfade.json + the docstring of
# .harmony/probe-crossfade.py (which does the REST calls and decodes every PNG with PIL+numpy).
#
# Screen-safe: open -g (never plain open / foreground exec), no screen capture, no Output window, no
# synthetic input; graceful quit, pkill only if still running after 30 s. REFUSES if Audio-DNA is already
# running. The caller holds /tmp/audiodna-live.lock.
#
# usage: probe-crossfade.sh [out-base] [case,case,...]
#   XFADE_APP  app bundle to launch (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   XFADE_PY   python with PIL+numpy+requests (default: <root>/.venv, else the main checkout's .venv)
#   XFADE_ENV  optional VAR=value passed to the app via open --env (e.g. AUDIODNA_FBO_TRACE=1 on a build with
#              .harmony/.reports/s-rta-0926/xfade-trace.diff applied; the trace lands in <out>/err.log)
# Every run captures into a FRESH dir: mktemp -d "<out-base>/xfade.XXXXXX" (out-base default /tmp).
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${XFADE_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${XFADE_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
MEDIA="$ROOT/media"; [ -f "$MEDIA/P16_01_baseline.png" ] || MEDIA="$MAIN/media"
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set XFADE_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests (set XFADE_PY)"; exit 64; }
[ -f "$MEDIA/P16_01_baseline.png" ] || { echo "REFUSE: media/P16_01_baseline.png not found"; exit 64; }
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: Audio-DNA already running"; exit 64; }
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/xfade.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
ENVARGS=(); [ -n "${XFADE_ENV:-}" ] && ENVARGS=(--env "$XFADE_ENV")
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" ${ENVARGS[@]+"${ENVARGS[@]}"} "$APP"
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-crossfade.py" "$ROOT" "$OUT" "$MEDIA" "${2:-}"; RC=$?
else echo "FAIL  app never answered /api/health"; fi
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { pkill -f 'MacOS/Audio-DN[A]'; sleep 2; }
if pgrep -f 'MacOS/Audio-DN[A]' >/dev/null; then echo "FAIL  app still running"; RC=1; else echo "PASS  app terminated"; fi
echo; [ "$RC" -eq 0 ] && echo "PROBE-CROSSFADE GREEN" || echo "PROBE-CROSSFADE RED"
exit "$RC"
