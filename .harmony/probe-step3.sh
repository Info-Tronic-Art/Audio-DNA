#!/bin/bash
# probe-step3.sh -- the STEP 3 live gate. s-rta-0923 / s-rta-0924.
# Recipe: .harmony/specs/s-rta-0923-step3-plan.md section 4, AS AMENDED by
# .harmony/.reports/s-rta-0924/step3-critic.md AMENDMENTS A5/A6 (BINDING,
# overrides the plan's own section 4 text wherever they conflict). Lane
# S3-G (this file + the fixture + the click-WAV generator) is a SCAFFOLD
# lane per this dispatch's builder packet: it scripts the recipe against
# endpoints (/api/perf/*) that S3-B/S3-C build in the SAME wave, so this
# script CANNOT be run yet. Validate instead with:
#   bash -n .harmony/probe-step3.sh
#   shellcheck .harmony/probe-step3.sh        # if installed
#   python3 -m json.tool .harmony/probe-step3.json
# Harmony runs this for real once the wave lands ("the party that builds
# never verifies" -- same rule as probe-lane3.sh) against ONE clean forced
# rebuild in build-gate/.
#
# WHAT IT PROVES (plan section 4 + critic A6)
#   1. Discrete half: arm (file-mode audio, onset markers) -> perform over
#      REST (4 clip triggers, a tempo change, a column trigger, a
#      continuous opacity write) -> stop -> the resulting take.json/audio
#      store are internally consistent (Ruling 28 v3 format) and the T2
#      click-track alignment numbers are within the D10.3 bound.
#   2. Load + replay (both WithAudio and WallClock) reproduces the
#      recorded activeClipColumn sequence 0->1->2->3 and the tempo change.
#   3. Overdub safety (R5): replay never re-records itself.
#   4. R13 (critic B5, HANDOFF :2633, LIVE): a device-rate/analysis-rate
#      mismatch is now machine-checked as an ARM PRECONDITION, not just a
#      disclosed risk -- this gate REFUSES to arm on a mismatched device
#      rather than silently recording a take with an unreliable beat clock.
#
# RIG FACTS (do not re-derive -- .harmony/gotchas.md, .harmony/VALIDATION.md,
# probe-lane3.sh's own header, all re-confirmed by the critic this session):
#   * ApiServer = http://127.0.0.1:7070 (IPv4 ONLY), always on in production
#     mode -- NO --test-mode (that starts no analysis thread, T4/critic B5;
#     a step-3 gate on --test-mode would prove nothing about the live beat
#     grid -- plan section 4's own "secondary path" paragraph).
#   * Launch via `open`, not the raw binary (gotcha 2026-07-17). First
#     launch after a rebuild needs the mic-permission (TCC) prompt clicked
#     once -- if /api/health never answers, screencapture -x and LOOK
#     before concluding (same as probe-lane3.sh).
#   * SELF-MATCH: `pgrep -f 'MacOS/Audio-DNA'` matches ITS OWN argv (pgrep's
#     command line contains the literal search string), which breaks any
#     "wait until gone" loop -- this script never writes the bare literal
#     "MacOS/Audio-DNA" as a pgrep -f pattern; every occurrence below uses
#     the bracket trick 'MacOS/Audio-DN[A]' (rig rule, this dispatch's
#     builder packet).
#   * SCREEN-SAFETY LAW: never open the Output window (title "Audio-DNA
#     Output"); no endpoint this recipe touches reaches
#     Renderer/MainComponent's openOutputOnDisplay path (critic A6
#     re-verified the call sites: MainComponent.cpp :498, :672, :3076,
#     :4745 -- menu/selector/keyboard only). Teardown is a GRACEFUL
#     `osascript` quit (NOT pkill) so ~MainComponent runs
#     recorderHost_.shutdown() before the process exits (plan section 4
#     Teardown paragraph) -- pkill only as an absolute last-resort fallback
#     if the graceful quit does not clear the process within the wait loop.
#   * R13 device-rate precondition (critic A5/A6): this gate REFUSES to
#     arm if /api/perf/status reports deviceRate != 48000 or rateMismatch
#     == true, and if the app's stderr log contains the analysis-thread's
#     own "assumes" warning. Reconnecting a non-48kHz device (e.g. the
#     soundcore P31i BT headset at 16kHz, HANDOFF :2633) is a REFUSE, not a
#     silent low-fidelity take.
#
# WHAT THIS SCRIPT DOES NOT COVER (disclosed, not silent -- critic non-
# blocking findings N2, N6, N8, N11):
#   * Deck-switch replay (Origin::Engine at the genre caller, handleDeckSwitch
#     capture) -- the critic's fixture (.harmony/probe-step3.json, layer 0
#     columns 0..3, ONE deck) intentionally does not define a second deck;
#     S3-A's headless test_recorder_host suite (test 8, "discrete replay
#     fires through Dispatch in (at,seq) order") is the load-bearing check
#     for that path. Adding a second deck to broaden this gate is future
#     work, not this lane's scope.
#   * The continuous-write "GRIP" oracle (probe-lane3.sh's exact-timing
#     held/resume pattern) is NOT reproduced here for record+replay of a
#     continuous lane -- s167's synthesized-Decaying-end window is
#     `comp.gripHoldMs` (critic N7), not a hardcoded constant, so a portable
#     timing oracle needs to read that value first; leaving EXACT-TIMING
#     verification to Boris's MIDI-grip-feel check (plan section 5,
#     only-Boris item 3). Two things this gate DOES check now (independent
#     review fix, this session): (1) the STRUCTURAL claim on take.json's
#     layer/scalar:opacity lane -- exactly one gesture, grip "decaying",
#     >=3 breakpoints (`curve` is the flat breakpoint array per
#     AutomationCurve::toVar, src/model/ControlPath.h + Lane.h schema,
#     re-derived from source, not the plan's prose); (2) a replay-shape
#     check -- during withAudio replay this gate samples layer 0 opacity
#     over the glide window and asserts it visits an intermediate value
#     near the 0.7 peak before settling near the final 0.2, i.e. it
#     glides rather than jumps -- NOT an exact-timing check.
#   * The crash-readability row (plan section 4, "optional, destructive")
#     is included but gated behind STEP3_RUN_CRASH_TEST=1 (unset by
#     default) since it `kill -9`s the running app.
set -u
OUT="${1:-/tmp/audiodna-step3}"; mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${STEP3_BUILD_DIR:-build-gate}"
APPBUNDLE="$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app"
A='http://127.0.0.1:7070'
FIXTURE="$ROOT/.harmony/probe-step3.json"
CLICK_WAV="${STEP3_CLICK_WAV:-/tmp/click_48k.wav}"
TAKES_DIR="$HOME/Documents/Audio-DNA/Takes"
AUDIO_DIR="$HOME/Documents/Audio-DNA/Audio"
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
skip(){ echo "SKIP  $1"; }

