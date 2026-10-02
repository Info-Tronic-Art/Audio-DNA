#!/bin/bash
# g6-perf.sh -- FINAL GATE G6 (ruling-tsan.md "FINAL GATES" G6 + plan-tsan.md (5) G6): Release perf A/B, main (PRE) vs
# merged main (POST), scenarios b and d, using the lane's own TSan sweep driver (probe-tsan.sh + probe-tsan.py) on RELEASE
# apps. Written by lane tsan-gatetools; Harmony RUNS it after the merge.
#
# usage: g6-perf.sh <PRE_APP> <POST_APP> <OUT_DIR> [n=5] [--dry-run]
#   PRE_APP   Release Audio-DNA.app copy of pre-merge main      POST_APP  Release Audio-DNA.app of merged main
#   n         rounds; each round = pre-b, post-b, pre-d, post-d (interleaved launch by launch) => n launches per arm per scenario
#   --dry-run print every command it would run (no launch / lock / quit / patch)
# Env: PROBE_DIR  dir holding probe-tsan.sh + probe-tsan.py (default /Users/boriskarpman/projects/RealTimeAudio/.harmony; BEFORE the
#                 merge: /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/tsan/.harmony)
#      G6_EXTEND  extra rounds on a FLAG (default 5 = the pooled 10);  G6_NO_EXTEND=1  never extend (print the FLAG only)
#
# HOW IT USES THE DRIVER (read from probe-tsan.sh's header + body, lane worktree):
#   SPEC items "N:scen@arm" (N unique per launch), TSAN_APP_<arm> = the arm's bundle, TSAN_PY, TSAN_MEDIA (fixtures, made once
#   with ffmpeg by the driver), TSAN_HISTORY (irrelevant on a Release app). The driver REFUSES an app without libclang_rt.tsan, and
#   hard-codes ROOT from its own location, so g6 runs a PATCHED COPY (<OUT>/patched/probe-tsan.perf.sh) with exactly three
#   substrings replaced (each must occur exactly once or the patch aborts): the ROOT= resolution -> dirname(PROBE_DIR), the
#   `otool ... libclang_rt.tsan ... exit 64` guard -> a no-op, and a `LOADAVG tsan-N {...}` echo after each launch banner (the
#   per-launch load average). Everything else (launch via open -g, the scenario, graceful quit, validity checks, UNC / .ips
#   checks, built-in-audio pre-check) is the driver unchanged. A launch the driver marks valid=no is re-run (up to 2 more
#   attempts, new N), never counted.
# Per launch the metrics come from <OUT>/batch/runs/tsan-N/state.json = the scenario's ONE final GET /api/state (src/api/ApiServer.cpp
# /api/state route: "fps" 1378, "peak_frame_time_ms" 1386 = takePeakFrameTimeMs(), "peak_callback_ms" 1388 = takePeakCallbackMs()
# -- the take* getters are reset-on-read, so a launch's value is the peak since the app's start (nobody else reads /api/state)).
# Lock: acquire_lock (NOT acquire_quiet_lock) held per round (4 launches), LANE=harmony-gate; load average printed per launch.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
DRY=0; POS=()
for a in "$@"; do if [ "$a" = "--dry-run" ]; then DRY=1; else POS+=("$a"); fi; done
[ "${#POS[@]}" -ge 3 ] || { sed -n '2,10p' "$0" | sed 's/^# \{0,1\}//'; exit 64; }
abspath() { python3 -c 'import os,sys; print(os.path.abspath(sys.argv[1]))' "$1"; }
PRE_APP="$(abspath "${POS[0]}")"; POST_APP="$(abspath "${POS[1]}")"; OUT="$(abspath "${POS[2]}")"; N_ROUNDS="${POS[3]:-5}"
case "$N_ROUNDS" in ''|*[!0-9]*|0) echo "REFUSE: n must be a positive integer (got '$N_ROUNDS')"; exit 64 ;; esac
REAL_ROOT=/Users/boriskarpman/projects/RealTimeAudio
PROBE_DIR="${PROBE_DIR:-$REAL_ROOT/.harmony}"
PYV="$REAL_ROOT/.venv/bin/python"
EXTEND="${G6_EXTEND:-5}"
LIB="${GATE_LOCK_LIB:-/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/267d1ba0-b658-4610-9f47-ef72f4b49872/scratchpad/lib/lock.sh}"   # GATE_LOCK_LIB: test seam only
ROWS_PY="$HERE/g4-rows.py"
PATCHED="$OUT/patched/probe-tsan.perf.sh"
BATCH="$OUT/batch"
RULE="G6 pre-registered rule (verbatim): FLAG if a POST median of peak_frame_time_ms or peak_callback_ms exceeds PRE's by more than 15 % AND by more than 0.5 ms; a FLAG => run 5 more per arm (the script supports n) and judge the pooled 10: holds => FAIL \"named render-thread regression\", else INFO. Final line \"G6: PASS\" / \"G6: FLAG <...>\"."

