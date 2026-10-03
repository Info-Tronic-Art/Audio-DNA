#!/bin/bash
# probe-quit-ours-selftest.sh -- s-rta-1003 lane bf9b merge-in, stage M2 (Harmony ruling R-N1: no script ever quits or
# kills an Audio-DNA it did not start). Proves every branch of .harmony/probe-quit-ours.sh and the start refusal of
# every probe that sources it, with NO Audio-DNA launched, quit, signalled or sent a request.
#
# Part 1 (always; needs nothing): the REAL helper is sourced through its seam -- adna_pids lists DUMMY processes
#   (plain `sleep`, re-parented so they are reaped when they die), QUIT_OURS_UCOMM=sleep, and `osascript` is a shell
#   FUNCTION of this file (a function shadows PATH, so the real osascript can never run from the helper here): it
#   logs the call and, when OSA_QUITS=1, ends OURPID the way a graceful quit would.
#     a  refuse_foreign_start: a foreign pid running -> returns 1 and names the pid; nothing running -> returns 0
#     b  record_ourpid: one pid -> recorded; the process appears 1 s after the "launch" -> still recorded;
#        none -> empty + WARN; two -> empty + WARN
#     c  quit_ours, ours only -> ONE quit event, ours gone, returns 0
#     d  quit_ours, ours ignores the event -> after 30 s OURPID alone is killed, returns 0       (skipped with --fast)
#     e  quit_ours, a foreign pid appeared before the quit -> NO quit event, "FOREIGN ... untouched" names it, the
#        foreign pid is alive after, ours is gone (SIGTERM to OURPID only), returns 1
#     f  quit_ours, no pid recorded -> says so; nothing running: returns 0; a foreign pid running: untouched, returns 1
#     g  quit_ours, OURPID's ucomm is no longer ours (a recycled pid) -> no event, no signal
#     h  ask_ours_to_quit: foreign -> no event, returns 1; ours only -> one event.  kill_ours: OURPID only.
# Part 2 (only with --sweep; the caller holds the live lock and NO Audio-DNA runs): every .harmony/*.sh that sources
#   the helper is run for real with PATH shims -- `ps -eo pid=,ucomm=` reports ONE foreign Audio-DNA (pid 99999),
#   `open` / `osascript` only log, `curl` always fails (no request can reach any app) -- and must: exit non-zero,
#   print the REFUSE line naming pid 99999, call `open` 0 times and `osascript` 0 times. probe-outputs.sh is NEVER
#   run, shimmed or not (SCREEN-SAFETY LAW): it is checked statically only.
# Part 3 (always, static): every .harmony/*.sh that launches Audio-DNA sources the helper, calls refuse_foreign_start,
#   and has one record_ourpid per launch line; no by-name quit / kill is left outside the helper.
#
# usage: probe-quit-ours-selftest.sh [--fast] [--sweep] [helper.sh]     (default helper: the sibling of this file)
# Prints one "ok" / "FAIL" line per check, then "SELFTEST <n> ok / <m> FAIL"; exit 0 iff no FAIL.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
FAST=0; SWEEP=0; HELPER="$HERE/probe-quit-ours.sh"
for a in "$@"; do case "$a" in --fast) FAST=1;; --sweep) SWEEP=1;; *) HELPER="$a";; esac; done
echo "target helper: $HELPER"
D="$(mktemp -d "${TMPDIR:-/tmp}/qo-selftest.XXXXXX")" || exit 64
OK=0; BAD=0
check(){ if [ "$1" = 1 ]; then OK=$((OK+1)); echo "   ok    $2"; else BAD=$((BAD+1)); echo "   FAIL  $2"; fi; }

