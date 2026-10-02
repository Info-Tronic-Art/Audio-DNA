#!/bin/bash
# g4-parity.sh -- FINAL GATE G4 (ruling-tsan.md "FINAL GATES" G4 + plan-tsan.md (5) G4): behaviour parity of merged main
# against pre-merge main, over the probes' own printed rows. Written by lane tsan-gatetools; Harmony RUNS it after the merge.
#
# usage: g4-parity.sh <PRE_APP> <POST_APP> <OUT_DIR> [probe-filter] [--dry-run]
#   PRE_APP   a Release Audio-DNA.app bundle COPY of pre-merge main (no build dir exists for it)
#   POST_APP  /Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app (merged main)
#   OUT_DIR   <OUT_DIR>/<probe>/<arm>-<n>.log (+ .d run dirs), <OUT_DIR>/rows.tsv, runs.tsv, g4-final.txt
#   probe-filter  optional substring (case pattern *x*): only the matching probes run (re-runs)
#   --dry-run  print every command (env + argv + timeout) without a launch, a lock, a quit or a patch
# Env: G4_ROOT (default the real checkout), PROBE_DIR (default $G4_ROOT/.harmony -- the probes and their fixtures; BOTH
#   arms run the SAME probe files), G4_BUILDDIR_MODE=symlink|patch (default symlink; patch = the fallback if `open` of a
#   symlinked bundle misbehaves: the build-dir probes then run a patched copy with APPBUNDLE pinned to the app path).
#
# Order: per probe, pre-1, post-1, pre-2, post-2 (arms alternate, each probe twice per arm); the lock is held per probe (acquire
# before its 4 runs, release after; ~20 min cap is YOUR call -- step3 is the long one: split with a probe-filter if needed);
# quit_app between runs; acquire_lock (NOT acquire_quiet_lock). Every run is wrapped in a hard timeout (process-group kill);
# a timed-out / REFUSEd / row-less run is INVALID: its rows are '-' (MISSING), never PASS.
#
# HOW EACH PROBE TAKES ITS APP (read from the probe headers; the per-probe table is in the Builder report):
#   env var *_APP ...... crossfade XFADE_APP, render-state RSTATE_APP, deck-clock DCLOCK_APP, routines ROUTINES_APP,
#                        routine-display ROUTINE_DISPLAY_APP, async-load ASYNCLOAD_APP, deck-tabs DECKTABS_APP,
#                        media-open MEDIAOPEN_APP, video VIDEO_APP
#   BUILD DIR only ..... step3 STEP3_BUILD_DIR, mastersignal MS_BUILD_DIR: a SHIM dir <OUT>/shim/<arm>/AudioDNA_artefacts/
#                        Release/Audio-DNA.app -> SYMLINK to the arm's app (never copied / re-signed), passed as a path
#                        RELATIVE to the real root ($ROOT/$BUILD_DIR is how both probes build the path)
#   hard-coded ......... lane3, deck-path (no env, no arg): a PATCHED COPY of the probe under <OUT>/patched/ with exactly two
#                        substrings replaced (the ROOT= resolution -> the real root, the APP= path -> the arm's app); the patch
#                        refuses unless each substring occurs exactly once. The copy's header lists both replacements.
# Row extraction + the verdicts live in g4-rows.py (header there). Pre-registered verdict, printed verbatim below.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
DRY=0; POS=()
for a in "$@"; do if [ "$a" = "--dry-run" ]; then DRY=1; else POS+=("$a"); fi; done
[ "${#POS[@]}" -ge 3 ] || { sed -n '2,8p' "$0" | sed 's/^# \{0,1\}//'; exit 64; }
abspath() { python3 -c 'import os,sys; print(os.path.abspath(sys.argv[1]))' "$1"; }
PRE_APP="$(abspath "${POS[0]}")"; POST_APP="$(abspath "${POS[1]}")"; OUT="$(abspath "${POS[2]}")"; FILTER="${POS[3]:-}"
REAL_ROOT="${G4_ROOT:-/Users/boriskarpman/projects/RealTimeAudio}"
HARM="${PROBE_DIR:-$REAL_ROOT/.harmony}"
PYV="$REAL_ROOT/.venv/bin/python"
BDMODE="${G4_BUILDDIR_MODE:-symlink}"
LIB="${GATE_LOCK_LIB:-/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/267d1ba0-b658-4610-9f47-ef72f4b49872/scratchpad/lib/lock.sh}"   # GATE_LOCK_LIB: test seam only
ROWS_PY="$HERE/g4-rows.py"
PROBES="crossfade render-state deck-clock step3 routines routine-display async-load lane3 mastersignal deck-tabs deck-path media-open video"
RULE="G4 pre-registered verdict (verbatim): FAIL iff the row PASSED on PRE in >= 1 of 2 AND FAILED on POST in 2 of 2; a row red on both arms = PRE-EXISTING (listed); a row missing on an arm = MISSING (listed, never PASS). Final line: \"G4: PASS\" or \"G4: FAIL <rows>\"."