say() { echo "$(date +%T) $*"; }
pq() { printf '%q ' "$@"; echo; }
run() { if [ "$DRY" = 1 ]; then printf 'DRY: '; pq "$@"; return 0; else "$@"; fi; }

export LANE=harmony-gate
if [ -f "$LIB" ]; then . "$LIB"; else echo "REFUSE: $LIB missing"; [ "$DRY" = 1 ] || exit 64; fi
[ "$DRY" = 1 ] && { acquire_lock() { echo "DRY: acquire_lock (LANE=harmony-gate)"; }; release_lock() { echo "DRY: release_lock"; }; \
                    quit_app() { echo "DRY: quit_app"; }; adna() { :; }; }
quit_if_running() {
  if [ "$DRY" = 1 ]; then echo "DRY: quit_app (only if an Audio-DNA is running)"; return 0; fi
  if [ -n "$(adna)" ]; then quit_app; else echo "no Audio-DNA running (quit_app skipped)"; fi
}
with_timeout() {   # with_timeout SECS cmd... -- hard timeout, kills the whole process group, rc 124 on timeout
  local t=$1; shift
  perl -e 'my $t = shift; my $pid = fork(); die "fork" unless defined $pid;
           if (!$pid) { setpgrp(0, 0); exec @ARGV; exit 127 }
           $SIG{ALRM} = sub { kill "KILL", -$pid; waitpid($pid, 0); exit 124 };
           alarm $t; waitpid($pid, 0); my $s = $?; exit(($s & 127) ? 128 + ($s & 127) : ($s >> 8))' "$t" "$@"
}

echo "=== g6-perf $(date '+%F %T')  dry=$DRY  n=$N_ROUNDS  extend=$EXTEND${G6_NO_EXTEND:+ (G6_NO_EXTEND set: never)}"
echo "$RULE"
echo "PRE_APP : $PRE_APP"; echo "POST_APP: $POST_APP"; echo "OUT_DIR : $OUT"; echo "PROBE_DIR (probe-tsan.sh + .py): $PROBE_DIR"

# --- the patched driver copy ------------------------------------------------------------------------------------------
ROOT_SUBST='ROOT="$(cd "$(dirname "$0")/.." && pwd)"'
OTOOL_SUBST="otool -L \"\$APP/Contents/MacOS/Audio-DNA\" 2>/dev/null | grep -q 'libclang_rt.tsan' || { echo \"REFUSE: \$APP is not a TSan build (no libclang_rt.tsan)\"; exit 64; }"
BANNER_SUBST='echo "--- tsan-$N scenario $SC arm $ARM  $(date +%T)  app $APP"'
LOADAVG_ADD='; echo "LOADAVG tsan-$N $(sysctl -n vm.loadavg)"'
if [ "$DRY" = 0 ]; then
  for f in "$PRE_APP" "$POST_APP"; do [ -x "$f/Contents/MacOS/Audio-DNA" ] || { echo "REFUSE: $f is not an Audio-DNA.app bundle"; exit 64; }; done
  [ "$PRE_APP" != "$POST_APP" ] || { echo "REFUSE: PRE_APP == POST_APP"; exit 64; }
  for f in probe-tsan.sh probe-tsan.py; do [ -f "$PROBE_DIR/$f" ] || { echo "REFUSE: $PROBE_DIR/$f missing (set PROBE_DIR)"; exit 64; }; done
  [ -x "$PYV" ] && [ -f "$ROWS_PY" ] || { echo "REFUSE: $PYV or $ROWS_PY missing"; exit 64; }
  [ -z "$(adna)" ] || { echo "REFUSE: an Audio-DNA is running"; exit 64; }
  [ -e "$BATCH/runs" ] && { echo "REFUSE: $BATCH/runs exists (use a fresh OUT_DIR)"; exit 64; }
  mkdir -p "$OUT" "$BATCH"
  echo "PRE  sha256: $(shasum -a 256 "$PRE_APP/Contents/MacOS/Audio-DNA" | cut -c1-16)"; echo "POST sha256: $(shasum -a 256 "$POST_APP/Contents/MacOS/Audio-DNA" | cut -c1-16)"
  : > "$OUT/map.tsv"
  trap 'echo "interrupted"; exit 130' INT TERM
  trap 'quit_if_running >/dev/null 2>&1; release_lock >/dev/null 2>&1' EXIT