# --- helpers -----------------------------------------------------------
# jpath NAME PYEXPR: evaluate PYEXPR against the parsed JSON `d` returned by
# a GET (or the JSON echoed back by a POST). "NA" on any parse/key error --
# callers treat "NA" as "field absent" (probe-lane3.sh convention).
jget(){ # $1 = URL  $2 = python expr against `d`
  curl -s --max-time 5 "$1" | python3 -c "
import json, sys
try:
    d = json.load(sys.stdin)
    print($2)
except Exception as e:
    print('NA(%s)' % e)
"
}
comp_active_col(){ jget "$A/api/composition" "d['decks'][0]['layers'][0]['activeClipColumn']"; }
perf_status(){ curl -s --max-time 5 "$A/api/perf/status"; }
perf_field(){ perf_status | python3 -c "
import json, sys
try:
    d = json.load(sys.stdin)
    print($1)
except Exception as e:
    print('NA(%s)' % e)
"; }
take_field(){ # $1 = folder  $2 = python expr against `d` (parsed take.json)
  python3 -c "
import json
try:
    with open('$1/take.json') as f:
        d = json.load(f)
    print($2)
except Exception as e:
    print('NA(%s)' % e)
"
}

# --- 0. click-track WAV --------------------------------------------------
python3 "$ROOT/.harmony/gen-click-wav.py" "$CLICK_WAV" >/dev/null \
  && ok "click-track WAV generated at $CLICK_WAV" \
  || { no "click-track WAV generation FAILED"; }

# --- 1. preconditions ------------------------------------------------------
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE (set STEP3_BUILD_DIR to override the build dir name)"; exit 64; }
[ -f "$CLICK_WAV" ] || { echo "REFUSE: click WAV missing after generation step"; exit 64; }
python3 -m json.tool "$FIXTURE" >/dev/null 2>&1 && ok "fixture $FIXTURE is valid JSON" || no "fixture $FIXTURE is NOT valid JSON"

# --- 2. launch (production, NO --test-mode -- T4) -------------------------
open --stdout /tmp/adna-step3-out.log --stderr /tmp/adna-step3-err.log "$APPBUNDLE"
for i in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: health never came up on $A (production launch needs the mic-permission prompt clicked once -- screencapture -x and LOOK before concluding)"; exit 1; }
PID="$(pgrep -f 'MacOS/Audio-DN[A]' | head -1)"
[ -n "$PID" ] || { echo "FAIL: health answered but no Audio-DNA process was found"; exit 1; }
ok "app launched, /api/health answered, PID=$PID"

curl -s --max-time 6 http://127.0.0.1:7070/api/bpm >/dev/null
sleep 10
B1="$(jget "$A/api/bpm" "d.get('totalBarCount','NA')")"
[ "$B1" != "NA" ] && ok "analysis thread live: totalBarCount readable ($B1)" || no "totalBarCount not readable -- is the analysis thread running? (production mode required, T4)"

# --- 3. load the 4-clip fixture BEFORE arm (critic A6/B4: the default
#        deck has no clips -- trigger_clip on an empty cell CLEARS the
#        layer instead of activating it) -----------------------------------
curl -s --max-time 6 -X POST "$A/api/load_composition" -H 'Content-Type: application/json' \
  -d "{\"path\":\"$FIXTURE\"}" | grep -q '"ok":[[:space:]]*true' \
  && ok "load_composition accepted probe-step3.json" || no "load_composition rejected probe-step3.json"
sleep 1

# --- 4. R13 precondition oracle (critic A5/A6/B5) --------------------------
# The gate REFUSES to arm on a device-rate/analysis-rate mismatch rather
# than silently recording a take whose beat/bpm lane is wrong. The
# analysis-thread ctor warning is grepped from stderr, not stdout (the
# warning is a std::cerr line per MainComponent.cpp :547-560 (critic T33
# region) / AnalysisThread's own log).
DEV_RATE="$(perf_field "d.get('deviceRate','NA')")"
RATE_MISMATCH="$(perf_field "d.get('rateMismatch','NA')")"
if [ "$DEV_RATE" = "48000" ] && [ "$RATE_MISMATCH" = "False" -o "$RATE_MISMATCH" = "false" ]; then
    ok "R13 precondition: deviceRate=48000, rateMismatch=false"
else
    no "R13 precondition FAILED: deviceRate=$DEV_RATE rateMismatch=$RATE_MISMATCH -- REFUSING to arm (a non-48kHz device makes the beat/bpm lane unreliable, HANDOFF :2633)"
fi
if grep -q "analysis pipeline assumes" /tmp/adna-step3-err.log 2>/dev/null; then
    no "R13 precondition FAILED: stderr carries the analysis-rate-assumption warning"
else
    ok "R13 precondition: no analysis-rate-assumption warning on stderr"
fi
R13_OK=0
[ "$DEV_RATE" = "48000" ] && { [ "$RATE_MISMATCH" = "False" ] || [ "$RATE_MISMATCH" = "false" ]; } \
  && ! grep -q "analysis pipeline assumes" /tmp/adna-step3-err.log 2>/dev/null && R13_OK=1

if [ "$R13_OK" != "1" ]; then
    echo "R13 precondition failed -- skipping the arm/perform/stop/replay rows below (would only produce an untrustworthy take)."
else

# --- 5. arm with deterministic audio (file mode) + onset markers (T2) -----
TAKE_NAME="step3gate1"
curl -s --max-time 6 -X POST "$A/api/perf/record" -H 'Content-Type: application/json' \
  -d "{\"name\":\"$TAKE_NAME\",\"audio\":true,\"audioFile\":\"$CLICK_WAV\",\"onsetMarkers\":true}" \
  | grep -q '"ok":[[:space:]]*true' && ok "perf/record accepted (file-mode audio, onset markers)" \
  || no "perf/record refused"
sleep 2
REC="$(perf_field "d.get('recording','NA')")"
[ "$REC" = "True" -o "$REC" = "true" ] && ok "perf/status: recording=true" || no "perf/status: recording is not true ($REC)"
ASSET="$(perf_field "d.get('assetId','NA')")"
echo "$ASSET" | grep -Eq '^[0-9a-f]{32}$' && ok "assetId looks like a 32-hex fingerprint-less id ($ASSET)" || no "assetId ($ASSET) does not look like 32 hex chars"
GAPD="$(perf_field "d.get('gapDetection','NA')")"
[ "$GAPD" != "NA" ] && ok "perf/status: gapDetection present ($GAPD)" || no "perf/status: gapDetection missing (R14 glue not reaching Status)"
FRAMES1="$(perf_field "d.get('framesWritten','NA')")"
awk -v x="$FRAMES1" 'BEGIN{exit !(x+0>0)}' 2>/dev/null && ok "perf/status: framesWritten>0 ($FRAMES1)" || no "perf/status: framesWritten not >0 ($FRAMES1)"

TAKE_FOLDER="$TAKES_DIR/$TAKE_NAME.adna-take"
[ -f "$TAKE_FOLDER/take.json" ] && ok "provisional take.json exists at arm+2s (v2 5.6 #1)" || no "no provisional take.json at $TAKE_FOLDER/take.json"
MTIME_ARM="$(stat -f %m "$TAKE_FOLDER/take.json" 2>/dev/null || echo 0)"

# --- 6. perform over REST (each a Human-origin capture) --------------------
for c in 0 1 2 3; do
    curl -s --max-time 6 -X POST "$A/api/trigger_clip" -H 'Content-Type: application/json' -d "{\"layer\":0,\"column\":$c}" >/dev/null
    sleep 3
    got="$(comp_active_col)"
    [ "$got" = "$c" ] && ok "trigger_clip column $c -> activeClipColumn=$c" || no "trigger_clip column $c -> activeClipColumn=$got (expected $c)"
done
curl -s --max-time 6 -X POST "$A/api/set_bpm" -H 'Content-Type: application/json' -d '{"bpm":128}' >/dev/null
sleep 2
BPM_NOW="$(jget "$A/api/bpm" "d.get('bpm','NA')")"
awk -v x="$BPM_NOW" 'BEGIN{exit !(x+0>=127.5 && x+0<=128.5)}' 2>/dev/null && ok "set_bpm(128) took effect (bpm=$BPM_NOW)" || no "set_bpm(128) did not take effect (bpm=$BPM_NOW)"
curl -s --max-time 6 -X POST "$A/api/trigger_column" -H 'Content-Type: application/json' -d '{"column":2}' >/dev/null
sleep 2
# Continuous row (critic N6 wording -- structural check only, see header):
curl -s --max-time 6 -X POST "$A/api/set_layer_opacity" -H 'Content-Type: application/json' -d '{"layer":0,"opacity":0.4}' >/dev/null
sleep 1
curl -s --max-time 6 -X POST "$A/api/set_layer_opacity" -H 'Content-Type: application/json' -d '{"layer":0,"opacity":0.7}' >/dev/null
sleep 1
curl -s --max-time 6 -X POST "$A/api/set_layer_opacity" -H 'Content-Type: application/json' -d '{"layer":0,"opacity":0.2}' >/dev/null
sleep 1
OP_NOW="$(jget "$A/api/composition" "d['decks'][0]['layers'][0]['opacity']")"
awk -v x="$OP_NOW" 'BEGIN{exit !(x+0>=0.15 && x+0<=0.25)}' 2>/dev/null && ok "set_layer_opacity(0.2) landed (opacity=$OP_NOW)" || no "set_layer_opacity(0.2) did not land (opacity=$OP_NOW)"

sleep 40   # cross the RecorderHost::kCheckpointSeconds=60s periodic-save boundary (arm was ~t=0)
MTIME_NOW="$(stat -f %m "$TAKE_FOLDER/take.json" 2>/dev/null || echo 0)"
[ "$MTIME_NOW" -gt "$MTIME_ARM" ] 2>/dev/null && ok "take.json mtime advanced past the arm-time write ($MTIME_ARM -> $MTIME_NOW, periodic-save fired)" || no "take.json mtime did NOT advance past the arm-time write (arm=$MTIME_ARM now=$MTIME_NOW) -- periodic-save may not have fired"

# --- 7. stop -----------------------------------------------------------
curl -s --max-time 6 -X POST "$A/api/perf/stop" >/dev/null
sleep 3
REC2="$(perf_field "d.get('recording','NA')")"
[ "$REC2" = "False" -o "$REC2" = "false" ] && ok "perf/status: recording=false after stop" || no "perf/status: still recording after stop ($REC2)"
LASTERR="$(perf_field "d.get('lastError','NA')")"
[ -z "$LASTERR" -o "$LASTERR" = "" ] && ok "perf/status: lastError empty" || no "perf/status: lastError=$LASTERR"

# --- 8. disk checks (Ruling 28 v3 format) -------------------------------
ASSET_DIR="$AUDIO_DIR/$ASSET.adna-audio"
[ -f "$ASSET_DIR/audio.wav" ] && [ -f "$ASSET_DIR/audio.json" ] \
  && ok "Audio store has audio.wav + audio.json ($ASSET_DIR)" \
  || no "Audio store missing audio.wav/audio.json at $ASSET_DIR"
TAKE_FILES="$(ls -1 "$TAKE_FOLDER" 2>/dev/null | wc -l | tr -d ' ')"
[ "$TAKE_FILES" = "1" ] && ok "take folder has exactly one file (take.json only, Ruling 28)" || no "take folder has $TAKE_FILES files, expected 1 (take.json only)"

VERSION="$(take_field "$TAKE_FOLDER" "d.get('version','NA')")"
[ "$VERSION" = "3" ] && ok "take.json version == 3" || no "take.json version == $VERSION (expected 3)"
SEG_RATE="$(take_field "$TAKE_FOLDER" "d['audio']['segments'][0]['rate']")"
[ "$SEG_RATE" = "48000" ] && ok "audio.segments[0].rate == 48000" || no "audio.segments[0].rate == $SEG_RATE"
SEG_FRAMES="$(take_field "$TAKE_FOLDER" "d['audio']['segments'][0]['frames']")"
awk -v x="$SEG_FRAMES" 'BEGIN{exit !(x+0>0)}' 2>/dev/null && ok "audio.segments[0].frames > 0 ($SEG_FRAMES)" || no "audio.segments[0].frames not >0 ($SEG_FRAMES)"
FP="$(take_field "$TAKE_FOLDER" "d['audio']['segments'][0]['fingerprint']")"
echo "$FP" | grep -q '^fp1:' && ok "audio.segments[0].fingerprint carries the fp1: prefix" || no "audio.segments[0].fingerprint ($FP) missing the fp1: prefix"
GAPD_TAKE="$(take_field "$TAKE_FOLDER" "type(d['audio'].get('gapDetection')).__name__")"
[ "$GAPD_TAKE" = "bool" ] && ok "take.json audio.gapDetection is a boolean" || no "take.json audio.gapDetection is not a boolean ($GAPD_TAKE)"
MODE="$(take_field "$TAKE_FOLDER" "d['audio'].get('mode','NA')")"
[ "$MODE" = "file" ] && ok "take.json audio.mode == file" || no "take.json audio.mode == $MODE (expected file)"

# Continuous row structural check (plan section 4 "Continuous rows" bullet,
# critic MAJOR fix): layer/scalar:opacity lane -- one gesture, grip
# "decaying", >=3 breakpoints. `curve` is a flat breakpoint array
# (AutomationCurve::toVar returns the array directly, not an object --
# src/model/ControlPath.h + Lane.h/AutomationCurve.h schema).
OP_LANE_KIND="$(take_field "$TAKE_FOLDER" "next((lane.get('kind','NA') for lane in d.get('lanes',[]) if lane.get('key',{}).get('scope')=='layer' and lane.get('key',{}).get('control')=='scalar' and lane.get('key',{}).get('scalar')=='opacity'), 'NA')")"
OP_N_GESTURES="$(take_field "$TAKE_FOLDER" "sum(1 for lane in d.get('lanes',[]) if lane.get('key',{}).get('scope')=='layer' and lane.get('key',{}).get('control')=='scalar' and lane.get('key',{}).get('scalar')=='opacity' for g in lane.get('gestures',[]))")"
OP_GRIP="$(take_field "$TAKE_FOLDER" "next((g.get('grip','NA') for lane in d.get('lanes',[]) if lane.get('key',{}).get('scope')=='layer' and lane.get('key',{}).get('control')=='scalar' and lane.get('key',{}).get('scalar')=='opacity' for g in lane.get('gestures',[])), 'NA')")"
OP_N_BREAKPOINTS="$(take_field "$TAKE_FOLDER" "next((len(g.get('curve',[])) for lane in d.get('lanes',[]) if lane.get('key',{}).get('scope')=='layer' and lane.get('key',{}).get('control')=='scalar' and lane.get('key',{}).get('scalar')=='opacity' for g in lane.get('gestures',[])), 0)")"
[ "$OP_LANE_KIND" = "continuous" ] && ok "layer/scalar:opacity lane kind == continuous" || no "layer/scalar:opacity lane kind == $OP_LANE_KIND (expected continuous)"
[ "$OP_N_GESTURES" = "1" ] && ok "layer/scalar:opacity lane has exactly 1 gesture" || no "layer/scalar:opacity lane has $OP_N_GESTURES gestures (expected 1)"
[ "$OP_GRIP" = "decaying" ] && ok "layer/scalar:opacity gesture grip == decaying" || no "layer/scalar:opacity gesture grip == $OP_GRIP (expected decaying)"
awk -v x="$OP_N_BREAKPOINTS" 'BEGIN{exit !(x+0>=3)}' 2>/dev/null && ok "layer/scalar:opacity gesture has >=3 breakpoints ($OP_N_BREAKPOINTS)" || no "layer/scalar:opacity gesture has $OP_N_BREAKPOINTS breakpoints, expected >=3"

N_ACTIVECLIP="$(take_field "$TAKE_FOLDER" "sum(1 for lane in d.get('lanes',[]) if lane.get('key',{}).get('control')=='activeClip' for p in lane.get('points',[]))")"
awk -v x="$N_ACTIVECLIP" 'BEGIN{exit !(x+0>=4)}' 2>/dev/null && ok "layer/activeClip lane has >=4 points ($N_ACTIVECLIP)" || no "layer/activeClip lane has $N_ACTIVECLIP points, expected >=4"
TEMPO_V="$(take_field "$TAKE_FOLDER" "next((p['v'] for lane in d.get('lanes',[]) if lane.get('key',{}).get('control')=='tempo' for p in lane.get('points',[])), 'NA')")"
[ "$TEMPO_V" = "12800" ] && ok "comp/tempo point v == 12800 (centi-BPM for 128, plan R-8 unit)" || no "comp/tempo point v == $TEMPO_V (expected 12800)"
CHK0="$(take_field "$TAKE_FOLDER" "d.get('checkpoint0',{}).get('activeDeckIndex','NA')")"
[ "$CHK0" = "0" ] && ok "checkpoint0.activeDeckIndex == 0" || no "checkpoint0.activeDeckIndex == $CHK0"
CHKEND="$(take_field "$TAKE_FOLDER" "'yes' if d.get('checkpointEnd') else 'no'")"
[ "$CHKEND" = "yes" ] && ok "checkpointEnd present" || no "checkpointEnd missing"
DEV_RATE_TAKE="$(take_field "$TAKE_FOLDER" "d.get('rateMismatch', d.get('deviceRate','NA'))")"
echo "(informational) take-level rate fields: $DEV_RATE_TAKE"

# --- 9. T2 alignment: locate impulses in the captured take audio, compare
#        against take.markers[] (onsetMarkers:true). Best-effort via
#        $ROOT/.venv (numpy) -- SKIP if unavailable, mirroring
#        probe-lane3.sh's PIL-unavailable pattern. --------------------------
if [ -x "$ROOT/.venv/bin/python" ]; then
    ALIGN="$("$ROOT/.venv/bin/python" - "$ASSET_DIR/audio.wav" "$TAKE_FOLDER/take.json" <<'PYEOF'
import sys, json, wave
import numpy as np
wav_path, take_path = sys.argv[1], sys.argv[2]
try:
    with wave.open(wav_path, 'rb') as w:
        rate = w.getframerate()
        ch = w.getnchannels()
        n = w.getnframes()
        raw = w.readframes(n)
    samples = np.frombuffer(raw, dtype=np.int16).reshape(-1, ch)[:, 0].astype(np.float64)
    thresh = 0.5 * 30000
    impulses = np.where(samples >= thresh)[0]
    # collapse consecutive hits into one impulse index each
    peaks = []
    last = -10
    for idx in impulses:
        if idx - last > 100:
            peaks.append(int(idx))
        last = idx
    with open(take_path) as f:
        d = json.load(f)
    firstSample = d['audio']['segments'][0].get('firstSample', 0)
    markers = [m for m in d.get('markers', []) if m.get('action') == 'onset']
    if not peaks or not markers:
        print("NA(no peaks or no onset markers)")
    else:
        offsets = []
        for m in markers:
            assetFrame = m['sample'] - firstSample
            # nearest peak
            nearest = min(peaks, key=lambda p: abs(p - assetFrame))
            offsets.append((assetFrame - nearest) * 1000.0 / rate)
        mean_off = sum(offsets) / len(offsets)
        drift = offsets[-1] - offsets[0] if len(offsets) > 1 else 0.0
        srt = sorted(abs(o) for o in offsets)
        p95 = srt[int(0.95 * (len(srt) - 1))] if srt else 0.0
        print(f"mean_offset_ms={mean_off:.2f} drift_ms={drift:.2f} p95_jitter_ms={p95:.2f} n_markers={len(markers)} n_peaks={len(peaks)}")
except Exception as e:
    print(f"NA({e})")
PYEOF
)"
    echo "T2 alignment: $ALIGN"
    if echo "$ALIGN" | grep -q '^mean_offset_ms='; then
        MEAN_MS="$(echo "$ALIGN" | sed -n 's/.*mean_offset_ms=\([0-9.-]*\).*/\1/p')"
        DRIFT_MS="$(echo "$ALIGN" | sed -n 's/.*drift_ms=\([0-9.-]*\).*/\1/p')"
        awk -v x="$DRIFT_MS" 'BEGIN{exit !(x<=1.0 && x>=-1.0)}' 2>/dev/null \
          && ok "T2 alignment: drift within +-1ms over the take ($ALIGN)" \
          || no "T2 alignment: drift exceeds +-1ms ($ALIGN)"
        echo "(informational) T2 mean offset ${MEAN_MS}ms (expect ~20-30ms per D10.3: hop + tick + ring depth)"
    else
        no "T2 alignment could not be computed ($ALIGN)"
    fi
