#!/bin/bash
# probe-deck-path.sh — the DECK-PATH pixel probe. s167.
#
# WHY THIS EXISTS
# For a full session, "do global effects actually composite?" could not be answered, because
# every probe ran the STANDALONE-SOURCE path (POST /api/load_source), where applyGlobalEffects
# is gated off. There was no way to get a deck live without a human clicking. s167 added
# POST /api/load_composition, which closes that hole -- this script is what it unlocked.
#
# WHAT IT PROVES (all verified green on 2026-09-05, HEAD f56f4e9)
#   1. render_frame is a real, DETERMINISTIC, REPEATABLE pixel oracle.
#   2. Global effects DO composite on the deck path: Invert changes the frame, and removing it
#      restores the baseline BYTE-FOR-BYTE (no residue).
#   3. Layer opacity is WIRED (0.0 -> a 4.6 KB black frame; 1.0 -> exact baseline restore).
#   4. masterOpacity and clipOpacity are RENDER-DEAD: both accept the value, echo it back
#      happily, and change not one pixel.
#
# SCREEN-SAFETY LAW: this launches the REAL APP. It refuses if one is already running, quits
# gracefully via AppleScript (never pkill), and verifies 0 windows in the FULL CGWindowList.
# Never leave this script's app running.
#
# RIG FACTS IT DEPENDS ON (do not re-derive, they cost three probe runs to learn):
#   * TestServer  = http://[::1]:8080     (IPv6 ONLY, needs --test-mode)
#   * ApiServer   = http://127.0.0.1:7070 (IPv4 ONLY)
#   * render_frame lives on 7070 and REQUIRES {"output_path": "..."} -- it writes a PNG.
#   * set_clip_opacity's field is "clipOpacity", NOT "opacity".
#   * Probe with Invert / Vignette / Thermal. A warp effect can be invisible on a uniform source.
set -u
OUT="${1:-/tmp/audiodna-probe}"; mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APP="$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA"
T='http://[::1]:8080'; A='http://127.0.0.1:7070'
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
R(){ curl -s --max-time 20 -X POST "$A/api/render_frame" -H 'Content-Type: application/json' \
       -d "{\"output_path\":\"$OUT/$1\"}" >/dev/null; }
H(){ md5 -q "$OUT/$1"; }

pgrep -f 'MacOS/Audio-DNA' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -x "$APP" ] || { echo "REFUSE: no built app at $APP"; exit 64; }

"$APP" --test-mode > "$OUT/app.log" 2>&1 &
PID=$!
for i in $(seq 1 40); do [ -n "$(curl -s --max-time 2 "$T/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$T/api/health" 2>/dev/null)" ] || { echo "FAIL: health never came up"; kill $PID 2>/dev/null; exit 1; }

curl -s --max-time 6 -X POST "$A/api/load_composition" -H 'Content-Type: application/json' \
  -d "{\"path\":\"$ROOT/.harmony/probe-composition.json\"}" | grep -q '"ok":[[:space:]]*true' \
  && ok "load_composition accepted the probe fixture" || no "load_composition rejected the fixture"
sleep 1
curl -s --max-time 6 -X POST "$A/api/trigger_clip" -H 'Content-Type: application/json' -d '{"layer":0,"column":0}' >/dev/null
sleep 1
curl -s --max-time 5 "$A/api/composition" | grep -q '"activeClipColumn": 0' \
  && ok "deck is live: a clip is triggered and active" || no "no active clip -- the deck path is NOT engaged, everything below is void"

R b1.png; sleep 1; R b2.png
[ "$(H b1.png)" = "$(H b2.png)" ] && ok "render_frame is deterministic (two identical captures)" || no "render_frame is NOT deterministic -- fix the oracle before trusting anything"
BASE=$(H b1.png)

curl -s --max-time 6 -X POST "$T/api/add_global_effect" -H 'Content-Type: application/json' -d '{"name":"Invert"}' >/dev/null
sleep 1; R inv.png
[ "$(H inv.png)" != "$BASE" ] && ok "GLOBAL EFFECTS COMPOSITE ON THE DECK PATH (Invert changed the frame)" || no "Invert changed nothing -- global effects do not reach the deck composite"
curl -s --max-time 6 -X POST "$T/api/remove_global_effect" -H 'Content-Type: application/json' -d '{"index":0}' >/dev/null
sleep 1; R rem.png
[ "$(H rem.png)" = "$BASE" ] && ok "removing the effect restores the baseline byte-for-byte" || no "removal left residue -- the frame did not return to baseline"

curl -s --max-time 6 -X POST "$A/api/set_layer_opacity" -H 'Content-Type: application/json' -d '{"layer":0,"opacity":0.0}' >/dev/null
sleep 1; R lo0.png
[ "$(H lo0.png)" != "$BASE" ] && ok "layer opacity is WIRED (0.0 changed the frame)" || no "layer opacity did nothing -- a REGRESSION, it worked on 2026-09-05"
curl -s --max-time 6 -X POST "$A/api/set_layer_opacity" -H 'Content-Type: application/json' -d '{"layer":0,"opacity":1.0}' >/dev/null
sleep 1; R lo1.png
[ "$(H lo1.png)" = "$BASE" ] && ok "layer opacity restores exactly" || no "layer opacity 1.0 did not restore the baseline"

# These two are EXPECTED-DEAD until the renderer lane lands. When they start FAILING here,
# that is the feature working -- flip the assertions then.
curl -s --max-time 6 -X POST "$T/api/set_composition_params" -H 'Content-Type: application/json' -d '{"masterOpacity":0.0}' >/dev/null
sleep 1; R mo0.png
[ "$(H mo0.png)" = "$BASE" ] && ok "masterOpacity still render-dead (expected today)" || echo "NOTE  masterOpacity now CHANGES the frame -- the renderer lane has landed; update this probe"
curl -s --max-time 6 -X POST "$T/api/set_composition_params" -H 'Content-Type: application/json' -d '{"masterOpacity":1.0}' >/dev/null
curl -s --max-time 6 -X POST "$T/api/set_clip_opacity" -H 'Content-Type: application/json' -d '{"layer":0,"column":0,"clipOpacity":0.0}' >/dev/null
sleep 1; R co0.png
[ "$(H co0.png)" = "$BASE" ] && ok "clipOpacity still render-dead (expected today)" || echo "NOTE  clipOpacity now CHANGES the frame -- the renderer lane has landed; update this probe"

osascript -e "tell application \"System Events\" to tell (first process whose unix id is $PID) to quit" >/dev/null 2>&1
sleep 2; kill -0 "$PID" 2>/dev/null && osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 20); do kill -0 "$PID" 2>/dev/null || break; sleep 1; done
pgrep -f 'MacOS/Audio-DNA' >/dev/null && no "APP STILL RUNNING AFTER QUIT -- screen-safety breach, deal with it now" || ok "app quit gracefully, no process remains"
W=$("$ROOT/.venv/bin/python" -c "
import Quartz
wl=Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName',''))]))" 2>/dev/null)
[ "$W" = "0" ] && ok "0 Audio-DNA windows in the FULL window list" || no "$W Audio-DNA window(s) STILL ON SCREEN"
echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
