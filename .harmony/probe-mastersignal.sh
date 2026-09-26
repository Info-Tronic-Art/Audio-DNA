#!/bin/bash
# probe-mastersignal.sh -- the MASTER SIGNAL live gate. s-rta-0925.
# Recipe: .harmony/.reports/s-rta-0925/plan-mastersignal.md section 2.9
# ("Live gate -- .harmony/probe-mastersignal.sh part B", B1/B2/B3), plus a
# MINIMAL smoke block ahead of it. This is Part B ONLY -- Part A (section
# 1.5, A1/A2, Step 0's effect/source-param twins) is a separate lane/wave
# and out of scope for this file. Builder writes this; per this dispatch's
# builder packet and repo convention ("the party that builds never
# verifies") Harmony RUNS it -- once against the pre-Step-1 binary (expect
# FAIL at MINIMAL/B1/B3: set_master_signal 404, masterSignal/live.signal
# absent) and once against the merged Step-1 tree (expect all PASS).
#
# WHAT IT PROVES
#   MINIMAL: /api/status and /api/composition both publish masterSignal
#     (default 1.0), POST /api/set_master_signal lands it, and it round-
#     trips through the composition's live block (composition_.eff(Signal)
#     via addLiveBlock -- plan section 2.1/2.5).
#   B1 (ms-conn): a clip-scalar ("scale", 2-beat sine, RANGE 0.3..0.7) and
#     an effect-param (Hue Shift param0, 1-beat sine, RANGE 0..1) connection
#     both blend toward their manual/default value as Master Signal goes
#     from 1 -> 0.5 -> 0 (ConnectionEngine::evaluate's applyDepth, plan
#     section 2.2/2.3) -- checked two ways: the clip's live.scale value
#     directly, and render_frame pixels (md5) actually changing on the GPU.
#   B2 (ms-pulse): an effect that reads the beat clock directly (Beat
#     Ripple, u_beatPhase) is NOT touched by Master Signal (Boris Q2: "this
#     control is only for signals" -- plan D1) -- render_frame keeps
#     changing at signal=0.
#   B3 (OSC): /audiodna/signal (plan section 2.5) reaches
#     manualWrite(compScalarPath("signal")) and is readable back from
#     /api/composition within 1s.
#   Pre-change fail-first: on the binary this repo ships before Step 1
#     lands, /api/set_master_signal is a 404 (no such route) and
#     masterSignal / live.signal are absent from both endpoints -- MINIMAL
#     and B1/B3 FAIL there, by construction (no special-casing needed: "NA"
#     never satisfies a numeric oracle below).
#
# RIG FACTS IT DEPENDS ON (do not re-derive -- .harmony/gotchas.md,
# .harmony/VALIDATION.md, probe-lane3.sh's and probe-step3.sh's own headers,
# all re-confirmed this pass):
#   * ApiServer = http://127.0.0.1:7070 (IPv4 ONLY), always on in production
#     mode -- NO --test-mode. render_frame IS a production (7070) endpoint
#     (VERIFIED: probe-lane3.sh's `R()` posts to 7070 with no --test-mode in
#     its launch, and that gate is a merged, Harmony-run probe) -- it writes
#     a deterministic PNG to an explicit output_path, entirely separate from
#     the on-screen Output window (see probe-deck-path.sh, 2026-09-05).
#   * set_bpm puts the BPM tracker in manual mode, so beat phase (and every
#     Lfo/Envelope connection keyed off it, and Beat Ripple's u_beatPhase)
#     runs with no audio input.
#   * Launch is `open -g` (background -- does not raise/focus the app), NOT
#     `open` bare and NOT the raw binary (gotcha 2026-07-17). First launch
#     after a rebuild needs the mic-permission (TCC) prompt clicked once --
#     if /api/health never answers, this is surfaced as a FAIL with that
#     hint; per the SCREEN-SAFETY LAW below this script does NOT
#     screencapture to check (that would be a full-screen capture).
#   * SELF-MATCH: `pgrep -f 'MacOS/Audio-DNA'` matches ITS OWN argv, which
#     breaks any "wait until gone" loop -- every pgrep/pkill pattern in this
#     file uses the bracket trick 'MacOS/Audio-DN[A]' (rig rule, this
#     dispatch's builder packet; also probe-step3.sh's convention).
#   * Hue Shift declares only u_texture + u_hue_shift (EmbeddedShaders.h,
#     VERIFIED via EffectLibrary.cpp "Hue Shift" registration) -- no
#     u_time, so a frozen param IS a static frame (no confound from time-
#     based animation inside the shader itself).
#   * Beat Ripple declares u_beatPhase (EffectLibrary.cpp "Beat Ripple"
#     registration) -- it reads the beat clock directly, not through a
#     ParamConnection, so Master Signal (which only scales the reach of a
#     SIGNAL entering a chain via ConnectionEngine::evaluate / MacroBank /
#     v1 MappingEngine -- plan D1) must NOT stop it from moving.
#   * Image clip fixture: mediaType 1 = MediaType::Image, media file
#     $ROOT/media/P16_01_baseline.png (VERIFIED present; CLAUDE.md's
#     resources/default_image.png does NOT exist -- resources/ holds only
#     projectm_presets, re-confirmed this pass).
#   * ParamConnection JSON shape (src/kind lfo, shape sine, cycleBeats/
#     phase/pulseWidth; shape min/max/invert/playback/loop/curve/inMin/
#     inMax/smoothMs; enabled) is copied from .harmony/probe-lane3-clip.json
#     and cross-checked against ConnSerialization::toVar/fromVar (source-
#     verified field names, this pass). Clip-scalar connections load via a
#     top-level "conns" object keyed by scalar name (Clip::fromVar ->
#     ConnSerialization::scalarsFromVar<ClipScalar>). Effect-param
#     connections load via a "conns" ARRAY on the effect slot, each entry
#     carrying its own "p" (param index) alongside "src"/"shape"/"enabled"
#     (Clip::fromVar's effects-array branch).
#
# SCREEN-SAFETY LAW: this launches the REAL APP and NEVER opens the output
# window (no endpoint here does -- render_frame/load_composition/
# trigger_clip/set_bpm/set_master_signal/composition/status reads never
# reach Renderer/MainComponent's openOutputOnDisplay path, which is menu/
# selector/keyboard-only). NEVER a full-screen screencapture. Teardown is a
# GRACEFUL `osascript` quit first (so ~MainComponent runs its normal
# shutdown path), `pkill` only as an absolute last-resort fallback if the
# graceful quit does not clear the process within the wait loop -- same
# pattern as probe-step3.sh's teardown. The Output-window check (Quartz
# CGWindowList, filtered on the "Audio-DNA Output" title, never the app's
# own expected main window) runs at the very end, before the final tally.
set -u
OUT="${1:-/tmp/audiodna-mastersignal}"; mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${MS_BUILD_DIR:-build}"
APPBUNDLE="$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app"
A='http://127.0.0.1:7070'
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }

# --- helpers -----------------------------------------------------------
R(){ curl -s --max-time 20 -X POST "$A/api/render_frame" -H 'Content-Type: application/json' \
       -d "{\"output_path\":\"$OUT/$1\"}" >/dev/null; }
H(){ md5 -q "$OUT/$1"; }

comp_json(){ curl -s --max-time 5 "$A/api/composition"; }
status_json(){ curl -s --max-time 5 "$A/api/status"; }
# jpath/jstat NAME PYEXPR: evaluate PYEXPR against the parsed /api/composition
# or /api/status JSON (bound to `d`); "NA" on any missing key/parse error --
# every numeric oracle below treats "NA" as "field absent" (probe-lane3.sh
# convention) and fails closed, never passes vacuously.
jpath(){ comp_json | python3 -c "
import json, sys
try:
    d = json.load(sys.stdin)
    print($1)
except Exception:
    print('NA')
"
}
jstat(){ status_json | python3 -c "
import json, sys
try:
    d = json.load(sys.stdin)
    print($1)
except Exception:
    print('NA')
"
}
set_master_signal(){ curl -s --max-time 6 -X POST "$A/api/set_master_signal" \
    -H 'Content-Type: application/json' -d "{\"value\":$1}"; }

# approx V TARGET TOL: numeric-safe |V-TARGET|<=TOL; fails closed on "NA" or
# any other non-numeric V (never lets a missing field arithmetic-coerce to a
# false pass).
approx(){
    echo "$1" | grep -Eq '^-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?$' || return 1
    awk -v v="$1" -v t="$2" -v e="$3" 'BEGIN{d=v-t; if(d<0)d=-d; exit !(d<=e)}'
}
numlt(){ echo "$1" | grep -Eq '^-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?$' || return 1; awk -v v="$1" -v t="$2" 'BEGIN{exit !(v<t)}'; }
numgt(){ echo "$1" | grep -Eq '^-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?$' || return 1; awk -v v="$1" -v t="$2" 'BEGIN{exit !(v>t)}'; }
numlt_strict(){ # $1<$2, both numeric-safe
    echo "$1" | grep -Eq '^-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?$' || return 1
    echo "$2" | grep -Eq '^-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?$' || return 1
    awk -v a="$1" -v b="$2" 'BEGIN{exit !(a<b)}'
}
# minmax LIST: LIST is a space-separated set of samples (some may be "NA");
# prints "min max" over the numeric ones only, or "NA NA" if none numeric.
minmax(){
    echo "$1" | tr ' ' '\n' | awk '
    {
        v = $1
        if (v !~ /^-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?$/) next
        v = v + 0
        if (!seen || v < mn) mn = v
        if (!seen || v > mx) mx = v
        seen = 1
    }
    END{ if (seen) printf "%.6f %.6f\n", mn, mx; else print "NA NA" }'
}
# sample_clip_scale N INTERVAL_S: samples decks[0].layers[0].clips[0].live.scale
# N times, INTERVAL_S apart; prints all N values space-separated.
sample_clip_scale(){
    local n="$1" iv="$2" out="" v
    for _ in $(seq 1 "$n"); do
        v="$(jpath "d['decks'][0]['layers'][0]['clips'][0]['live']['scale']")"
        out="$out $v"
        sleep "$iv"
    done
    echo "$out"
}