else
    skip "T2 alignment (.venv/bin/python with numpy unavailable)"
fi

# --- 10. load + replay (WithAudio then WallClock) --------------------------
curl -s --max-time 6 -X POST "$A/api/perf/load" -H 'Content-Type: application/json' \
  -d "{\"folder\":\"$TAKE_FOLDER\"}" >/dev/null
sleep 1
AUDIOSTATUS="$(perf_field "d.get('audioStatus','NA')")"
[ "$AUDIOSTATUS" = "Resolved" ] && ok "perf/load: audioStatus == Resolved" || no "perf/load: audioStatus == $AUDIOSTATUS (expected Resolved)"
UNRES="$(perf_field "d.get('unresolved','NA')")"
[ "$UNRES" = "0" ] && ok "perf/load: unresolved == 0" || no "perf/load: unresolved == $UNRES"

curl -s --max-time 6 -X POST "$A/api/perf/play" -H 'Content-Type: application/json' -d '{"withAudio":true}' >/dev/null
SEEN="0 -1 -1 -1 -1"; LAST=-1; ORDER_OK=1
OP_MAX="0"; OP_LAST=""
for i in $(seq 1 30); do
    v="$(comp_active_col)"
    if [ "$v" != "$LAST" ]; then
        echo "  replay(withAudio): t~${i}s activeClipColumn=$v"
        if [ "$LAST" != "-1" ] && [ "$v" -lt "$LAST" ] 2>/dev/null; then ORDER_OK=0; fi
        LAST="$v"
    fi
    # Replay-shape sample for the continuous opacity row (critic MAJOR fix,
    # not an exact-timing check -- plan section 4's "on replay layer 0
    # opacity follows 0.4->0.7->0.2 smoothly" bullet): opacity should visit
    # something near the 0.7 peak on its way through the glide, not just
    # jump straight to the final 0.2.
    OP_LAST="$(jget "$A/api/composition" "d['decks'][0]['layers'][0]['opacity']")"
    OP_MAX="$(awk -v x="$OP_LAST" -v m="$OP_MAX" 'BEGIN{ xv=(x=="NA")?-1:x+0; mv=m+0; print (xv>mv)?xv:mv }')"
    [ "$LAST" = "3" ] && break
    sleep 1