say() { echo "$(date +%T) $*"; }
pq() { printf '%q ' "$@"; echo; }               # print a command with shell quoting
run() { if [ "$DRY" = 1 ]; then printf 'DRY: '; pq "$@"; return 0; else "$@"; fi; }

# --- lock + quit helpers (the rig's lib; LANE=harmony-gate) -------------------------------------------------------
export LANE=harmony-gate
if [ -f "$LIB" ]; then . "$LIB"; else echo "REFUSE: $LIB missing"; [ "$DRY" = 1 ] || exit 64; fi
[ "$DRY" = 1 ] && { acquire_lock() { echo "DRY: acquire_lock (LANE=harmony-gate)"; }; release_lock() { echo "DRY: release_lock"; }; \
                    quit_app() { echo "DRY: quit_app"; }; outwins() { echo "DRY: outwins"; }; adna() { :; }; }
quit_if_running() {   # quit_app only when an Audio-DNA exists (an osascript quit on a closed app could launch it)
  if [ "$DRY" = 1 ]; then echo "DRY: quit_app (only if an Audio-DNA is running)"; return 0; fi
  if [ -n "$(adna)" ]; then quit_app; else echo "no Audio-DNA running (quit_app skipped)"; fi
  LAST_QUIT=$(date +%s)
}
with_timeout() {   # with_timeout SECS cmd... -- hard timeout, kills the whole process group, rc 124 on timeout
  local t=$1; shift
  perl -e 'my $t = shift; my $pid = fork(); die "fork" unless defined $pid;
           if (!$pid) { setpgrp(0, 0); exec @ARGV; exit 127 }
           $SIG{ALRM} = sub { kill "KILL", -$pid; waitpid($pid, 0); exit 124 };
           alarm $t; waitpid($pid, 0); my $s = $?; exit(($s & 127) ? 128 + ($s & 127) : ($s >> 8))' "$t" "$@"
}

