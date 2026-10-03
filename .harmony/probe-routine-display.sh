#!/bin/bash
# probe-routine-display.sh -- s-rta-0927 routine display, slice A (.harmony/.reports/s-rta-0927/
# plan-routine-display-A.md section 5.7). Live witness + window-only screenshots for the deck's ROUTINES row, the
# routine bands on the layer strips, the V fader following the model, and the status keys the display is built on.
#
# usage: probe-routine-display.sh OUT_DIR [--hook]
#   ROUTINE_DISPLAY_APP  app bundle (default: <root>/${ROUTINE_DISPLAY_BUILD_DIR:-build}/AudioDNA_artefacts/Release/Audio-DNA.app)
#   ROUTINE_DISPLAY_PY   python with pyobjc Quartz + PIL + numpy (default: <root>/.venv, else the main checkout's .venv)
#
# Fixture .harmony/probe-routine-display.json (@ROOT@ substituted into a copy under OUT_DIR): deck "A" (active; layers
# L1/L2/L3, a test_card.png clip on L1 and L3 column 1) and deck "B" (one layer), plus three hand-written routines --
# no take is recorded: pad 1 "Drop" (32 beats, once; restores L1 opacity 0.3 and L3 opacity 0.8, then one recorded move
# on L1 opacity 0.3 -> 0.9 over routine beats 8..28), pad 2 "Build" (16 beats, loop; restores L1 opacity 0.6), pad 3
# "Wash" (4 beats; restores a layer "L6" that does not exist -> the red "!"). POST /api/set_bpm 120 puts the tracker in
# manual LOCKED mode (a bar = 2 s).
#
# Phase 1 (always, production binary, no env): permanent rows
#   d0 load_composition ok; bank[0] is "Drop"
#   d1 bank[0] carries deck/layers/fireSeq/startsOn/restartPending          (RED pre-change: absent)
#   d2 fire pad 1 (quantize 4bar) -> within 0.5 s pending, deck 0, layers [0,2], startsOn "4bar"
#   d13 (fix round) set pad 1's quantize to "bar" WHILE it waits -> within 0.5 s its startsOn reads "bar" (the edit
#      reaches the pending start; RED: the waiting routine kept the "4bar" it was pressed with)
#   d3 on the edge: running, layers [0,2], position advancing; L1 opacity 0.3 / L3 0.8 (the restore)
#   d14 (fix round) press pad 1 again while it plays -> restartPending and startsOn "bar" (the grid the restart lands
#      on, the pad's restart mark + tooltip; RED: startsOn "" while running); shot 13-restart-pending
#   d15 (fix round 2) set pad 1's quantize to "4bar" WHILE its restart waits -> within 0.5 s startsOn reads "4bar"
#      (the edit reaches the pending restart; RED: the restart kept the "bar" it was pressed with). Re-fired first
#      if the d14 restart already landed during the shot; one retry when the restart lands between the press and
#      the edit (inconclusive, never a pass). Quantize goes back to "bar" before d4.
#   d4 fire pad 2 -> bank[1].fireSeq > bank[0].fireSeq
#   d5 switch_deck 1 -> bank[0].deck == 0 and its position keeps growing (a deck switch never stops a routine)
#   d6 switch_deck 0; stop pad 1 -> idle, layers []
#   d8 fire pad 3 -> preambleUnresolved == 1 while running AND 3 s later while idle (RED: an idle pad reported 0)
#   d9 window shots 08-empty, 01-idle, 02-waiting, 03-playing, 04-two-on-one, 05-offdeck, 06-removed, 09-warning,
#      12-fader-follow, 13-restart-pending -- non-blank; decoded: 02-waiting has more teal (#4a9a8a) pixels than
#      01-idle (the pad frame), 03-playing has more routine-cue pixels (kRoutineCue #b4ff2e, chartreuse, decoded by
#      hue 68..100 deg) than 01-idle (band names + the V fill) (RED: no row, no bands; fix-round RED: the cue was
#      cyan) and d12 (fix round) NO more accent-cyan (#00e5ff) pixels than 01-idle (+100 at most: the routine cue is
#      never the app's accent cyan; RED: +300 and more)
#   d11 the app quits, 0 Audio-DNA windows in the FULL window list
# Phase 2 (--hook): ONLY on a build carrying the TEMPORARY routine-display screenshot hook (MainComponent constructor
#   end, NEVER committed): AUDIODNA_DEBUG_SHOW=<state> fires at AUDIODNA_DEBUG_AT ms after the constructor; the probe
#   loads the fixture and fires routines over REST before that.
#   d7  07-layer-x   (layer_x:0 -> deckView_->onLayerClearClip(0) with Drop + Build playing): both idle, L1's clip cleared
#   d10 10-pad-menu  (pad_menu:1 -> showRoutinePadMenu(1) + an in-app createComponentSnapshot of the top-level window,
#                     AUDIODNA_DEBUG_SNAP; a PopupMenu is dismissed within ~50 ms in a background app), 11-record-tab
#                     (AUDIODNA_DEBUG_TAB=record), 03-playing-inspector (AUDIODNA_DEBUG_LAYER=0: the Layer tab shows L1
#                     while Drop's recorded move holds its opacity -- the C7 cyan digits + ROUTINE cue)
#
# Screen-safe: open -g only (never plain open, never the Output window), window-only captures by Quartz window id
# (never full-screen, never fixed-coordinate crops), no synthetic input, graceful quit (pkill only after 30 s).
# PROBE RIG GATE: refuses (exit 64) unless /tmp/audiodna-live.lock/owner exists; if AUDIODNA_LOCK_OWNER is set it must
# match the owner file's first field. adna_* match the kernel ucomm "Audio-DNA" exactly.
set -u
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held -- mkdir /tmp/audiodna-live.lock && echo \"<lane> \$\$ \$(date +%s)\" > $LOCK_OWNER_FILE first"; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
fi
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }

ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
. "$ROOT/.harmony/probe-quit-ours.sh"   # refuse_foreign_start / record_ourpid / quit_ours: quit ONLY the app this run launched
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${ROUTINE_DISPLAY_APP:-$ROOT/${ROUTINE_DISPLAY_BUILD_DIR:-build}/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${ROUTINE_DISPLAY_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
OUT="${1:-}"; [ -n "$OUT" ] || { echo "usage: $0 OUT_DIR [--hook]"; exit 64; }
HOOK=0; [ "${2:-}" = "--hook" ] && HOOK=1
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set ROUTINE_DISPLAY_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import Quartz, json, numpy, PIL' 2>/dev/null || { echo "REFUSE: no python with pyobjc Quartz + PIL + numpy (set ROUTINE_DISPLAY_PY)"; exit 64; }
refuse_foreign_start || exit 64
lsof -nP -iTCP:7070 -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port 7070 already has a listener"; exit 64; }
mkdir -p "$OUT" || exit 64
OUT="$(cd "$OUT" && pwd)"   # absolute: the app's cwd is not ours (AUDIODNA_DEBUG_SNAP)
FIX="$OUT/probe-routine-display.json"
sed "s|@ROOT@|$ROOT|g" "$ROOT/.harmony/probe-routine-display.json" > "$FIX" || exit 64
echo "app: $APP"; echo "out: $OUT"

PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }

shot() {   # shot NAME -> $OUT/NAME-w<k>.png, one per on-screen Audio-DNA window (w0 = the largest = the main window)
  "$PY" - "$OUT" "$1" <<'PYEOF'
import sys, subprocess, Quartz
out, name = sys.argv[1], sys.argv[2]
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
wins = []
for w in wl:
    if str(w.get('kCGWindowOwnerName', '')) != 'Audio-DNA' or int(w.get('kCGWindowLayer', 0)) >= 20:
        continue
    b = w.get('kCGWindowBounds', {})
    if float(b.get('Width', 0)) < 50 or float(b.get('Height', 0)) < 50:
        continue
    wins.append((float(b['Width']) * float(b['Height']), int(w['kCGWindowNumber'])))
wins.sort(reverse=True)
for k, (_, wid) in enumerate(wins):
    path = "%s/%s-w%d.png" % (out, name, k)
    subprocess.run(["screencapture", "-x", "-o", "-l", str(wid), path], check=False)
    print("  shot %s (window %d)" % (path, wid))
if not wins:
    print("  shot %s: NO Audio-DNA window on screen" % name)
PYEOF
}
# px NAME -> "<nonblank 0/1> <teal count> <cyan count> <routine-cue count>" decoded from $OUT/NAME-w0.png (counts: the deck region)
px() {
  "$PY" - "$OUT/$1-w0.png" <<'PYEOF'
import sys
import numpy as np
from PIL import Image
try:
    a = np.asarray(Image.open(sys.argv[1]).convert('RGB')).astype(int)
except Exception:
    print("0 0 0 0"); sys.exit(0)
nonblank = int(a.std() > 4.0)
# the deck (ROUTINES row, strips, grid) sits in the window's left half, 15-50 % down, under the TopBar and the
# SignalBar -- whose meters are cyan and move with the room's audio, so they are cut away (window-relative, never a
# screen coordinate)
h, w = a.shape[:2]
a = a[int(h * 0.15):int(h * 0.50), :int(w * 0.5)]
def near(rgb, tol):
    return int((np.abs(a - np.array(rgb)).max(axis=2) <= tol).sum())
# window captures are colour-managed (the display profile): #4a9a8a lands as ~(95,152,138) and #00e5ff as
# ~(104,226,251) on this rig -- accept the raw hex and its measured rendering
teal = near((0x4a, 0x9a, 0x8a), 10) + near((95, 152, 138), 10)
cyan = int(((a[:, :, 0] < 130) & (a[:, :, 1] > 195) & (a[:, :, 2] > 225)).sum())
# the routine cue kRoutineCue #b4ff2e (fix round): chartreuse, decoded by hue so the display profile's shift does not
# matter -- hue 68..100 deg, saturation > 0.5, value > 0.6 (no other UI colour sits in 60..120 deg)
mx = a.max(axis=2).astype(float); mn = a.min(axis=2).astype(float); d = np.maximum(mx - mn, 1.0)
r, g, b = a[:, :, 0], a[:, :, 1], a[:, :, 2]
hue = np.where(mx == g, 60.0 * ((b - r) / d) + 120.0, np.where(mx == r, (60.0 * ((g - b) / d)) % 360.0, 60.0 * ((r - g) / d) + 240.0))
cue = int(((mx == g) & (hue >= 68.0) & (hue <= 100.0) & ((mx - mn) / np.maximum(mx, 1.0) > 0.5) & (mx > 153)).sum())
print(nonblank, teal, cyan, cue)
PYEOF
}
st() {   # st EXPR -> python expression over d = /api/routine/status
  curl -s --max-time 5 "$A/api/routine/status" | "$PY" -c "import sys, json; d = json.load(sys.stdin); b = d['bank']; print($1)" 2>/dev/null
}
cj() {   # cj EXPR -> python expression over d = /api/composition
  curl -s --max-time 5 "$A/api/composition" | "$PY" -c "import sys, json; d = json.load(sys.stdin); print($1)" 2>/dev/null
}
post() { curl -s --max-time 6 -X POST "$A$1" -H 'Content-Type: application/json' -d "$2"; }
wait_health() { local _; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && return 0; sleep 1; done; return 1; }
now_ms() { "$PY" -c 'import time; print(int(time.time() * 1000))'; }
wait_for() {   # wait_for SECONDS EXPR(status) -> 0 when EXPR prints True within SECONDS (fractions allowed)
  local end=$(( $(now_ms) + $("$PY" -c "print(int($1 * 1000))") ))
  while :; do [ "$(st "$2")" = "True" ] && return 0; [ "$(now_ms)" -ge "$end" ] && return 1; sleep 0.05; done
}
launch() {   # launch [--env K=V ...]
  : > "$OUT/out.log"; : > "$OUT/err.log"   # open --stdout/--stderr APPEND: truncate first
  open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" "$@" "$APP"
  record_ourpid; echo "ours: pid ${OURPID:-none}"
}
quit_app() {
  quit_ours   # ONLY the pid launch() recorded (probe-quit-ours.sh)
}
zero_windows() {
  local W
  W="$("$PY" -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if str(w.get('kCGWindowOwnerName', '')) == 'Audio-DNA']))" 2>/dev/null)"
  if ! adna_running && [ "$W" = "0" ]; then ok "$1: app quit, 0 Audio-DNA windows in the FULL window list"
  else no "$1: app running=$(adna_running && echo yes || echo no), windows=$W"; fi
}
setup_show() {   # load the fixture, 120 BPM, light L1 and L3
  local R
  R="$(post /api/load_composition "{\"path\":\"$FIX\"}")"
  echo "$R" | grep -q '"ok":[[:space:]]*true' || { echo "  load_composition: $R"; return 1; }
  sleep 2
  post /api/set_bpm '{"bpm":120}' >/dev/null
  post /api/trigger_clip '{"layer":0,"column":0}' >/dev/null
  post /api/trigger_clip '{"layer":2,"column":0}' >/dev/null
  sleep 1
  return 0
}

