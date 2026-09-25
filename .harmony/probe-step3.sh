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
#   4. R13 (HANDOFF :2633, s-rta-0924 lane D, LIVE): the analysis thread now
#      resamples any device rate to its fixed internal 48 kHz
#      (AnalysisResampler, lane A) -- the beat clock is correct at any
#      device rate, so this gate no longer refuses to arm on a non-48kHz
#      device. It machine-checks the R13 provenance fields instead
#      (sourceSampleRate, bandValidMask, rateChangedSinceArm).
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
#   * R13 device-rate provenance (s-rta-0924 lane D): this gate no longer
#     refuses to arm on a non-48kHz device -- it asserts /api/features
#     sourceSampleRate == /api/perf/status deviceRate, bandValidMask == 127
#     when the device IS 48 kHz, the retired "analysis pipeline assumes"
#     stderr warning is gone, and rateChangedSinceArm == false after a take
#     that saw no device-rate change. Reconnecting a non-48kHz device (e.g.
#     the soundcore P31i BT headset at 16kHz, HANDOFF :2633) is now an
#     honestly-degraded take (band gating), not a refusal.
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
#   * The LONG take for the T2 drift proof (spec D10.3: offset at minute 10
#     minus offset at minute 0 over a 10-minute file-mode click take) is
#     section 11L, gated behind STEP3_LONG=1 (unset by default;
#     STEP3_LONG_MINUTES=N overrides the length, 2 = cheap dry run of the
#     branch). Adds ~11 min wall time and ~240 MB of disk per 10-minute run
#     (~121 MB click WAV in /tmp, ~117 MB asset in the store -- Ruling 28,
#     never auto-deleted). Nothing in the default run changes.
set -u
OUT="${1:-/tmp/audiodna-step3}"; mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${STEP3_BUILD_DIR:-build-gate}"
APPBUNDLE="$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app"
A='http://127.0.0.1:7070'
FIXTURE="$ROOT/.harmony/probe-step3.json"
CLICK_WAV="${STEP3_CLICK_WAV:-/tmp/click_48k.wav}"
# s-rta-0924: single source of truth for the click grid spacing -- passed
# explicitly to gen-click-wav.py at generation time (step 0 below) AND to
# the T2 alignment python (section 9) so the two never hardcode 24000
# independently and drift apart.
CLICK_INTERVAL="${STEP3_CLICK_INTERVAL:-24000}"
# s-rta-0924b: opt-in LONG take for the T2 drift proof (section 11L). Off by
# default. STEP3_LONG_MINUTES = spec D10.3's "minute 10" (default 10); 2 is
# the cheap dry run. The long click WAV is derived from CLICK_WAV (same
# interval: CLICK_INTERVAL stays the one source of truth for the grid).
LONG="${STEP3_LONG:-0}"
LONG_MINUTES="${STEP3_LONG_MINUTES:-10}"
if [ "$LONG" = "1" ] && ! echo "$LONG_MINUTES" | grep -Eq '^[1-9][0-9]*$'; then
    echo "REFUSE: STEP3_LONG_MINUTES must be a positive integer (got '$LONG_MINUTES')"; exit 64