done
[ "$LAST" = "3" ] && [ "$ORDER_OK" = "1" ] && ok "replay(withAudio): activeClipColumn reached 3, non-decreasing" || no "replay(withAudio): activeClipColumn sequence wrong (last=$LAST order_ok=$ORDER_OK)"
awk -v x="$OP_MAX" 'BEGIN{exit !(x+0>=0.55)}' 2>/dev/null && ok "replay(withAudio): layer 0 opacity glide reached near the 0.7 peak (max observed=$OP_MAX)" || no "replay(withAudio): layer 0 opacity glide never reached near 0.7 (max observed=$OP_MAX) -- may be jumping instead of gliding"
awk -v x="$OP_LAST" 'BEGIN{exit !(x!="NA" && x+0>=0.1 && x+0<=0.3)}' 2>/dev/null && ok "replay(withAudio): layer 0 opacity settled near the final 0.2 (last observed=$OP_LAST)" || no "replay(withAudio): layer 0 opacity did not settle near 0.2 (last observed=$OP_LAST)"
BPM_REPLAY="$(jget "$A/api/bpm" "d.get('bpm','NA')")"
awk -v x="$BPM_REPLAY" 'BEGIN{exit !(x+0>=127.5 && x+0<=128.5)}' 2>/dev/null && ok "replay(withAudio): bpm reached 128 after the tempo point ($BPM_REPLAY)" || no "replay(withAudio): bpm=$BPM_REPLAY (expected ~128)"
SKIPPED="$(perf_field "d.get('skipped','NA')")"
[ "$SKIPPED" = "1" ] && ok "replay(withAudio): status.skipped==1 (the arm-time 'audio play' point, WithAudio policy)" || no "replay(withAudio): status.skipped==$SKIPPED (expected 1)"
curl -s --max-time 6 -X POST "$A/api/perf/stop_play" >/dev/null
sleep 1
PLAYING1="$(perf_field "d.get('playing','NA')")"
[ "$PLAYING1" = "False" -o "$PLAYING1" = "false" ] && ok "perf/stop_play: playing==false" || no "perf/stop_play: playing==$PLAYING1"