# ---------------------------------------------------------------- Phase 1 ------------------------------------------
echo "== Phase 1 (production, no env)"
launch
if ! wait_health; then no "d0 app never answered /api/health"; quit_app; echo; echo "$PASS PASS / $FAIL FAIL"; exit 1; fi
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
[ "$L7070" = "Audio-DNA" ] || no "port 7070 is answered by '$L7070', not Audio-DNA"
sleep 3; shot 08-empty
if setup_show && [ "$(st "b[0]['name']")" = "Drop" ]; then ok "d0 load_composition ok; pad 1 is Drop ($(st "','.join(x['name'] for x in b[:3])"))"
else no "d0 fixture loaded with Drop on pad 1 (got '$(st "b[0]['name']")')"; fi
shot 01-idle

HAS="$(st "all(k in b[0] for k in ('deck','layers','fireSeq','startsOn','restartPending','touchesComp'))")"
[ "$HAS" = "True" ] && ok "d1 bank[0] carries deck/layers/fireSeq/startsOn/restartPending/touchesComp" \
  || no "d1 bank[0] carries deck/layers/fireSeq/startsOn/restartPending/touchesComp (got: ${HAS:-nothing})"

post /api/routine/set '{"slot":0,"quantize":"4bar"}' >/dev/null
T0=$(date +%s)
post /api/routine/fire '{"slot":0}' >/dev/null
if wait_for 0.5 "b[0]['state'] == 'pending'"; then
  G="$(st "(b[0].get('deck'), b[0].get('layers'), b[0].get('startsOn'))")"
  [ "$G" = "(0, [0, 2], '4bar')" ] && ok "d2 fired pad 1: pending, deck 0, layers [0, 2], startsOn 4bar ($G)" || no "d2 pending witness: expected (0, [0, 2], '4bar'), got $G"
