#!/bin/bash
# probe-onset-render.sh -- live gate for the onset render-path fix. s-rta-0924b.
# Recipe: .harmony/.reports/s-rta-0924b/plan-onset-render.md section 5, as
# amended by critic-onset-render.md + the Harmony ruling (test-mode onsetCount
# mirror resolved in ONE place under TestServer's lock -- not exercised here:
# this gate runs in PRODUCTION mode, where AnalysisThread is the only writer).
# Written by the Builder, NOT run by the Builder ("the party that builds never
# verifies"). Validate the script itself with:
#   bash -n .harmony/probe-onset-render.sh
#   shellcheck .harmony/probe-onset-render.sh      # if installed
#
# ORACLE: after a deterministic click train, the main Renderer's pulse-frame
# counter delta (/api/status renderOnsetPulses) must EQUAL the analysis onset
# counter delta (/api/features onsetCount). Pre-fix the two cannot agree: the
# raw one-hop onsetDetected bool is LOST below the ~93.75 Hz analysis rate and
# DUPLICATED above it (sign set by fps -- recorded below to interpret a RED).
# On a pre-fix build renderOnsetPulses is simply absent (NA) -> FAIL; a live
# RED with numbers needs an instrumented intermediate build (plan section 5,
# "fail-first at the live level") -- otherwise test B in
# tests/test_integration_pipeline.cpp (RED: 25 == 57 at 60 fps, 83 at 120 fps)
# plus db72d56's RecorderHost evidence stand in for it.
#
# RIG FACTS (same as probe-step3.sh's header -- do not re-derive):
#   * ApiServer = http://127.0.0.1:7070 (IPv4 ONLY), production mode, NO
#     --test-mode (test mode starts no analysis thread).
#   * Launch via `open`, not the raw binary. First launch after a rebuild
#     needs the mic-permission (TCC) prompt clicked once -- if /api/health
#     never answers, screencapture -x and LOOK before concluding.
#   * SELF-MATCH: every pgrep -f pattern uses the bracket trick
#     'MacOS/Audio-DN[A]', never the bare literal.
#   * The preview must be VISIBLE: a detached GL context renders no frames,
#     so the pulse counter would not advance (gotchas: "fps reads ~110").
#   * SCREEN-SAFETY LAW: never open the Output window; teardown is a graceful
#     osascript quit, pkill only as a last resort.
set -u
OUT="${1:-/tmp/audiodna-onset-render}"; mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${ONSET_BUILD_DIR:-build-lane}"
APPBUNDLE="$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app"
VENV_PY="${ONSET_VENV_PY:-$ROOT/.venv/bin/python}"
A='http://127.0.0.1:7070'
FIXTURE="$ROOT/.harmony/probe-step3.json"
CLICK_WAV="${ONSET_CLICK_WAV:-/tmp/click_onset_30s.wav}"
CLICK_DURATION_S=30          # 60 bursts, one every 0.5 s (gen-click-wav.py default interval)
EXPECTED_CLICKS=60
MIN_ONSETS=54                # >= 90% of 60 clicks (probe-step3 T2 tolerance)
TAKE_NAME="onsetrender"
TAKES_DIR="$HOME/Documents/Audio-DNA/Takes"
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
info(){ echo "INFO  $1"; }

jget(){ # $1 = URL  $2 = python expr against `d`; "NA(...)" on any error
  curl -s --max-time 5 "$1" | python3 -c "
import json, sys
try:
    d = json.load(sys.stdin)
    print($2)
except Exception as e:
    print('NA(%s)' % e)
"
}
isint(){ echo "$1" | grep -Eq '^[0-9]+$'; }
features_onsets(){ jget "$A/api/features" "d['onsetCount']"; }
render_pulses(){ jget "$A/api/status" "d['renderOnsetPulses']"; }
fps(){ jget "$A/api/status" "round(d['fps'], 1)"; }

# --- 0. click-track WAV (stdlib only, no .venv needed) ---------------------
python3 "$ROOT/.harmony/gen-click-wav.py" "$CLICK_WAV" --duration-s "$CLICK_DURATION_S" >/dev/null \
  && ok "click-track WAV generated at $CLICK_WAV ($CLICK_DURATION_S s, $EXPECTED_CLICKS bursts)" \
  || no "click-track WAV generation FAILED"

# --- 1. preconditions + production launch ----------------------------------
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE (set ONSET_BUILD_DIR to override)"; exit 64; }
[ -f "$CLICK_WAV" ] || { echo "REFUSE: click WAV missing after generation step"; exit 64; }

open --stdout "$OUT/adna-out.log" --stderr "$OUT/adna-err.log" "$APPBUNDLE"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: health never came up on $A (TCC mic prompt? screencapture -x and LOOK)"; exit 1; }
PID="$(pgrep -f 'MacOS/Audio-DN[A]' | head -1)"
[ -n "$PID" ] || { echo "FAIL: health answered but no Audio-DNA process was found"; exit 1; }
ok "app launched (production), /api/health answered, PID=$PID"

sleep 10
B1="$(jget "$A/api/bpm" "d.get('totalBarCount','NA')")"
[ "$B1" != "NA" ] && ok "analysis thread live: totalBarCount readable ($B1)" || no "totalBarCount not readable -- analysis thread not running?"

# --- 2. content + GL attach -------------------------------------------------
curl -s --max-time 6 -X POST "$A/api/load_composition" -H 'Content-Type: application/json' \
  -d "{\"path\":\"$FIXTURE\"}" | grep -q '"ok":[[:space:]]*true' \
  && ok "load_composition accepted probe-step3.json" || no "load_composition rejected probe-step3.json"
