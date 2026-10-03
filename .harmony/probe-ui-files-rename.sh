#!/bin/bash
# probe-ui-files-rename.sh -- s-rta-1002b lane ui (BF3 codec line + Show in Finder, BF8 double-click deck rename).
# G3 of .harmony/.reports/s-rta-1002b/ruling-ui.md (V1-V4, R1-R10), plus the G4 window captures that need no hook
# (--shots: C1-C5, C8-C14 with NB1 / NB2) and the two hook-only component snapshots (--hook STATE: C6 / C7).
#
# usage: probe-ui-files-rename.sh OUT_DIR [--shots] [--hook cell-menu|cell-tooltip]
#   UIFR_APP     app bundle (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app); launched
#                `open -g ... --args --test-mode` (never brought to the front, never an Output window)
#   UIFR_ATTACH=1  do NOT launch or quit: the caller already started ONE Audio-DNA (--test-mode) and quits it after
#                (e.g. the live-lock helper's start_app / quit_app). It attaches ONLY to the harness's own test-mode app
#                (the bf10 attach rule, probe-milkdrop.sh; Harmony ruling R-N1): the one running Audio-DNA pid must
#                equal UIFR_ATTACH_PID, else the pid the lock helper's start_app recorded (LOCK_LIB + LANE); that pid
#                must own the 8080 listener and 8080 /api/health must answer. Otherwise: REFUSE, exit 2, no request
#                sent. With --hook the caller must have launched the
#                HOOK build with --env AUDIODNA_DEBUG_SHOW=<state> --env AUDIODNA_DEBUG_SNAP=<OUT>/<C6|C7 file name>.
#   UIFR_PY      python with pyobjc Quartz (default: <root>/.venv, else the main checkout's .venv)
#
# Rows (exit 0 iff every row PASSes; the bars are the ruling's G3 strings, copied, never re-thresholded):
#   V1-V4 GET /api/debug/clip_media, POST /api/debug/reveal_clip, POST /api/debug/inspect_clip on deck A (8 cells).
#   R1-R11 GET /api/debug/deck_tabs, POST /api/debug/{tab_click,tab_dblclick,deck_rename,undo,duplicate_deck},
#   /api/switch_deck, /api/load_composition, GET /api/composition. Pre-change apps have none of the debug routes: every
#   V / R row FAILs. Lane bf9b (Harmony ruling H-6, s-rta-1003): a deck switch between same-shape decks no longer
#   rebuilds the tab row -- R3 / R4a / R9a read "builds +0" (RED on a pre-bf9b app: +1); R11 is the positive control
#   (Duplicate Deck rebuilds it: +1).
# The composition is written at run time into OUT (absolute paths): deck "A" = 2 layers x 4 columns
#   L0: C0 video_h264_64x64.mp4 | C1 video_prores_hq_64x64_2997.mov | C2 video_hapq_64x64_60.mov |
#       C3 video_hevc_main10_64x64_23976.mp4 (its CLIP NAME is long: C14)
#   L1: C0 test_card.png | C1 a sequence of 3 PNGs (OUT/media) | C2 OUT/media/missing_clip.mp4 (never exists; its
#       folder does) | C3 the source "perlin_noise"
#   decks "B" and "C" = copies of "A" (fresh deck and clip ids); active deck 0.
# SETTLE (pre-registered): rows are read only after POST /api/load_composition answered {"ok":true} AND /api/state
#   load.opens_pending == 0, load.staged == 0, load.queued == 0. A V1 / V4 video read that is null is re-read every
#   200 ms for up to 5 s (every retry printed); still null at 5 s = FAIL.
# --shots (after R10, same launch): window captures of the main Audio-DNA window by Quartz window id only
#   (screencapture -x -o -l <id>): C1 tab row default, C2 box open on the showing tab (+ NB2), C3 "Intro" typed,
#   C4 renamed tab, C8 H.264 clip inspector, C9 HAP Q, C10 missing file, C11 PNG, C12 image sequence, C13 procedural
#   source, C14 long clip name, then C5: a composition of N decks, N = the smallest count >= 12 whose tabs are 60 px
#   at this row width (DeckTabRow::layout), box on the LAST tab (+ NB1). Every capture is decoded (size, colours).
#   In captures the app is in the background: the box shows no caret / focus outline (rig artefact; C15 = G1b's P4).
# --hook STATE (a TEMPORARY hook build only -- never committed, G2b): loads the composition; the hook waits until
#   cell (0, 0)'s tooltip carries the codec line, opens that cell's menu (cell-menu = C6) or shows the app tooltip over
#   it (cell-tooltip = C7) and snapshots the top-level window into AUDIODNA_DEBUG_SNAP. Rows: the snapshot exists and
#   decodes. The V / R rows are skipped in this mode.
#
# Screen-safe: open -g only, --test-mode, window-only captures, no synthetic OS input, never an Output window.
# Quits ONLY the Audio-DNA pid this script launched (quit_ours, .harmony/probe-quit-ours.sh: osascript quit, then a
# kill of THAT pid after 30 s); refuses to start when any Audio-DNA runs (it may be Boris's). PROBE RIG GATE: refuses (exit 64) unless
# /tmp/audiodna-live.lock/owner exists and, when AUDIODNA_LOCK_OWNER is set, its first field matches.
set -u
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held (/tmp/audiodna-live.lock/owner)"; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
fi
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }

ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${UIFR_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
ATTACH="${UIFR_ATTACH:-0}"
PY="${UIFR_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
OUT="${1:-}"; [ -n "$OUT" ] || { echo "usage: $0 OUT_DIR [--shots] [--hook cell-menu|cell-tooltip]"; exit 64; }
shift
SHOTS=0; HOOK=""
while [ $# -gt 0 ]; do
  case "$1" in
    --shots) SHOTS=1 ;;
    --hook) HOOK="${2:-}"; shift ;;
    *) echo "unknown argument $1"; exit 64 ;;
  esac
  shift