else no "d2 pad 1 pending within 0.5 s (state $(st "b[0]['state']"))"; fi
sleep 1
[ "$(st "b[0]['state']")" = "pending" ] && { shot 02-waiting; echo "  02-waiting shot while pending (the witness)"; } || no "d9 02-waiting: pad 1 no longer pending at the shot"
post /api/routine/set '{"slot":0,"quantize":"bar"}' >/dev/null
if wait_for 0.5 "b[0].get('startsOn') == 'bar'"; then ok "d13 quantize set to bar while waiting: startsOn now $(st "b[0].get('startsOn')") (state $(st "b[0]['state']"))"
else no "d13 quantize set to bar while waiting: startsOn still '$(st "b[0].get('startsOn')")' (state $(st "b[0]['state']"))"; fi
if wait_for 10 "b[0]['state'] == 'running'"; then
  P1="$(st "b[0]['position']")"; sleep 0.6; P2="$(st "b[0]['position']")"
  G="$(st "b[0].get('layers')")"
  ADV="$("$PY" -c "print($P2 > $P1)")"
  OP="$(cj "'%.2f %.2f' % (d['decks'][0]['layers'][0]['opacity'], d['decks'][0]['layers'][2]['opacity'])")"
  [ "$G" = "[0, 2]" ] && [ "$ADV" = "True" ] && ok "d3 on the edge: running, layers $G, position $P1 -> $P2" || no "d3 running witness: layers $G, position $P1 -> $P2"
  [ "$OP" = "0.30 0.80" ] && ok "d3 the restore landed: L1 / L3 opacity $OP" || no "d3 restore: L1 / L3 opacity expected 0.30 0.80, got $OP"
