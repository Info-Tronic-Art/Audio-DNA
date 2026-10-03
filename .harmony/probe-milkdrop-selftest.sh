#!/bin/bash
# probe-milkdrop-selftest.sh -- s-rta-1002b lane bf10 fix round (Harmony rulings R1 / R2 on review-bf10-gates-r1).
# Proves, with NO app launched and NO request sent to any app:
#   R1  probe-milkdrop.sh MILKDROP_ATTACH=1 attaches ONLY to the test-mode app this run's harness started: the one
#       running Audio-DNA pid must equal MILKDROP_ATTACH_PID, or else the pid the lock helper's start_app recorded
#       (LOCK_LIB + LANE); its pid must own the 8080 listener and 8080 /api/health must answer. Otherwise it prints
#       "REFUSE: the running Audio-DNA is not a test-mode app this run started (it may be Boris's)" and exits 2 before
#       any request (the pid / listener checks send nothing; /api/health is asked only once both match).
#   R2  MILKDROP_P1 replaces the pinned P1 only in calibration mode (MILKDROP_MODE=pre); in lane (gate) mode the probe
#       exits 2; every probe-milkdrop.py run prints "P1 = <preset path>" first.
# How: the REAL probe-milkdrop.sh runs with PATH shims for ps / lsof / curl (a fake Audio-DNA pid table, a fake 8080
# listener, a fake /api/health that logs every call) and MILKDROP_PY = a stub python that only records whether the
# probe body would have run -- the body (probe-milkdrop.py) is where every 7070 / 8080 request is sent. The py cases
# run the REAL probe-milkdrop.py with a row name that does not exist, so neither version sends a request.
# Needs: the live lock held by the caller (probe-milkdrop.sh's own lock gate runs first -- nothing is launched while it
# is held); a python with numpy + requests + PIL for the py cases (SELFTEST_PY, else <root>/.venv, else the main
# checkout's .venv).
# usage: probe-milkdrop-selftest.sh [probe-milkdrop.sh] [probe-milkdrop.py]   (default: the siblings of this file)
# Prints one "ok" / "FAIL" line per check, then "SELFTEST <n> ok / <m> FAIL"; exit 0 iff no FAIL.
set -u
[ -f /tmp/audiodna-live.lock/owner ] || { echo "REFUSE: no live lock held (probe-milkdrop.sh's lock gate would refuse every case) -- take it with the lock helper first"; exit 64; }
HERE="$(cd "$(dirname "$0")" && pwd)"
SH="${1:-$HERE/probe-milkdrop.sh}"; PYF="${2:-$HERE/probe-milkdrop.py}"
ROOT="$(cd "$HERE/.." && pwd)"
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
RPY="${SELFTEST_PY:-}"
if [ -z "$RPY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { RPY="$c"; break; }; done; fi
[ -n "$RPY" ] && "$RPY" -c 'import numpy, requests, PIL' 2>/dev/null || { echo "REFUSE: no python with numpy+requests+PIL (set SELFTEST_PY)"; exit 64; }
echo "target sh: $SH"; echo "target py: $PYF"
D="$(mktemp -d "${TMPDIR:-/tmp}/md-selftest.XXXXXX")" || exit 64
mkdir -p "$D/shim" "$D/out"
# ps: the probe's adna_pids runs `ps -eo pid=,ucomm=`; SHIM_PIDS lists fake Audio-DNA pids.
cat > "$D/shim/ps" <<'EOF'
#!/bin/bash
for p in ${SHIM_PIDS:-}; do echo "$p Audio-DNA"; done
EOF
# lsof: -iTCP:7070 / -iTCP:8080 listeners; -t prints the pid only (SHIM_L8080 = the 8080 listener's pid).
cat > "$D/shim/lsof" <<'EOF'
#!/bin/bash
port=""; terse=0
for a in "$@"; do case "$a" in -iTCP:7070) port=7070;; -iTCP:8080) port=8080;; -t) terse=1;; esac; done
if [ "$port" = 7070 ] && [ -n "${SHIM_L7070:-}" ]; then printf 'COMMAND PID\n%s %s\n' "$SHIM_L7070" "${SHIM_PIDS%% *}"; exit 0; fi
if [ "$port" = 8080 ] && [ -n "${SHIM_L8080:-}" ]; then
  if [ $terse = 1 ]; then echo "$SHIM_L8080"; else printf 'COMMAND PID\nAudio-DNA %s\n' "$SHIM_L8080"; fi; exit 0
