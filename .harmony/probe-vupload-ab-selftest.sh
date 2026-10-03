#!/bin/bash
# probe-vupload-ab-selftest.sh -- s-rta-1002b lane bf10 fix round (Harmony ruling R3 on review-bf10-gates-r1 SHOULD 3).
# Proves that .harmony/probe-vupload-ab.sh samples EVERY launch attempt for compilers, including the re-run of a
# tainted launch: a re-run keeps the same "$LOG" name, so a "$LOG.done" left by the previous attempt used to end the
# sampler before its first sample (empty .compilers -> "compilers seen 0" -> a compiler-loaded re-run was accepted).
# How (no app, no lock, no real compiler): the REAL driver runs 1 round with
#   LOCK_LIB  = a stub helper (acquire_quiet_lock / release_lock succeed, adna = no app, outwins prints zeros);
#   SCRIPT    = a stub probe that sleeps 5 s and prints a DATA line naming its attempt number; on attempts 1 and 2 a
#               fake compiler "runs" for those 5 s (a flag file);
#   PATH shim = pgrep answers "clang" with a fake pid while the flag file exists (nothing else);
#   VIDEO_PY  = a stub python (the UserNotificationCenter count prints 0; the summary is skipped).
# Expected: r1 A attempt 1 TAINTED, its re-run (attempt 2) TAINTED too, attempt 3 accepted; r1 B (attempt 4) accepted.
# usage: probe-vupload-ab-selftest.sh [probe-vupload-ab.sh]   (default: the sibling of this file)
# Prints one "ok" / "FAIL" line per check, then "SELFTEST <n> ok / <m> FAIL"; exit 0 iff no FAIL.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
DRV="${1:-$HERE/probe-vupload-ab.sh}"
echo "target driver: $DRV"
D="$(mktemp -d "${TMPDIR:-/tmp}/ab-selftest.XXXXXX")" || exit 64
mkdir -p "$D/shim" "$D/A.app/Contents/MacOS" "$D/B.app/Contents/MacOS"
echo A > "$D/A.app/Contents/MacOS/Audio-DNA"; echo B > "$D/B.app/Contents/MacOS/Audio-DNA"
cat > "$D/shim/pgrep" <<'EOF'
#!/bin/bash
[ "${1:-}" = -x ] && [ "${2:-}" = clang ] && [ -f "$SELFTEST_D/compiler-on" ] && { echo 99999; exit 0; }
exit 1
EOF
cat > "$D/stubpy" <<'EOF'
#!/bin/bash
[ "${1:-}" = -c ] && { echo 0; exit 0; }
echo "summary skipped (selftest)"
EOF
cat > "$D/lock.sh" <<'EOF'
acquire_quiet_lock(){ return 0; }
release_lock(){ return 0; }
adna(){ :; }
outwins(){ echo "audio-dna windows 0, Output-named 0"; }
EOF
cat > "$D/probe.sh" <<'EOF'
#!/bin/bash
n=$(( $(cat "$SELFTEST_D/attempts" 2>/dev/null || echo 0) + 1 )); echo $n > "$SELFTEST_D/attempts"
[ $n -le 2 ] && touch "$SELFTEST_D/compiler-on"
sleep 5
rm -f "$SELFTEST_D/compiler-on"
case "$VIDEO_APP" in */A.app) cp "$SELFTEST_D/out/r1_A.log.compilers" "$SELFTEST_D/compilers-attempt-$n" 2>/dev/null;; esac
echo "DATA stub attempt=$n"
EOF
chmod +x "$D/shim/pgrep" "$D/stubpy"
OUT="$(env PATH="$D/shim:$PATH" SELFTEST_D="$D" LOCK_LIB="$D/lock.sh" LANE=selftest VIDEO_PY="$D/stubpy" \
      bash "$DRV" "$D/A.app" "$D/B.app" 1 "$D/probe.sh" stub "$D/out" 2>&1)"; RC=$?
echo "$OUT" | grep -E '\[r1 |TAINTED|rounds ' | sed 's/^/   | /'
OK=0; BAD=0
check(){ if [ "$1" = 1 ]; then OK=$((OK+1)); echo "   ok    $2"; else BAD=$((BAD+1)); echo "   FAIL  $2"; fi; }
for n in 1 2 3; do
  f="$D/compilers-attempt-$n"
  if [ -f "$f" ]; then echo "   attempt $n (r1 A) .compilers lines so far: $(wc -l < "$f" | tr -d ' '), compiler samples: $(awk '$2 > 0' "$f" | wc -l | tr -d ' ')"
  else echo "   attempt $n: not an r1 A launch"; fi
done
check "$([ $RC = 0 ] && echo 1)" "driver exit 0 (got $RC)"
check "$([ "$(awk '$2 > 0' "$D/compilers-attempt-2" 2>/dev/null | wc -l | tr -d ' ')" -ge 1 ] && echo 1)" "the re-run (attempt 2) has a non-empty .compilers file with the compiler sampled"
check "$([ "$(grep -c '\[r1 A\] rc 0, compilers seen 1' <<< "$OUT")" = 2 ] && echo 1)" "r1 A: attempt 1 AND its re-run both report 'compilers seen 1'"
check "$(grep -q 'rounds 1 x 2 arms done (tainted 2)' <<< "$OUT" && echo 1)" "the run ends with 'tainted 2'"
check "$(grep -qx '1 A DATA stub attempt=3' "$D/out/ab.tsv" 2>/dev/null && echo 1)" "ab.tsv keeps r1 A from attempt 3 (the first launch with no compiler)"
check "$(grep -qx '1 B DATA stub attempt=4' "$D/out/ab.tsv" 2>/dev/null && echo 1)" "ab.tsv keeps r1 B from attempt 4"
rm -rf "$D"
echo; echo "SELFTEST $OK ok / $BAD FAIL"
[ "$BAD" -eq 0 ]