curl -s --max-time 6 -X POST "$A/api/perf/play" -H 'Content-Type: application/json' -d '{"withAudio":false}' >/dev/null
LAST=-1
for i in $(seq 1 30); do
    v="$(comp_active_col)"
    [ "$v" != "$LAST" ] && { echo "  replay(wallClock): t~${i}s activeClipColumn=$v"; LAST="$v"; }
    [ "$LAST" = "3" ] && break
    sleep 1
done
[ "$LAST" = "3" ] && ok "replay(wallClock): activeClipColumn reached 3" || no "replay(wallClock): activeClipColumn stalled at $LAST"
SKIPPED2="$(perf_field "d.get('skipped','NA')")"
[ "$SKIPPED2" = "0" ] && ok "replay(wallClock): status.skipped==0 (audio points fire on wall-clock replay)" || no "replay(wallClock): status.skipped==$SKIPPED2 (expected 0)"
curl -s --max-time 6 -X POST "$A/api/perf/stop_play" >/dev/null
sleep 1

# --- 11. overdub safety (R5) ---------------------------------------------
curl -s --max-time 6 -X POST "$A/api/perf/play" -H 'Content-Type: application/json' -d '{"withAudio":true}' >/dev/null
sleep 1
curl -s --max-time 6 -X POST "$A/api/perf/record" -H 'Content-Type: application/json' \
  -d "{\"name\":\"step3gate2\",\"overdubAssetId\":\"$ASSET\"}" \
  | grep -q '"ok":[[:space:]]*true' && ok "overdub arm accepted against the stored asset" || no "overdub arm refused"