# ---------------------------------------------------------------- part 1: the helper's branches on dummy processes
DUMMIES=""
dummy(){   # dummy [delay-s] -> echoes the pid of a re-parented `sleep 300`
  ( [ -n "${1:-}" ] && sleep "$1"; sleep 300 >/dev/null 2>&1 & echo $! > "$D/dummy.pid" ) >/dev/null 2>&1
  cat "$D/dummy.pid"
}
alive(){ kill -0 "$1" 2>/dev/null && [ "$(ps -o ucomm= -p "$1" 2>/dev/null | tr -d ' ')" = sleep ]; }
adna_pids(){ local p; for p in $DUMMIES; do alive "$p" && echo "$p"; done; }
OSA_QUITS=0
osascript(){ echo "$*" >> "$D/osa-calls"; [ "$OSA_QUITS" = 1 ] && [ -n "$OURPID" ] && kill "$OURPID" 2>/dev/null; return 0; }
osa_calls(){ [ -f "$D/osa-calls" ] && wc -l < "$D/osa-calls" | tr -d ' ' || echo 0; }
reset(){ local p; for p in $DUMMIES; do kill "$p" 2>/dev/null; done; DUMMIES=""; OURPID=""; OSA_QUITS=0; rm -f "$D/osa-calls"; QUIT_OURS_UCOMM=sleep; }
export QUIT_OURS_UCOMM=sleep
. "$HELPER"
[ "$(type -t osascript)" = function ] || { echo "REFUSE: osascript is not this file's function -- not running the helper"; exit 64; }

echo "== part 1: helper branches (dummy processes, no Audio-DNA)"
reset
refuse_foreign_start > "$D/o" 2>&1; rc=$?
check "$([ $rc = 0 ] && [ ! -s "$D/o" ] && echo 1 || echo 0)" "a1 nothing running -> refuse_foreign_start returns 0, prints nothing"
F="$(dummy)"; DUMMIES="$F"
refuse_foreign_start > "$D/o" 2>&1; rc=$?
check "$([ $rc = 1 ] && grep -q "^REFUSE: Audio-DNA already running (pid $F)" "$D/o" && echo 1 || echo 0)" "a2 foreign running at start -> returns 1, the line names pid $F: $(cat "$D/o")"
check "$(alive "$F" && echo 1 || echo 0)" "a3 the foreign pid is alive after the refusal"

reset; P="$(dummy)"; DUMMIES="$P"
record_ourpid > "$D/o" 2>&1
check "$([ "$OURPID" = "$P" ] && echo 1 || echo 0)" "b1 one pid after launch -> OURPID = $P (got '${OURPID}')"
reset; ( sleep 1; sleep 300 >/dev/null 2>&1 & echo $! > "$D/late.pid" ) >/dev/null 2>&1 &
adna_pids(){ local p; for p in $DUMMIES $(cat "$D/late.pid" 2>/dev/null); do alive "$p" && echo "$p"; done; }
rm -f "$D/late.pid"; record_ourpid > "$D/o" 2>&1
for _ in 1 2 3 4 5 6; do [ -s "$D/late.pid" ] && break; sleep 0.5; done; L="$(cat "$D/late.pid" 2>/dev/null)"   # never leave the late dummy behind
check "$([ -n "$L" ] && [ "$OURPID" = "$L" ] && echo 1 || echo 0)" "b2 the process appears 1 s after the launch -> still recorded (OURPID '${OURPID}', late pid '${L}')"
kill "$L" 2>/dev/null; rm -f "$D/late.pid"
adna_pids(){ local p; for p in $DUMMIES; do alive "$p" && echo "$p"; done; }
reset; QUIT_OURS_RECORD_TRIES=2 record_ourpid > "$D/o" 2>&1
check "$([ -z "$OURPID" ] && grep -q '^WARN  0 Audio-DNA pids' "$D/o" && echo 1 || echo 0)" "b3 no pid after launch -> OURPID empty + WARN: $(cat "$D/o")"
reset; P="$(dummy)"; F="$(dummy)"; DUMMIES="$P $F"; record_ourpid > "$D/o" 2>&1
check "$([ -z "$OURPID" ] && grep -q '^WARN  2 Audio-DNA pids' "$D/o" && echo 1 || echo 0)" "b4 two pids after launch -> none is ours + WARN"