# --- per-probe configuration ----------------------------------------------------------------------------------------
# sets: P_SCRIPT P_MODE(agg|seq|ordinal) P_TMO(s) P_KIND(env|builddir|patch) P_APPENV P_PYENV P_BDENV P_X(row filter arg) P_EXTRA(env)
probe_cfg() {
  P_SCRIPT=""; P_MODE=agg; P_TMO=900; P_KIND=env; P_APPENV=""; P_PYENV=""; P_BDENV=""; P_X=""; P_EXTRA=""; P_APPVAR=""
  case "$1" in
    crossfade)       P_SCRIPT=probe-crossfade.sh;       P_MODE=agg;     P_TMO=900;  P_APPENV=XFADE_APP;   P_PYENV=XFADE_PY ;;
    render-state)    P_SCRIPT=probe-render-state.sh;    P_MODE=agg;     P_TMO=2400; P_APPENV=RSTATE_APP;  P_PYENV=RSTATE_PY
                     P_X="r1_temporal,r1_control,r1_ring,r1_retrigger,r1_counts,r1_cells" ;;   # r1_counts waits up to 30 min for a compiler
    deck-clock)      P_SCRIPT=probe-deck-clock.sh;      P_MODE=agg;     P_TMO=900;  P_APPENV=DCLOCK_APP;  P_PYENV=DCLOCK_PY ;;     # its 7 rows are all B1 / B2
    step3)           P_SCRIPT=probe-step3.sh;           P_MODE=seq;     P_TMO=1500; P_KIND=builddir; P_BDENV=STEP3_BUILD_DIR ;;
    routines)        P_SCRIPT=probe-routines.sh;        P_MODE=seq;     P_TMO=1200; P_APPENV=ROUTINES_APP; P_PYENV=ROUTINES_PY; P_EXTRA="ROUTINES_RECORD_PAUSE=1.8" ;;
    routine-display) P_SCRIPT=probe-routine-display.sh; P_MODE=seq;     P_TMO=900;  P_APPENV=ROUTINE_DISPLAY_APP; P_PYENV=ROUTINE_DISPLAY_PY ;;
    async-load)      P_SCRIPT=probe-async-load.sh;      P_MODE=agg;     P_TMO=900;  P_APPENV=ASYNCLOAD_APP; P_PYENV=ASYNCLOAD_PY; P_X="a4_load_then_trigger" ;;
    lane3)           P_SCRIPT=probe-lane3.sh;           P_MODE=ordinal; P_TMO=600;  P_KIND=patch; P_APPVAR='APPBUNDLE="$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app"' ;;
    mastersignal)    P_SCRIPT=probe-mastersignal.sh;    P_MODE=seq;     P_TMO=600;  P_KIND=builddir; P_BDENV=MS_BUILD_DIR ;;
    deck-tabs)       P_SCRIPT=probe-deck-tabs.sh;       P_MODE=seq;     P_TMO=600;  P_APPENV=DECKTABS_APP; P_PYENV=DECKTABS_PY ;;
    deck-path)       P_SCRIPT=probe-deck-path.sh;       P_MODE=ordinal; P_TMO=400;  P_KIND=patch
                     P_APPVAR='APP="$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA"' ;;
    media-open)      P_SCRIPT=probe-media-open.sh;      P_MODE=agg;     P_TMO=600;  P_APPENV=MEDIAOPEN_APP; P_PYENV=MEDIAOPEN_PY; P_X="m6_retrigger_seek" ;;
    video)           P_SCRIPT=probe-video.sh;           P_MODE=agg;     P_TMO=900;  P_APPENV=VIDEO_APP;   P_PYENV=VIDEO_PY
                     P_X="w3_retrigger_midgop_1080,w6b_retrigger_mid_fade" ;;
    *) echo "internal: unknown probe $1"; exit 70 ;;
  esac
}
ROOT_SUBST='ROOT="$(cd "$(dirname "$0")/.." && pwd)"'
BD_SUBST='APPBUNDLE="$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app"'
app_of() { [ "$1" = pre ] && echo "$PRE_APP" || echo "$POST_APP"; }
shim_of() { echo "$OUT/shim/$1"; }
relpath_root() { python3 -c 'import os,sys; print(os.path.relpath(os.path.realpath(sys.argv[1]), os.path.realpath(sys.argv[2])))' "$1" "$REAL_ROOT"; }

