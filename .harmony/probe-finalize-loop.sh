#!/bin/bash
# probe-finalize-loop.sh -- s-rta-0924b finalize-truncation live check.
# N record/stop cycles on the PRODUCTION app (built-in mic / input mode by default -- the config
# the defect was observed in; the mechanism is source-mode independent) and FAIL if ANY stop
# reports a finalize problem ("truncated (header N frames < framesWritten N+512)": one device
# block lost at stop, AudioTap::stopInternal ordering) via three INDEPENDENT oracles:
#   (1) /api/perf/status: lastFinalizeError per stop, finalizeErrors run total (F4)
#   (2) disk: each take's audio.unreliableFrom is null AND audio.json frames == audio.wav frames
#   (3) stderr: no "[Recorder] asset ...: truncated" line in the app log
# Rig rules exactly as probe-step3.sh: IPv4 127.0.0.1:7070, launch via `open` with stdout/stderr
# logs, bracket pgrep 'MacOS/Audio-DN[A]', graceful osascript quit, SCREEN-SAFETY (no endpoint
# used here reaches the Output window). Ruling 28: the N takes/assets it creates are NOT deleted
# (names finloop-<stamp>-<i>; the asset ids are printed at the end for Boris).
# Usage: bash .harmony/probe-finalize-loop.sh [N=40]
#   FINLOOP_BUILD_DIR=build-gate   FINLOOP_MODE=input|file (file: STEP3_CLICK_WAV, default
#   /tmp/click_48k.wav, generated if missing)   FINLOOP_HOLD_S=1.0 (base record length; a random
#   0-0.9 s is added per cycle so the stop's phase against the 10.7 ms callback grid varies)
# Statistics: at the observed ~2/23 (8.7%) rate a PRE-FIX run of 40 shows >=1 truncation with
# ~97% probability (0.913^40 = 2.7%); a POST-FIX 0/40 bounds the residual rate below ~7.2% (95%,
# rule of three). Use N=100 when time allows (0/100 -> <3%). The deterministic unit test
# (test_audio_tap_sync "[truncation]") is the proof; this loop is the live smoke check.
# NOT covered here (finalize-truncation-critic.md): a push() already in flight when stop() begins
# being dropped with NO counter disagreeing -- that window is nanoseconds wide and invisible to all
# three oracles; only test_audio_tap_sync's "already in flight when stop() begins" case guards it.
# A clean run of this loop must not be read as clearing that ordering.
set -u
N="${1:-40}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${FINLOOP_BUILD_DIR:-build-gate}"
APPBUNDLE="$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app"
A='http://127.0.0.1:7070'
MODE="${FINLOOP_MODE:-input}"
HOLD="${FINLOOP_HOLD_S:-1.0}"
CLICK_WAV="${STEP3_CLICK_WAV:-/tmp/click_48k.wav}"
TAKES_DIR="$HOME/Documents/Audio-DNA/Takes"
AUDIO_DIR="$HOME/Documents/Audio-DNA/Audio"
ERRLOG=/tmp/adna-finloop-err.log
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
perf_field(){ curl -s --max-time 5 "$A/api/perf/status" | python3 -c "
import json, sys
try:
    d = json.load(sys.stdin); v = d.get('$1', 'NA'); print('' if v is None else v)
except Exception as e:
    print('NA(%s)' % e)"; }
wait_field(){ # $1 field $2 expected(python truthiness word: true|false) $3 max tenths
  local i=0; while [ $i -lt "$3" ]; do v="$(perf_field "$1")"; case "$2:$v" in true:True|true:true|false:False|false:false) return 0;; esac; sleep 0.1; i=$((i+1)); done; return 1; }

echo "$N" | grep -Eq '^[1-9][0-9]*$' || { echo "REFUSE: N must be a positive integer (got '$N')"; exit 64; }
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE (FINLOOP_BUILD_DIR overrides)"; exit 64; }
if [ "$MODE" = "file" ] && [ ! -f "$CLICK_WAV" ]; then
    python3 "$ROOT/.harmony/gen-click-wav.py" "$CLICK_WAV" --interval 24000 >/dev/null || { echo "REFUSE: click WAV generation failed"; exit 64; }
fi

: > "$ERRLOG"
open --stdout /tmp/adna-finloop-out.log --stderr "$ERRLOG" "$APPBUNDLE"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: /api/health never answered (mic-permission prompt? screencapture -x and LOOK)"; exit 1; }
ok "app launched, /api/health answered"
sleep 3
FIN0="$(perf_field finalizeErrors)"
[ "$FIN0" = "0" ] && ok "perf/status finalizeErrors == 0 at start" || no "perf/status finalizeErrors == '$FIN0' at start (field missing = build without F4)"

