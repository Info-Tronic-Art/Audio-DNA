#!/bin/bash
# probe-one-save-selftest.sh -- lane one-save S1: proves the rows of .harmony/probe-one-save.py WITHOUT Audio-DNA.
# NO app is launched, quit, signalled or sent a request; nothing of the user's is read. A stub server on a free
# 127.0.0.1 port (.harmony/probe-one-save-stub.py) models the ruled save behaviour and, per mode, one mutant of it:
#   good  -> OS-L13, OS-L14, OS-L14b, OS-L21 all PASS (4 PASS, 0 FAIL; exit 0)
#   mu4   -> OS-L13 FAILs  (MU-OS-4: no copy is ever made) -- and with it OS-L14 (the write goes on) and OS-L14b
#   mu5   -> OS-L14 FAILs  (MU-OS-5: a failed copy is ignored)             -- every other row as on "good"
#   mu23  -> OS-L14b FAILs (MU-OS-23: an unreadable target needs no copy)  -- every other row as on "good"
#   mu21  -> OS-L21 FAILs  (MU-OS-21: the write is not verified; a cut-off file reported as saved)
#   old   -> OS-L13, OS-L14, OS-L14b, OS-L21 all FAIL (the app before the lane: no show_file route, no copy, no version)
# and: os_l21 without ONESAVE_FULLDISK prints "INFO  OS-L21: not run" and is NOT counted as a pass; an unknown row
# name is refused (exit 64). Part 2 checks probe-boxes.py's K7 clause (k7_saved_shape) on recorded files.
# The full disk of os_l21 is MODELLED here (the stub treats a folder holding filler.bin as full; ONESAVE_NOFILL=1
# keeps the probe from writing a real filler): the real 2 MB image is Harmony's run (ruling M-1).
# usage: bash .harmony/probe-one-save-selftest.sh          (python3 >= 3.8, stdlib only)
# Prints one "ok" / "FAIL" line per check, then "SELFTEST <n> ok / <m> FAIL"; exit 0 iff no FAIL.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"; ROOT="$(cd "$HERE/.." && pwd)"
PY="${ONESAVE_PY:-python3}"
D="$(mktemp -d "${TMPDIR:-/tmp}/onesave-selftest.XXXXXX")" || exit 64
OK=0; BAD=0; STUB=""
cleanup() { [ -n "$STUB" ] && kill "$STUB" 2>/dev/null; wait 2>/dev/null; }
trap cleanup EXIT
check() { if [ "$1" = 1 ]; then OK=$((OK + 1)); echo "ok    $2"; else BAD=$((BAD + 1)); echo "FAIL  $2"; fi; }
want() {   # want <log> <mode> <row tag> <PASS|FAIL>
  local got; got="$(grep -cE "^$4  $3: " "$1")"
  check "$([ "$got" = 1 ] && echo 1 || echo 0)" "$2: $3 prints exactly one $4 line (found $got)"
}
PORT="$("$PY" -c 'import socket; s = socket.socket(); s.bind(("127.0.0.1", 0)); print(s.getsockname()[1]); s.close()')"
run_mode() {   # run_mode <mode> -> $D/<mode>.log, RCM = the probe's exit code
  local m="$1"
  "$PY" "$HERE/probe-one-save-stub.py" "$PORT" "$m" & STUB=$!
  local up=0; for _ in $(seq 1 50); do curl -s --max-time 1 "http://127.0.0.1:$PORT/api/health" >/dev/null 2>&1 && { up=1; break; }; sleep 0.1; done
  mkdir -p "$D/$m/disk"
  if [ "$up" = 1 ]; then
    : > "$D/$m/disk/filler.bin"
    ONESAVE_API="http://127.0.0.1:$PORT" ONESAVE_FULLDISK="$D/$m/disk" ONESAVE_NOFILL=1 "$PY" "$HERE/probe-one-save.py" "$ROOT" "$D/$m" "${2:-}" > "$D/$m.log" 2>&1
    RCM=$?
  else echo "stub never answered" > "$D/$m.log"; RCM=99; fi
  kill "$STUB" 2>/dev/null; wait "$STUB" 2>/dev/null; STUB=""
}
echo "== part 1: the rows against the stub (port $PORT, out $D)"
run_mode good
for t in OS-L13 OS-L14 OS-L14b OS-L21; do want "$D/good.log" good "$t" PASS; done
check "$([ "$RCM" = 0 ] && grep -q '^PY 4 PASS, 0 FAIL, 0 INFO-only' "$D/good.log" && echo 1 || echo 0)" "good: exit 0 and 'PY 4 PASS, 0 FAIL' (rc $RCM)"
check "$(grep -q '^INFO  OS-L13: M-2 the time of writeShow' "$D/good.log" && echo 1 || echo 0)" "good: OS-L13 prints the M-2 line"
check "$([ "$(grep -c '^--- ' "$D/good.log")" = 4 ] && grep -q '^PY-ROWS registered 4$' "$D/good.log" && echo 1 || echo 0)" "good: 4 row headers, 4 registered (the launcher's EXPECTED_ROWS pin)"
for spec in "mu4 OS-L13,OS-L14,OS-L14b" "mu5 OS-L14" "mu23 OS-L14b" "mu21 OS-L21"; do
  set -- $spec; m="$1"; red=",$2,"
  run_mode "$m"
  for t in OS-L13 OS-L14 OS-L14b OS-L21; do
    case "$red" in *",$t,"*) want "$D/$m.log" "$m" "$t" FAIL;; *) want "$D/$m.log" "$m" "$t" PASS;; esac
  done
  check "$([ "$RCM" = 1 ] && echo 1 || echo 0)" "$m: the probe exits 1 (rc $RCM)"