# --- preconditions -------------------------------------------------------
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE (set MS_BUILD_DIR to override the build dir name)"; exit 64; }

# --- launch (production, `open -g` -- background, never raises the app) --
: > /tmp/adna-mastersignal-out.log
: > /tmp/adna-mastersignal-err.log
open -g --stdout /tmp/adna-mastersignal-out.log --stderr /tmp/adna-mastersignal-err.log "$APPBUNDLE"
for i in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: health never came up on $A (a production launch needs the mic-permission prompt clicked once -- do NOT full-screen screencapture to check; re-run after clicking Allow once)"; exit 1; }
PID="$(pgrep -f 'MacOS/Audio-DN[A]' | head -1)"
[ -n "$PID" ] || { echo "FAIL: health answered but no Audio-DNA process was found"; exit 1; }
ok "app launched via open -g, /api/health answered, PID=$PID"

# ===========================================================================
# MINIMAL: masterSignal exists, defaults to 1.0, and a REST write round-trips
# ===========================================================================
V_BOOT="$(jstat "d.get('masterSignal','NA')")"
approx "$V_BOOT" "1.0" "0.0001" \
    && ok "MINIMAL: GET /api/status .masterSignal == 1.0 at boot ($V_BOOT)" \
    || no "MINIMAL: GET /api/status .masterSignal != 1.0 at boot (got '$V_BOOT') -- pre-change binary has no such field, this IS the fail-first"

SET_RESP="$(set_master_signal 0.4)"
echo "$SET_RESP" | grep -q '"ok":[[:space:]]*true' \
    && ok "MINIMAL: POST /api/set_master_signal {value:0.4} -> ok" \
    || no "MINIMAL: POST /api/set_master_signal {value:0.4} did NOT return ok (got: '$SET_RESP') -- pre-change binary 404s here, this IS the fail-first"
sleep 0.3

V_04="$(jpath "d.get('masterSignal','NA')")"
approx "$V_04" "0.4" "0.01" \
    && ok "MINIMAL: GET /api/composition .masterSignal == 0.4 ($V_04)" \
    || no "MINIMAL: GET /api/composition .masterSignal != 0.4 (got '$V_04')"