sleep 5
curl -s --max-time 6 -X POST "$A/api/trigger_clip" -H 'Content-Type: application/json' -d '{"layer":0,"column":1}' >/dev/null
sleep 10
curl -s --max-time 6 -X POST "$A/api/perf/stop_play" >/dev/null
curl -s --max-time 6 -X POST "$A/api/perf/stop" >/dev/null
sleep 3
GATE2_FOLDER="$TAKES_DIR/step3gate2.adna-take"
if [ -f "$GATE2_FOLDER/take.json" ]; then
    G2_ACTIVECLIP="$(take_field "$GATE2_FOLDER" "sum(1 for lane in d.get('lanes',[]) if lane.get('key',{}).get('control')=='activeClip' for p in lane.get('points',[]))")"
    [ "$G2_ACTIVECLIP" = "1" ] && ok "overdub take has exactly 1 activeClip point (only the REST trigger sent during overdub, not the replay)" || no "overdub take has $G2_ACTIVECLIP activeClip points (expected 1 -- replay must not self-record, R5)"
    G2_FIRST="$(take_field "$GATE2_FOLDER" "d['audio']['segments'][0]['firstSample']")"
    [ "$G2_FIRST" = "0" ] && ok "overdub take firstSample==0 (v2 5.2)" || no "overdub take firstSample==$G2_FIRST (expected 0)"
    G2_ID="$(take_field "$GATE2_FOLDER" "d['audio']['segments'][0]['id']")"
    [ "$G2_ID" = "$ASSET" ] && ok "overdub take references the SAME asset id ($ASSET)" || no "overdub take references $G2_ID, expected the original asset $ASSET"
    N_ASSETS="$(ls -1 "$AUDIO_DIR" 2>/dev/null | grep -c '\.adna-audio$')"
    [ "$N_ASSETS" = "1" ] && ok "Audio/ still has exactly one asset after overdub ($N_ASSETS)" || no "Audio/ has $N_ASSETS assets after overdub (expected 1 -- overdub must not create a new one)"
