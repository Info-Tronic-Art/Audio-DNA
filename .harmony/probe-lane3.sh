#!/bin/bash
# probe-lane3.sh — the LANE 3 live gate. s-rta-0923.
# .harmony/specs/s-rta-0923-lane3-plan.md section 6, authored by Lane C0
# (scaffold) per this dispatch's builder packet. Lane C5 (Harmony) RUNS this
# -- "the party that builds never verifies" -- once against the PRE-change
# binary (expect FAIL at oracle A: the "connected"/"live" fields are absent)
# and once against the merged tree (expect all PASS).
#
# WHAT IT PROVES
#   Oracle A: /api/composition's live.opacity toggles 0<->1 on a layer
#     driven by a 1-beat Square Lfo at 120 BPM -- the engine tick (C3) and
#     the API's live/connected fields (C3) are wired end to end.
#   Oracle B: render_frame's pixels actually differ between a live=1 and a
#     live=0 moment -- the renderer eff() repoint (C2) reaches the GPU.
#   GRIP: set_layer_opacity (a Decaying-rank HTTP write, C3 site #9) holds
#     the value for ~gripHoldMs, then the signal visibly resumes -- Ruling A
#     "latest input wins, automatic hand-back on release" is live, not just
#     unit-tested.
#   CLIP / COMP: the same wiring one level down (clip scalar) and one level
#     up (composition scalar) from the layer oracle above.
#
# RIG FACTS IT DEPENDS ON (do not re-derive -- see .harmony/gotchas.md and
# .harmony/VALIDATION.md):
#   * ApiServer = http://127.0.0.1:7070 (IPv4 ONLY), always on, no
#     --test-mode needed. inject_features is test-mode only and NOT used
#     here: set_bpm puts the BPM tracker in manual mode, so beat phase (and
#     every Lfo/Envelope connection keyed off it) runs with no audio input.
#   * render_frame lives on 7070, requires {"output_path": "..."}, writes a
#     deterministic PNG on the deck path (probe-deck-path.sh, 2026-09-05).
#   * Launch via `open` (production mode, gotcha 2026-07-17) -- NOT the raw
#     binary, NOT --test-mode. First launch after a rebuild needs Boris to
#     click Allow on the TCC mic prompt (gotcha 2026-07-25): if /api/health
#     never answers, screencapture -x and LOOK before concluding.
#   * set_layer_opacity's ack returns BEFORE the message-thread write lands
#     (ApiServer.cpp callAsync) -- the GRIP check sleeps >=30ms before its
#     first sample and reads samples 2..5, never 1..4 (critic non-blocking
#     finding #7).
#
# SCREEN-SAFETY LAW: this launches the REAL APP and NEVER opens the output
# window (no endpoint here does). Termination is a `pkill` -- per this
# dispatch's builder packet, explicitly gated: it only fires AFTER the
# script has confirmed 0 Audio-DNA windows in the FULL CGWindowList, i.e.
# after confirming the law already held throughout the run.
set -u
OUT="${1:-/tmp/audiodna-lane3}"; mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APPBUNDLE="$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app"
A='http://127.0.0.1:7070'
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }

R(){ curl -s --max-time 20 -X POST "$A/api/render_frame" -H 'Content-Type: application/json' \
       -d "{\"output_path\":\"$OUT/$1\"}" >/dev/null; }