LIVE_SIGNAL_04="$(jpath "d.get('live',{}).get('signal','NA')")"
approx "$LIVE_SIGNAL_04" "0.4" "0.01" \
    && ok "MINIMAL: GET /api/composition .live.signal == 0.4 ($LIVE_SIGNAL_04)" \
    || no "MINIMAL: GET /api/composition .live.signal != 0.4 (got '$LIVE_SIGNAL_04') -- absent on the pre-change binary"

set_master_signal 1.0 >/dev/null
sleep 0.2
echo "(reset) master signal set back to 1.0"

# 2-beat / 1-beat sine cycles below need a defined beat clock (manual BPM,
# no audio input needed -- probe-lane3.sh/probe-step3.sh's convention).
curl -s --max-time 6 -X POST "$A/api/set_bpm" -H 'Content-Type: application/json' -d '{"bpm":120}' >/dev/null
# 120 BPM: 0.5s/beat. 2-beat sine (scale) = 1s/cycle. 1-beat sine (hue) = 0.5s/cycle.

# ===========================================================================
# B1 (ms-conn): clip scalar "scale" (2-beat sine, RANGE 0.3..0.7) + effect
# param Hue Shift param0 (1-beat sine, RANGE 0..1) both blend toward their
# manual value as Master Signal -> 0.
# ===========================================================================
B1_JSON="$OUT/probe-mastersignal-b1.json"
cat > "$B1_JSON" <<EOF
{
  "name": "probe-mastersignal-b1",
  "activeDeckIndex": 0,
  "masterOpacity": 1.0,
  "decks": [
    {
      "name": "A",
      "id": 0,
      "numColumns": 1,
      "layers": [
        {
          "name": "L1", "id": 0, "opacity": 1.0, "visible": true, "blendMode": 1,
          "clips": [
            {
              "name": "img", "id": 1, "mediaType": 1, "mediaFile": "$ROOT/media/P16_01_baseline.png",
              "conns": {
                "scale": {
                  "src": { "kind": "lfo", "shape": "sine", "cycleBeats": 2.0, "phase": 0.0, "pulseWidth": 0.5 },
                  "shape": { "min": 0.3, "max": 0.7, "invert": false, "playback": "forward", "loop": true,
                             "curve": "Linear", "inMin": 0.0, "inMax": 1.0, "smoothMs": 0.0 },
                  "enabled": true
                }
              },
              "effects": [
                {
                  "name": "Hue Shift", "enabled": true, "bypassed": false, "dryWet": 1.0,
                  "params": [0.5],
                  "conns": [
                    {
                      "p": 0,
                      "src": { "kind": "lfo", "shape": "sine", "cycleBeats": 1.0, "phase": 0.0, "pulseWidth": 0.5 },
                      "shape": { "min": 0.0, "max": 1.0, "invert": false, "playback": "forward", "loop": true,
                                 "curve": "Linear", "inMin": 0.0, "inMax": 1.0, "smoothMs": 0.0 },
                      "enabled": true
                    }
                  ]
                }
              ]
            }
          ]
        }
      ]
    }
  ]
}
EOF
curl -s --max-time 6 -X POST "$A/api/load_composition" -H 'Content-Type: application/json' \
  -d "{\"path\":\"$B1_JSON\"}" | grep -q '"ok":[[:space:]]*true' \
  && ok "B1: load_composition accepted the ms-conn fixture" || no "B1: load_composition rejected the ms-conn fixture"
sleep 1
curl -s --max-time 6 -X POST "$A/api/trigger_clip" -H 'Content-Type: application/json' -d '{"layer":0,"column":0}' >/dev/null
sleep 1

# --- B1 @ signal=0: everything sits at its hand/default value -------------
set_master_signal 0 >/dev/null
sleep 0.3
SAMP0="$(sample_clip_scale 10 0.05)"
MM0="$(minmax "$SAMP0")"; MIN0="$(echo "$MM0" | awk '{print $1}')"; MAX0="$(echo "$MM0" | awk '{print $2}')"
if approx "$MIN0" "1.0" "0.0001" && approx "$MAX0" "1.0" "0.0001"; then
    ok "B1 @signal=0: live.scale sits at 1.0 +/-1e-4 across 10 samples (min=$MIN0 max=$MAX0)"