else
    no "overdub take.json missing at $GATE2_FOLDER"
fi

# --- 12. crash-readability (OPTIONAL, DESTRUCTIVE -- run only if asked) ---
if [ "${STEP3_RUN_CRASH_TEST:-0}" = "1" ]; then
    curl -s --max-time 6 -X POST "$A/api/perf/record" -H 'Content-Type: application/json' \
      -d '{"name":"step3gate3","audio":true,"audioFile":"'"$CLICK_WAV"'"}' >/dev/null
    sleep 30
    KPID="$(pgrep -f 'MacOS/Audio-DN[A]' | head -1)"
    [ -n "$KPID" ] && kill -9 "$KPID" 2>/dev/null
    for _ in $(seq 1 20); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
    open --stdout /tmp/adna-step3-out2.log --stderr /tmp/adna-step3-err2.log "$APPBUNDLE"
    for i in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
    GATE3_FOLDER="$TAKES_DIR/step3gate3.adna-take"
    curl -s --max-time 6 -X POST "$A/api/perf/load" -H 'Content-Type: application/json' \
      -d "{\"folder\":\"$GATE3_FOLDER\"}" >/dev/null
    sleep 1
    AS3="$(perf_field "d.get('audioStatus','NA')")"
    echo "$AS3" | grep -qi 'Incomplete' && ok "crash-readability: perf/load reports Incomplete before repair" || no "crash-readability: audioStatus=$AS3 (expected Incomplete before repair)"
    curl -s --max-time 6 -X POST "$A/api/perf/repair" >/dev/null
    sleep 1
    AS3R="$(perf_field "d.get('audioStatus','NA')")"
    [ "$AS3R" = "Resolved" ] && ok "crash-readability: perf/repair -> Resolved" || no "crash-readability: repair left audioStatus=$AS3R (expected Resolved)"
    ASSET3_DIR="$AUDIO_DIR/$ASSET.adna-audio"
    ASSET3="$(perf_field "d.get('assetId','$ASSET')")"
    ASSET3_DIR="$AUDIO_DIR/$ASSET3.adna-audio"
    if [ -f "$ASSET3_DIR/audio.wav" ] && [ -x "$ROOT/.venv/bin/python" ]; then
        DUR="$("$ROOT/.venv/bin/python" -c "
