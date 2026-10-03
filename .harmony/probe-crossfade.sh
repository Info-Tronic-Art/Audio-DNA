#!/bin/bash
# probe-crossfade.sh -- s-rta-0926 xfade lane. Live witness for the clip-to-clip crossfade bug (a Dissolve
# between two clips that BOTH have clip effects held the outgoing clip for the whole transition, then
# hard-cut) and for the rest of its bug class: a texture held by the compositor while a later pass writes
# the same FBO (10 cases: 6 crossfades a-f, 4 single-state g-j). s-rta-0926b render lane added crossfades
# k-l (R3: the OUTGOING clip must keep its transform and its opacity-before-effects order). Cases, fixtures
# and thresholds: .harmony/probe-crossfade.json + the docstring of
# .harmony/probe-crossfade.py (which does the REST calls and decodes every PNG with PIL+numpy).
#
# Screen-safe: open -g (never plain open / foreground exec), no screen capture, no Output window, no
# synthetic input; quit_ours: graceful quit of the launched pid, kill of that pid after 30 s. REFUSES if Audio-DNA is already
# running. The caller holds /tmp/audiodna-live.lock (now enforced -- see PROBE RIG GATE below).
#
# usage: probe-crossfade.sh [out-base] [case,case,...]
#   XFADE_APP  app bundle to launch (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   XFADE_PY   python with PIL+numpy+requests (default: <root>/.venv, else the main checkout's .venv)
#   XFADE_ENV  optional VAR=value passed to the app via open --env (e.g. AUDIODNA_FBO_TRACE=1 on a build with
#              .harmony/.reports/s-rta-0926/xfade-trace.diff applied; the trace lands in <out>/err.log)
# Every run captures into a FRESH dir: mktemp -d "<out-base>/xfade.XXXXXX" (out-base default /tmp).
# PROBE RIG GATE (probehygiene2, s-rta-0926b): refuses (exit 64) unless
# /tmp/audiodna-live.lock/owner exists; if AUDIODNA_LOCK_OWNER is set, it must match the owner
# file's first field. Every pgrep below is replaced by adna_pids/adna_running; every quit is quit_ours
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
# run -- immune to argv[0] spoofing) is exactly "Audio-DNA". adna_running builds on it.
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
. "$ROOT/.harmony/probe-quit-ours.sh"   # refuse_foreign_start / record_ourpid / quit_ours: quit ONLY the app this run launched
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${XFADE_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${XFADE_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
MEDIA="$ROOT/media"; [ -f "$MEDIA/P16_01_baseline.png" ] || MEDIA="$MAIN/media"
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set XFADE_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests (set XFADE_PY)"; exit 64; }
[ -f "$MEDIA/P16_01_baseline.png" ] || { echo "REFUSE: media/P16_01_baseline.png not found"; exit 64; }
refuse_foreign_start || exit 64
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/xfade.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
ENVARGS=(); [ -n "${XFADE_ENV:-}" ] && ENVARGS=(--env "$XFADE_ENV")
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" ${ENVARGS[@]+"${ENVARGS[@]}"} "$APP"
record_ourpid; echo "ours: pid ${OURPID:-none}"
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-crossfade.py" "$ROOT" "$OUT" "$MEDIA" "${2:-}"; RC=$?
else echo "FAIL  app never answered /api/health"; fi
quit_ours || RC=1
if adna_running; then echo "FAIL  app still running"; RC=1; else echo "PASS  app terminated"; fi
echo; [ "$RC" -eq 0 ] && echo "PROBE-CROSSFADE GREEN" || echo "PROBE-CROSSFADE RED"
exit "$RC"