reset; P="$(dummy)"; DUMMIES="$P"; record_ourpid >/dev/null; OSA_QUITS=1
quit_ours > "$D/o" 2>&1; rc=$?
check "$([ $rc = 0 ] && [ "$(osa_calls)" = 1 ] && ! alive "$P" && echo 1 || echo 0)" "c  ours only -> quits: rc $rc, quit events $(osa_calls), ours alive $(alive "$P" && echo yes || echo no)"

if [ "$FAST" = 0 ]; then
  reset; P="$(dummy)"; DUMMIES="$P"; record_ourpid >/dev/null; OSA_QUITS=0; T0=$(date +%s)
  quit_ours > "$D/o" 2>&1; rc=$?; DT=$(( $(date +%s) - T0 ))
  check "$([ $rc = 0 ] && [ "$(osa_calls)" = 1 ] && ! alive "$P" && [ $DT -ge 30 ] && echo 1 || echo 0)" "d  ours ignores the quit event -> killed after the 30 s wait: rc $rc, events $(osa_calls), ${DT} s, ours alive $(alive "$P" && echo yes || echo no)"
else echo "   skip  d (--fast)"; fi

reset; P="$(dummy)"; DUMMIES="$P"; record_ourpid >/dev/null; F="$(dummy)"; DUMMIES="$P $F"; OSA_QUITS=1
quit_ours > "$D/o" 2>&1; rc=$?
check "$([ $rc = 1 ] && [ "$(osa_calls)" = 0 ] && echo 1 || echo 0)" "e1 foreign appeared before the quit -> refuse: rc $rc, quit events $(osa_calls)"
check "$(grep -q "^FOREIGN Audio-DNA pid $F running -- untouched" "$D/o" && echo 1 || echo 0)" "e2 the line names the foreign pid: $(cat "$D/o")"
check "$(alive "$F" && echo 1 || echo 0)" "e3 the foreign pid $F is alive after (left alone)"
check "$(alive "$P" && echo 0 || echo 1)" "e4 ours ($P) is gone (SIGTERM to OURPID only)"

reset
quit_ours > "$D/o" 2>&1; rc=$?
check "$([ $rc = 0 ] && [ "$(osa_calls)" = 0 ] && grep -q 'no pid recorded by this run -- nothing is quit' "$D/o" && echo 1 || echo 0)" "f1 no pid recorded, nothing running -> says so, rc $rc, events $(osa_calls): $(cat "$D/o")"
F="$(dummy)"; DUMMIES="$F"; OSA_QUITS=1
quit_ours > "$D/o" 2>&1; rc=$?
check "$([ $rc = 1 ] && [ "$(osa_calls)" = 0 ] && grep -q 'no pid recorded by this run -- nothing is quit' "$D/o" && grep -q "^FOREIGN Audio-DNA pid $F" "$D/o" && alive "$F" && echo 1 || echo 0)" "f2 no pid recorded, a foreign pid running -> quits nothing and says so: rc $rc, events $(osa_calls), foreign alive $(alive "$F" && echo yes || echo no)"

reset; P="$(dummy)"; DUMMIES="$P"; record_ourpid >/dev/null; QUIT_OURS_UCOMM=not-ours; OSA_QUITS=1
quit_ours > "$D/o" 2>&1; rc=$?
check "$([ "$(osa_calls)" = 0 ] && alive "$P" && echo 1 || echo 0)" "g  OURPID's ucomm is no longer ours (recycled pid) -> no event, no signal: events $(osa_calls), pid alive $(alive "$P" && echo yes || echo no)"