fi
run mkdir -p "$OUT/patched"
run python3 "$ROWS_PY" patch "$PROBE_DIR/probe-tsan.sh" "$PATCHED" \
    --replace "$ROOT_SUBST" "ROOT=\"$(dirname "$PROBE_DIR")\"" \
    --replace "$OTOOL_SUBST" ': # g6: Release apps -- the TSan-build check is removed' \
    --replace "$BANNER_SUBST" "$BANNER_SUBST$LOADAVG_ADD"

# --- before the batch: orphaned CPU burners + load ----------------------------------------------------------------------
echo "=== before the batch: top 5 by %CPU (ps -Ao pcpu=,etime=,comm=) and load average"
if [ "$DRY" = 1 ]; then echo "DRY: ps -Ao pcpu=,etime=,comm= | sort -rn | head -5"; echo "DRY: sysctl -n vm.loadavg   (uptime)"
else
  ps -Ao pcpu=,etime=,comm= | sort -rn | head -5
  echo "load average: $(sysctl -n vm.loadavg)"
  ps -Ao pcpu=,comm= | awk '$1 > 50 {print "WARNING: " $0 " %CPU -- an orphaned burner skews a perf A/B"}' | head -3
fi

# --- rounds ----------------------------------------------------------------------------------------------------------------
NEXT_N=1; ROUNDS_DONE=0
valid_n() { [ -f "$BATCH/launches.tsv" ] && awk -F'\t' -v n="$1" '$2 == n && /valid=yes/ {f = 1} END {exit !f}' "$BATCH/launches.tsv"; }
run_round() {   # run_round <round-number>
  local r="$1" items="" scen arm n tries=0 pend i
  for scen in b d; do for arm in pre post; do items="$items $scen@$arm"; done; done   # pre-b post-b pre-d post-d
  say "=== round $r"
  acquire_lock || { echo "FATAL: lock not acquired"; exit 2; }
  local LK0; LK0=$(date +%s)
  pend="$items"
  while [ -n "$pend" ] && [ "$tries" -lt 3 ]; do
    tries=$((tries + 1)); local spec="" this=""
    for i in $pend; do
      n=$NEXT_N; NEXT_N=$((NEXT_N + 1)); spec="$spec $n:$i"; this="$this $n:$i"
      [ "$DRY" = 1 ] || printf '%s\t%s\t%s\t%s\n' "$n" "${i%@*}" "${i#*@}" "$r" >> "$OUT/map.tsv"
    done
    spec="${spec# }"
    if [ "$DRY" = 1 ]; then
      echo "DRY: round $r attempt $tries  SPEC=\"$spec\""
      echo "DRY:   (the driver prints 'LOADAVG tsan-N { .. }' per launch into $BATCH/batch-<stamp>.log)"
      printf 'DRY:   '; pq with_timeout 2400 env "TSAN_APP_pre=$PRE_APP" "TSAN_APP_post=$POST_APP" "TSAN_PY=$PYV" "TSAN_MEDIA=$OUT/media" TSAN_HISTORY=4 \
          bash "$PATCHED" "$BATCH" "$spec"
      echo "DRY:   > $OUT/round-$r.log 2>&1 (stdin /dev/null); then quit_app; a launch with valid=no in $BATCH/launches.tsv is re-run (<= 2 more attempts)"
      pend=""; continue
    fi
    with_timeout 2400 env "TSAN_APP_pre=$PRE_APP" "TSAN_APP_post=$POST_APP" "TSAN_PY=$PYV" "TSAN_MEDIA=$OUT/media" TSAN_HISTORY=4 \
        bash "$PATCHED" "$BATCH" "$spec" </dev/null >> "$OUT/round-$r.log" 2>&1
    say "round $r attempt $tries driver rc=$?"
    quit_if_running
    pend=""
    for i in $this; do
      n=${i%%:*}
      valid_n "$n" || { pend="$pend ${i#*:}"; say "launch tsan-$n (${i#*:}) INVALID -- will be re-run"; }
    done
  done
  [ -z "$pend" ] || say "WARNING: round $r still has invalid launches after 3 attempts:$pend"
  [ "$DRY" = 1 ] || say "lock held $(( $(date +%s) - LK0 )) s for round $r"
  release_lock
  ROUNDS_DONE=$((ROUNDS_DONE + 1))
}