else no "d3 pad 1 running within 10 s (state $(st "b[0]['state']"))"; fi
# bar 5 of 8: position 16..20 beats (8..10 s after the start)
wait_for 12 "16.5 <= b[0]['position'] <= 19.0" && { shot 03-playing; echo "  03-playing at position $(st "round(b[0]['position'], 2)") (L1 opacity $(cj "round(d['decks'][0]['layers'][0]['opacity'], 3)"))"; } \
  || no "d9 03-playing: never saw position 16.5..19 (at $(st "b[0]['position']"))"

# fix round: press pad 1 again while it plays -- a restart waits for the next bar
post /api/routine/fire '{"slot":0}' >/dev/null
if wait_for 0.5 "b[0].get('restartPending') == True"; then
  G="$(st "(b[0].get('restartPending'), b[0].get('startsOn'), b[0]['state'])")"
  [ "$G" = "(True, 'bar', 'running')" ] && ok "d14 pad 1 pressed again while playing: (restartPending, startsOn, state) $G" \
    || no "d14 restart pending witness: expected (True, 'bar', 'running'), got $G"
  shot 13-restart-pending
else no "d14 pad 1 restartPending within 0.5 s of a second press (got $(st "b[0].get('restartPending')"))"; fi

# fix round 2: a Quantize edit made while that restart waits reaches it (the pad menu writes the same routine)
D15=""
for _try in 1 2; do
  post /api/routine/set '{"slot":0,"quantize":"bar"}' >/dev/null
  [ "$(st "b[0].get('restartPending')")" = "True" ] || { post /api/routine/fire '{"slot":0}' >/dev/null; wait_for 0.5 "b[0].get('restartPending') == True"; }
  post /api/routine/set '{"slot":0,"quantize":"4bar"}' >/dev/null
  if wait_for 0.5 "b[0].get('startsOn') == '4bar'"; then D15=pass; break; fi
  G="$(st "(b[0].get('restartPending'), b[0].get('startsOn'), b[0]['state'])")"
  [ "$G" = "(False, '', 'running')" ] && { echo "  d15 try $_try inconclusive: the restart landed before the edit $G"; continue; }
  D15="$G"; break