reset; P="$(dummy)"; DUMMIES="$P"; record_ourpid >/dev/null; F="$(dummy)"; DUMMIES="$P $F"
ask_ours_to_quit > "$D/o" 2>&1; rc=$?
check "$([ $rc = 1 ] && [ "$(osa_calls)" = 0 ] && grep -q "^FOREIGN Audio-DNA pid $F" "$D/o" && alive "$P" && alive "$F" && echo 1 || echo 0)" "h1 ask_ours_to_quit with a foreign pid -> no event, rc $rc, both alive"
kill_ours; sleep 0.5
check "$(alive "$F" && ! alive "$P" && echo 1 || echo 0)" "h2 kill_ours signals OURPID only: ours alive $(alive "$P" && echo yes || echo no), foreign alive $(alive "$F" && echo yes || echo no)"
reset; P="$(dummy)"; DUMMIES="$P"; record_ourpid >/dev/null; OSA_QUITS=1
ask_ours_to_quit > "$D/o" 2>&1; rc=$?; sleep 0.5
check "$([ $rc = 0 ] && [ "$(osa_calls)" = 1 ] && ! alive "$P" && echo 1 || echo 0)" "h3 ask_ours_to_quit, ours only -> one event: rc $rc, events $(osa_calls)"
reset; kill_ours; rc=$?
check "$([ $rc != 0 ] && echo 1 || echo 0)" "h4 kill_ours with no pid recorded signals nothing (rc $rc)"
reset
unset -f osascript adna_pids