done
case "$HOOK" in ""|cell-menu|cell-tooltip) ;; *) echo "REFUSE: --hook takes cell-menu or cell-tooltip"; exit 64 ;; esac
FX="$ROOT/tests/fixtures"
[ -n "$PY" ] && "$PY" -c 'import Quartz, json' 2>/dev/null || { echo "REFUSE: no python with pyobjc Quartz (set UIFR_PY)"; exit 64; }
for f in video_h264_64x64.mp4 video_prores_hq_64x64_2997.mov video_hapq_64x64_60.mov video_hevc_main10_64x64_23976.mp4 test_card.png; do
  [ -f "$FX/$f" ] || { echo "REFUSE: fixture $FX/$f missing"; exit 64; }
done
mkdir -p "$OUT" || exit 64
OUT="$(cd "$OUT" && pwd)"
SNAP=""
[ "$HOOK" = "cell-menu" ] && SNAP="$OUT/C6-cell-menu.png"
[ "$HOOK" = "cell-tooltip" ] && SNAP="$OUT/C7-cell-tooltip.png"

. "$ROOT/.harmony/probe-quit-ours.sh"   # refuse_foreign_start / record_ourpid / quit_ours: quit ONLY the app this run launched
if [ "$ATTACH" = "1" ]; then
  [ -n "$(adna_pids)" ] || { echo "REFUSE: UIFR_ATTACH=1 but no Audio-DNA is running"; exit 64; }
  [ "$(adna_pids | wc -l | tr -d ' ')" = "1" ] || { echo "REFUSE: more than one Audio-DNA running"; exit 64; }
  # An Audio-DNA no harness started is Boris's: the R rows rename decks and load a composition over 7070 / 8080.
  RUNPID="$(adna_pids | tr -d ' \n')"
  WANTPID="${UIFR_ATTACH_PID:-}"
  if [ -z "$WANTPID" ] && [ -n "${LOCK_LIB:-}" ]; then   # the helper names start_app's pid record OURPID
    WANTPID="$(LANE="${LANE:-${LOCK_LANE:-}}"; . "$LOCK_LIB" >/dev/null 2>&1 && cat "$OURPID" 2>/dev/null | tr -d ' \n')"
  fi
  L8080="$(lsof -nP -t -iTCP:8080 -sTCP:LISTEN 2>/dev/null | sort -u | tr -d ' \n')"
  if [ -z "$WANTPID" ] || [ "$RUNPID" != "$WANTPID" ] || [ "$L8080" != "$RUNPID" ] \
     || [ -z "$(curl -s --max-time 2 -H 'Connection: close' http://localhost:8080/api/health)" ]; then
    echo "REFUSE: the running Audio-DNA is not a test-mode app this run started (it may be Boris's)"
    echo "        running pid ${RUNPID:-none}; expected ${WANTPID:-none} (UIFR_ATTACH_PID, else LOCK_LIB + LANE's start_app record); 8080 listener ${L8080:-none}"
    exit 2
  fi
  echo "attach: Audio-DNA pid $RUNPID (started and quit by the caller)"
else
  [ -d "$APP" ] || { echo "REFUSE: no app at $APP (set UIFR_APP)"; exit 64; }
  refuse_foreign_start || exit 64
  for p in 7070 8080; do lsof -nP -iTCP:$p -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $p already has a listener"; exit 64; }; done
  : > "$OUT/app-out.log"; : > "$OUT/app-err.log"   # open --stdout / --stderr APPEND: truncate first
  ENVS=()
  [ -n "$HOOK" ] && { rm -f "$SNAP"; ENVS=(--env "AUDIODNA_DEBUG_SHOW=$HOOK" --env "AUDIODNA_DEBUG_SNAP=$SNAP"); }
  echo "launch: $APP ${ENVS[*]:-} --test-mode"
  # ${ENVS[@]+...}: macOS /bin/bash 3.2 treats an EMPTY array as unbound under set -u (the default standalone path)
  open -g --stdout "$OUT/app-out.log" --stderr "$OUT/app-err.log" ${ENVS[@]+"${ENVS[@]}"} "$APP" --args --test-mode
  record_ourpid; echo "app pid: ${OURPID:-none}"
  for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 -H 'Connection: close' "$A/api/health")" ] && break; sleep 1; done