fi
exit 1
EOF
# curl: logs every call; answers /api/health iff SHIM_HEALTH=1.
cat > "$D/shim/curl" <<'EOF'
#!/bin/bash
echo "$*" >> "$SHIM_DIR/curl-calls"
[ "${SHIM_HEALTH:-0}" = 1 ] && { echo '{"status":"ok"}'; exit 0; }
exit 7
EOF
# stub python for the .sh cases: the import check passes; the probe body only leaves a mark.
cat > "$D/stubpy" <<'EOF'
#!/bin/bash
[ "${1:-}" = -c ] && exit 0
echo "BODY $*" >> "$SHIM_DIR/py-invoked"; exit 0
EOF
# a lock helper with the real one's contract: sourcing it with LANE set names OURPID = start_app's pid record.
cat > "$D/fake-lock.sh" <<EOF
: "\${LANE:?set LANE}"
OURPID=$D/.ours-pid-\$LANE
EOF
chmod +x "$D/shim/ps" "$D/shim/lsof" "$D/shim/curl" "$D/stubpy"
OK=0; BAD=0
check(){ if [ "$1" = 1 ]; then OK=$((OK+1)); echo "   ok    $2"; else BAD=$((BAD+1)); echo "   FAIL  $2"; fi; }
REFUSE_R1="REFUSE: the running Audio-DNA is not a test-mode app this run started (it may be Boris's)"
# run_sh <name> <expect: refuse|body|p1refuse> [VAR=value ...] -- the real probe-milkdrop.sh, attach mode, shims first.
run_sh(){
  local name=$1 want=$2; shift 2
  rm -f "$D/curl-calls" "$D/py-invoked"
  local out rc
  out="$(env -u MILKDROP_ATTACH_PID -u MILKDROP_P1 -u MILKDROP_MODE -u LOCK_LIB -u MILKDROP_APP -u VIDEO_APP \
        PATH="$D/shim:$PATH" SHIM_DIR="$D" MILKDROP_ATTACH=1 MILKDROP_PY="$D/stubpy" SHIM_L7070=Audio-DNA "$@" \
        bash "$SH" "$D/out" 2>&1)"; rc=$?
  local body=0 curls=0
  [ -f "$D/py-invoked" ] && body=1
  [ -f "$D/curl-calls" ] && curls=$(wc -l < "$D/curl-calls" | tr -d ' ')
  echo "-- $name: rc $rc, probe body ran $body, curl calls $curls"
  case "$want" in
    refuse)
      check "$([ $rc = 2 ] && echo 1)" "$name: exit 2 (got $rc)"
      check "$(grep -qF "$REFUSE_R1" <<< "$out" && echo 1)" "$name: prints the R1 REFUSE line"
      check "$([ $body = 0 ] && echo 1)" "$name: the probe body (every 7070 / 8080 request) never ran";;
    refuse_nocurl)
      check "$([ $rc = 2 ] && echo 1)" "$name: exit 2 (got $rc)"
      check "$(grep -qF "$REFUSE_R1" <<< "$out" && echo 1)" "$name: prints the R1 REFUSE line"
      check "$([ $body = 0 ] && echo 1)" "$name: the probe body (every 7070 / 8080 request) never ran"
      check "$([ "$curls" = 0 ] && echo 1)" "$name: no request at all (not even /api/health; got $curls)";;
    body)
      check "$([ $rc = 0 ] && echo 1)" "$name: exit 0 (got $rc)"
      check "$([ $body = 1 ] && echo 1)" "$name: the probe body ran (our own test-mode app)";;
    p1refuse)
      check "$([ $rc = 2 ] && echo 1)" "$name: exit 2 (got $rc)"
      check "$(grep -q 'REFUSE: MILKDROP_P1' <<< "$out" && echo 1)" "$name: prints the MILKDROP_P1 REFUSE line"
      check "$([ $body = 0 ] && echo 1)" "$name: the probe body never ran";;
  esac
  [ "$BAD" -gt 0 ] && [ -n "${SELFTEST_VERBOSE:-}" ] && echo "$out" | sed 's/^/      | /'
  return 0
}
echo "R1 -- MILKDROP_ATTACH=1"
run_sh "a1 foreign app, no record, no MILKDROP_ATTACH_PID" refuse_nocurl SHIM_PIDS=4242 SHIM_L8080=4242 SHIM_HEALTH=1
run_sh "a2 production app (no 8080), MILKDROP_ATTACH_PID matches" refuse_nocurl SHIM_PIDS=4242 MILKDROP_ATTACH_PID=4242
run_sh "a3 MILKDROP_ATTACH_PID != the running pid" refuse_nocurl SHIM_PIDS=4242 SHIM_L8080=4242 SHIM_HEALTH=1 MILKDROP_ATTACH_PID=5555
echo 5555 > "$D/.ours-pid-selftest"
run_sh "a4 start_app record != the running pid" refuse_nocurl SHIM_PIDS=4242 SHIM_L8080=4242 SHIM_HEALTH=1 LOCK_LIB="$D/fake-lock.sh" LANE=selftest
run_sh "a5 8080 listener is ours but /api/health is silent" refuse SHIM_PIDS=4242 SHIM_L8080=4242 SHIM_HEALTH=0 MILKDROP_ATTACH_PID=4242
run_sh "a6 two Audio-DNA running" refuse_nocurl "SHIM_PIDS=4242 4343" SHIM_L8080=4242 SHIM_HEALTH=1 MILKDROP_ATTACH_PID=4242
run_sh "a7 LOCK_LIB set but no record for this LANE" refuse_nocurl SHIM_PIDS=4242 SHIM_L8080=4242 SHIM_HEALTH=1 LOCK_LIB="$D/fake-lock.sh" LANE=nolane
run_sh "a8 8080 listener is another pid" refuse_nocurl SHIM_PIDS=4242 SHIM_L8080=7777 SHIM_HEALTH=1 MILKDROP_ATTACH_PID=4242
echo 4242 > "$D/.ours-pid-selftest"
run_sh "p1 ours via the start_app record (LOCK_LIB + LANE)" body SHIM_PIDS=4242 SHIM_L8080=4242 SHIM_HEALTH=1 LOCK_LIB="$D/fake-lock.sh" LANE=selftest
run_sh "p2 ours via MILKDROP_ATTACH_PID" body SHIM_PIDS=4242 SHIM_L8080=4242 SHIM_HEALTH=1 MILKDROP_ATTACH_PID=4242
echo "R2 -- MILKDROP_P1 (probe-milkdrop.sh, on an attach that R1 accepts)"
run_sh "b1 MILKDROP_P1 in lane mode" p1refuse SHIM_PIDS=4242 SHIM_L8080=4242 SHIM_HEALTH=1 MILKDROP_ATTACH_PID=4242 MILKDROP_P1=x.milk
run_sh "b2 MILKDROP_P1 in gate mode (MILKDROP_MODE=lane, explicit)" p1refuse SHIM_PIDS=4242 SHIM_L8080=4242 SHIM_HEALTH=1 MILKDROP_ATTACH_PID=4242 MILKDROP_P1=x.milk MILKDROP_MODE=lane
run_sh "b3 MILKDROP_P1 in calibration mode (MILKDROP_MODE=pre)" body SHIM_PIDS=4242 SHIM_L8080=4242 SHIM_HEALTH=1 MILKDROP_ATTACH_PID=4242 MILKDROP_P1=x.milk MILKDROP_MODE=pre
echo "R2 -- probe-milkdrop.py (the real file; row 'selftest_no_such_row' -> no request in any version)"
PIN="$("$RPY" -c "import json,sys; print(json.load(open(sys.argv[1]))['p1'])" "$ROOT/.harmony/probe-milkdrop.json")"
PDIR="$("$RPY" -c "import json,sys; print(json.load(open(sys.argv[1]))['presetDir'])" "$ROOT/.harmony/probe-milkdrop.json")"
run_py(){  # run_py <name> [VAR=value ...]  -> sets PO (output) and PRC
  local name=$1; shift
  PO="$(env -u MILKDROP_P1 -u MILKDROP_MODE "$@" "$RPY" "$PYF" "$ROOT" "$D/out" selftest_no_such_row 2>&1)"; PRC=$?
  echo "-- $name: rc $PRC, first line: $(head -1 <<< "$PO")"
}
run_py "c1 MILKDROP_P1 in lane mode" MILKDROP_P1=x.milk
check "$([ $PRC = 2 ] && echo 1)" "c1: exit 2 (got $PRC)"
check "$(grep -q 'REFUSE: MILKDROP_P1' <<< "$PO" && echo 1)" "c1: prints the MILKDROP_P1 REFUSE line"
check "$(grep -q 'ERROR unknown row' <<< "$PO" || echo 1)" "c1: refused before any row ran"
run_py "c2 no override, lane mode"
check "$([ "$(head -1 <<< "$PO")" = "P1 = $ROOT/$PDIR/$PIN" ] && echo 1)" "c2: first line is 'P1 = <pinned preset path>'"
check "$([ $PRC = 1 ] && echo 1)" "c2: runs on (the unknown row -> exit 1; got $PRC)"
run_py "c3 MILKDROP_P1 in calibration mode" MILKDROP_P1=x.milk MILKDROP_MODE=pre
check "$(head -1 <<< "$PO" | grep -qF "P1 = $ROOT/$PDIR/x.milk" && head -1 <<< "$PO" | grep -q OVERRIDE && echo 1)" "c3: first line is 'P1 = <override path>' marked OVERRIDE"
check "$([ $PRC = 1 ] && echo 1)" "c3: runs on (the unknown row -> exit 1; got $PRC)"
run_py "c4 MILKDROP_P1 empty = no override" MILKDROP_P1=
check "$([ "$(head -1 <<< "$PO")" = "P1 = $ROOT/$PDIR/$PIN" ] && echo 1)" "c4: first line is the pinned P1"
rm -rf "$D"
echo; echo "SELFTEST $OK ok / $BAD FAIL"
[ "$BAD" -eq 0 ]