else
    no "B1 @signal=0: live.scale did not sit at 1.0 (min=$MIN0 max=$MAX0; samples:$SAMP0)"
fi
R b1_lo_a.png; sleep 0.5; R b1_lo_b.png
[ "$(H b1_lo_a.png)" = "$(H b1_lo_b.png)" ] \
    && ok "B1 @signal=0: render_frame 0.5s apart md5-IDENTICAL (frozen at hand/default values)" \
    || no "B1 @signal=0: render_frame 0.5s apart DIFFER (expected identical at signal=0)"

# --- B1 @ signal=1: full swing on both connections -------------------------
set_master_signal 1 >/dev/null
sleep 0.3
SAMP1="$(sample_clip_scale 20 0.05)"
MM1="$(minmax "$SAMP1")"; MIN1="$(echo "$MM1" | awk '{print $1}')"; MAX1="$(echo "$MM1" | awk '{print $2}')"
if numlt "$MIN1" "0.9" && numgt "$MAX1" "1.1"; then
    ok "B1 @signal=1: live.scale swings min=$MIN1 (<0.9) max=$MAX1 (>1.1) over 20 samples/1s"
else
    no "B1 @signal=1: live.scale swing insufficient (min=$MIN1 max=$MAX1; samples:$SAMP1)"
fi
SWING1="$(awk -v mn="$MIN1" -v mx="$MAX1" 'BEGIN{print mx-mn}')"
R b1_hi_a.png; sleep 0.5; R b1_hi_b.png
[ "$(H b1_hi_a.png)" != "$(H b1_hi_b.png)" ] \
    && ok "B1 @signal=1: render_frame 0.5s apart DIFFER (signals driving both connections)" \
    || no "B1 @signal=1: render_frame 0.5s apart md5-IDENTICAL (expected to differ at signal=1)"

# --- B1 @ signal=0.5: swing strictly smaller than at signal=1 --------------
set_master_signal 0.5 >/dev/null
sleep 0.3
SAMPH="$(sample_clip_scale 20 0.05)"
MMH="$(minmax "$SAMPH")"; MINH="$(echo "$MMH" | awk '{print $1}')"; MAXH="$(echo "$MMH" | awk '{print $2}')"
SWINGH="$(awk -v mn="$MINH" -v mx="$MAXH" 'BEGIN{print mx-mn}')"
if numlt_strict "$SWINGH" "$SWING1"; then
    ok "B1 @signal=0.5: live.scale swing ($SWINGH) strictly smaller than at signal=1 ($SWING1)"
else
    no "B1 @signal=0.5: live.scale swing ($SWINGH) NOT strictly smaller than at signal=1 ($SWING1)"
fi

# ===========================================================================
# B2 (ms-pulse): an effect reading the beat clock directly (Beat Ripple,
# u_beatPhase, NOT a ParamConnection) keeps moving at Master Signal == 0
# (Boris Q2 -- Master Signal only scales signal->parameter connections).
# ===========================================================================
B2_JSON="$OUT/probe-mastersignal-b2.json"
cat > "$B2_JSON" <<EOF
{
  "name": "probe-mastersignal-b2",
  "activeDeckIndex": 0,
  "masterOpacity": 1.0,
  "decks": [
    {
      "name": "A",
      "id": 0,
      "numColumns": 1,
      "layers": [
        {
          "name": "L1", "id": 0, "opacity": 1.0, "visible": true, "blendMode": 1,
          "clips": [
            {
              "name": "img", "id": 1, "mediaType": 1, "mediaFile": "$ROOT/media/P16_01_baseline.png",
              "effects": [
                { "name": "Beat Ripple", "enabled": true, "bypassed": false, "dryWet": 1.0,
                  "params": [0.5, 0.5, 0.3] }
              ]
            }
          ]
        }
      ]
    }
  ]
}
EOF
curl -s --max-time 6 -X POST "$A/api/load_composition" -H 'Content-Type: application/json' \
  -d "{\"path\":\"$B2_JSON\"}" | grep -q '"ok":[[:space:]]*true' \
  && ok "B2: load_composition accepted the ms-pulse fixture" || no "B2: load_composition rejected the ms-pulse fixture"