done
run_mode old
for t in OS-L13 OS-L14 OS-L14b OS-L21; do want "$D/old.log" old "$t" FAIL; done
check "$(grep -q '^INFO  OS-L21: this app has no show_file route .* Outcome O1' "$D/old.log" && echo 1 || echo 0)" "old: OS-L21 names outcome O1 (a cut-off file, nothing reported)"
# os_l21 with no disk: INFO, never a pass
"$PY" "$HERE/probe-one-save-stub.py" "$PORT" good & STUB=$!
for _ in $(seq 1 50); do curl -s --max-time 1 "http://127.0.0.1:$PORT/api/health" >/dev/null 2>&1 && break; sleep 0.1; done
env -u ONESAVE_FULLDISK ONESAVE_API="http://127.0.0.1:$PORT" "$PY" "$HERE/probe-one-save.py" "$ROOT" "$D/nodisk" os_l21 > "$D/nodisk.log" 2>&1; RCN=$?
check "$(grep -q '^INFO  OS-L21: not run' "$D/nodisk.log" && ! grep -q '^PASS' "$D/nodisk.log" && grep -q '^PY 0 PASS, 0 FAIL, 1 INFO-only' "$D/nodisk.log" && echo 1 || echo 0)" "os_l21 without ONESAVE_FULLDISK: INFO, 0 PASS (rc $RCN)"
ONESAVE_API="http://127.0.0.1:$PORT" "$PY" "$HERE/probe-one-save.py" "$ROOT" "$D/unknown" no_such_row > "$D/unknown.log" 2>&1; RCU=$?
check "$([ "$RCU" = 64 ] && echo 1 || echo 0)" "an unknown row name is refused (rc $RCU)"
kill "$STUB" 2>/dev/null; wait "$STUB" 2>/dev/null; STUB=""
# with no server at all every row FAILs (a dead app is never a pass)
ONESAVE_API="http://127.0.0.1:$PORT" "$PY" "$HERE/probe-one-save.py" "$ROOT" "$D/dead" os_l13,os_l14,os_l14b > "$D/dead.log" 2>&1; RCD=$?
check "$([ "$RCD" = 1 ] && [ "$(grep -c '^FAIL  OS-L' "$D/dead.log")" = 3 ] && echo 1 || echo 0)" "no server: 3 FAIL lines, exit 1 (rc $RCD)"

echo "== part 2: probe-boxes.py K7's clause (k7_saved_shape) on recorded files"
"$PY" - "$HERE/probe-boxes.py" > "$D/k7.log" 2>&1 <<'PYEOF'
import ast, json, sys
src = open(sys.argv[1]).read()
fn = [n for n in ast.parse(src).body if isinstance(n, ast.FunctionDef) and n.name == "k7_saved_shape"]
assert len(fn) == 1, "k7_saved_shape not found in probe-boxes.py"
ns = {"json": json}
exec(compile(ast.Module(body=fn, type_ignores=[]), "k7", "exec"), ns)
shape = ns["k7_saved_shape"]
new = '{\n "version": 2,\n "name": "x",\n "layers": [{}, {}],\n "decks": [],\n "keys": {},\n "layout": {}\n}'
before = '{\n "name": "x",\n "layers": [{}, {}],\n "decks": []\n}'                       # the app before the lane
late = '{\n "name": "x",\n "version": 2,\n "layers": [], "decks": [], "keys": {}, "layout": {}\n}'   # version not first
nokeys = '{\n "version": 2,\n "name": "x",\n "layers": [], "decks": [], "layout": {}\n}'
v1 = '{\n "version": 1,\n "name": "x",\n "layers": [], "decks": [], "keys": {}, "layout": {}\n}'
vstr = '{\n "version": "2",\n "name": "x",\n "layers": [], "decks": [], "keys": {}, "layout": {}\n}'
for name, text, want in (("new", new, True), ("before the lane", before, False), ("version not first", late, False),
                         ("no keys block", nokeys, False), ("version 1", v1, False), ("version a string", vstr, False),
                         ("not json", "not json!!", False)):
    ok, facts = shape(text)
    print(("ok   " if ok == want else "BAD  ") + f"{name}: {ok} ({facts})")
PYEOF
RCK=$?
check "$([ "$RCK" = 0 ] && [ "$(grep -c '^ok   ' "$D/k7.log")" = 7 ] && ! grep -q '^BAD' "$D/k7.log" && echo 1 || echo 0)" "k7_saved_shape: the new shape passes, 6 other shapes fail (rc $RCK; $(grep -c '^ok   ' "$D/k7.log") of 7)"
[ "$BAD" -gt 0 ] && { echo "--- logs kept in $D"; sed 's/^/    /' "$D/k7.log" | head -20; }
echo "SELFTEST $OK ok / $BAD FAIL"
[ "$BAD" -eq 0 ]