done
if [ "$D15" = "pass" ]; then ok "d15 quantize set to 4bar while the restart waits: (restartPending, startsOn) $(st "(b[0].get('restartPending'), b[0].get('startsOn'))")"
else no "d15 quantize set to 4bar while the restart waits: (restartPending, startsOn, state) ${D15:-inconclusive twice}"; fi
post /api/routine/set '{"slot":0,"quantize":"bar"}' >/dev/null

post /api/routine/fire '{"slot":1}' >/dev/null
if wait_for 5 "b[1]['state'] == 'running'"; then
  G="$(st "(b[1].get('fireSeq', 0), b[0].get('fireSeq', 0))")"
  [ "$(st "b[1].get('fireSeq', 0) > b[0].get('fireSeq', 0)")" = "True" ] && ok "d4 pad 2 fired later: fireSeq $G" || no "d4 bank[1].fireSeq > bank[0].fireSeq (got $G)"
  sleep 0.5; shot 04-two-on-one
else no "d4 pad 2 running within 5 s"; fi

post /api/switch_deck '{"deck":1}' >/dev/null
sleep 1
P1="$(st "b[0]['position']")"; sleep 1; P2="$(st "b[0]['position']")"
G="$(st "(b[0].get('deck'), b[0]['state'])")"
[ "$(st "b[0].get('deck')")" = "0" ] && [ "$("$PY" -c "print($P2 > $P1)")" = "True" ] \
  && ok "d5 on deck B: pad 1 still (deck, state) $G, position $P1 -> $P2" || no "d5 deck switch: (deck, state) $G, position $P1 -> $P2"
