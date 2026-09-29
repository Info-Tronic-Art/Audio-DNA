#!/bin/bash
# probe-video-w10-all.sh -- s-rta-0929 vupload (plan-vupload.md HARMONY ADOPTION ADDENDUM 2, VU17): probe-video.sh's
# w10_pixel_identity on ALL THREE video upload paths in ONE invocation, against ONE reference dir:
#   blit    (no env)                                  -> every "[VideoPlayer] Opened:" line says upload=iosurface-blit
#   client  (ADNA_VIDEO_FORCE_FALLBACK=client)       -> upload=iosurface-client
#   malloc  (ADNA_VIDEO_FORCE_FALLBACK=malloc)       -> upload=malloc
# Each arm is one launch of .harmony/probe-video.sh w10_pixel_identity with VIDEO_REF_DIR = <ref-dir> (the pre-lane app's
# captures, written by a VIDEO_REF_WRITE run): identity, code band, alpha ramp and VU5 are that row's own asserts. The
# wrapper adds the WITNESS that the arm really took its path: the arm's err.log carries >= 1 Opened line and every Opened
# line names the arm's path (an app that ignores the TEST-ONLY hook, or predates the "upload=" field, is RED here).
# The env hook exists only in AUDIODNA_TEST_SERVER builds.
#
# usage: probe-video-w10-all.sh <ref-dir> [out-base]
#   VIDEO_APP / VIDEO_PY / VIDEO_FIXTURES as .harmony/probe-video.sh (VIDEO_ENV is set per arm by this wrapper).
# The caller holds /tmp/audiodna-live.lock (each arm's probe-video.sh re-checks it and refuses without it). Screen-safe as
# probe-video.sh: open -g, production mode, no Output window, no screen capture, graceful quit.
# Ends with "PROBE-VIDEO-W10-ALL GREEN" (exit 0) only when all three arms are PROBE-VIDEO GREEN and witnessed.
set -u
REF="${1:-}"; BASE="${2:-/tmp}"
[ -n "$REF" ] && [ -d "$REF" ] || { echo "REFUSE: usage: probe-video-w10-all.sh <ref-dir> [out-base] (ref-dir = the pre-lane w10 captures)"; exit 64; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
RC=0; SUMMARY=()
for arm in blit client malloc; do
  case $arm in
    blit)   env="";                                 want="iosurface-blit" ;;
    client) env="ADNA_VIDEO_FORCE_FALLBACK=client"; want="iosurface-client" ;;
    malloc) env="ADNA_VIDEO_FORCE_FALLBACK=malloc"; want="malloc" ;;
  esac
  mkdir -p "$BASE/w10-$arm"
  echo "=== arm $arm (VIDEO_ENV='${env}', expect upload=$want) $(date +%T)"
  LOG="$(mktemp "$BASE/w10-$arm/run.XXXXXX")"
  VIDEO_ENV="$env" VIDEO_REF_DIR="$REF" bash "$ROOT/.harmony/probe-video.sh" "$BASE/w10-$arm" w10_pixel_identity > "$LOG" 2>&1; prc=$?
  grep -E '^(PASS|FAIL|INFO)' "$LOG" | sed 's/^/  /'
  OUT="$(sed -n 's/^out: //p' "$LOG" | head -1)"
  opened=0; right=0
  if [ -n "$OUT" ] && [ -f "$OUT/err.log" ]; then
    opened="$(grep -c '^\[VideoPlayer\] Opened:' "$OUT/err.log")"
    right="$(grep '^\[VideoPlayer\] Opened:' "$OUT/err.log" | grep -c ", upload=$want,")"
  fi
  py="$(grep -E '^PY [0-9]+ PASS / [0-9]+ FAIL' "$LOG" | tail -1)"; verdict="$(tail -1 "$LOG")"
  if [ "$opened" -ge 1 ] && [ "$opened" -eq "$right" ]; then
    echo "PASS  $arm: witness -- $right / $opened Opened lines say upload=$want"; wit=ok
  else
    echo "FAIL  $arm: witness -- $right / $opened Opened lines say upload=$want (the arm did not take its path)"; wit=FAIL
  fi
  { [ $prc -eq 0 ] && [ "$wit" = ok ]; } || RC=1
  SUMMARY+=("ARM $arm: ${py:-no PY line} | $verdict | witness $right/$opened upload=$want | log $LOG")
done
echo; printf '%s\n' "${SUMMARY[@]}"
[ "$RC" -eq 0 ] && echo "PROBE-VIDEO-W10-ALL GREEN" || echo "PROBE-VIDEO-W10-ALL RED"
exit "$RC"