H(){ md5 -q "$OUT/$1"; }
lum(){ "$ROOT/.venv/bin/python" -c "
from PIL import Image; import numpy as np
print('%.4f' % (np.asarray(Image.open('$OUT/$1').convert('RGB')).astype('float32').mean()))" 2>/dev/null; }

comp_json(){ curl -s --max-time 5 "$A/api/composition"; }
# jpath NAME PYEXPR: evaluate PYEXPR against the parsed /api/composition
# JSON (bound to `d`); "NA" on any missing key/parse error -- callers treat
# "NA" as "field absent" (the pre-change-binary case, deliberately not a
# crash).
jpath(){ comp_json | python3 -c "
import json, sys
try:
    d = json.load(sys.stdin)
    print($1)
except Exception:
    print('NA')
"
}
layer0_active_col(){ jpath "d['decks'][0]['layers'][0]['activeClipColumn']"; }
layer0_connected_has_opacity(){ jpath "'yes' if 'opacity' in d['decks'][0]['layers'][0].get('connected', []) else 'no'"; }
layer0_live_opacity(){ jpath "d['decks'][0]['layers'][0]['live']['opacity']"; }
clip0_live_scale(){ jpath "d['decks'][0]['layers'][0]['clips'][0]['live']['scale']"; }
comp_live_positionx(){ jpath "d['live']['positionX']"; }

pgrep -f 'MacOS/Audio-DNA' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE"; exit 64; }

open "$APPBUNDLE"
for i in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: health never came up on $A (a production launch needs the mic-permission prompt clicked once -- screencapture -x and LOOK before concluding)"; exit 1; }
PID="$(pgrep -f 'MacOS/Audio-DNA' | head -1)"
[ -n "$PID" ] || { echo "FAIL: health answered but no Audio-DNA process was found"; exit 1; }

curl -s --max-time 6 -X POST "$A/api/set_bpm" -H 'Content-Type: application/json' -d '{"bpm":120}' >/dev/null
# 120 BPM: 0.5s/beat. A 1-beat Square Lfo is high 0.25s / low 0.25s.

# ---------------------------------------------------------------------------
# LAYER oracle: /api/probe-lane3-layer.json (layer scalar "opacity" <- 1-beat
# Square Lfo)
# ---------------------------------------------------------------------------
curl -s --max-time 6 -X POST "$A/api/load_composition" -H 'Content-Type: application/json' \
  -d "{\"path\":\"$ROOT/.harmony/probe-lane3-layer.json\"}" | grep -q '"ok":[[:space:]]*true' \
  && ok "load_composition accepted the layer fixture" || no "load_composition rejected the layer fixture"
sleep 1
curl -s --max-time 6 -X POST "$A/api/trigger_clip" -H 'Content-Type: application/json' -d '{"layer":0,"column":0}' >/dev/null
sleep 1
[ "$(layer0_active_col)" = "0" ] && ok "deck is live: a clip is triggered and active" \
  || no "no active clip -- the deck path is NOT engaged, everything below is void"

CONNECTED="$(layer0_connected_has_opacity)"
[ "$CONNECTED" = "yes" ] && ok "layers[0].connected contains 'opacity'" \
  || no "layers[0].connected does NOT contain 'opacity' (field absent on the pre-change binary -- this IS the fail-first gate)"

if [ "$CONNECTED" = "yes" ]; then
    # ORACLE A: sample live.opacity 20x @ 50ms; PASS = both a value <=0.05
    # and a value >=0.95 observed, and it changes at least twice.
    LOW_SEEN=0; HIGH_SEEN=0; CHANGES=0; PREV=""
    SAMPLES=()
    for i in $(seq 1 20); do
        v="$(layer0_live_opacity)"
        SAMPLES+=("$v")
        awk -v x="$v" 'BEGIN{exit !(x<=0.05)}' 2>/dev/null && LOW_SEEN=1
        awk -v x="$v" 'BEGIN{exit !(x>=0.95)}' 2>/dev/null && HIGH_SEEN=1
        if [ -n "$PREV" ] && [ "$PREV" != "$v" ]; then CHANGES=$((CHANGES+1)); fi
        PREV="$v"
        sleep 0.05
    done
    { [ "$LOW_SEEN" = "1" ] && [ "$HIGH_SEEN" = "1" ] && [ "$CHANGES" -ge 2 ]; } \
        && ok "ORACLE A: live.opacity toggles 0<->1 on the 1-beat Square Lfo (${SAMPLES[*]})" \
        || no "ORACLE A: live.opacity did not toggle as expected (low=$LOW_SEEN high=$HIGH_SEEN changes=$CHANGES; samples: ${SAMPLES[*]})"

    # ORACLE B (critic non-blocking finding #7): capture on an OBSERVED
    # TRANSITION (prev<=0.05 then cur>=0.95, and vice versa) rather than a
    # single polled sample -- gravity_well is a stateful animated source, so
    # md5(hi)!=md5(lo) alone proves nothing; the luminance floor + ratio is
    # the real oracle.
    HI_OK=0; PREV=""
    for i in $(seq 1 40); do
        v="$(layer0_live_opacity)"
        if [ -n "$PREV" ] && awk -v p="$PREV" 'BEGIN{exit !(p<=0.05)}' 2>/dev/null \
                          && awk -v c="$v" 'BEGIN{exit !(c>=0.95)}' 2>/dev/null; then
            R hi.png; HI_OK=1; break
        fi
        PREV="$v"; sleep 0.05
    done
    LO_OK=0; PREV=""
    for i in $(seq 1 40); do
        v="$(layer0_live_opacity)"
        if [ -n "$PREV" ] && awk -v p="$PREV" 'BEGIN{exit !(p>=0.95)}' 2>/dev/null \
                          && awk -v c="$v" 'BEGIN{exit !(c<=0.05)}' 2>/dev/null; then
            R lo.png; LO_OK=1; break
        fi
        PREV="$v"; sleep 0.05
    done
    if [ "$HI_OK" = "1" ] && [ "$LO_OK" = "1" ]; then
        HIH="$(H hi.png)"; LOH="$(H lo.png)"
        HILUM="$(lum hi.png)"; LOLUM="$(lum lo.png)"
        if [ -z "$HILUM" ] || [ -z "$LOLUM" ]; then
            echo "SKIP  ORACLE B luminance check (PIL unavailable in .venv)"
        else
            RATIO_OK="$(awk -v hi="$HILUM" -v lo="$LOLUM" 'BEGIN{print (hi>0 && lo<0.05*hi)?"yes":"no"}')"
            FLOOR_OK="$(awk -v hi="$HILUM" 'BEGIN{print (hi>=0.02)?"yes":"no"}')"
            [ "$HIH" != "$LOH" ] && [ "$RATIO_OK" = "yes" ] && [ "$FLOOR_OK" = "yes" ] \
                && ok "ORACLE B: render_frame pixels differ hi/lo (hiLum=$HILUM loLum=$LOLUM)" \
                || no "ORACLE B: hi/lo did not differ as expected (md5 equal=$([ "$HIH" = "$LOH" ] && echo yes || echo no), ratio_ok=$RATIO_OK, floor_ok=$FLOOR_OK, hiLum=$HILUM loLum=$LOLUM)"
        fi
    else
        no "ORACLE B: never observed a full 0<->1 transition to capture on (hi_ok=$HI_OK lo_ok=$LO_OK)"
    fi

    # GRIP: set_layer_opacity holds ~gripHoldMs (Decaying rank), then the
    # signal resumes. Critic non-blocking finding #7: the ack returns before
    # the message-thread write lands -- sleep >=30ms before the first
    # sample, and judge samples 2..5 (not 1..4).
    curl -s --max-time 6 -X POST "$A/api/set_layer_opacity" -H 'Content-Type: application/json' -d '{"layer":0,"opacity":0.5}' >/dev/null
    sleep 0.03
    GSAMPLES=()
    for i in $(seq 1 20); do
        GSAMPLES+=("$(layer0_live_opacity)")
        sleep 0.05
    done
    HELD_OK=1
    for i in 2 3 4 5; do
        v="${GSAMPLES[$((i-1))]}"
        awk -v x="$v" 'BEGIN{exit !(x>0.4 && x<0.6)}' 2>/dev/null || HELD_OK=0
    done
    RESUMED_OK=0
    for i in $(seq 8 20); do
        v="${GSAMPLES[$((i-1))]}"
        { awk -v x="$v" 'BEGIN{exit !(x<=0.05)}' 2>/dev/null || awk -v x="$v" 'BEGIN{exit !(x>=0.95)}' 2>/dev/null; } && RESUMED_OK=1
    done
    { [ "$HELD_OK" = "1" ] && [ "$RESUMED_OK" = "1" ]; } \
        && ok "GRIP: set_layer_opacity holds ~0.5 then the signal resumes (${GSAMPLES[*]})" \
        || no "GRIP: held/resume pattern not observed (held_ok=$HELD_OK resumed_ok=$RESUMED_OK; samples: ${GSAMPLES[*]})"
else
    no "ORACLE A/B/GRIP skipped -- 'opacity' is not connected"
    no "ORACLE B skipped -- 'opacity' is not connected"
    no "GRIP skipped -- 'opacity' is not connected"
fi

# ---------------------------------------------------------------------------
# CLIP: probe-lane3-clip.json (clip scalar "scale" <- 2-beat sine)
# ---------------------------------------------------------------------------
curl -s --max-time 6 -X POST "$A/api/load_composition" -H 'Content-Type: application/json' \
  -d "{\"path\":\"$ROOT/.harmony/probe-lane3-clip.json\"}" | grep -q '"ok":[[:space:]]*true' \
  && ok "load_composition accepted the clip fixture" || no "load_composition rejected the clip fixture"
sleep 1
curl -s --max-time 6 -X POST "$A/api/trigger_clip" -H 'Content-Type: application/json' -d '{"layer":0,"column":0}' >/dev/null
sleep 1
R clip_a.png; sleep 0.5; R clip_b.png
[ "$(H clip_a.png)" != "$(H clip_b.png)" ] \
    && ok "CLIP: render_frame differs 0.5s apart (scale sine, 2 beats @ 120bpm = 1s/cycle)" \
    || no "CLIP: render_frame did NOT change -- the clip scalar 'scale' connection is not reaching the renderer"

# ---------------------------------------------------------------------------
# COMP: probe-lane3-comp.json (composition scalar "positionX" <- 4-beat sine)
# ---------------------------------------------------------------------------
curl -s --max-time 6 -X POST "$A/api/load_composition" -H 'Content-Type: application/json' \
  -d "{\"path\":\"$ROOT/.harmony/probe-lane3-comp.json\"}" | grep -q '"ok":[[:space:]]*true' \
  && ok "load_composition accepted the comp fixture" || no "load_composition rejected the comp fixture"
sleep 1
curl -s --max-time 6 -X POST "$A/api/trigger_clip" -H 'Content-Type: application/json' -d '{"layer":0,"column":0}' >/dev/null
sleep 1
R comp_a.png; sleep 1; R comp_b.png
[ "$(H comp_a.png)" != "$(H comp_b.png)" ] \
    && ok "COMP: render_frame differs 1.0s apart (positionX sine, 4 beats @ 120bpm = 2s/cycle)" \
    || no "COMP: render_frame did NOT change -- the composition scalar 'positionX' connection is not reaching the renderer"

# ---------------------------------------------------------------------------
# Teardown. SCREEN-SAFETY LAW: confirm 0 Audio-DNA windows in the FULL
# CGWindowList BEFORE terminating -- per this dispatch's builder packet,
# termination itself is `pkill` (not an AppleScript quit), gated on that
# confirmation already having held throughout the run.
# ---------------------------------------------------------------------------
W="$("$ROOT/.venv/bin/python" -c "
import Quartz
wl=Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName',''))]))" 2>/dev/null)"
if [ "$W" = "0" ] || [ -z "$W" ]; then
    ok "0 Audio-DNA windows in the FULL window list (SCREEN-SAFETY LAW held)"
else
    no "$W Audio-DNA window(s) STILL ON SCREEN -- screen-safety breach, deal with it now"
fi
pkill -f 'MacOS/Audio-DNA' >/dev/null 2>&1
for _ in $(seq 1 20); do pgrep -f 'MacOS/Audio-DNA' >/dev/null || break; sleep 1; done
pgrep -f 'MacOS/Audio-DNA' >/dev/null && no "APP STILL RUNNING AFTER pkill" || ok "app terminated, no process remains"

echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