shot 05-offdeck

post /api/switch_deck '{"deck":0}' >/dev/null
sleep 1
post /api/routine/stop '{"slot":0}' >/dev/null
sleep 0.5
G="$(st "(b[0]['state'], b[0].get('layers'), b[1]['state'])")"
[ "$G" = "('idle', [], 'running')" ] && ok "d6 stop pad 1: idle, layers [] (Build still running) $G" || no "d6 stop pad 1: expected ('idle', [], 'running'), got $G"
shot 06-removed

post /api/routine/stop '{"slot":1}' >/dev/null
post /api/routine/fire '{"slot":2}' >/dev/null
if wait_for 4 "b[2]['state'] == 'running'"; then
  U1="$(st "b[2]['preambleUnresolved']")"
  wait_for 6 "b[2]['state'] == 'idle'"
  sleep 3
  U2="$(st "(b[2]['state'], b[2]['preambleUnresolved'])")"
  [ "$U1" = "1" ] && [ "$U2" = "('idle', 1)" ] && ok "d8 pad 3: preambleUnresolved 1 while running, $U2 3 s after its end" \
    || no "d8 pad 3 warning persists: running $U1, after the end $U2 (expected 1 and ('idle', 1))"
  shot 09-warning
else no "d8 pad 3 running within 4 s"; fi

post /api/set_layer_opacity '{"layer":1,"opacity":0.3}' >/dev/null
sleep 1
echo "  L2 opacity $(cj "round(d['decks'][0]['layers'][1]['opacity'], 3)") (REST set_layer_opacity 0.3)"
shot 12-fader-follow

for n in 08-empty 01-idle 02-waiting 03-playing 04-two-on-one 05-offdeck 06-removed 09-warning 12-fader-follow 13-restart-pending; do
  read -r NB TEAL CYAN CUE <<< "$(px "$n")"
  echo "  $n: nonblank=$NB teal=$TEAL cyan=$CYAN cue=$CUE"
  eval "TEAL_${n//-/_}=$TEAL; CYAN_${n//-/_}=$CYAN; CUE_${n//-/_}=${CUE:-0}"
  [ "$NB" = "1" ] || no "d9 $n non-blank"
done
[ "${TEAL_02_waiting:-0}" -gt $(( ${TEAL_01_idle:-0} + 150 )) ] && ok "d9 02-waiting has the waiting pad's teal frame (teal ${TEAL_01_idle:-0} -> ${TEAL_02_waiting:-0})" \
  || no "d9 02-waiting teal frame (teal ${TEAL_01_idle:-0} -> ${TEAL_02_waiting:-0}, need > +150)"
[ "${CUE_03_playing:-0}" -gt $(( ${CUE_01_idle:-0} + 300 )) ] && ok "d9 03-playing has the routine-cue band names / V fill (cue ${CUE_01_idle:-0} -> ${CUE_03_playing:-0})" \
  || no "d9 03-playing routine-cue bands / V fill (cue ${CUE_01_idle:-0} -> ${CUE_03_playing:-0}, need > +300)"
[ "${CYAN_03_playing:-0}" -le $(( ${CYAN_01_idle:-0} + 100 )) ] && ok "d12 03-playing adds no accent cyan: the routine cue is its own hue (cyan ${CYAN_01_idle:-0} -> ${CYAN_03_playing:-0})" \
  || no "d12 03-playing added accent cyan (cyan ${CYAN_01_idle:-0} -> ${CYAN_03_playing:-0}, allowed +100): the routine cue is the app's accent cyan"

quit_app
zero_windows "d11"
cp "$OUT/err.log" "$OUT/phase1-err.log" 2>/dev/null