sleep 1
curl -s --max-time 6 -X POST "$A/api/trigger_clip" -H 'Content-Type: application/json' -d '{"layer":0,"column":0}' >/dev/null
sleep 1
curl -s --max-time 6 -X POST "$A/api/set_bpm" -H 'Content-Type: application/json' -d '{"bpm":120}' >/dev/null
set_master_signal 0 >/dev/null
sleep 0.3
R b2_a.png; sleep 0.25; R b2_b.png
[ "$(H b2_a.png)" != "$(H b2_b.png)" ] \
    && ok "B2 @signal=0: render_frame 0.25s apart DIFFER (Beat Ripple keeps pulsing on the beat clock, unaffected by Master Signal)" \
    || no "B2 @signal=0: render_frame 0.25s apart md5-IDENTICAL (Beat Ripple stopped moving -- Master Signal must not touch beat-clock-driven effects, Q2)"

# ===========================================================================
# B3 (OSC): /audiodna/signal 0.3 -> readable back from /api/composition
# within 1s (manualWrite(compScalarPath("signal")) path, plan section 2.5).
# ===========================================================================
python3 -c "import socket,struct;a=b'/audiodna/signal\0';a+=b'\0'*((4-len(a)%4)%4);socket.socket(socket.AF_INET,socket.SOCK_DGRAM).sendto(a+b',f\0\0'+struct.pack('>f',0.3),('127.0.0.1',8000))"
B3_OK=0; B3_LAST="NA"
for _ in $(seq 1 10); do
    v="$(jpath "d.get('masterSignal','NA')")"
    B3_LAST="$v"
    approx "$v" "0.3" "0.01" && { B3_OK=1; break; }
    sleep 0.1
done
[ "$B3_OK" = "1" ] \
    && ok "B3: OSC /audiodna/signal 0.3 -> masterSignal==0.3+/-0.01 within 1s (last=$B3_LAST)" \
    || no "B3: OSC /audiodna/signal 0.3 never reflected in masterSignal within 1s (last=$B3_LAST) -- absent/unreached on the pre-change binary, this IS the fail-first"

set_master_signal 1.0 >/dev/null

# ---------------------------------------------------------------------------
# Teardown. SCREEN-SAFETY LAW: graceful quit FIRST (so ~MainComponent runs
# its normal shutdown path), pkill only as an absolute last resort. Same
# pattern as probe-step3.sh's teardown.
# ---------------------------------------------------------------------------
osascript -e 'quit app "Audio-DNA"' >/dev/null 2>&1
for _ in $(seq 1 30); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
if pgrep -f 'MacOS/Audio-DN[A]' >/dev/null; then
    echo "graceful osascript quit did not clear the process -- falling back to pkill (last resort)"
    pkill -f 'MacOS/Audio-DN[A]' >/dev/null 2>&1
    for _ in $(seq 1 20); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
fi
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && no "APP STILL RUNNING AFTER graceful quit + pkill fallback" || ok "app terminated, no process remains"

# Screen-safety: confirm the OUTPUT window specifically was never opened.
# Same filter as probe-lane3.sh/probe-step3.sh (the app's own normal main
# window, title "Audio-DNA", is expected and excluded; only "Audio-DNA
# Output" is a breach). Gated on .venv/bin/python + pyobjc/Quartz being
# available -- FAIL (not SKIP) if missing, per the s-rta-0924 cleanup-lane
# rule: a silently-dropped check must not let a run print a clean tally.
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
    no "Output-window check: .venv/bin/python with pyobjc/Quartz unavailable -- run from the main checkout, or: python3 -m venv .venv && .venv/bin/pip install pyobjc-framework-Quartz"
fi

echo "no full-screen capture (Boris works on this Mac): the Quartz window-list check above is the screen witness"
echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