setup_probe() {   # setup_probe <probe>: shims / patched copies for that probe (both arms)
  probe_cfg "$1"; local arm app
  for arm in pre post; do
    app="$(app_of $arm)"
    if [ "$P_KIND" = builddir ] && [ "$BDMODE" = symlink ]; then
      run mkdir -p "$(shim_of $arm)/AudioDNA_artefacts/Release"
      run ln -sfn "$app" "$(shim_of $arm)/AudioDNA_artefacts/Release/Audio-DNA.app"
    elif [ "$P_KIND" = patch ] || { [ "$P_KIND" = builddir ] && [ "$BDMODE" = patch ]; }; then
      local vsub="$P_APPVAR" newapp
      [ "$P_KIND" = builddir ] && vsub="$BD_SUBST"
      if [ "$1" = deck-path ]; then newapp="APP=\"$app/Contents/MacOS/Audio-DNA\""; else newapp="APPBUNDLE=\"$app\""; fi
      run mkdir -p "$OUT/patched"
      run python3 "$ROWS_PY" patch "$HARM/$P_SCRIPT" "$OUT/patched/${P_SCRIPT%.sh}.$arm.sh" \
          --replace "$ROOT_SUBST" "ROOT=\"$REAL_ROOT\"" --replace "$vsub" "$newapp"
    fi
  done
}

build_cmd() {   # build_cmd <probe> <arm> <rundir>  -> CMD[] ENVV[]
  probe_cfg "$1"; local arm="$2" rd="$3" app; app="$(app_of $arm)"
  ENVV=(); CMD=()
  [ -n "$P_PYENV" ] && ENVV+=("$P_PYENV=$PYV")
  [ -n "$P_EXTRA" ] && ENVV+=("$P_EXTRA")
  case "$P_KIND" in
    env)      ENVV+=("$P_APPENV=$app"); CMD=(bash "$HARM/$P_SCRIPT") ;;
    builddir) if [ "$BDMODE" = symlink ]; then
                ENVV+=("$P_BDENV=$(relpath_root "$(shim_of $arm)")"); CMD=(bash "$HARM/$P_SCRIPT")
              else CMD=(bash "$OUT/patched/${P_SCRIPT%.sh}.$arm.sh"); fi ;;
    patch)    CMD=(bash "$OUT/patched/${P_SCRIPT%.sh}.$arm.sh") ;;
  esac
  CMD+=("$rd"); [ -n "$P_X" ] && CMD+=("$P_X")
}

run_one() {   # run_one <probe> <arm> <n>
  local probe="$1" arm="$2" n="$3" rd lg rc t0 dt np nf
  probe_cfg "$probe"
  rd="$OUT/$probe/$arm-$n.d"; lg="$OUT/$probe/$arm-$n.log"
  build_cmd "$probe" "$arm" "$rd"
  if [ "$DRY" = 1 ]; then
    printf 'DRY: [%s %s-%s timeout=%ss] ' "$probe" "$arm" "$n" "$P_TMO"
    printf '%q ' env ${ENVV[@]+"${ENVV[@]}"} "${CMD[@]}"
    echo "</dev/null > $lg 2>&1; quit_app"
    return 0
  fi
  mkdir -p "$OUT/$probe"; quit_if_running
  say "RUN $probe $arm-$n (timeout ${P_TMO}s) app=$(app_of $arm)"
  t0=$(date +%s)
  with_timeout "$P_TMO" env ${ENVV[@]+"${ENVV[@]}"} "${CMD[@]}" </dev/null > "$lg" 2>&1; rc=$?
  dt=$(( $(date +%s) - t0 ))
  [ "$rc" = 124 ] && echo "G4-TIMEOUT rc=124 after ${P_TMO}s" >> "$lg"
  echo "G4-RUN probe=$probe arm=$arm n=$n rc=$rc secs=$dt" >> "$lg"
  printf '%s\t%s\t%s\t%s\t%s\n' "$probe" "$arm" "$n" "$rc" "$dt" >> "$OUT/runs.tsv"
  np=$(grep -c '^PASS  ' "$lg"); nf=$(grep -c '^FAIL' "$lg")
  say "DONE $probe $arm-$n rc=$rc ${dt}s  PASS-lines=$np FAIL-lines=$nf"
  quit_if_running
  sleep 3
}