import wave
with wave.open('$ASSET3_DIR/audio.wav','rb') as w:
    print(w.getnframes()/w.getframerate())")"
        awk -v x="$DUR" 'BEGIN{exit !(x+0>=12)}' 2>/dev/null && ok "crash-readability: recovered WAV has >=12s of audio (${DUR}s, D-A10 bound)" || no "crash-readability: recovered WAV only ${DUR}s (expected >=12s)"
    else
        skip "crash-readability WAV duration check (.venv unavailable or file missing)"
    fi
else
    skip "crash-readability row (set STEP3_RUN_CRASH_TEST=1 to run it -- destructive, kill -9)"
fi

fi  # R13_OK

# --- 13. teardown (SCREEN-SAFETY LAW) --------------------------------------
# Graceful quit FIRST (~MainComponent runs recorderHost_.shutdown before the
# process exits) -- never pkill while a window could still be open.
osascript -e 'quit app "Audio-DNA"' >/dev/null 2>&1
for _ in $(seq 1 30); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
if pgrep -f 'MacOS/Audio-DN[A]' >/dev/null; then
    echo "graceful osascript quit did not clear the process -- falling back to pkill (last resort)"
    pkill -f 'MacOS/Audio-DN[A]' >/dev/null 2>&1
    for _ in $(seq 1 20); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
fi
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && no "APP STILL RUNNING AFTER graceful quit + pkill fallback" || ok "app terminated, no process remains"

# Screen-safety: confirm the OUTPUT window specifically was never opened.
# Same filter as probe-lane3.sh (2026-09-24 fix): the app's own normal main
# window ("Audio-DNA") is expected and excluded; only "Audio-DNA Output" is
# a breach.
if [ -x "$ROOT/.venv/bin/python" ]; then
    W="$("$ROOT/.venv/bin/python" -c "
import Quartz
wl=Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName',''))
           and 'Output' in str(w.get('kCGWindowName',''))]))" 2>/dev/null)"
    if [ "$W" = "0" ] || [ -z "$W" ]; then
        ok "0 Audio-DNA Output windows in the FULL window list (SCREEN-SAFETY LAW held)"
    else
        no "$W Audio-DNA Output window(s) were open during the run -- screen-safety breach"
    fi
else
    skip "Output-window check (.venv/bin/python with pyobjc/Quartz unavailable)"
fi

# Final screen state, for the handoff (plan section 4 Teardown paragraph:
# "state the screen state in the handoff").
screencapture -x /tmp/step3-eos.png 2>/dev/null \
  && echo "screenshot: /tmp/step3-eos.png (read it before concluding -- no black overlay, no dialog expected)" \
  || echo "screencapture failed (no display attached / headless run)"

echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT; take folders under $TAKES_DIR; asset under $AUDIO_DIR)"
[ "$FAIL" -eq 0 ] || exit 1
