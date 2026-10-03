#!/bin/bash
# probe-deck-tabs.sh -- s-rta-0926b plan6 (lane decks). Live witness + window-only screenshots for the deck tab row
# ("+" / right-click a tab), composition confirm, the library row menu, and F1 unique deck ids.
#
# usage: probe-deck-tabs.sh OUT_DIR [--hook]
#   DECKTABS_APP  app bundle to launch (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   DECKTABS_PY   python with pyobjc Quartz (default: <root>/.venv, else the main checkout's .venv)
#
# Phase 1 (always, production binary, no env): permanent rows
#   R1 POST /api/load_composition <fixture> ok        R2 GET /api/composition reports decks[*].id (RED pre-change: absent)
#   R3 the three deck ids are pairwise distinct        R4 activeDeck == 1 (the fixture's)
#   R5 POST /api/switch_deck {"deck":2} -> activeDeck == 2
#   R6 app quits, 0 Audio-DNA windows in the FULL window list
#   shots 01-default (before the load), 02-three-decks (after R1), 03-active-C (after R5)
# Phase 2 (--hook): ONLY on a build carrying the TEMPORARY plan6 screenshot hook (MainComponent constructor end,
#   NEVER committed -- .harmony/notebook.md plan6 entry): one launch per state with open -g --env AUDIODNA_DEBUG_SHOW=...
#   04-plus-menu, 05-deck-menu, 06-rename-dialog, 07-replace-confirm, 08-browser-compositions, 09-library-menu,
#   10-library-delete-confirm, 11-after-remove (s-rta-1003: was 11-remove-hint, the capture of the tab row's "Undo
#   Remove" button; the button is gone -- Boris: "I don't wanna see an under removed button at all. We just use
#   control Z." -- so the state keeps only its REST clause: deck A removed, decks B,C, B still active). A JUCE PopupMenu is dismissed within ~50 ms while the app is not the
#   foreground process (juce_PopupMenu.cpp checkButtonState -> doesAnyJuceCompHaveFocus), and this rig never brings
#   the app to the front, so the three MENU states (04/05/09) are shot by the hook itself: a createComponentSnapshot
#   of the live top-level window taken synchronously right after the menu opens (<NAME>-snapshot.png; the GL preview
#   is black in a component snapshot). Dialogs and the tab row are Quartz window captures (<NAME>-w<k>.png).
#   09/10 need a library row: the script puts ONE clearly-named temp file, _probe-deck-tabs.json, in the app's
#   compositions dir (~/Library/AudioDNA/compositions -- JUCE userApplicationDataDirectory is ~/Library on macOS)
#   and removes it (and the dir, only if this script created it) on exit. The hook dismisses every menu/dialog
#   with result 0 at T+14 s (cancelAllModalComponents), so nothing is loaded, renamed or deleted.
#
# Fixture .harmony/probe-deck-tabs.json = .harmony/probe-composition.json's one deck, three times, as "A"/"B"/"C",
# every deck "id": 0 (what a file saved by the pre-F1 New Deck carries), activeDeckIndex 1. Made by:
#   python3 - <<'PY'
#   import json, copy
#   src = json.load(open(".harmony/probe-composition.json")); out = copy.deepcopy(src); out["decks"] = []
#   for n in ("A", "B", "C"):
#       d = copy.deepcopy(src["decks"][0]); d["name"] = n; d["id"] = 0; out["decks"].append(d)
#   out["activeDeckIndex"] = 1; json.dump(out, open(".harmony/probe-deck-tabs.json", "w"), indent=1)
#   PY
#
# Screen-safe: open -g only (never plain open, never the Output window), window-only captures by Quartz window id
# (never full-screen, never fixed-coordinate crops), no synthetic input, graceful quit (pkill only after 30 s).
# PROBE RIG GATE (probehygiene2): refuses (exit 64) unless /tmp/audiodna-live.lock/owner exists; if
# AUDIODNA_LOCK_OWNER is set it must match the owner file's first field. adna_* match the kernel ucomm
# "Audio-DNA" exactly (never a linker line, never a spoofed argv[0]).
set -u
# --- live-lock gate: refuse unless the caller holds /tmp/audiodna-live.lock (rig rule) ---
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
APP="${DECKTABS_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${DECKTABS_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
OUT="${1:-}"; [ -n "$OUT" ] || { echo "usage: $0 OUT_DIR [--hook]"; exit 64; }
HOOK=0; [ "${2:-}" = "--hook" ] && HOOK=1
FIX="$ROOT/.harmony/probe-deck-tabs.json"
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set DECKTABS_APP)"; exit 64; }
[ -f "$FIX" ] || { echo "REFUSE: fixture $FIX missing"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import Quartz, json' 2>/dev/null || { echo "REFUSE: no python with pyobjc Quartz (set DECKTABS_PY)"; exit 64; }
refuse_foreign_start || exit 64
lsof -nP -iTCP:7070 -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port 7070 already has a listener"; exit 64; }
mkdir -p "$OUT" || exit 64
echo "app: $APP"; echo "out: $OUT"

PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }

# shot NAME -> $OUT/NAME-w<k>.png, one per on-screen Audio-DNA window (largest first: w0 = the main window, then any
# dialog). Window-only: screencapture -x -o -l <CGWindowID>.
shot() {
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
jget() { curl -s --max-time 5 "$A/api/composition" | "$PY" -c "import sys, json; d = json.load(sys.stdin); print($1)" 2>/dev/null; }
wait_health() { local _; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && return 0; sleep 1; done; return 1; }
launch() {   # launch [--env K=V ...]
  : > "$OUT/out.log"; : > "$OUT/err.log"   # open --stdout/--stderr APPEND (gotchas): truncate first
  open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" "$@" "$APP"
  record_ourpid; echo "ours: pid ${OURPID:-none}"
}
quit_app() {
  quit_ours   # ONLY the pid launch() recorded (probe-quit-ours.sh)
}
zero_windows() {   # $1 = row label
  local W
  W="$("$PY" -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if str(w.get('kCGWindowOwnerName', '')) == 'Audio-DNA']))" 2>/dev/null)"
  if ! adna_running && [ "$W" = "0" ]; then ok "$1: app quit, 0 Audio-DNA windows in the FULL window list"
  else no "$1: app running=$(adna_running && echo yes || echo no), windows=$W"; fi
}

# ---------------------------------------------------------------- Phase 1 ------------------------------------------
echo "== Phase 1 (production, no env)"
launch
if ! wait_health; then no "R0 app never answered /api/health"; quit_app; echo; echo "$PASS PASS / $FAIL FAIL"; exit 1; fi
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
[ "$L7070" = "Audio-DNA" ] || no "port 7070 is answered by '$L7070', not Audio-DNA"
sleep 3; shot 01-default
R="$(curl -s --max-time 10 -X POST "$A/api/load_composition" -H 'Content-Type: application/json' -d "{\"path\":\"$FIX\"}")"
echo "$R" | grep -q '"ok":[[:space:]]*true' && ok "R1 load_composition accepted the three-deck fixture" || no "R1 load_composition: $R"
sleep 2
IDS="$(jget "','.join(str(x['id']) if 'id' in x else 'ABSENT' for x in d['decks'])")"
NAMES="$(jget "','.join(x['name'] for x in d['decks'])")"
echo "  decks: names=$NAMES ids=$IDS"
case ",$IDS," in *ABSENT*|",,") no "R2 /api/composition decks[].id present (got: ${IDS:-nothing})";; *) ok "R2 /api/composition reports decks[].id ($IDS)";; esac
DISTINCT="$(jget "len(d['decks']) == 3 and all('id' in x for x in d['decks']) and len(set(x['id'] for x in d['decks'])) == 3")"
[ "$DISTINCT" = "True" ] && ok "R3 the three deck ids are pairwise distinct ($IDS; the file had 0,0,0)" || no "R3 deck ids pairwise distinct (ids: ${IDS:-none})"
AD="$(jget "d['activeDeck']")"
[ "$AD" = "1" ] && ok "R4 activeDeck == 1 (B)" || no "R4 activeDeck == 1 (got ${AD:-none})"
shot 02-three-decks
curl -s --max-time 5 -X POST "$A/api/switch_deck" -H 'Content-Type: application/json' -d '{"deck":2}' >/dev/null
sleep 2
AD="$(jget "d['activeDeck']")"
[ "$AD" = "2" ] && ok "R5 switch_deck 2 -> activeDeck == 2 (C)" || no "R5 activeDeck after switch_deck 2 (got ${AD:-none})"
shot 03-active-C
quit_app
zero_windows "R6"
cp "$OUT/err.log" "$OUT/phase1-err.log" 2>/dev/null