# --- start ----------------------------------------------------------------------------------------------------------
echo "=== g4-parity $(date '+%F %T')  dry=$DRY"
echo "$RULE"
echo "PRE_APP : $PRE_APP"; echo "POST_APP: $POST_APP"; echo "OUT_DIR : $OUT"; echo "probes  : ${FILTER:+filter *$FILTER* of }$PROBES"
echo "PROBE_DIR (both arms run these files): $HARM   buildir-mode: $BDMODE"
if [ "$DRY" = 0 ]; then
  for f in "$PRE_APP" "$POST_APP"; do [ -x "$f/Contents/MacOS/Audio-DNA" ] || { echo "REFUSE: $f is not an Audio-DNA.app bundle"; exit 64; }; done
  [ "$PRE_APP" != "$POST_APP" ] || { echo "REFUSE: PRE_APP == POST_APP"; exit 64; }
  [ -x "$PYV" ] || { echo "REFUSE: $PYV missing"; exit 64; }
  [ -f "$ROWS_PY" ] || { echo "REFUSE: $ROWS_PY missing"; exit 64; }
  for p in $PROBES; do probe_cfg "$p"; [ -f "$HARM/$P_SCRIPT" ] || { echo "REFUSE: $HARM/$P_SCRIPT missing"; exit 64; }; done
  [ -z "$(adna)" ] || { echo "REFUSE: an Audio-DNA is running"; exit 64; }
  echo "PRE  sha256: $(shasum -a 256 "$PRE_APP/Contents/MacOS/Audio-DNA" | cut -c1-16)  $(git -C "$REAL_ROOT" rev-parse --short HEAD 2>/dev/null | sed 's/^/(checkout HEAD /;s/$/)/')"
  echo "POST sha256: $(shasum -a 256 "$POST_APP/Contents/MacOS/Audio-DNA" | cut -c1-16)"
  mkdir -p "$OUT"
  : > "$OUT/runs.tsv"
  trap 'echo "interrupted"; exit 130' INT TERM
  trap 'quit_if_running >/dev/null 2>&1; release_lock >/dev/null 2>&1' EXIT
fi

LAST_QUIT=$(date +%s)
SPECS=""
for probe in $PROBES; do
  case "$probe" in *$FILTER*) ;; *) continue ;; esac
  probe_cfg "$probe"; SPECS="$SPECS $probe:$P_MODE"
  echo "--- probe $probe ($P_SCRIPT, rows keyed '$P_MODE'${P_X:+, row filter: $P_X})"
  setup_probe "$probe"
  acquire_lock || { echo "FATAL: lock not acquired"; exit 2; }
  LK0=$(date +%s)
  for n in 1 2; do run_one "$probe" pre "$n"; run_one "$probe" post "$n"; done
  [ "$DRY" = 1 ] || { held=$(( $(date +%s) - LK0 )); say "lock held $held s for $probe"; [ "$held" -gt 1200 ] && say "WARNING: lock held > 20 min for $probe"; }
  release_lock
done

if [ "$DRY" = 1 ]; then
  echo "DRY: sleep until >= 15 s after the last quit; then:"
  echo "DRY: outwins            # 0 Output-named windows expected"
  echo "DRY: $PYV -c <Quartz kCGWindowListOptionAll count of UserNotificationCenter windows>"
  echo "DRY: python3 $ROWS_PY judge $OUT${SPECS}   # rows.tsv + verdicts"
  echo "DRY: print $OUT/g4-final.txt   # 'G4: PASS' or 'G4: FAIL <rows>' (always the last line)"
  exit 0
fi

# --- after all probes ------------------------------------------------------------------------------------------------
wait_s=$(( 16 - ($(date +%s) - LAST_QUIT) )); [ "$wait_s" -gt 0 ] && sleep "$wait_s"
echo "=== window checks ($(( $(date +%s) - LAST_QUIT )) s after the last quit)"
outwins
"$PYV" -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print('UserNotificationCenter windows (OptionAll): %d' % len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName', ''))]))"
echo "=== verdicts"
python3 "$ROWS_PY" judge "$OUT" $SPECS
rc=$?
cat "$OUT/g4-final.txt"
exit $rc