eval_all() {   # eval_all <label> -> prints the table + medians; the last lines are machine lines G6EVAL ...
python3 - "$OUT" "$1" "$ROUNDS_DONE" <<'PYEOF'
import glob, json, os, re, statistics, sys
out, label, rounds = sys.argv[1], sys.argv[2], int(sys.argv[3])
batch = os.path.join(out, "batch")
maps = {}
for l in open(os.path.join(out, "map.tsv")):
    p = l.split()
    if len(p) == 4:
        maps[int(p[0])] = (p[1], p[2], int(p[3]))
valid = {}
tsv = os.path.join(batch, "launches.tsv")
if os.path.exists(tsv):
    for l in open(tsv):
        p = l.rstrip("\n").split("\t")
        if len(p) > 2 and p[1].isdigit():
            valid[int(p[1])] = any(x == "valid=yes" for x in p)
load = {}
for f in sorted(glob.glob(os.path.join(batch, "batch-*.log"))):
    for l in open(f, errors="replace"):
        m = re.match(r"LOADAVG tsan-(\d+) \{ ([\d.]+) ", l)
        if m:
            load[int(m.group(1))] = m.group(2)
METRICS = ("fps", "peak_frame_time_ms", "peak_callback_ms")
launches = []
for n in sorted(maps):
    sc, arm, rnd = maps[n]
    rec = {"n": n, "sc": sc, "arm": arm, "rnd": rnd, "valid": valid.get(n, False), "why": ""}
    sp = os.path.join(batch, "runs", "tsan-%d" % n, "state.json")
    try:
        j = json.load(open(sp))
        for k in METRICS:
            rec[k] = float(j[k])
    except Exception as e:
        rec["valid"] = False
        rec["why"] = "no state.json metrics (%s)" % (type(e).__name__)
    if not valid.get(n, False) and not rec["why"]:
        rec["why"] = "driver valid=no / launch not run"
    launches.append(rec)
print("=== %s: per-launch table (counted = valid launches only)" % label)
print("%-3s %-4s %-3s %-5s %-6s %8s %10s %10s %8s  %s" % ("rnd", "N", "sc", "arm", "valid", "fps", "peakFrame", "peakCb", "load1m", "note"))
for r in launches:
    print("%-3d %-4d %-3s %-5s %-6s %8s %10s %10s %8s  %s" % (
        r["rnd"], r["n"], r["sc"], r["arm"], "yes" if r["valid"] else "NO",
        "%.1f" % r["fps"] if "fps" in r else "-", "%.2f" % r["peak_frame_time_ms"] if "peak_frame_time_ms" in r else "-",
        "%.2f" % r["peak_callback_ms"] if "peak_callback_ms" in r else "-", load.get(r["n"], "-"), r["why"]))
med = {}
print("=== %s: medians per (scenario, arm)  [count of valid launches]" % label)
for sc in ("b", "d"):
    for arm in ("pre", "post"):
        v = [r for r in launches if r["valid"] and r["sc"] == sc and r["arm"] == arm]
        med[(sc, arm)] = {k: statistics.median([r[k] for r in v]) for k in METRICS} if v else None
        med[(sc, arm)] and med[(sc, arm)].update(count=len(v))
        if v:
            m = med[(sc, arm)]
            print("%s@%-4s [%2d]  fps %.1f   peak_frame_time_ms %.2f   peak_callback_ms %.2f" % (sc, arm, len(v), m["fps"], m["peak_frame_time_ms"], m["peak_callback_ms"]))
        else:
            print("%s@%-4s [ 0]  (no valid launch)" % (sc, arm))
flags, incomplete = [], []
for sc in ("b", "d"):
    for arm in ("pre", "post"):
        c = med[(sc, arm)]["count"] if med[(sc, arm)] else 0
        if c < rounds:
            incomplete.append("%s@%s(%d/%d)" % (sc, arm, c, rounds))
    pre, post = med[(sc, "pre")], med[(sc, "post")]
    if not pre or not post:
        continue
    for m in ("peak_frame_time_ms", "peak_callback_ms"):
        a, b = pre[m], post[m]
        pct = (b - a) / a * 100.0 if a > 0 else float("inf")
        hit = b > a * 1.15 and (b - a) > 0.5
        print("%s %-19s PRE %.2f  POST %.2f  delta %+.2f ms (%+.1f %%)  %s" % (sc, m, a, b, b - a, pct, "FLAG" if hit else "ok"))
        if hit:
            flags.append("%s:%s" % (sc, m))
print("G6EVAL flags=%s" % ",".join(flags))
print("G6EVAL incomplete=%s" % ",".join(incomplete))
PYEOF
}