# ---------------------------------------------------------------- Phase 2 ------------------------------------------
if [ "$HOOK" -eq 1 ]; then
  echo "== Phase 2 (--hook: TEMPORARY screenshot hook build)"
  trap 'adna_running && quit_app' EXIT
  # state|name|at ms|snapshot(0/1)|routines to fire before the hook (space-separated slots)
  for spec in "layer_x:0|07-layer-x|12000|0|0 1" \
              "pad_menu:1|10-pad-menu|12000|1|1" \
              "tab:record|11-record-tab|6000|0|" \
              "inspect:0|03-playing-inspector|12000|0|0"; do
    IFS='|' read -r STATE NAME AT USESNAP FIRE <<< "$spec"
    echo "-- $NAME ($STATE)"
    ENVS=(--env "AUDIODNA_DEBUG_AT=$AT")
    case "$STATE" in
      tab:record) ENVS+=(--env "AUDIODNA_DEBUG_TAB=record") ;;
      inspect:*)  ENVS+=(--env "AUDIODNA_DEBUG_LAYER=${STATE#inspect:}") ;;
      *)          ENVS+=(--env "AUDIODNA_DEBUG_SHOW=$STATE") ;;
    esac
    [ "$USESNAP" -eq 1 ] && { rm -f "$OUT/$NAME-snapshot.png"; ENVS+=(--env "AUDIODNA_DEBUG_SNAP=$OUT/$NAME-snapshot.png"); }
    T0=$(date +%s)
    launch "${ENVS[@]}"
    if ! wait_health; then no "$NAME: app never answered /api/health"; quit_app; continue; fi
    setup_show || no "$NAME: fixture not loaded"
    for s in $FIRE; do post /api/routine/fire "{\"slot\":$s}" >/dev/null; done
    if [ "$STATE" = "layer_x:0" ]; then
      wait_for 8 "b[0]['state'] == 'running' and b[1]['state'] == 'running'" && shot 07-layer-x-before
      G="$(st "(b[0]['state'], b[1]['state'])")"; echo "  before the hook: (Drop, Build) $G"
    fi
    if [ "${STATE%%:*}" = "inspect" ]; then
      # C7: the Layer tab shows L1 (hook); Drop's recorded move holds L1 opacity over routine beats 8..28
      wait_for 16 "16.5 <= b[0]['position'] <= 19.0" && { shot "$NAME"; echo "  $NAME at position $(st "round(b[0]['position'], 2)")"; } \
        || no "$NAME: never saw position 16.5..19 (at $(st "b[0]['position']"))"
      [ -s "$OUT/$NAME-w0.png" ] && ok "d10 $NAME shot" || no "d10 $NAME shot"
      quit_app; zero_windows "$NAME"; cp "$OUT/err.log" "$OUT/$NAME-err.log" 2>/dev/null
      continue
    fi
    while [ $(( $(date +%s) - T0 )) -lt $(( AT / 1000 + 3 )) ]; do sleep 1; done
    shot "$NAME"
    if [ "$STATE" = "layer_x:0" ]; then
      G="$(st "(b[0]['state'], b[1]['state'])")"; C="$(cj "d['decks'][0]['layers'][0]['activeClipColumn']")"
      [ "$G" = "('idle', 'idle')" ] && [ "$C" = "-1" ] && ok "d7 layer X on L1: Drop and Build (both touch L1) idle $G, L1 clip cleared ($C)" \
        || no "d7 layer X on L1: expected ('idle', 'idle') and -1, got $G and $C"
    fi
    if [ "$USESNAP" -eq 1 ]; then
      [ -s "$OUT/$NAME-snapshot.png" ] && ok "d10 $NAME: in-app snapshot written right after the menu opened" || no "d10 $NAME: no in-app snapshot"
    fi
    [ "$STATE" = "tab:record" ] && { [ -s "$OUT/$NAME-w0.png" ] && ok "d10 $NAME shot" || no "d10 $NAME shot"; }
    while [ $(( $(date +%s) - T0 )) -lt $(( AT / 1000 + 6 )) ]; do sleep 1; done
    quit_app
    zero_windows "$NAME"
    cp "$OUT/err.log" "$OUT/$NAME-err.log" 2>/dev/null
  done
  trap - EXIT
fi

echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