# ---------------------------------------------------------------- part 3: static -- every launching script uses it
echo "== part 3: static sweep of .harmony/*.sh and *.py"
LAUNCH_RE='(^|[^a-zA-Z_"#`.])open (-g|-n|--stdout|"\$APP)|^"\$APP" .*&'
for f in "$HERE"/*.sh; do
  b="$(basename "$f")"
  case "$b" in probe-quit-ours.sh|probe-quit-ours-selftest.sh) continue;; esac
  nl="$(grep -v '^ *#' "$f" | grep -cE "$LAUNCH_RE")"
  [ "$nl" -gt 0 ] || continue
  src="$(grep -c 'probe-quit-ours\.sh"' "$f")"; ref="$(grep -v '^ *#' "$f" | grep -c 'refuse_foreign_start')"
  rec="$(grep -v '^ *#' "$f" | grep -c 'record_ourpid')"
  open_n="$(grep -v '^ *#' "$f" | grep -E "$LAUNCH_RE" | grep -vc '\\$')"   # a two-branch launch (if / else) records once after both
  if grep -q '^# QUIT-OURS: ' "$f"; then
    echo "   note  $b: $(grep -m1 '^# QUIT-OURS: ' "$f" | cut -c3-)"; src=1; ref=1; rec=$nl
  fi
  check "$([ "$src" -ge 1 ] && [ "$ref" -ge 1 ] && [ "$rec" -ge 1 ] && echo 1 || echo 0)" "s  $b: launches $nl, sources the helper $src, refuse_foreign_start $ref, record_ourpid $rec"
done
HITS="$(grep -nE 'tell application "Audio-DNA" to quit|quit app "Audio-DNA"|pkill.*Audio-DNA|killall.*Audio-DNA|adna_kill|kill .*pgrep.*Audio-DNA|pkill.*AudioDNA|killall.*AudioDNA|kill +(-[A-Z0-9]+ +)?\$\(adna_pids|kill +\$p\b' "$HERE"/*.sh "$HERE"/*.py 2>/dev/null \
  | grep -v "^$HERE/probe-quit-ours\.sh:" | grep -v "^$HERE/probe-quit-ours-selftest\.sh:" | grep -vE '^[^:]+:[0-9]+: *#')"
check "$([ -z "$HITS" ] && echo 1 || echo 0)" "t  by-name quit / kill outside probe-quit-ours.sh (code lines): $(printf '%s' "$HITS" | grep -c .) hit(s)"
[ -n "$HITS" ] && printf '%s\n' "$HITS" | sed 's/^/         /'

# ---------------------------------------------------------------- part 2: every converted probe refuses a foreign app
if [ "$SWEEP" = 1 ]; then
  echo "== part 2: start refusal of every probe that sources the helper (PATH shims, fake foreign pid 99999)"
  [ -f /tmp/audiodna-live.lock/owner ] || { echo "REFUSE --sweep: no live lock held (the probes' own lock gate would refuse first)"; exit 64; }
  REAL_PS="$(command -v ps)"
  [ -z "$("$REAL_PS" -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}')" ] || { echo "REFUSE --sweep: an Audio-DNA is running -- the sweep runs only on a machine with none"; exit 64; }
  mkdir -p "$D/shim" "$D/out"
  cat > "$D/shim/ps" <<EOF
#!/bin/bash
case "\$*" in *"pid=,ucomm="*) echo "99999 Audio-DNA"; exit 0;; esac
exec "$REAL_PS" "\$@"
EOF
  for t in open osascript; do printf '#!/bin/bash\necho "$*" >> "%s/%s-calls"\nexit 0\n' "$D" "$t" > "$D/shim/$t"; done
  printf '#!/bin/bash\nexit 7\n' > "$D/shim/curl"
  cat > "$D/shim/pgrep" <<'EOF'
#!/bin/bash
case "$*" in *Audio-DNA*) case "$*" in *-fl*|*-lf*) echo "99999 /fake/Audio-DNA.app/Contents/MacOS/Audio-DNA";; *) echo 99999;; esac; exit 0;; esac
exit 1
EOF
  chmod +x "$D/shim/"*
  ROOT="$(cd "$HERE/.." && pwd)"
  SWEEP_APP="${SELFTEST_APP:-}"
  if [ -z "$SWEEP_APP" ]; then for c in "$ROOT"/build/AudioDNA_artefacts/Release/Audio-DNA.app "$ROOT"/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app; do [ -d "$c" ] && { SWEEP_APP="$c"; break; }; done; fi
  echo "   app handed to every *_APP variable (never launched: open is a shim): ${SWEEP_APP:-none}"
  for f in "$HERE"/*.sh; do
    b="$(basename "$f")"
    grep -q 'probe-quit-ours\.sh"' "$f" || continue
    case "$b" in probe-quit-ours-selftest.sh) continue;; probe-outputs.sh) echo "   skip  $b (never run, shimmed or not: it opens Output windows)"; continue;; esac
    rm -f "$D/open-calls" "$D/osascript-calls"
    ENVV=()
    for v in $(grep -oE '\$\{[A-Z0-9_]+_(APP|APP_BEFORE|BUILD_DIR)(:-|\})' "$f" | sed -E 's/^\$\{//; s/(:-|\})$//' | sort -u); do
      case "$v" in *_BUILD_DIR) ENVV+=("$v=$(dirname "$(dirname "$(dirname "$SWEEP_APP")")")");; *) ENVV+=("$v=$SWEEP_APP");; esac
    done
    out="$(env ${ENVV[@]+"${ENVV[@]}"} PATH="$D/shim:$PATH" perl -e 'alarm 60; exec @ARGV' bash "$f" "$D/out" 2>&1 </dev/null)"; rc=$?
    opens=0; osas=0
    [ -f "$D/open-calls" ] && opens=$(wc -l < "$D/open-calls" | tr -d ' ')
    [ -f "$D/osascript-calls" ] && osas=$(wc -l < "$D/osascript-calls" | tr -d ' ')
    line="$(printf '%s\n' "$out" | grep -m1 -E 'REFUS.*99999|99999.*REFUS' | cut -c1-110)"
    [ -z "$line" ] && line="(no refusal naming 99999) last line: $(printf '%s\n' "$out" | tail -1 | cut -c1-120)"
    check "$([ $rc != 0 ] && [ "$opens" = 0 ] && [ "$osas" = 0 ] && printf '%s\n' "$out" | grep -q 99999 && echo 1 || echo 0)" "r  $b: rc $rc, open calls $opens, osascript calls $osas -- $line"
  done
fi
rm -rf "$D"
echo "SELFTEST $OK ok / $BAD FAIL"
[ "$BAD" -eq 0 ]