# ---------------------------------------------------------------- Phase 2 ------------------------------------------
if [ "$HOOK" -eq 1 ]; then
  echo "== Phase 2 (--hook: TEMPORARY screenshot hook build)"
  LIBDIR="$HOME/Library/AudioDNA/compositions"; LIBFILE="$LIBDIR/_probe-deck-tabs.json"; MADE_LIBDIR=0
  lib_cleanup() {
    [ -f "$LIBFILE" ] && rm -f "$LIBFILE" && echo "  removed temp library file $LIBFILE"
    [ "$MADE_LIBDIR" -eq 1 ] && rmdir "$LIBDIR" 2>/dev/null && echo "  removed $LIBDIR (created by this run, empty)"
    MADE_LIBDIR=0
  }
  trap 'lib_cleanup; adna_running && quit_app' EXIT
  # state|name|comp(0/1)|snapshot(0/1)|library row(0/1)
  for spec in "plus_menu|04-plus-menu|0|1|0" \
              "deck_menu:0|05-deck-menu|1|1|0" \
              "deck_rename:0|06-rename-dialog|1|0|0" \
              "replace_confirm|07-replace-confirm|0|0|0" \
              "browser_compositions|08-browser-compositions|0|0|0" \
              "library_menu:0|09-library-menu|0|1|1" \
              "library_delete_confirm:0|10-library-delete-confirm|0|0|1" \
              "remove_hint:0|11-after-remove|1|0|0"; do
    IFS='|' read -r STATE NAME USECOMP USESNAP USELIB <<< "$spec"
    echo "-- $NAME ($STATE)"
    if [ "$USELIB" -eq 1 ]; then
      [ -d "$LIBDIR" ] || { mkdir -p "$LIBDIR" && MADE_LIBDIR=1; }
      if [ -e "$LIBFILE" ]; then no "$NAME: $LIBFILE already exists -- not touching it"; continue; fi
      cp "$FIX" "$LIBFILE" && echo "  temp library file $LIBFILE (the only row this run adds)"
    fi
    ENVS=(--env "AUDIODNA_DEBUG_SHOW=$STATE")
    [ "$USECOMP" -eq 1 ] && ENVS+=(--env "AUDIODNA_DEBUG_COMP=$FIX")
    [ "$USESNAP" -eq 1 ] && { rm -f "$OUT/$NAME-snapshot.png"; ENVS+=(--env "AUDIODNA_DEBUG_SNAP=$OUT/$NAME-snapshot.png"); }
    T0=$(date +%s)
    launch "${ENVS[@]}"
    if ! wait_health; then no "$NAME: app never answered /api/health"; quit_app; [ "$USELIB" -eq 1 ] && lib_cleanup; continue; fi
    # the hook fires at constructor + 4.5 s (the COMP load at + 2.5 s): shoot at launch + ~9 s, quit after the
    # hook's T+14 s dismissal
    while [ $(( $(date +%s) - T0 )) -lt 9 ]; do sleep 1; done
    shot "$NAME"
    if [ "$USESNAP" -eq 1 ]; then
      [ -s "$OUT/$NAME-snapshot.png" ] && ok "$NAME: in-app snapshot written right after the menu opened" || no "$NAME: no in-app snapshot"
    fi
    if [ "$STATE" = "remove_hint:0" ]; then
      NM="$(jget "','.join(x['name'] for x in d['decks'])")"; AD="$(jget "d['activeDeck']")"
      [ "$NM" = "B,C" ] && [ "$AD" = "0" ] && ok "$NAME: deck A removed from before the active deck -> decks B,C, B still active (index 0)" \
        || no "$NAME: after removing A expected decks B,C active 0 (got '$NM' active ${AD:-none})"
    fi
    while [ $(( $(date +%s) - T0 )) -lt 17 ]; do sleep 1; done
    quit_app
    zero_windows "$NAME"
    cp "$OUT/err.log" "$OUT/$NAME-err.log" 2>/dev/null
    [ "$USELIB" -eq 1 ] && lib_cleanup
  done
  lib_cleanup; trap - EXIT
fi

echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