STAMP="$(date +%H%M%S)"; TRUNC=0; CYCLES_OK=0; ASSETS=""
for i in $(seq 1 "$N"); do
    NAME="finloop-$STAMP-$i"
    if [ "$MODE" = "file" ]; then BODY="{\"name\":\"$NAME\",\"audio\":true,\"audioFile\":\"$CLICK_WAV\"}"; else BODY="{\"name\":\"$NAME\",\"audio\":true}"; fi
    curl -s --max-time 6 -X POST "$A/api/perf/record" -H 'Content-Type: application/json' -d "$BODY" >/dev/null
    if ! wait_field recording true 50; then no "cycle $i: recording never became true"; continue; fi
    AM="$(perf_field audioMode)"
    if [ "$i" = "1" ]; then
        [ "$AM" = "$MODE" ] && ok "cycle 1: audioMode == $MODE" || no "cycle 1: audioMode == '$AM', expected '$MODE' (switch the app's audio source, or set FINLOOP_MODE)"
    fi
    sleep "$(awk -v h="$HOLD" -v r="$RANDOM" 'BEGIN{printf "%.3f", h + (r % 900) / 1000.0}')"
    FW="$(perf_field framesWritten)"
    curl -s --max-time 6 -X POST "$A/api/perf/stop" >/dev/null
    if ! wait_field recording false 50; then no "cycle $i: recording never became false after stop"; continue; fi
    FIN="$(perf_field lastFinalizeError)"; LERR="$(perf_field lastError)"
    ASSET="$(perf_field assetId)"; ASSETS="$ASSETS $ASSET"
    TF="$TAKES_DIR/$NAME.adna-take"
    DISK="$(python3 - "$TF/take.json" "$AUDIO_DIR/$ASSET.adna-audio" <<'PY'
import json, sys, wave, os
try:
    t = json.load(open(sys.argv[1])); seg = t['audio']['segments'][0]
    a = json.load(open(os.path.join(sys.argv[2], 'audio.json')))
    w = wave.open(os.path.join(sys.argv[2], 'audio.wav'), 'rb'); n = w.getnframes()
    print("segframes=%d sidecarframes=%d wavframes=%d unreliableFrom=%s" % (seg['frames'], a['frames'], n, t['audio'].get('unreliableFrom')))
except Exception as e:
    print("NA(%s)" % e)
PY
)"
    if [ -n "$FIN" ]; then
        TRUNC=$((TRUNC+1)); echo "  cycle $i: TRUNCATION  lastFinalizeError='$FIN' lastError='$LERR' framesWritten(before stop)=$FW  $DISK"
    else
        case "$DISK" in
            *"unreliableFrom=None"*) SF="${DISK#*segframes=}"; SF="${SF%% *}"; WF="${DISK#*wavframes=}"; WF="${WF%% *}"
                if [ "$SF" = "$WF" ] && [ "${FW:-0}" != "0" ] && [ "$FW" != "NA" ]; then CYCLES_OK=$((CYCLES_OK+1)); echo "  cycle $i: clean  $DISK"; else no "cycle $i: status clean but disk/frames disagree ($DISK framesWritten=$FW)"; fi;;
            *) no "cycle $i: status clean but disk says $DISK";;
        esac
    fi
done

FIN_END="$(perf_field finalizeErrors)"
[ "$TRUNC" = "0" ] && ok "$N record/stop cycles, 0 stops reported a finalize problem (oracle 1: lastFinalizeError)" || no "$N record/stop cycles, $TRUNC truncated stop(s) (oracle 1)"
[ "$FIN_END" = "$TRUNC" ] && ok "perf/status finalizeErrors ($FIN_END) agrees with the per-cycle count ($TRUNC)" || no "perf/status finalizeErrors ($FIN_END) != per-cycle count ($TRUNC)"
[ "$CYCLES_OK" = "$N" ] && ok "disk: all $N takes have unreliableFrom null and take frames == audio.wav frames (oracle 2)" || no "disk: only $CYCLES_OK of $N takes are clean on disk (oracle 2)"
# grep -c prints "0" AND exits 1 on no match -- never append "|| echo 0" (that yields "0\n0").
TL="$(grep -c 'truncated (header' "$ERRLOG" 2>/dev/null)"; TL="${TL:-0}"
[ "$TL" = "0" ] && ok "stderr: 0 '[Recorder] asset ...: truncated' lines (oracle 3)" || no "stderr: $TL truncated-asset line(s) in $ERRLOG (oracle 3)"

osascript -e 'quit app "Audio-DNA"' >/dev/null 2>&1
for _ in $(seq 1 30); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { pkill -f 'MacOS/Audio-DN[A]' >/dev/null 2>&1; sleep 2; }
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && no "APP STILL RUNNING after graceful quit + pkill" || ok "app terminated"
echo; echo "$N cycles, $TRUNC truncations.  $PASS PASS / $FAIL FAIL"
echo "assets created this run (Ruling 28, not deleted):$ASSETS"
echo "takes: $TAKES_DIR/finloop-$STAMP-*.adna-take"
[ "$FAIL" -eq 0 ] || exit 1