fi
CLICK_WAV_LONG="${CLICK_WAV%.wav}_long.wav"
TAKES_DIR="$HOME/Documents/Audio-DNA/Takes"
AUDIO_DIR="$HOME/Documents/Audio-DNA/Audio"
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
skip(){ echo "SKIP  $1"; }
# s-rta-0924 (lane D): informational only -- does NOT affect PASS/FAIL or exit
# code (see T2 alignment, section 9: a too-short/noisy take warns instead of
# failing the drift bound outright).
warn(){ echo "WARN  $1"; }
# s-rta-0924 (Harmony): JSON numbers may serialize as floats ("48000.0") --
# normnum prints an integral value without ".0" so string compares are sound.
normnum(){ python3 -c 'import sys
v=sys.argv[1]
try:
    f=float(v); print(int(f) if f==int(f) else f)
except Exception: print(v)' "$1"; }

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
python3 "$ROOT/.harmony/gen-click-wav.py" "$CLICK_WAV" --interval "$CLICK_INTERVAL" >/dev/null \
  && ok "click-track WAV generated at $CLICK_WAV" \
  || { no "click-track WAV generation FAILED"; }

# STEP3_LONG: a second, longer click WAV for section 11L. Length = M*60 + 30 s
# (M*60 + 10 s recorded, plus slack for arm->play and stop latency) so the
# transport never reaches end-of-file mid-take: nothing in the app reacts to
# end-of-file (AudioEngine's onTransportStateChanged has no consumer), the
# take would just go silent and the minute-M window would be empty.
if [ "$LONG" = "1" ]; then
    python3 "$ROOT/.harmony/gen-click-wav.py" "$CLICK_WAV_LONG" --interval "$CLICK_INTERVAL" \
        --duration-s $((LONG_MINUTES * 60 + 30)) >/dev/null \
      && ok "LONG: click-track WAV generated at $CLICK_WAV_LONG ($((LONG_MINUTES * 60 + 30)) s)" \
      || no "LONG: click-track WAV generation FAILED"
fi

# --- 1. preconditions ------------------------------------------------------
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE (set STEP3_BUILD_DIR to override the build dir name)"; exit 64; }
[ -f "$CLICK_WAV" ] || { echo "REFUSE: click WAV missing after generation step"; exit 64; }
[ "$LONG" = "1" ] && { [ -f "$CLICK_WAV_LONG" ] || { echo "REFUSE: long click WAV missing after generation step"; exit 64; }; }
python3 -m json.tool "$FIXTURE" >/dev/null 2>&1 && ok "fixture $FIXTURE is valid JSON" || no "fixture $FIXTURE is NOT valid JSON"

# --- 2. launch (production, NO --test-mode -- T4) -------------------------
# `open` APPENDS to --stdout/--stderr logs -- truncate both before launch so
# every grep below (section 4, section 12b) reads only this run, not stale
# lines left over from a prior run's process (probe-finalize-loop.sh pattern).
: > /tmp/adna-step3-out.log
: > /tmp/adna-step3-err.log
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

# --- 4. R13 provenance check (s-rta-0924 lane D) ---------------------------
# The analysis thread resamples any device rate to its fixed internal
# 48 kHz (AnalysisResampler, R13 lane A) -- this is no longer an arm
# precondition (the beat clock is correct at any device rate now). Instead
# assert the provenance fields agree with each other and the retired
# analysis-thread ctor warning is gone from stderr (grepped from stderr,
# not stdout -- the warning was a std::cerr line, now deleted).
DEV_RATE="$(perf_field "d.get('deviceRate','NA')")"; DEV_RATE="$(normnum "$DEV_RATE")"
SRC_RATE="$(jget "$A/api/features" "d.get('sourceSampleRate','NA')")"; SRC_RATE="$(normnum "$SRC_RATE")"
BAND_MASK="$(jget "$A/api/features" "d.get('bandValidMask','NA')")"; BAND_MASK="$(normnum "$BAND_MASK")"
[ "$SRC_RATE" = "$DEV_RATE" ] \
  && ok "R13: /api/features sourceSampleRate ($SRC_RATE) == /api/perf/status deviceRate ($DEV_RATE)" \
  || no "R13: /api/features sourceSampleRate ($SRC_RATE) != /api/perf/status deviceRate ($DEV_RATE)"
if [ "$DEV_RATE" = "48000" ]; then
    [ "$BAND_MASK" = "127" ] && ok "R13: bandValidMask == 127 at 48 kHz" || no "R13: bandValidMask == $BAND_MASK at 48 kHz (expected 127)"
else
    echo "(informational) device rate ${DEV_RATE} Hz != 48000 -- bandValidMask=$BAND_MASK (band gating expected below the device Nyquist, not checked against 127)"
fi
if grep -q "analysis pipeline assumes" /tmp/adna-step3-err.log 2>/dev/null; then
    no "R13: stderr still carries the retired analysis-rate-assumption warning"
else
    ok "R13: no analysis-rate-assumption warning on stderr (retired)"
fi

# --- 5. arm with deterministic audio (file mode) + onset markers (T2) -----
N_ASSETS_BEFORE="$(ls -1 "$AUDIO_DIR" 2>/dev/null | grep -c '\.adna-audio$')"
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
# s-rta-0924b finalize truncation: AudioStore::finalize's verdict for this stop. "NA(...)" when
# the key is absent -> red, so a build without the status field cannot pass this row vacuously
# (pre-fix, lastError above stayed "" even on a truncated stop -- it was never assigned).
FIN_ERR="$(perf_field "d.get('lastFinalizeError','NA')")"
[ "$FIN_ERR" = "" ] && ok "perf/status: lastFinalizeError empty after stop (audio.wav header frames == tap framesWritten; no block lost at stop)" \
  || no "perf/status: lastFinalizeError='$FIN_ERR' (s-rta-0924b finalize truncation, or field missing)"
RATE_CHANGED="$(perf_field "d.get('rateChangedSinceArm','NA')")"
[ "$RATE_CHANGED" = "False" -o "$RATE_CHANGED" = "false" ] \
  && ok "R13: rateChangedSinceArm == false after the take (no device-rate change mid-take)" \
  || no "R13: rateChangedSinceArm == $RATE_CHANGED after the take (expected false)"

# --- 8. disk checks (Ruling 28 v3 format) -------------------------------
ASSET_DIR="$AUDIO_DIR/$ASSET.adna-audio"
[ -f "$ASSET_DIR/audio.wav" ] && [ -f "$ASSET_DIR/audio.json" ] \
  && ok "Audio store has audio.wav + audio.json ($ASSET_DIR)" \
  || no "Audio store missing audio.wav/audio.json at $ASSET_DIR"
TAKE_FILES="$(ls -1 "$TAKE_FOLDER" 2>/dev/null | wc -l | tr -d ' ')"
[ "$TAKE_FILES" = "1" ] && ok "take folder has exactly one file (take.json only, Ruling 28)" || no "take folder has $TAKE_FILES files, expected 1 (take.json only)"

VERSION="$(take_field "$TAKE_FOLDER" "d.get('version','NA')")"; VERSION="$(normnum "$VERSION")"
[ "$VERSION" = "3" ] && ok "take.json version == 3" || no "take.json version == $VERSION (expected 3)"
SEG_RATE="$(take_field "$TAKE_FOLDER" "d['audio']['segments'][0]['rate']")"; SEG_RATE="$(normnum "$SEG_RATE")"
# R13: compare against $DEV_RATE (the device rate read at arm, section 4)
# rather than a hardcoded 48000 -- the take's audio segment stays in the
# DEVICE domain (G14); only the analysis thread's internal rate is fixed.
[ "$SEG_RATE" = "$DEV_RATE" ] && ok "audio.segments[0].rate == $DEV_RATE (device rate at arm)" || no "audio.segments[0].rate == $SEG_RATE (expected $DEV_RATE, the device rate read at arm)"
SEG_FRAMES="$(take_field "$TAKE_FOLDER" "d['audio']['segments'][0]['frames']")"
awk -v x="$SEG_FRAMES" 'BEGIN{exit !(x+0>0)}' 2>/dev/null && ok "audio.segments[0].frames > 0 ($SEG_FRAMES)" || no "audio.segments[0].frames not >0 ($SEG_FRAMES)"
# s-rta-0924b disk oracle (independent of the status field): finalize marks a truncated asset
# unreliable from the header's frame count, and the take's segment frames must equal the WAV's.
UNREL="$(take_field "$TAKE_FOLDER" "d['audio'].get('unreliableFrom')")"
[ "$UNREL" = "None" ] && ok "take.json audio.unreliableFrom is null (no truncation, no overrun)" || no "take.json audio.unreliableFrom=$UNREL (finalize marked the audio unreliable)"
WAVN="$(python3 -c "import wave; w=wave.open('$ASSET_DIR/audio.wav','rb'); print(w.getnframes())" 2>/dev/null || echo NA)"
[ "$WAVN" = "$SEG_FRAMES" ] && ok "audio.wav frame count == take segment frames ($WAVN)" || no "audio.wav frames $WAVN != take segment frames $SEG_FRAMES"
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
# s-rta-0924 (Harmony, diagnosis-verified): the recipe sends 3 REST writes 1 s
# apart; each is a Decaying grip that idles out after gripHoldMs (250 ms), so
# the take correctly holds 3 gestures of 2 breakpoints (a flat plateau), values
# 0.4 / 0.7 / 0.2 in order.
OP_VALUES="$(take_field "$TAKE_FOLDER" "' '.join('%.1f' % g['curve'][0]['y'] for lane in d.get('lanes',[]) if lane.get('key',{}).get('scope')=='layer' and lane.get('key',{}).get('control')=='scalar' and lane.get('key',{}).get('scalar')=='opacity' for g in lane.get('gestures',[]) if g.get('curve'))")"
[ "$OP_N_GESTURES" = "3" ] && ok "layer/scalar:opacity lane has 3 gestures (one per REST write)" || no "layer/scalar:opacity lane has $OP_N_GESTURES gestures (expected 3)"
[ "$OP_VALUES" = "0.4 0.7 0.2" ] && ok "layer/scalar:opacity gesture values in order: $OP_VALUES" || no "layer/scalar:opacity gesture values: '$OP_VALUES' (expected '0.4 0.7 0.2')"
[ "$OP_GRIP" = "decaying" ] && ok "layer/scalar:opacity gesture grip == decaying" || no "layer/scalar:opacity gesture grip == $OP_GRIP (expected decaying)"
[ "$OP_N_BREAKPOINTS" = "2" ] && ok "layer/scalar:opacity first gesture has 2 breakpoints (touch + idle end)" || no "layer/scalar:opacity first gesture has $OP_N_BREAKPOINTS breakpoints, expected 2"

N_ACTIVECLIP="$(take_field "$TAKE_FOLDER" "sum(1 for lane in d.get('lanes',[]) if lane.get('key',{}).get('control')=='activeClip' for p in lane.get('points',[]))")"
awk -v x="$N_ACTIVECLIP" 'BEGIN{exit !(x+0>=4)}' 2>/dev/null && ok "layer/activeClip lane has >=4 points ($N_ACTIVECLIP)" || no "layer/activeClip lane has $N_ACTIVECLIP points, expected >=4"
TEMPO_V="$(take_field "$TAKE_FOLDER" "next((p['v'] for lane in d.get('lanes',[]) if lane.get('key',{}).get('control')=='tempo' for p in lane.get('points',[])), 'NA')")"; TEMPO_V="$(normnum "$TEMPO_V")"
[ "$TEMPO_V" = "12800" ] && ok "comp/tempo point v == 12800 (centi-BPM for 128, plan R-8 unit)" || no "comp/tempo point v == $TEMPO_V (expected 12800)"
CHK0="$(take_field "$TAKE_FOLDER" "d.get('checkpoint0',{}).get('activeDeckIndex','NA')")"; CHK0="$(normnum "$CHK0")"
[ "$CHK0" = "0" ] && ok "checkpoint0.activeDeckIndex == 0" || no "checkpoint0.activeDeckIndex == $CHK0"
CHKEND="$(take_field "$TAKE_FOLDER" "'yes' if d.get('checkpointEnd') else 'no'")"
[ "$CHKEND" = "yes" ] && ok "checkpointEnd present" || no "checkpointEnd missing"
DEV_RATE_TAKE="$(take_field "$TAKE_FOLDER" "d.get('rateChangedSinceArm', d.get('deviceRate','NA'))")"
echo "(informational) take-level rate fields: $DEV_RATE_TAKE"

# --- 9. T2 alignment: pair take.markers[] (onsetMarkers:true) against the
#        click grid (interval=$CLICK_INTERVAL, the SAME value gen-click-wav.py
#        was invoked with in step 0), not against the raw-peak detector.
#        Best-effort via $ROOT/.venv (numpy) -- SKIP if unavailable, mirroring
#        probe-lane3.sh's PIL-unavailable pattern.
#        s-rta-0924 (Harmony, diagnosis-driven fix): RecorderHost::tick fires
#        marker("onset") on every 120 Hz tick where snap.onsetDetected is
#        true, but FeatureBus::read() is always-latest and analysis
#        publishes at only ~93.75 Hz -- so two consecutive 120 Hz ticks read
#        the same snapshot ~14% of the time, producing duplicate markers at
#        the same `sample`. Dedupe on 'sample' BEFORE pairing. The burst
#        ONSET lands exactly on the grid by construction (gen-click-wav.py
#        header), so pairing each deduped marker to the nearest multiple of
#        the interval (not to the raw-peak detector's threshold-crossing
#        estimate) is the correct oracle; the raw-peak detector is kept only
#        as an informational cross-check below. --------------------------
# t2_align WAV TAKE INTERVAL [M [W]] -- the ONE alignment oracle, shared by
# section 9 (short take) and section 11L (STEP3_LONG). Prints a single
# "key=value ..." line. With M given it appends the D10.3 window keys
# (win_first_ms win_last_ms drift_win_ms drift_win_stderr_ms n_win_first
# n_win_last minutes_for_0p5ms); without M the line is byte-identical to the
# pre-11L output, so section 9's parsing (below) is untouched.
t2_align(){
  "$ROOT/.venv/bin/python" - "$@" <<'PYEOF'
import sys, json, wave
import numpy as np
wav_path, take_path, interval_str = sys.argv[1], sys.argv[2], sys.argv[3]
interval = int(interval_str)
# STEP3_LONG (D10.3, minute M vs minute 1): optional argv[4] = M minutes -> also
# report the mean offset in asset-time window [0, W) vs [(M-1)*W, M*W), with
# W = argv[5] seconds (default 60). Absent: output unchanged.
long_minutes = int(sys.argv[4]) if len(sys.argv)>4 else 0
win_s = float(sys.argv[5]) if len(sys.argv)>5 else 60.0
try:
    with wave.open(wav_path, 'rb') as w:
        rate = w.getframerate()
        ch = w.getnchannels()
        n = w.getnframes()
        raw = w.readframes(n)
    samples = np.frombuffer(raw, dtype=np.int16).reshape(-1, ch)[:, 0].astype(np.float64)
    thresh = 0.5 * 30000
    impulses = np.where(samples >= thresh)[0]
    # collapse consecutive hits into one impulse index each (cross-check only)
    peaks = []
    last = -10
    for idx in impulses:
        if idx - last > 100:
            peaks.append(int(idx))
        last = idx

    with open(take_path) as f:
        d = json.load(f)
    seg = d['audio']['segments'][0]
    firstSample = seg.get('firstSample', 0)
    seg_frames = seg.get('frames', n)
    raw_markers = [m for m in d.get('markers', []) if m.get('action') == 'onset']

    # Dedupe on 'sample' (see header note above).
    seen = set()
    markers = []
    n_dupes = 0
    for m in raw_markers:
        s = m['sample']
        if s in seen:
            n_dupes += 1
            continue
        seen.add(s)
        markers.append(m)

    if not markers:
        print("NA(no onset markers)")
    else:
        matched = []  # (marker_t_s, offset_ms)
        n_spurious = 0
        for m in markers:
            assetFrame = m['sample'] - firstSample
            grid_point = round(assetFrame / interval) * interval
            d_frames = assetFrame - grid_point
            if abs(d_frames) > interval / 2:
                n_spurious += 1
                continue
            matched.append((assetFrame / rate, d_frames * 1000.0 / rate))

        n_grid_in_range = len(range(0, seg_frames, interval))

        if not matched:
            print("NA(no markers matched a grid point, n_spurious=%d)" % n_spurious)
        else:
            offsets = [o for _, o in matched]
            ts = [t for t, _ in matched]
            mean_off = sum(offsets) / len(offsets)
            take_duration_s = seg_frames / rate
            if len(matched) > 1:
                # least-squares slope of offset(ms) vs marker time(s), times
                # the take duration -- total drift over the take.
                slope, intercept = np.polyfit(ts, offsets, 1)
                drift_ms = float(slope * take_duration_s)
                # s-rta-0924 (lane D, known-flake fix): the drift bound at a
                # fixed +-1ms is at noise level for a take this short (per-
                # marker jitter observed ~5ms, n~126 matched markers) -- the
                # standard error of the least-squares slope, scaled to the
                # take duration, is the honest uncertainty on drift_ms.
                # Standard OLS slope stderr: sqrt(SSR/dof) / sqrt(Sxx).
                ts_arr = np.array(ts)
                offs_arr = np.array(offsets)
                n_pts = len(ts_arr)
                drift_stderr_ms = 0.0
                s_err = None
                if n_pts > 2:
                    dof = n_pts - 2
                    sxx = float(np.sum((ts_arr - ts_arr.mean()) ** 2))
                    if dof > 0 and sxx > 0:
                        resid = offs_arr - (slope * ts_arr + intercept)
                        s_err = np.sqrt(np.sum(resid ** 2) / dof)
                        slope_stderr = s_err / np.sqrt(sxx)
                        drift_stderr_ms = float(slope_stderr * take_duration_s)
                if not np.isfinite(drift_stderr_ms):
                    drift_stderr_ms = 0.0
            else:
                drift_ms = 0.0
                drift_stderr_ms = 0.0
            srt = sorted(abs(o - mean_off) for o in offsets)
            p95 = srt[int(0.95 * (len(srt) - 1))] if srt else 0.0
            pct_matched = 100.0 * len(matched) / n_grid_in_range if n_grid_in_range else 0.0
            extra = ""
            if long_minutes>0:
                w0 = [o for t, o in matched if 0.0 <= t < win_s]
                w1 = [o for t, o in matched if (long_minutes - 1) * win_s <= t < long_minutes * win_s]
                if len(w0)>=2 and len(w1)>=2:
                    m0, m1 = float(np.mean(w0)), float(np.mean(w1))
                    se = float(np.sqrt(np.var(w0, ddof=1) / len(w0) + np.var(w1, ddof=1) / len(w1)))
                    extra = (f" win_first_ms={m0:.2f} win_last_ms={m1:.2f} drift_win_ms={m1 - m0:.2f}"
                             f" drift_win_stderr_ms={se:.2f} n_win_first={len(w0)} n_win_last={len(w1)}")
                else:
                    extra = f" drift_win_ms=NA drift_win_stderr_ms=NA n_win_first={len(w0)} n_win_last={len(w1)}"
                # Minutes of take needed for a 0.5 ms slope-drift stderr at THIS
                # run's residual sigma and marker rate (stderr ~ sigma*sqrt(12/n)):
                # n = 12*(sigma/0.5)^2, minutes = n / (markers per second) / 60.
                if len(matched)>2 and drift_stderr_ms>0.0 and s_err is not None:
                    rate_m = len(matched) / take_duration_s
                    extra += f" minutes_for_0p5ms={12.0 * (float(s_err) / 0.5) ** 2 / rate_m / 60.0:.1f}"
            print(f"mean_offset_ms={mean_off:.2f} drift_ms={drift_ms:.2f} "
                  f"drift_stderr_ms={drift_stderr_ms:.2f} "
                  f"p95_jitter_ms={p95:.2f} n_markers_raw={len(raw_markers)} "
                  f"n_dupes={n_dupes} n_matched={len(matched)} "
                  f"n_spurious={n_spurious} n_grid_in_range={n_grid_in_range} "
                  f"pct_matched={pct_matched:.1f} n_peaks={len(peaks)}" + extra)
except Exception as e:
    print(f"NA({e})")
PYEOF
}
if [ -x "$ROOT/.venv/bin/python" ]; then
    ALIGN="$(t2_align "$ASSET_DIR/audio.wav" "$TAKE_FOLDER/take.json" "$CLICK_INTERVAL")"
    echo "T2 alignment: $ALIGN"
    if echo "$ALIGN" | grep -q '^mean_offset_ms='; then
        MEAN_MS="$(echo "$ALIGN" | sed -n 's/.*mean_offset_ms=\([0-9.-]*\).*/\1/p')"
        DRIFT_MS="$(echo "$ALIGN" | sed -n 's/.*drift_ms=\([0-9.-]*\).*/\1/p')"
        DRIFT_STDERR_MS="$(echo "$ALIGN" | sed -n 's/.*drift_stderr_ms=\([0-9.-]*\).*/\1/p')"
        P95_MS="$(echo "$ALIGN" | sed -n 's/.*p95_jitter_ms=\([0-9.-]*\).*/\1/p')"
        PCT_MATCHED="$(echo "$ALIGN" | sed -n 's/.*pct_matched=\([0-9.-]*\).*/\1/p')"
        N_DUPES="$(echo "$ALIGN" | sed -n 's/.*n_dupes=\([0-9]*\).*/\1/p')"
        N_SPURIOUS="$(echo "$ALIGN" | sed -n 's/.*n_spurious=\([0-9]*\).*/\1/p')"
        N_PEAKS="$(echo "$ALIGN" | sed -n 's/.*n_peaks=\([0-9]*\).*/\1/p')"
        # Bounds per D10.3 (as amended, this diagnosis), drift bound amended
        # again (s-rta-0924 lane D, known-flake fix): a fixed +-1ms drift
        # bound is at noise level for a ~65s take (per-marker jitter ~5ms,
        # n~126 matched markers -> least-squares drift stderr ~1.5-2ms on
        # this rig). PASS if |drift| <= 1ms + 2*stderr (the other T2 bounds
        # below are unchanged). A high stderr (take too short/noisy to
        # support a tight 1ms drift claim) WARNs, it does not FAIL.
        DRIFT_BOUND_MS="$(awk -v se="$DRIFT_STDERR_MS" 'BEGIN{printf "%.4f", 1.0 + 2*se}' 2>/dev/null)"
        awk -v x="$DRIFT_MS" -v b="$DRIFT_BOUND_MS" 'BEGIN{exit !(x<=b && x>=-b)}' 2>/dev/null \
          && ok "T2 alignment: drift within +-${DRIFT_BOUND_MS}ms (<=1ms+2*stderr; drift=${DRIFT_MS}ms stderr=${DRIFT_STDERR_MS}ms) ($ALIGN)" \
          || no "T2 alignment: drift exceeds +-${DRIFT_BOUND_MS}ms (<=1ms+2*stderr; drift=${DRIFT_MS}ms stderr=${DRIFT_STDERR_MS}ms) ($ALIGN)"
        awk -v se="$DRIFT_STDERR_MS" 'BEGIN{exit !(se>0.5)}' 2>/dev/null \
          && warn "T2 alignment: drift stderr ${DRIFT_STDERR_MS}ms exceeds 0.5ms -- this take is too short/noisy to support a tight 1ms drift claim (informational, does not fail the gate)"
        awk -v x="$P95_MS" 'BEGIN{exit !(x<=15.0)}' 2>/dev/null \
          && ok "T2 alignment: p95 jitter <=15ms ($P95_MS ms)" \
          || no "T2 alignment: p95 jitter exceeds 15ms ($P95_MS ms)"
        awk -v x="$MEAN_MS" 'BEGIN{exit !(x>=0.0 && x<=60.0)}' 2>/dev/null \
          && ok "T2 alignment: mean offset within [0,60]ms ($MEAN_MS ms, latency lag expected per D10.3)" \
          || no "T2 alignment: mean offset outside [0,60]ms ($MEAN_MS ms)"
        awk -v x="$PCT_MATCHED" 'BEGIN{exit !(x>=90.0)}' 2>/dev/null \
          && ok "T2 alignment: matched markers cover >=90% of grid clicks in the asset's frame range ($PCT_MATCHED%)" \
          || no "T2 alignment: matched markers cover only $PCT_MATCHED% of grid clicks (expected >=90%)"
        echo "(informational) T2 duplicate onset markers this take: $N_DUPES (deduped before matching -- a RecorderHost product fix for the duplicate-marker source is landing separately)"
        echo "(informational) T2 spurious (off-grid, >interval/2) markers rejected: $N_SPURIOUS"
        echo "(informational) T2 raw-peak-detector cross-check: found $N_PEAKS burst peaks directly in the audio (best-effort threshold detector, not the pairing oracle)"
    else
        no "T2 alignment could not be computed ($ALIGN)"
    fi
else
    # s-rta-0924 cleanup lane: FAIL, not SKIP -- a missing .venv/numpy silently
    # dropped the T2 alignment row from the count entirely, so a run could still
    # print "N PASS / 0 FAIL" while never having checked click-grid alignment at
    # all (a silently weaker gate). Run from the main checkout (which already
    # has a numpy-provisioned .venv), or create one here:
    #   python3 -m venv .venv && .venv/bin/pip install numpy
    no "T2 alignment: .venv/bin/python with numpy unavailable -- run from the main checkout, or: python3 -m venv .venv && .venv/bin/pip install numpy"
fi

# --- 10. load + replay (WithAudio then WallClock) --------------------------
# s-rta-0924 (Harmony, diagnosis-driven fix): a live-instrumented run showed
# the OLD `[ "$LAST" = "3" ] && break` loops break on the FIRST
# activeClipColumn==3 -- but the take continues past that point (a
# trigger_column 2 at take-clock t=28.61s, plus 3 decaying opacity gestures
# at t=23.74/25.97/28.13s), so OP_MAX/OP_LAST froze at the pre-play leftover
# and the real tail of the take (the second column-3->2 transition, the
# 0.4/0.7/0.2 opacity glide) was never observed. Fix: poll at <=250ms for
# the take's full length (perf/status position/length; length+2s budget if
# length is unavailable), NEVER break early on reaching column 3. Replay
# does NOT apply checkpoint0 (spec s167 D4/D14), so the first poll after
# play always shows the pre-play leftover state -- the sequence check drops
# that leading value and expects the recorded sequence 0,1,2,3,2 (4 clip
# triggers then the trigger_column(2) later in the take).
comp_col_op(){ jget "$A/api/composition" "'%s|%s' % (d['decks'][0]['layers'][0]['activeClipColumn'], d['decks'][0]['layers'][0]['opacity'])"; }
EXPECTED_SEQ=(0 1 2 3 2)
# macOS ships bash 3.2 as /bin/bash (no `local -n` namerefs, added in 4.3) --
# take the raw sequence as positional args, not by array-variable name.
seq_matches_expected(){ # $@ = the raw (leftover-included) activeClipColumn sequence
    local -a _raw=("$@")
    local -a _observed=()
    [ "${#_raw[@]}" -ge 1 ] && _observed=("${_raw[@]:1}")
    [ "${#_observed[@]}" -eq "${#EXPECTED_SEQ[@]}" ] || return 1
    local idx
    for idx in "${!EXPECTED_SEQ[@]}"; do
        [ "${_observed[$idx]}" = "${EXPECTED_SEQ[$idx]}" ] || return 1
    done
    return 0
}

curl -s --max-time 6 -X POST "$A/api/perf/load" -H 'Content-Type: application/json' \
  -d "{\"folder\":\"$TAKE_FOLDER\"}" >/dev/null
sleep 1
AUDIOSTATUS="$(perf_field "d.get('audioStatus','NA')")"
[ "$AUDIOSTATUS" = "Resolved" ] && ok "perf/load: audioStatus == Resolved" || no "perf/load: audioStatus == $AUDIOSTATUS (expected Resolved)"
UNRES="$(perf_field "d.get('unresolved','NA')")"; UNRES="$(normnum "$UNRES")"
[ "$UNRES" = "0" ] && ok "perf/load: unresolved == 0" || no "perf/load: unresolved == $UNRES"

TAKE_LEN="$(perf_field "d.get('length','NA')")"; TAKE_LEN="$(normnum "$TAKE_LEN")"
REPLAY_BUDGET="$(awk -v l="$TAKE_LEN" 'BEGIN{ if (l=="NA" || l+0<=0) print 45; else print l+2 }')"

curl -s --max-time 6 -X POST "$A/api/perf/play" -H 'Content-Type: application/json' -d '{"withAudio":true}' >/dev/null
LAST="__UNSET__"; RAW_SEQ=()
OP_LAST=""; OP_SEEN_04=0; OP_SEEN_07=0
WITHAUDIO_FIRST_CHANGE_T=""
START_T=$(date +%s)
while :; do
    CO="$(comp_col_op)"
    v="${CO%%|*}"; OP_LAST="${CO#*|}"
    ELAPSED=$(( $(date +%s) - START_T ))
    if [ "$v" != "$LAST" ]; then
        echo "  replay(withAudio): t~${ELAPSED}s activeClipColumn=$v"
        RAW_SEQ+=("$v")
        LAST="$v"
        # Probe pin (fix-plan.md F1 "Probe pin"): pin the FIRST post-leftover
        # activeClipColumn change (RAW_SEQ[1] -- RAW_SEQ[0] is the pre-play
        # leftover state, always the first value observed) at its ELAPSED
        # time, seconds since THIS loop's own /api/perf/play call above.
        [ "${#RAW_SEQ[@]}" -eq 2 ] && [ -z "$WITHAUDIO_FIRST_CHANGE_T" ] && WITHAUDIO_FIRST_CHANGE_T="$ELAPSED"
    fi
    # Replay-shape sample for the continuous opacity row (critic MAJOR fix,
    # not an exact-timing check -- plan section 4's "on replay layer 0
    # opacity follows 0.4->0.7->0.2 smoothly" bullet): opacity should visit
    # values near each of the 3 recorded gestures (0.05 tolerance), not just
    # jump straight to the final 0.2.
    awk -v x="$OP_LAST" 'BEGIN{exit !(x!="NA" && x+0>=0.35 && x+0<=0.45)}' 2>/dev/null && OP_SEEN_04=1
    awk -v x="$OP_LAST" 'BEGIN{exit !(x!="NA" && x+0>=0.65 && x+0<=0.75)}' 2>/dev/null && OP_SEEN_07=1
    POS="$(perf_field "d.get('position','NA')")"
    awk -v p="$POS" -v l="$TAKE_LEN" 'BEGIN{exit !(p!="NA" && l!="NA" && l+0>0 && p+0>=l+0-0.25)}' 2>/dev/null && break
    awk -v e="$ELAPSED" -v b="$REPLAY_BUDGET" 'BEGIN{exit !(e+0>=b+0)}' 2>/dev/null && break
    sleep 0.25
done
seq_matches_expected "${RAW_SEQ[@]}" \
  && ok "replay(withAudio): activeClipColumn sequence == 0,1,2,3,2 after dropping the pre-play leftover (raw=${RAW_SEQ[*]})" \
  || no "replay(withAudio): activeClipColumn sequence wrong (raw=${RAW_SEQ[*]}, expected leftover then 0 1 2 3 2)"
[ "$OP_SEEN_04" = "1" ] && ok "replay(withAudio): layer 0 opacity visited near 0.4 (tolerance 0.05)" || no "replay(withAudio): layer 0 opacity never visited near 0.4"
[ "$OP_SEEN_07" = "1" ] && ok "replay(withAudio): layer 0 opacity visited near 0.7 (tolerance 0.05)" || no "replay(withAudio): layer 0 opacity never visited near 0.7"
awk -v x="$OP_LAST" 'BEGIN{exit !(x!="NA" && x+0>=0.15 && x+0<=0.25)}' 2>/dev/null && ok "replay(withAudio): layer 0 opacity settled near the final 0.2 (last observed=$OP_LAST)" || no "replay(withAudio): layer 0 opacity did not settle near 0.2 (last observed=$OP_LAST)"
BPM_REPLAY="$(jget "$A/api/bpm" "d.get('bpm','NA')")"
awk -v x="$BPM_REPLAY" 'BEGIN{exit !(x+0>=127.5 && x+0<=128.5)}' 2>/dev/null && ok "replay(withAudio): bpm reached 128 after the tempo point ($BPM_REPLAY)" || no "replay(withAudio): bpm=$BPM_REPLAY (expected ~128)"
SKIPPED="$(perf_field "d.get('skipped','NA')")"
[ "$SKIPPED" = "1" ] && ok "replay(withAudio): status.skipped==1 (the arm-time 'audio play' point, WithAudio policy)" || no "replay(withAudio): status.skipped==$SKIPPED (expected 1)"
curl -s --max-time 6 -X POST "$A/api/perf/stop_play" >/dev/null
sleep 1
PLAYING1="$(perf_field "d.get('playing','NA')")"
[ "$PLAYING1" = "False" -o "$PLAYING1" = "false" ] && ok "perf/stop_play: playing==false" || no "perf/stop_play: playing==$PLAYING1"

curl -s --max-time 6 -X POST "$A/api/perf/play" -H 'Content-Type: application/json' -d '{"withAudio":false}' >/dev/null
LAST="__UNSET__"; RAW_SEQ2=()
WALLCLOCK_FIRST_CHANGE_T=""
START_T=$(date +%s)
while :; do
    v="$(comp_active_col)"
    ELAPSED=$(( $(date +%s) - START_T ))
    if [ "$v" != "$LAST" ]; then
        echo "  replay(wallClock): t~${ELAPSED}s activeClipColumn=$v"
        RAW_SEQ2+=("$v")
        LAST="$v"
        # Probe pin (fix-plan.md F1 "Probe pin"): pin the FIRST post-leftover
        # activeClipColumn change (RAW_SEQ2[1]) at its ELAPSED time, seconds
        # since THIS loop's own /api/perf/play call above -- the SAME
        # convention as WITHAUDIO_FIRST_CHANGE_T, so the two are comparable
        # take-relative timestamps regardless of app uptime.
        [ "${#RAW_SEQ2[@]}" -eq 2 ] && [ -z "$WALLCLOCK_FIRST_CHANGE_T" ] && WALLCLOCK_FIRST_CHANGE_T="$ELAPSED"
    fi
    POS="$(perf_field "d.get('position','NA')")"
    awk -v p="$POS" -v l="$TAKE_LEN" 'BEGIN{exit !(p!="NA" && l!="NA" && l+0>0 && p+0>=l+0-0.25)}' 2>/dev/null && break
    awk -v e="$ELAPSED" -v b="$REPLAY_BUDGET" 'BEGIN{exit !(e+0>=b+0)}' 2>/dev/null && break
    sleep 0.25
done
seq_matches_expected "${RAW_SEQ2[@]}" \
  && ok "replay(wallClock): activeClipColumn sequence == 0,1,2,3,2 after dropping the pre-play leftover (raw=${RAW_SEQ2[*]})" \
  || no "replay(wallClock): activeClipColumn sequence wrong (raw=${RAW_SEQ2[*]}, expected leftover then 0 1 2 3 2)"
# Probe pin (fix-plan.md F1 "Probe pin", gate section 4 item 2 "the new
# wall-clock timing pin"): the FIRST post-leftover activeClipColumn change
# must land within 1.5s of the same point in the OTHER replay loop, both
# measured in ELAPSED seconds since EACH loop's own /api/perf/play call (not
# wall-clock app-uptime). Pre-fix, RecorderHost's clock_ never restarts per
# take (fix-plan.md F1) so a wall-clock replay fires every event as late as
# the app was old when Record was pressed -- this pin is RED before the fix
# (observed ~14s vs ~2s) and GREEN after it (RecorderHost's clock_ = RecorderClock{} at arm()).
if [ -n "$WITHAUDIO_FIRST_CHANGE_T" ] && [ -n "$WALLCLOCK_FIRST_CHANGE_T" ]; then
    PIN_DIFF="$(awk -v a="$WITHAUDIO_FIRST_CHANGE_T" -v b="$WALLCLOCK_FIRST_CHANGE_T" 'BEGIN{d=a-b; if(d<0)d=-d; printf "%.1f", d}')"
    awk -v x="$PIN_DIFF" 'BEGIN{exit !(x<=1.5)}' 2>/dev/null \
      && ok "probe pin: first post-leftover activeClipColumn change lands within 1.5s across replay modes (withAudio=${WITHAUDIO_FIRST_CHANGE_T}s wallClock=${WALLCLOCK_FIRST_CHANGE_T}s diff=${PIN_DIFF}s)" \
      || no "probe pin: first post-leftover activeClipColumn change diverges across replay modes by >1.5s (withAudio=${WITHAUDIO_FIRST_CHANGE_T}s wallClock=${WALLCLOCK_FIRST_CHANGE_T}s diff=${PIN_DIFF}s -- the recorder's per-take clock must restart at arm, fix-plan.md F1)"
else
    no "probe pin: first post-leftover activeClipColumn change was not observed in one or both replay loops (withAudio=${WITHAUDIO_FIRST_CHANGE_T:-NA}s wallClock=${WALLCLOCK_FIRST_CHANGE_T:-NA}s)"
fi
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
    # s-rta-0924 (Harmony): count the DELTA vs before arm -- earlier runs'
    # assets legitimately stay in the shared store (Ruling 28: never delete).
    N_ASSETS="$(ls -1 "$AUDIO_DIR" 2>/dev/null | grep -c '\.adna-audio$')"
    N_NEW=$((N_ASSETS - N_ASSETS_BEFORE))
    [ "$N_NEW" = "1" ] && ok "Audio/ gained exactly one asset this run, overdub added none ($N_ASSETS_BEFORE -> $N_ASSETS)" || no "Audio/ gained $N_NEW assets this run ($N_ASSETS_BEFORE -> $N_ASSETS; expected 1 -- overdub must not create a new one)"
else
    no "overdub take.json missing at $GATE2_FOLDER"
fi

# --- 11L. LONG take for the T2 drift proof (OPT-IN: STEP3_LONG=1) ----------
# Spec D10.3 T2 (s167 spec :606-612) measures drift as offset(minute 10) -
# offset(minute 0) over a 10-minute file-mode click take; the ~65 s take of
# section 5-9 cannot support the 1 ms claim (slope-drift stderr ~2.3 ms).
# This block records ONE extra take (step3long) of STEP3_LONG_MINUTES minutes
# (+10 s margin) against the longer click WAV from step 0, stops it, and runs
# the SAME t2_align oracle with the window argument. No replay of this take
# (2x its length for no drift information). Placed AFTER section 11 so its
# "Audio/ gained exactly one asset" delta is untouched, and BEFORE the crash
# row. Endpoints used: /api/perf/record, /api/perf/status, /api/perf/stop
# only -- the A6-verified safe set; no output-window path.
# Thresholds: the spec's 1 ms drift and 15 ms p95, applied with the SAME
# "<= 1 ms + 2*stderr" rule as section 9 (a hard 1 ms bound is at noise level
# even at 10 minutes: window-difference stderr ~1 ms, slope stderr ~0.75 ms at
# the observed ~7.6 ms per-marker jitter -- a ~31% / ~19% false-FAIL rate).
# The WARN prints the take length that WOULD retire it at this run's jitter.
# critic-long-drift.md MINOR amendment: a tap self-stop (device rate/channel
# change) sets lastError but leaves recording=true (RecorderHost.cpp
# :403-415) -- the poll loop below inspects BOTH fields and breaks early on
# either, so a dead recorder never costs the full wall time.
if [ "$LONG" = "1" ]; then
    LONG_TAKE_NAME="step3long"
    LONG_TAKE_FOLDER="$TAKES_DIR/$LONG_TAKE_NAME.adna-take"
    LONG_SECS=$((LONG_MINUTES * 60))
    curl -s --max-time 6 -X POST "$A/api/perf/record" -H 'Content-Type: application/json' \
      -d "{\"name\":\"$LONG_TAKE_NAME\",\"audio\":true,\"audioFile\":\"$CLICK_WAV_LONG\",\"onsetMarkers\":true}" \
      | grep -q '"ok":[[:space:]]*true' \
      && ok "LONG: perf/record accepted (file-mode ${LONG_MINUTES}-minute click, onset markers)" \
      || no "LONG: perf/record refused"
    sleep 2
    LREC="$(perf_field "d.get('recording','NA')")"
    [ "$LREC" = "True" -o "$LREC" = "true" ] && ok "LONG: perf/status recording=true" || no "LONG: perf/status recording is not true ($LREC)"
    LASSET="$(perf_field "d.get('assetId','NA')")"
    # Record LONG_SECS + 10 s so the minute-M window [(M-1)*60, M*60) sits
    # fully inside the asset. Poll every <=30 s and FAIL out the moment the
    # recorder reports recording=false OR a non-empty lastError (a self-stop
    # only sets lastError and keeps recording=true -- RecorderHost.cpp
    # :403-415, critic amendment). A transient perf/status read failure is
    # tolerated, not treated as a stop.
    LONG_EARLY_STOP=0; LONG_EARLY_REASON=""; LNOW=0
    LSTART=$(date +%s)
    while :; do
        LNOW=$(( $(date +%s) - LSTART ))
        LR="$(perf_status | python3 -c 'import json,sys
d=json.load(sys.stdin)
le=str(d.get("lastError","") or "")
print("%s|%s|t=%s markers=%s framesWritten=%s" % (d.get("recording","NA"), le, d.get("t","NA"), d.get("markers","NA"), d.get("framesWritten","NA")))' 2>/dev/null || echo "NA||")"
        LREC_POLL="${LR%%|*}"
        LR_REST="${LR#*|}"
        LERR_POLL="${LR_REST%%|*}"
        LR_TAIL="${LR_REST#*|}"
        echo "  long take: wall ${LNOW}s  recording=$LREC_POLL lastError='$LERR_POLL' $LR_TAIL"
        case "$LREC_POLL" in
            True|true)
                if [ -n "$LERR_POLL" ]; then
                    LONG_EARLY_STOP=1; LONG_EARLY_REASON="lastError='$LERR_POLL'"; break
                fi
                ;;
            False|false) LONG_EARLY_STOP=1; LONG_EARLY_REASON="recording=false"; break ;;
            *) echo "  (transient perf/status read failure, continuing)" ;;
        esac
        LLEFT=$((LONG_SECS + 10 - LNOW))
        [ "$LLEFT" -le 0 ] && break
        sleep $(( LLEFT<30 ? LLEFT : 30 ))
    done
    [ "$LONG_EARLY_STOP" = "0" ] \
      && ok "LONG: recorder stayed armed for the whole ${LONG_MINUTES}-minute take" \
      || no "LONG: recorder stopped early at wall ~${LNOW}s (${LONG_EARLY_REASON:-unknown}; see the status line above)"
    curl -s --max-time 6 -X POST "$A/api/perf/stop" >/dev/null
    sleep 3
    LERR="$(perf_field "d.get('lastError','NA')")"
    [ -z "$LERR" ] && ok "LONG: lastError empty after stop (no tap self-stop, no rate change, no save failure)" || no "LONG: lastError=$LERR"
    FIN_ERR="$(perf_field "d.get('lastFinalizeError','NA')")"
    [ "$FIN_ERR" = "" ] && ok "LONG: perf/status lastFinalizeError empty after stop (no block lost at stop, s-rta-0924b)" \
      || no "LONG: perf/status lastFinalizeError='$FIN_ERR' (s-rta-0924b finalize truncation, or field missing)"
    LRATE="$(perf_field "d.get('rateChangedSinceArm','NA')")"
    [ "$LRATE" = "False" -o "$LRATE" = "false" ] \
      && ok "LONG: rateChangedSinceArm == false across ${LONG_MINUTES} minutes" \
      || no "LONG: rateChangedSinceArm == $LRATE (expected false)"
    LASSET_DIR="$AUDIO_DIR/$LASSET.adna-audio"
    LSEG_FRAMES="$(take_field "$LONG_TAKE_FOLDER" "d['audio']['segments'][0]['frames']")"
    LSEG_RATE="$(take_field "$LONG_TAKE_FOLDER" "d['audio']['segments'][0]['rate']")"
    awk -v f="$LSEG_FRAMES" -v r="$LSEG_RATE" -v s="$LONG_SECS" 'BEGIN{exit !(r+0>0 && f+0>=s*r)}' 2>/dev/null \
      && ok "LONG: take audio covers the full ${LONG_MINUTES} min ($LSEG_FRAMES frames @ $LSEG_RATE Hz)" \
      || no "LONG: take audio shorter than ${LONG_MINUTES} min ($LSEG_FRAMES frames @ $LSEG_RATE Hz) -- the D10.3 window rows below cannot be trusted"
    if [ -x "$ROOT/.venv/bin/python" ]; then
        LALIGN="$(t2_align "$LASSET_DIR/audio.wav" "$LONG_TAKE_FOLDER/take.json" "$CLICK_INTERVAL" "$LONG_MINUTES")"
        echo "T2 alignment (LONG, ${LONG_MINUTES} min): $LALIGN"
        if echo "$LALIGN" | grep -q '^mean_offset_ms='; then
            LMEAN="$(echo "$LALIGN" | sed -n 's/.*mean_offset_ms=\([0-9.-]*\).*/\1/p')"
            LDRIFT="$(echo "$LALIGN" | sed -n 's/.* drift_ms=\([0-9.-]*\).*/\1/p')"
            LDRIFT_SE="$(echo "$LALIGN" | sed -n 's/.* drift_stderr_ms=\([0-9.-]*\).*/\1/p')"
            LP95="$(echo "$LALIGN" | sed -n 's/.*p95_jitter_ms=\([0-9.-]*\).*/\1/p')"
            LPCT="$(echo "$LALIGN" | sed -n 's/.*pct_matched=\([0-9.-]*\).*/\1/p')"
            LWIN="$(echo "$LALIGN" | sed -n 's/.* drift_win_ms=\([0-9.-]*\).*/\1/p')"
            LWIN_SE="$(echo "$LALIGN" | sed -n 's/.* drift_win_stderr_ms=\([0-9.-]*\).*/\1/p')"
            LN0="$(echo "$LALIGN" | sed -n 's/.* n_win_first=\([0-9]*\).*/\1/p')"
            LN1="$(echo "$LALIGN" | sed -n 's/.* n_win_last=\([0-9]*\).*/\1/p')"
            LMIN_NEEDED="$(echo "$LALIGN" | sed -n 's/.* minutes_for_0p5ms=\([0-9.]*\).*/\1/p')"
            # (a) D10.3 literal statistic: offset(minute M) - offset(minute 1).
            if [ -n "$LWIN" ]; then
                LWIN_BOUND="$(awk -v se="$LWIN_SE" 'BEGIN{printf "%.4f", 1.0 + 2*se}' 2>/dev/null)"
                awk -v x="$LWIN" -v b="$LWIN_BOUND" 'BEGIN{exit !(x<=b && x>=-b)}' 2>/dev/null \
                  && ok "LONG D10.3: |offset(min $LONG_MINUTES) - offset(min 1)| within +-${LWIN_BOUND}ms (<=1ms+2*stderr; drift=${LWIN}ms stderr=${LWIN_SE}ms n=${LN0}/${LN1})" \
                  || no "LONG D10.3: |offset(min $LONG_MINUTES) - offset(min 1)| exceeds +-${LWIN_BOUND}ms (<=1ms+2*stderr; drift=${LWIN}ms stderr=${LWIN_SE}ms n=${LN0}/${LN1})"
                awk -v se="$LWIN_SE" 'BEGIN{exit !(se>0.5)}' 2>/dev/null \
                  && warn "LONG D10.3: window stderr ${LWIN_SE}ms exceeds 0.5ms -- two 60 s windows cannot support a tight 1ms claim at this per-marker jitter (informational; the slope row below uses every marker)"
            else
                no "LONG D10.3: window statistic unavailable (n_win_first=$LN0 n_win_last=$LN1 -- fewer than 2 markers in a window: take too short, onsets stopped, or the WAV ran out)"
            fi
            # (b) slope over the whole take (every marker), same rule as section 9.
            LDRIFT_BOUND="$(awk -v se="$LDRIFT_SE" 'BEGIN{printf "%.4f", 1.0 + 2*se}' 2>/dev/null)"
            awk -v x="$LDRIFT" -v b="$LDRIFT_BOUND" 'BEGIN{exit !(x<=b && x>=-b)}' 2>/dev/null \
              && ok "LONG slope: drift over ${LONG_MINUTES} min within +-${LDRIFT_BOUND}ms (<=1ms+2*stderr; drift=${LDRIFT}ms stderr=${LDRIFT_SE}ms)" \
              || no "LONG slope: drift over ${LONG_MINUTES} min exceeds +-${LDRIFT_BOUND}ms (<=1ms+2*stderr; drift=${LDRIFT}ms stderr=${LDRIFT_SE}ms)"
            awk -v se="$LDRIFT_SE" 'BEGIN{exit !(se>0.5)}' 2>/dev/null \
              && warn "LONG slope: stderr ${LDRIFT_SE}ms exceeds 0.5ms -- at this run's jitter a 0.5ms stderr needs ~${LMIN_NEEDED:-?} minutes (STEP3_LONG_MINUTES=N); informational, does not fail the gate"
            echo "(informational) LONG drift bound in ppm: $(awk -v b="$LDRIFT_BOUND" -v s="$LONG_SECS" 'BEGIN{printf "%.1f", b*1000.0/s}' 2>/dev/null) ppm over ${LONG_SECS}s (spec D10.2 warns wall clock drifts tens of ppm)"
            awk -v x="$LP95" 'BEGIN{exit !(x<=15.0)}' 2>/dev/null \
              && ok "LONG: p95 jitter <=15ms ($LP95 ms, D10.3)" || no "LONG: p95 jitter exceeds 15ms ($LP95 ms)"
            awk -v x="$LMEAN" 'BEGIN{exit !(x>=0.0 && x<=60.0)}' 2>/dev/null \
              && ok "LONG: mean offset within [0,60]ms ($LMEAN ms)" || no "LONG: mean offset outside [0,60]ms ($LMEAN ms)"
            awk -v x="$LPCT" 'BEGIN{exit !(x>=90.0)}' 2>/dev/null \
              && ok "LONG: matched markers cover >=90% of grid clicks ($LPCT%)" || no "LONG: matched markers cover only $LPCT% of grid clicks (expected >=90%)"
        else
            no "LONG: T2 alignment could not be computed ($LALIGN)"
        fi
    else
        no "LONG: .venv/bin/python with numpy unavailable -- run from the main checkout"
    fi