fi

"$PY" - "$OUT" "$FX" "$SHOTS" "$HOOK" "$SNAP" <<'PYEOF'
import json, os, shutil, subprocess, sys, time, urllib.error, urllib.request
OUT, FX, SHOTS, HOOK, SNAP = sys.argv[1], sys.argv[2], sys.argv[3] == "1", sys.argv[4], sys.argv[5]
B = "http://127.0.0.1:7070"
PASS = FAIL = 0

def row(name, ok, info=""):
    global PASS, FAIL
    if ok: PASS += 1
    else: FAIL += 1
    print(("PASS  " if ok else "FAIL  ") + name + ("  | " + info if info else ""), flush=True)

def req(method, path, body=None, timeout=10):
    data = None if body is None else json.dumps(body).encode()
    r = urllib.request.Request(B + path, data=data, method=method,
                               headers={"Connection": "close", "Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(r, timeout=timeout) as f:
            return json.loads(f.read().decode() or "{}")
    except urllib.error.HTTPError as e:
        return {"_http": e.code}
    except Exception as e:
        return {"_err": repr(e)}

def get(path): return req("GET", path)
def post(path, body=None, timeout=10): return req("POST", path, {} if body is None else body, timeout)
def cm(l, c): return get("/api/debug/clip_media?layer=%d&column=%d" % (l, c))
def tabs(): return get("/api/debug/deck_tabs")
def comp(): return get("/api/composition")
def names():
    tabs()   # barrier: deck_tabs is read ON the message thread, after every queued (callAsync) debug action
    d = comp()
    return [x.get("name") for x in d.get("decks", [])] if "decks" in d else None
def k(d, *path):
    for p in path:
        if isinstance(d, dict) and p in d: d = d[p]
        elif isinstance(d, list) and isinstance(p, int) and -len(d) <= p < len(d): d = d[p]
        else: return None
    return d

# ---------------------------------------------------------------- composition (absolute paths) ---------------------
media = os.path.join(OUT, "media"); os.makedirs(media, exist_ok=True)
seq = []
for i in range(3):
    p = os.path.join(media, "seq_%d.png" % i); shutil.copyfile(os.path.join(FX, "test_card.png"), p); seq.append(p)
missing = os.path.join(media, "missing_clip.mp4")
if os.path.exists(missing): os.remove(missing)
VIDS = ["video_h264_64x64.mp4", "video_prores_hq_64x64_2997.mov", "video_hapq_64x64_60.mov",
        "video_hevc_main10_64x64_23976.mp4"]
LONG = "A very long clip name that runs right up to the Show in Finder button"
def layer(lid, clips): return {"name": "L%d" % (lid + 1), "id": lid, "opacity": 1.0, "visible": True, "blendMode": 1, "clips": clips}
def deck(name, did, base):
    v = lambda c, f, nm=None: {"name": nm or os.path.splitext(f)[0], "id": base + c + 1, "mediaType": 2, "mediaFile": os.path.join(FX, f)}
    l0 = [v(0, VIDS[0]), v(1, VIDS[1]), v(2, VIDS[2]), v(3, VIDS[3], LONG)]
    l1 = [{"name": "test_card", "id": base + 11, "mediaType": 1, "mediaFile": os.path.join(FX, "test_card.png")},
          {"name": "seq", "id": base + 12, "mediaType": 5, "sequenceFiles": seq, "sequenceFps": 2.5},
          {"name": "missing_clip", "id": base + 13, "mediaType": 2, "mediaFile": missing},
          {"name": "perlin", "id": base + 14, "mediaType": 4, "sourceType": "perlin_noise"}]
    return {"name": name, "id": did, "numColumns": 4, "layers": [layer(0, l0), layer(1, l1)]}
COMP = os.path.join(OUT, "probe-ui-files-rename.json")
json.dump({"name": "probe-ui-files-rename", "activeDeckIndex": 0, "masterOpacity": 1.0,
           "decks": [deck("A", 1, 100), deck("B", 2, 200), deck("C", 3, 300)]}, open(COMP, "w"), indent=1)
print("composition:", COMP, flush=True)

def load(path):
    t0 = time.time()
    r = post("/api/load_composition", {"path": path}, timeout=90)
    ok = r.get("ok") is True
    st = None
    for _ in range(100):
        st = k(get("/api/state"), "load")
        if ok and isinstance(st, dict) and st.get("opens_pending") == 0 and st.get("staged") == 0 and st.get("queued") == 0:
            break
        time.sleep(0.1)
    settled = ok and isinstance(st, dict) and st.get("opens_pending") == 0 and st.get("staged") == 0 and st.get("queued") == 0
    print("  load %s -> %s; settle %s (%.1f s) load=%s" % (os.path.basename(path), r, settled, time.time() - t0,
          json.dumps({x: st.get(x) for x in ("opens_pending", "staged", "queued")}) if isinstance(st, dict) else st), flush=True)
    return r, settled

def settle_video(l, c, label):
    r = cm(l, c); n = 0
    while isinstance(r, dict) and "video" in r and r.get("video") is None and n < 25:
        n += 1; print("  retry %d: %s video null" % (n, label), flush=True); time.sleep(0.2); r = cm(l, c)
    return r, n

# ---------------------------------------------------------------- captures -----------------------------------------
def shot(name):
    import Quartz
    wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
    wins = []
    for w in wl:
        if str(w.get("kCGWindowOwnerName", "")) != "Audio-DNA" or int(w.get("kCGWindowLayer", 0)) >= 20: continue
        if "Output" in str(w.get("kCGWindowName", "")): continue
        b = w.get("kCGWindowBounds", {})
        if float(b.get("Width", 0)) < 300 or float(b.get("Height", 0)) < 300: continue
        wins.append((float(b["Width"]) * float(b["Height"]), int(w["kCGWindowNumber"])))
    if not wins:
        row("%s capture" % name, False, "no Audio-DNA main window on screen"); return None
    wins.sort(reverse=True)
    path = os.path.join(OUT, name + ".png")
    if os.path.exists(path): os.remove(path)
    subprocess.run(["screencapture", "-x", "-o", "-l", str(wins[0][1]), path], check=False)
    return decode(name, path)

def decode(name, path):
    import Quartz
    from Foundation import NSURL
    if not os.path.isfile(path):
        row("%s decoded" % name, False, "no file %s" % path); return None
    src = Quartz.CGImageSourceCreateWithURL(NSURL.fileURLWithPath_(path), None)
    img = Quartz.CGImageSourceCreateImageAtIndex(src, 0, None) if src else None
    if img is None:
        row("%s decoded" % name, False, "%s does not decode" % path); return None
    w, h = Quartz.CGImageGetWidth(img), Quartz.CGImageGetHeight(img)
    data = Quartz.CGDataProviderCopyData(Quartz.CGImageGetDataProvider(img))
    bpr, bpp = Quartz.CGImageGetBytesPerRow(img), Quartz.CGImageGetBitsPerPixel(img) // 8
    buf = bytes(data); cols = set()
    for y in range(0, h, max(1, h // 64)):
        for x in range(0, w, max(1, w // 64)):
            o = y * bpr + x * bpp; cols.add(buf[o:o + 3])
    row("%s decoded" % name, w > 100 and h > 100 and len(cols) > 8, "%s %d x %d, %d distinct sampled colours" % (os.path.basename(path), w, h, len(cols)))
    return (w, h)

# ---------------------------------------------------------------- hook mode (C6 / C7) -------------------------------
if HOOK:
    r, settled = load(COMP)
    row("H0 load + settle (%s)" % HOOK, r.get("ok") is True and settled, json.dumps(r))
    for _ in range(300):
        if os.path.isfile(SNAP) and os.path.getsize(SNAP) > 0: break
        time.sleep(0.2)
    time.sleep(0.5)
    row("H1 %s: the hook wrote its component snapshot" % HOOK, os.path.isfile(SNAP) and os.path.getsize(SNAP) > 0, SNAP)
    decode("C6" if HOOK == "cell-menu" else "C7", SNAP)
    print("\n%d PASS / %d FAIL" % (PASS, FAIL)); sys.exit(0 if FAIL == 0 else 1)

# ---------------------------------------------------------------- G3 -----------------------------------------------
r, settled = load(COMP)
row("S0 load_composition {\"ok\":true} + settle (opens_pending / staged / queued == 0)", r.get("ok") is True and settled, json.dumps(r))
c0 = comp()
print("  decks %s active %s" % (names(), c0.get("activeDeck")), flush=True)

V1 = {(0, 0): ["H.264 High", "64 x 64, 30 frames per second"],
      (0, 1): ["ProRes 422 HQ", "64 x 64, 29.97 frames per second"],
      (0, 2): ["HAP Q", "64 x 64, 60 frames per second"],
      (0, 3): ["HEVC Main 10", "64 x 64, 23.976 frames per second"],
      (1, 0): ["PNG image"], (1, 1): ["Image sequence, 3 images"], (1, 2): ["File missing"], (1, 3): []}
for (l, c), lines in V1.items():
    lab = "L%dC%d" % (l, c)
    rr, n = settle_video(l, c, lab) if l == 0 else (cm(l, c), 0)
    line = ", ".join(lines)
    ok = rr.get("line") == line and rr.get("lines") == lines
    if (l, c) == (1, 2): ok = ok and rr.get("missing") is True
    if (l, c) == (1, 3): ok = ok and rr.get("file_backed") is False
    row("V1 %s line %r" % (lab, line), ok, "line %r lines %r missing %s file_backed %s video %s retries %d%s" % (
        rr.get("line"), rr.get("lines"), rr.get("missing"), rr.get("file_backed"), rr.get("video"), n,
        "" if "_http" not in rr and "_err" not in rr else " RESPONSE %s" % rr))

r00 = cm(0, 0); r11 = cm(1, 1); r13 = cm(1, 3); r12 = cm(1, 2)
want = "video_h264_64x64.mp4\nH.264 High, 64 x 64, 30 frames per second\nRight-click: Show in Finder"
row("V2 L0C0 tooltip", r00.get("tooltip") == want, repr(r00.get("tooltip")))
row("V2 L1C1 tooltip first line", isinstance(r11.get("tooltip"), str) and r11["tooltip"].split("\n")[0] == "Image sequence — 3 images at 2.5 images per second", repr(r11.get("tooltip")))
row("V2 L1C3 tooltip empty", r13.get("tooltip") == "", repr(r13.get("tooltip")))
row("V2 menus L0C0 / L1C2 / L1C3", r00.get("menu") == ["Show in Finder"] and r12.get("menu") == ["Show in Finder"] and r13.get("menu") == [],
    "%r %r %r" % (r00.get("menu"), r12.get("menu"), r13.get("menu")))

n0 = cm(0, 0).get("reveal_count")
post("/api/debug/reveal_clip", {"layer": 0, "column": 0}); time.sleep(0.3); a = cm(0, 0)
row("V3 reveal L0C0 -> the fixture's path, count +1", a.get("last_revealed") == os.path.join(FX, VIDS[0]) and isinstance(n0, int) and a.get("reveal_count") == n0 + 1,
    "%r count %s -> %s" % (a.get("last_revealed"), n0, a.get("reveal_count")))
post("/api/debug/reveal_clip", {"layer": 1, "column": 3}); time.sleep(0.3); b = cm(0, 0)
row("V3 reveal L1C3 (source) -> count +0", isinstance(a.get("reveal_count"), int) and b.get("reveal_count") == a.get("reveal_count"),
    "count %s -> %s" % (a.get("reveal_count"), b.get("reveal_count")))
post("/api/debug/reveal_clip", {"layer": 1, "column": 2}); time.sleep(0.3); m = cm(1, 2)
row("V3 reveal L1C2 (missing) -> its path, count +1", m.get("last_revealed") == missing and isinstance(b.get("reveal_count"), int) and m.get("reveal_count") == b.get("reveal_count") + 1,
    "%r count %s" % (m.get("last_revealed"), m.get("reveal_count")))

post("/api/debug/inspect_clip", {"layer": 0, "column": 0}); time.sleep(0.3); i0, n = settle_video(0, 0, "V4 L0C0")
row("V4 inspect L0C0", i0.get("inspector_shows") is True and i0.get("inspector_lines") == V1[(0, 0)] and i0.get("inspector_button_visible") is True,
    json.dumps({x: i0.get(x) for x in ("inspector_shows", "inspector_lines", "inspector_button_visible")}))
post("/api/debug/inspect_clip", {"layer": 1, "column": 3}); time.sleep(0.3); i3 = cm(1, 3)
row("V4 inspect L1C3 (source)", i3.get("inspector_lines") == [] and i3.get("inspector_button_visible") is False,
    json.dumps({x: i3.get(x) for x in ("inspector_shows", "inspector_lines", "inspector_button_visible")}))
# V4 for the other six cells (fix round F3): the Clip inspector's OWN rows, read live, == V1's lines; the button shows
# on every file-backed cell. L0C0 / L1C3 keep their rows above unchanged; L1C3 gets its own "shows" row.
row("V4 inspect L1C3 (source) shows the clip", i3.get("inspector_shows") is True, "inspector_shows %s" % i3.get("inspector_shows"))
for (l, c), lines in V1.items():
    if (l, c) in ((0, 0), (1, 3)): continue
    lab = "L%dC%d" % (l, c)
    post("/api/debug/inspect_clip", {"layer": l, "column": c}); time.sleep(0.3)
    ii, n = settle_video(l, c, "V4 " + lab) if l == 0 else (cm(l, c), 0)
    row("V4 inspect %s" % lab, ii.get("inspector_shows") is True and ii.get("inspector_lines") == lines and ii.get("inspector_button_visible") is True,
        json.dumps({x: ii.get(x) for x in ("inspector_shows", "inspector_lines", "inspector_button_visible")}) + " retries %d" % n)

# ---- BF8 rows
t = tabs(); tl = t.get("tabs") or []
ids = [x.get("id") for x in tl]
print("  deck_tabs: %s" % json.dumps({x: t.get(x) for x in ("ok", "active", "row_width", "tab_row_builds", "focus_home_count", "undo", "editor")} if "tabs" in t else t), flush=True)
row("R1 3 tabs A / B / C, active 0, editor closed, tooltips, undo.size < 90",
    [x.get("name") for x in tl] == ["A", "B", "C"] and t.get("active") == 0 and k(t, "editor", "open") is False
    and "Double-click: rename" in (tl[0].get("tooltip") or "") and all("Double-click: rename" not in (x.get("tooltip") or "") for x in tl[1:])
    and all("Right-click: Save / Rename / Duplicate / Remove" in (x.get("tooltip") or "") for x in tl)
    and isinstance(k(t, "undo", "size"), int) and k(t, "undo", "size") < 90,
    "names %r active %s tooltips %r undo %s" % ([x.get("name") for x in tl], t.get("active"), [x.get("tooltip") for x in tl], t.get("undo")))
bb = t.get("tab_row_builds"); post("/api/debug/tab_click", {"deck": 0}); t = tabs()
row("R2 tab_click 0 (showing) -> builds +0, active 0", isinstance(bb, int) and t.get("tab_row_builds") == bb and t.get("active") == 0,
    "builds %s -> %s active %s" % (bb, t.get("tab_row_builds"), t.get("active")))
# bf9b (Harmony ruling H-6, s-rta-1003): Boris -- "when I switch between decks, do not change the clips playing in the
# layers or how they are playing". A switch between same-shape decks is showDeck -> refresh: the tab row is NOT rebuilt
# (builds +0; it was +1 before bf9b). Every other clause of the row is unchanged. R11 proves the counter is alive.
bb = t.get("tab_row_builds"); post("/api/debug/tab_click", {"deck": 2}); t = tabs()
row("R3 tab_click 2 -> builds +0 (same-shape decks), active 2", isinstance(bb, int) and t.get("tab_row_builds") == bb and t.get("active") == 2,
    "builds %s -> %s active %s" % (bb, t.get("tab_row_builds"), t.get("active")))
bb = t.get("tab_row_builds"); post("/api/debug/tab_dblclick", {"deck": 1}); t = tabs()
# bf9b (H-6): Boris -- "when I switch between decks, do not change the clips playing in the layers or how they are
# playing": the switch does not rebuild the tab row (builds +0).
row("R4a tab_dblclick 1 (not showing) -> active 1, builds +0 (same-shape decks), editor closed",
    isinstance(bb, int) and t.get("active") == 1 and t.get("tab_row_builds") == bb and k(t, "editor", "open") is False,
    "builds %s -> %s active %s editor %s" % (bb, t.get("tab_row_builds"), t.get("active"), t.get("editor")))
bb = t.get("tab_row_builds"); post("/api/debug/tab_dblclick", {"deck": 1}); t = tabs(); e = t.get("editor") or {}; t1 = k(t, "tabs", 1) or {}
row("R4b tab_dblclick 1 (showing) -> builds +0, box open on B over its tab",
    isinstance(bb, int) and t.get("tab_row_builds") == bb and e.get("open") is True and len(ids) == 3 and e.get("deck_id") == ids[1] and e.get("text") == "B"
    and e.get("x") == t1.get("x") and e.get("y") == t1.get("y") and e.get("h") == 24 and isinstance(e.get("x"), int) and e["x"] + e["w"] <= t.get("row_width", -1),
    "builds %s -> %s editor %s tab1 %s row %s" % (bb, t.get("tab_row_builds"), e, {x: t1.get(x) for x in ("x", "y", "w", "h")}, t.get("row_width")))
u0 = t.get("undo") or {}; f0 = t.get("focus_home_count")
post("/api/debug/deck_rename", {"op": "type", "text": "  Intro  "}); post("/api/debug/deck_rename", {"op": "enter"}); t = tabs(); n_ = names()
row("R5 type '  Intro  ' + enter -> decks[1] 'Intro', one 'Rename Deck' step, focus home +1",
    n_ is not None and n_[1:2] == ["Intro"] and k(t, "editor", "open") is False and k(t, "tabs", 1, "label") == "Intro"
    and k(t, "undo", "top") == "Rename Deck" and isinstance(u0.get("index"), int) and k(t, "undo", "index") == u0["index"] + 1
    and isinstance(f0, int) and t.get("focus_home_count") == f0 + 1,
    "names %r label %r undo %s -> %s focus %s -> %s" % (n_, k(t, "tabs", 1, "label"), u0, t.get("undo"), f0, t.get("focus_home_count")))
post("/api/debug/undo", {"redo": False}); n_u = names(); post("/api/debug/undo", {"redo": True}); n_r = names(); t = tabs()
row("R6 undo -> 'B'; redo -> 'Intro', undo.top 'Rename Deck'",
    n_u is not None and n_u[1:2] == ["B"] and n_r is not None and n_r[1:2] == ["Intro"] and k(t, "undo", "top") == "Rename Deck",
    "after undo %r after redo %r undo %s" % (n_u, n_r, t.get("undo")))
u0 = t.get("undo") or {}; f0 = t.get("focus_home_count"); seen = []
for text, op in (("X", "escape"), ("", "enter"), ("Intro", "enter")):
    post("/api/debug/deck_rename", {"deck": 1, "op": "begin"}); post("/api/debug/deck_rename", {"op": "type", "text": text})
    post("/api/debug/deck_rename", {"op": op}); seen.append((names() or [None, None])[1])
t = tabs()
row("R7 escape / empty + enter / unchanged + enter -> 'Intro' each, undo index and size unchanged, focus home +3",
    seen == ["Intro", "Intro", "Intro"] and k(t, "undo", "index") == u0.get("index") and k(t, "undo", "size") == u0.get("size")
    and isinstance(f0, int) and t.get("focus_home_count") == f0 + 3 and k(t, "editor", "open") is False,
    "names %r undo %s -> %s focus %s -> %s" % (seen, u0, t.get("undo"), f0, t.get("focus_home_count")))
for text, op in (("Y", "outside_click"), ("Z", "focus_lost"), ("T", "tab")):
    u0 = t.get("undo") or {}; f0 = t.get("focus_home_count")
    post("/api/debug/deck_rename", {"deck": 1, "op": "begin"}); post("/api/debug/deck_rename", {"op": "type", "text": text})
    post("/api/debug/deck_rename", {"op": op}); t = tabs(); n_ = names()
    row("R8 %s keeps %r (one 'Rename Deck' step, focus home +1)" % (op, text),
        n_ is not None and n_[1:2] == [text] and k(t, "undo", "top") == "Rename Deck" and isinstance(u0.get("index"), int)
        and k(t, "undo", "index") == u0["index"] + 1 and isinstance(f0, int) and t.get("focus_home_count") == f0 + 1 and k(t, "editor", "open") is False,
        "names %r undo %s -> %s focus %s -> %s" % (n_, u0, t.get("undo"), f0, t.get("focus_home_count")))
bb = t.get("tab_row_builds"); n2 = (names() or [None, None, None])[2:3]
post("/api/debug/deck_rename", {"deck": 0, "op": "begin"}); post("/api/debug/deck_rename", {"op": "type", "text": "Q"})
post("/api/switch_deck", {"deck": 2})
for _ in range(30):
    t = tabs()
    if t.get("active") == 2: break
    time.sleep(0.1)
e = t.get("editor") or {}
# bf9b (H-6): Boris -- "when I switch between decks, do not change the clips playing in the layers or how they are
# playing": the switch does not rebuild the tab row (builds +0); the box stays on its deck over its tab.
row("R9a /api/switch_deck 2 while the box is open on A -> box stays on A over its tab, builds +0 (same-shape decks)",
    e.get("open") is True and len(ids) == 3 and e.get("deck_id") == ids[0] and e.get("x") == k(t, "tabs", 0, "x") and isinstance(bb, int)
    and t.get("tab_row_builds") == bb and t.get("active") == 2 and e.get("text") == "Q",
    "editor %s builds %s -> %s active %s" % (e, bb, t.get("tab_row_builds"), t.get("active")))
post("/api/debug/deck_rename", {"op": "enter"}); n_ = names()
row("R9b enter -> decks[0] 'Q', decks[2] unchanged", n_ is not None and n_[0:1] == ["Q"] and n_[2:3] == n2, "names %r (decks[2] before %r)" % (n_, n2))
t = tabs(); f0 = t.get("focus_home_count")
post("/api/debug/deck_rename", {"deck": 0, "op": "begin"}); time.sleep(0.2)
opened = k(tabs(), "editor", "open")
r, settled = load(COMP); t = tabs(); n_ = names()
row("R10 box open on A, then load_composition (same file) -> {\"ok\":true}, box closed, decks[0] 'A', focus home +1",
    opened is True and r.get("ok") is True and k(t, "editor", "open") is False and n_ is not None and n_[0:1] == ["A"]
    and isinstance(f0, int) and t.get("focus_home_count") == f0 + 1,
    "opened %s load %s editor %s names %r focus %s -> %s" % (opened, r, t.get("editor"), n_, f0, t.get("focus_home_count")))
# R11 (bf9b, H-6 positive control): the tab_row_builds counter is alive -- an action that legitimately rebuilds the tab
# row (Duplicate Deck: one more tab) reads +1. Then the composition is loaded again, so the captures see 3 decks.
t = tabs(); bb = t.get("tab_row_builds"); nt = len(t.get("tabs") or [])
rd = post("/api/debug/duplicate_deck", {"deck": 0})
for _ in range(50):
    t = tabs()
    if len(t.get("tabs") or []) == nt + 1: break
    time.sleep(0.1)
time.sleep(0.3); t = tabs()
row("R11 CONTROL duplicate_deck 0 -> %d tabs, builds +1 (the counter is alive)" % (nt + 1),
    isinstance(bb, int) and len(t.get("tabs") or []) == nt + 1 and t.get("tab_row_builds") == bb + 1,
    "duplicate_deck %s tabs %d -> %d builds %s -> %s active %s" % (rd, nt, len(t.get("tabs") or []), bb, t.get("tab_row_builds"), t.get("active")))
r, settled = load(COMP)
print("  R11 restore: load %s settled %s decks %r" % (r.get("ok"), settled, names()), flush=True)

# ---------------------------------------------------------------- G4 captures (--shots) ------------------------------
if SHOTS:
    print("== captures (window-only, Quartz window id)", flush=True)
    time.sleep(1.0); shot("C1-tab-row-default")
    post("/api/debug/deck_rename", {"deck": 0, "op": "begin"}); time.sleep(0.4); t = tabs(); e = t.get("editor") or {}; ta = k(t, "tabs", t.get("active", -1)) or {}
    row("NB2 box on the showing tab: x == tab x, w == tab w (100), h == 24", e.get("open") is True and e.get("x") == ta.get("x") and e.get("w") == ta.get("w") == 100 and e.get("h") == 24,
        "editor %s tab %s" % (e, {x: ta.get(x) for x in ("x", "y", "w", "h")}))
    shot("C2-rename-box-open")
    post("/api/debug/deck_rename", {"op": "type", "text": "Intro"}); time.sleep(0.4); shot("C3-typed-Intro")
    post("/api/debug/deck_rename", {"op": "enter"}); time.sleep(0.4); shot("C4-renamed-tab")
    for name, (l, c) in (("C8-inspector-h264", (0, 0)), ("C9-inspector-hapq", (0, 2)), ("C10-inspector-missing", (1, 2)),
                         ("C11-inspector-png", (1, 0)), ("C12-inspector-sequence", (1, 1)), ("C13-inspector-source", (1, 3)),
                         ("C14-inspector-long-name", (0, 3))):
        post("/api/debug/inspect_clip", {"layer": l, "column": c}); time.sleep(0.6); x = cm(l, c)
        print("  %s: inspector_shows %s lines %r button %s" % (name, x.get("inspector_shows"), x.get("inspector_lines"), x.get("inspector_button_visible")), flush=True)
        shot(name)
    rw = tabs().get("row_width")
    def tabw(row_w, n):   # DeckTabRow::layout's tab width
        w = 100
        if n * 102 + 24 > row_w: w = max(60, (row_w - 24 - n * 2) // n)
        return w
    N = next((n for n in range(12, 61) if isinstance(rw, int) and tabw(rw, n) == 60), None)
    if N is None:
        row("C5 deck count for 60-px tabs", False, "row_width %r" % rw)
    else:
        small = lambda i: {"name": "Deck %d" % (i + 1), "id": i + 1, "numColumns": 4,
                           "layers": [layer(0, [None, None, None, None])]}
        C5 = os.path.join(OUT, "probe-ui-files-rename-c5.json")
        json.dump({"name": "probe-ui-files-rename-c5", "activeDeckIndex": 0, "masterOpacity": 1.0, "decks": [small(i) for i in range(N)]}, open(C5, "w"), indent=1)
        r, settled = load(C5)
        post("/api/debug/deck_rename", {"deck": N - 1, "op": "begin"}); time.sleep(0.4); t = tabs(); e = t.get("editor") or {}; tl = k(t, "tabs", N - 1) or {}
        row("NB1 %d decks (row %d px, last tab %s px): box w 100, inside the row, on the last tab's y, h 24" % (N, rw, tl.get("w")),
            len(t.get("tabs") or []) == N and tl.get("w") == 60 and e.get("open") is True and e.get("w") == 100 and isinstance(e.get("x"), int) and e["x"] >= 0
            and e["x"] + e["w"] <= t.get("row_width", -1) and e.get("y") == tl.get("y") and e.get("h") == 24
            and e["x"] <= tl.get("x", -1) and e["x"] + e["w"] >= min(tl.get("x", 0) + tl.get("w", 0), t.get("row_width", 0)),
            "editor %s last tab %s row %s" % (e, {x: tl.get(x) for x in ("x", "y", "w", "h")}, t.get("row_width")))
        shot("C5-rename-box-last-60px-tab")
        post("/api/debug/deck_rename", {"op": "escape"})

print("\n%d PASS / %d FAIL   (artifacts in %s)" % (PASS, FAIL, OUT))
sys.exit(0 if FAIL == 0 else 1)
PYEOF
RC=$?
[ "$ATTACH" = "1" ] || { quit_ours; echo "app running after quit: $([ -n "$(adna_pids)" ] && echo YES || echo no)"; }
exit $RC