for r in $(seq 1 "$N_ROUNDS"); do run_round "$r"; done

if [ "$DRY" = 1 ]; then
  echo "DRY: python3 <eval> $OUT n=$N_ROUNDS   # per-launch table, per (scenario, arm) medians, the FLAG rule; prints 'G6EVAL flags=...'"
  echo "DRY: if flags non-empty and G6_NO_EXTEND unset: $EXTEND more rounds (same commands as above, new N), then the eval again on the pooled $((N_ROUNDS + EXTEND))"
  echo "DRY:   pooled flag still on the same (scenario, metric) => 'FAIL: named render-thread regression' + final 'G6: FLAG <rows> ...'; else INFO + 'G6: PASS'"
  echo "DRY: final line: 'G6: PASS' or 'G6: FLAG <...>'"
  exit 0
fi

EV1="$(eval_all "first $ROUNDS_DONE rounds")"; echo "$EV1" | grep -v '^G6EVAL'
F1="$(echo "$EV1" | sed -n 's/^G6EVAL flags=//p')"; INC1="$(echo "$EV1" | sed -n 's/^G6EVAL incomplete=//p')"
[ -z "$INC1" ] || echo "WARNING: fewer valid launches than rounds: $INC1"
FINAL=""
if [ -z "$F1" ]; then
  FINAL="G6: PASS"; [ -z "$INC1" ] || FINAL="G6: FLAG incomplete valid launches: $INC1"
elif [ -n "${G6_NO_EXTEND:-}" ]; then
  FINAL="G6: FLAG $F1 (G6_NO_EXTEND set: not extended -- run again with more rounds to judge the pooled 10)"
else
  echo "=== FLAG on the first $ROUNDS_DONE rounds: $F1 -- running $EXTEND more rounds per arm, pooled judgement"
  for r in $(seq $((ROUNDS_DONE + 1)) $((ROUNDS_DONE + EXTEND))); do run_round "$r"; done
  EV2="$(eval_all "pooled $ROUNDS_DONE rounds")"; echo "$EV2" | grep -v '^G6EVAL'
  F2="$(echo "$EV2" | sed -n 's/^G6EVAL flags=//p')"; INC2="$(echo "$EV2" | sed -n 's/^G6EVAL incomplete=//p')"
  [ -z "$INC2" ] || echo "WARNING: fewer valid launches than rounds: $INC2"
  HOLDS=""; NEWONLY=""
  for f in $(echo "$F2" | tr ',' ' '); do case ",$F1," in *",$f,"*) HOLDS="$HOLDS $f" ;; *) NEWONLY="$NEWONLY $f" ;; esac; done
  [ -z "$NEWONLY" ] || echo "INFO: flagged on the pooled set only (not on the first $((ROUNDS_DONE - EXTEND)) rounds):$NEWONLY"
  if [ -n "$HOLDS" ]; then
    echo "FAIL: named render-thread regression (the FLAG holds on the pooled $ROUNDS_DONE):$HOLDS"
    FINAL="G6: FLAG${HOLDS} (pooled $ROUNDS_DONE holds => FAIL named render-thread regression)"
  else
    echo "INFO: the FLAG ($F1) did not hold on the pooled $ROUNDS_DONE -- INFO, not a failure"
    FINAL="G6: PASS"; [ -z "$INC2" ] || FINAL="G6: FLAG incomplete valid launches: $INC2"
  fi
fi
echo "$FINAL" | tee "$OUT/g6-final.txt" >/dev/null
sleep 1
echo "$FINAL"
case "$FINAL" in "G6: PASS") exit 0 ;; *) exit 1 ;; esac