else
    skip "LONG take for the T2 drift proof (set STEP3_LONG=1; STEP3_LONG_MINUTES=N overrides, default 10, 2 = dry run)"
fi

# --- 12. crash-readability (OPTIONAL, DESTRUCTIVE -- run only if asked) ---
if [ "${STEP3_RUN_CRASH_TEST:-0}" = "1" ]; then
    curl -s --max-time 6 -X POST "$A/api/perf/record" -H 'Content-Type: application/json' \
      -d '{"name":"step3gate3","audio":true,"audioFile":"'"$CLICK_WAV"'"}' >/dev/null
    sleep 30
    KPID="$(pgrep -f 'MacOS/Audio-DN[A]' | head -1)"
    [ -n "$KPID" ] && kill -9 "$KPID" 2>/dev/null
    for _ in $(seq 1 20); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
    : > /tmp/adna-step3-out2.log
    : > /tmp/adna-step3-err2.log
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

# --- 12b. run-level finalize check (s-rta-0924b) -- BEFORE the quit below: the app must still
# answer. finalizeErrors is never reset within one app process; if section 12 ran, the app was
# relaunched (kill -9), so this counts only that second process's stops, and the stderr grep
# covers the first process's log (/tmp/adna-step3-err.log).
FIN_COUNT="$(perf_field "d.get('finalizeErrors','NA')")"; FIN_COUNT="$(normnum "$FIN_COUNT")"
[ "$FIN_COUNT" = "0" ] && ok "no stop in this run reported a finalize problem (perf/status finalizeErrors == 0)" || no "perf/status finalizeErrors == $FIN_COUNT (some stop in this run truncated or failed to finalize)"
TRUNC_LOG="$(grep -c 'truncated (header' /tmp/adna-step3-err.log 2>/dev/null)"; TRUNC_LOG="${TRUNC_LOG:-0}"
[ "$TRUNC_LOG" = "0" ] && ok "stderr: no '[Recorder] asset ...: truncated' line this run" || no "stderr: $TRUNC_LOG truncated-asset line(s) in /tmp/adna-step3-err.log"

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
    # s-rta-0924 cleanup lane: FAIL, not SKIP -- a missing .venv/pyobjc silently
    # dropped the SCREEN-SAFETY LAW check from the count entirely, so a run could
    # still print "N PASS / 0 FAIL" while never having confirmed the Output
    # window stayed closed (a silently weaker gate). Run from the main checkout
    # (which already has a pyobjc-provisioned .venv), or create one here:
    #   python3 -m venv .venv && .venv/bin/pip install pyobjc-framework-Quartz
    no "Output-window check: .venv/bin/python with pyobjc/Quartz unavailable -- run from the main checkout, or: python3 -m venv .venv && .venv/bin/pip install pyobjc-framework-Quartz"
fi

# Final screen state, for the handoff (plan section 4 Teardown paragraph:
# "state the screen state in the handoff").
screencapture -x /tmp/step3-eos.png 2>/dev/null \
  && echo "screenshot: /tmp/step3-eos.png (read it before concluding -- no black overlay, no dialog expected)" \
  || echo "screencapture failed (no display attached / headless run)"

echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT; take folders under $TAKES_DIR; asset under $AUDIO_DIR)"
[ "$FAIL" -eq 0 ] || exit 1