sleep 1
T_RF0="$(python3 -c 'import time; print(time.time())')"
RF_CODE="$(curl -s -o /dev/null -w '%{http_code}' --max-time 10 -X POST "$A/api/render_frame" \
  -H 'Content-Type: application/json' -d "{\"output_path\":\"$OUT/attach.png\"}")"
RF_MS="$(python3 -c "import time; print(int((time.time()-$T_RF0)*1000))")"
[ "$RF_CODE" = "200" ] && ok "render_frame 200 in ${RF_MS} ms (GL context attached -- preview visible)" \
  || no "render_frame returned $RF_CODE after ${RF_MS} ms -- preview not visible? the pulse counter cannot advance"
FPS0="$(fps)"; info "fps before the take: $FPS0"

# --- 3. baselines -----------------------------------------------------------
F0="$(features_onsets)"; S0="$(render_pulses)"
isint "$F0" && ok "/api/features onsetCount present ($F0)" || no "/api/features onsetCount missing ($F0)"
isint "$S0" && ok "/api/status renderOnsetPulses present ($S0)" || no "/api/status renderOnsetPulses missing ($S0) -- pre-fix build?"

# --- 4. arm: file-mode click train + onset markers (plays the WAV once) ----
curl -s --max-time 6 -X POST "$A/api/perf/record" -H 'Content-Type: application/json' \
  -d "{\"name\":\"$TAKE_NAME\",\"audio\":true,\"audioFile\":\"$CLICK_WAV\",\"onsetMarkers\":true}" \
  | grep -q '"ok":[[:space:]]*true' && ok "perf/record accepted (file-mode audio, onset markers)" \
  || no "perf/record refused"
T_ARM="$(python3 -c 'import time; print(time.time())')"

# --- 5. sample fps across the take (interprets a pre-fix sign) -------------
sleep_until(){ python3 -c "import time; d=$T_ARM+$1-time.time(); time.sleep(d if d>0 else 0)"; }
for t in 5 15 25; do
    sleep_until "$t"
    info "fps at t=${t}s: $(fps)"
done
sleep_until 34
curl -s --max-time 6 -X POST "$A/api/perf/stop" >/dev/null
sleep 2

# --- 6. final counters, stable (playback ended, no drift) -------------------
F1="$(features_onsets)"; S1="$(render_pulses)"
sleep 1
F1b="$(features_onsets)"; S1b="$(render_pulses)"
[ "$F1" = "$F1b" ] && [ "$S1" = "$S1b" ] \
  && ok "counters stable 1 s after stop (onsetCount=$F1, renderOnsetPulses=$S1)" \
  || no "counters still moving after stop (onsetCount $F1->$F1b, renderOnsetPulses $S1->$S1b) -- audio still feeding analysis?"

# --- 7. the oracle ------------------------------------------------------------
if isint "$F0" && isint "$F1" && isint "$S0" && isint "$S1"; then
    DF=$((F1 - F0)); DS=$((S1 - S0))
    info "onsetCount delta = $DF, renderOnsetPulses delta = $DS (fps before=$FPS0)"
    [ "$DF" -ge "$MIN_ONSETS" ] && ok "analysis detected $DF onsets (>= $MIN_ONSETS of $EXPECTED_CLICKS clicks)" \
      || no "analysis detected only $DF onsets (< $MIN_ONSETS of $EXPECTED_CLICKS) -- click train not reaching analysis?"
    [ "$DS" -eq "$DF" ] && ok "ORACLE: render pulse frames == onsets ($DS == $DF) -- no loss, no duplication" \
      || no "ORACLE: render pulse frames $DS != onsets $DF (fewer = loss, more = duplication; read the fps lines above)"
else
    no "ORACLE: a counter was unreadable (F0=$F0 F1=$F1 S0=$S0 S1=$S1)"
fi

# Informational third witness: RecorderHost's already delta-based onset markers.
TAKE_JSON="$TAKES_DIR/$TAKE_NAME.adna-take/take.json"
if [ -f "$TAKE_JSON" ]; then
    NM="$(python3 -c "
import json
with open('$TAKE_JSON') as f: d = json.load(f)
print(sum(1 for m in d.get('markers', []) if m.get('action') == 'onset'))" 2>/dev/null)"
    info "take.json onset markers = $NM (expect onsetCount delta +-1; baseline set at arm, F0 read just before)"
else
    info "take.json not found at $TAKE_JSON (informational only)"
fi

# --- 8. teardown (SCREEN-SAFETY LAW) ----------------------------------------
osascript -e 'quit app "Audio-DNA"' >/dev/null 2>&1
for _ in $(seq 1 30); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
if pgrep -f 'MacOS/Audio-DN[A]' >/dev/null; then
    echo "graceful osascript quit did not clear the process -- falling back to pkill (last resort)"
    pkill -f 'MacOS/Audio-DN[A]' >/dev/null 2>&1
    for _ in $(seq 1 20); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
fi
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && no "APP STILL RUNNING AFTER graceful quit + pkill fallback" || ok "app terminated, no process remains"

if [ -x "$VENV_PY" ]; then
    W="$("$VENV_PY" -c "
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
    no "Output-window check: $VENV_PY (pyobjc/Quartz) unavailable -- set ONSET_VENV_PY to the main checkout's .venv/bin/python"
fi

screencapture -x "$OUT/onset-render-eos.png" 2>/dev/null \
  && echo "screenshot: $OUT/onset-render-eos.png (read it before concluding -- no dialog expected)" \
  || echo "screencapture failed (no display attached / headless run)"

echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
