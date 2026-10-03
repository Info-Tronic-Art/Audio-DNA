#!/bin/bash
# probe-asan-live.sh -- lane bf9b fix (s-rta-1003; ruling .harmony/.reports/s-rta-1003/ruling-bf9b-merge.md AM-6, gate
# ASAN-LIVE). The live call-chain row of the memory fix: an AddressSanitizer build of the APP walks the real
# Load Deck / Undo / Redo / Duplicate Deck / Remove Deck / Load Composition chain over REST with the Layer tab open and
# bound, so the stack-move hook's ONE wiring line in MainComponent (the only line no unit test drives; lint B4h pins
# its text) is exercised: the Layer tab's 10 Hz refresh reads the layer the inspector holds.
#
# usage: probe-asan-live.sh [out-base]
#   ASAN_LIVE_APP  the ASan app bundle (default: <tree>/build-asan-app/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app;
#                  configure: -DADNA_SANITIZE=address -DCMAKE_BUILD_TYPE=RelWithDebInfo -DAUDIODNA_BUILD_TEST_SERVER=ON)
#   ASAN_LIVE_PY   python with requests + Pillow (default: <tree>/.venv, else the main checkout's .venv)
# Every run works in a FRESH dir: mktemp -d "<out-base>/asan-live.XXXXXX" (out-base default /tmp): the fixtures
# (.harmony/make-bf9b-check.py: nine-rows.json, bf9b-check.json), the app's stdout / stderr, and ASan's report files
# asan.<pid> (log_path).
#
# LAUNCH: open -g --env ADNA_INSPECT_LAYER=1 --env ASAN_OPTIONS=abort_on_error=0:halt_on_error=1:log_path=<out>/asan
#   <app> --args --test-mode  -- the DEFAULT 3-layer show, nothing loaded. abort_on_error=0: on a report the app ENDS
#   (no abort, so no crash dialog on the screen); the report is in <out>/asan.<pid>.
# STEPS (each followed by 1.0 s and GET /api/health == 200; every VALID clause is read by GET):
#   L0 VALID ui_text.inspected_layer == "Layer 2" and inspector_tab == "Layer" (required again at L1, L2, L3: the
#      ACTIVE tab's refresh is what reads the layer)
#   L1 POST /api/debug/load_deck nine-rows.json     VALID layers 9, numDecks 2, inspected_layer "Layer 2"
#   L2 POST /api/debug/undo                         VALID layers 3, numDecks 1, inspected_layer "Layer 2"
#   L3 POST /api/debug/undo {redo}                  VALID layers 9, numDecks 2, inspected_layer "Layer 2"
#   L4 POST /api/debug/duplicate_deck {0}; undo; redo   VALID numDecks 3 -> 2 -> 3
#   L5 show deck 1 (the loaded nine-rows deck), fire its row-1 clip, show deck 0, POST /api/debug/remove_deck {1}
#      VALID numDecks 2, retiredDeckCount 1, layers[1].activeClip.retired true; then undo: numDecks 3,
#      retiredDeckCount 0, not retired
#   L6 POST /api/load_composition bf9b-check.json (the model swap; LAST: it unbinds the Layer tab)
#      VALID 20 decks loaded, inspected_layer ""
# VERDICT (last line), exactly one of:
#   PROBE-ASAN-LIVE GREEN (7 steps, 0 INVALID, 0 "ERROR: AddressSanitizer", app alive at the end)      exit 0
#   PROBE-ASAN-LIVE RED (step L<k>)              a report file <out>/asan.* or a dead app                exit 1
#   PROBE-ASAN-LIVE INVALID (step L<k>: <clause>)                                                        exit 2
#   PROBE-ASAN-LIVE BLOCKED (<reason>)           the ASan app is not built or does not start             exit 3
# BLOCKED is never a pass (ruling section 5 rule 1). INVALID is RED (rule 2).
# FAILING ARM (ruling AM-6, fact FM-3): the same build dir with the wiring line `undoService_.onLayerStackMoved = ...`
# deleted (mutant MU3, never committed) must print `PROBE-ASAN-LIVE RED (step L1)`.
# WHAT THIS ROW DOES NOT REACH: the Clip inspector and Layer > Add / Remove Layer -- unit cases AS3, AS3b, AS4, AS5,
# AS6, AS7 under probe-asan-unit.sh only.
#
# Screen-safe: open -g only, no Output window, no synthetic input, no screen capture. QUITS ONLY THE APP IT LAUNCHED
# (quit_ours, .harmony/probe-quit-ours.sh); REFUSES when an Audio-DNA is already running. HTTP: Connection: close.
# PROBE RIG GATE: refuses (exit 64) unless /tmp/audiodna-live.lock/owner exists; if AUDIODNA_LOCK_OWNER is set it must
# match the owner file's first field.
set -u
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held -- mkdir /tmp/audiodna-live.lock && echo \"<lane> \$\$ \$(date +%s)\" > $LOCK_OWNER_FILE first"; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
fi
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
. "$ROOT/.harmony/probe-quit-ours.sh"   # refuse_foreign_start / record_ourpid / quit_ours
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${ASAN_LIVE_APP:-$ROOT/build-asan-app/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app}"
PY="${ASAN_LIVE_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
blocked() { echo; echo "PROBE-ASAN-LIVE BLOCKED ($1)"; exit 3; }

refuse_foreign_start || exit 64
for PORT in 7070 8080; do
  lsof -nP -iTCP:$PORT -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $PORT already has a listener"; exit 64; }
done
[ -n "$PY" ] && "$PY" -c 'import PIL, requests' 2>/dev/null || { echo "REFUSE: no python with Pillow + requests (set ASAN_LIVE_PY)"; exit 64; }
command -v ffmpeg >/dev/null 2>&1 || { echo "REFUSE: ffmpeg not on PATH (make-bf9b-check.py needs it)"; exit 64; }
BIN="$APP/Contents/MacOS/Audio-DNA"
[ -x "$BIN" ] || blocked "no ASan app at $APP -- build it, or set ASAN_LIVE_APP"
otool -L "$BIN" 2>/dev/null | grep -q 'libclang_rt.asan' || blocked "the app at $APP is not linked with AddressSanitizer"

BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/asan-live.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
"$PY" "$ROOT/.harmony/make-bf9b-check.py" "$OUT/check" > "$OUT/fixtures.log" 2>&1 \
  || { echo "REFUSE: make-bf9b-check.py failed (see $OUT/fixtures.log)"; exit 64; }
[ -f "$OUT/check/nine-rows.json" ] && [ -f "$OUT/check/bf9b-check.json" ] || { echo "REFUSE: fixtures missing in $OUT/check"; exit 64; }

open -g --env ADNA_INSPECT_LAYER=1 --env "ASAN_OPTIONS=abort_on_error=0:halt_on_error=1:log_path=$OUT/asan" \
  --stdout "$OUT/out.log" --stderr "$OUT/err.log" "$APP" --args --test-mode
record_ourpid; echo "ours: pid ${OURPID:-none}"
UP=0; for _ in $(seq 1 120); do
  [ "$(curl -s -o /dev/null -w '%{http_code}' --max-time 2 -H 'Connection: close' "$A/api/health")" = "200" ] && { UP=1; break; }
  ours_running || break
  sleep 1
done
reports() { cat "$OUT"/asan.* "$OUT/err.log" "$OUT/out.log" 2>/dev/null | grep -c 'ERROR: AddressSanitizer'; }
if [ "$UP" -ne 1 ]; then
  N="$(reports)"; ls "$OUT"/asan.* >/dev/null 2>&1 && head -5 "$OUT"/asan.* | sed 's/^/      /'
  quit_ours >/dev/null
  blocked "the ASan app did not answer /api/health within 120 s ($N ASan report line(s) at startup; out: $OUT)"
fi
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
[ "$L7070" = "Audio-DNA" ] || { quit_ours >/dev/null; blocked "port 7070 is answered by '$L7070', not Audio-DNA"; }
sleep 2

"$PY" - "$OUT" "$A" <<'PYEOF' | tee "$OUT/py.log"
import glob, json, os, sys, time
import requests

OUT, A = sys.argv[1], sys.argv[2]
S = requests.Session()
S.headers["Connection"] = "close"   # one fresh connection per request (the app's server drops idle ones)
NINE = os.path.join(OUT, "check", "nine-rows.json")
SHOW = os.path.join(OUT, "check", "bf9b-check.json")


def say(msg):
    print(msg, flush=True)


def get(path):
    return S.get(A + path, timeout=8).json()


def post(path, body=None):
    r = S.post(A + path, json=body or {}, timeout=15)
    return r.status_code


def asan_lines():
    n, first = 0, ""
    for p in sorted(glob.glob(os.path.join(OUT, "asan.*"))) + [os.path.join(OUT, "err.log"), os.path.join(OUT, "out.log")]:
        try:
            for ln in open(p, errors="replace"):
                if "ERROR: AddressSanitizer" in ln:
                    n += 1
                    first = first or f"{os.path.basename(p)}: {ln.strip()[:200]}"
        except OSError:
            pass
    return n, first


def alive():
    try:
        return S.get(A + "/api/health", timeout=5).status_code == 200
    except Exception:  # noqa: BLE001
        return False


def red(step, why):
    n, first = asan_lines()
    say(f"RED   {step}: {why}; {n} \"ERROR: AddressSanitizer\" line(s){': ' + first if first else ''}")
    say(f"VERDICT RED {step}")
    sys.exit(1)


def invalid(step, clause):
    say(f"INVALID  {step}: {clause}")
    say(f"VERDICT INVALID {step}: {clause}")
    sys.exit(2)


def facts():
    c = get("/api/composition")
    u = get("/api/debug/ui_text")
    return {"layers": len(c.get("layers") or []), "numDecks": len(c.get("decks") or []),
            "retired": c.get("retiredDeckCount"), "active": c.get("activeDeck"),
            "l1": ((c.get("layers") or [{}, {}])[1].get("activeClip") or {}) if len(c.get("layers") or []) > 1 else {},
            "decks": [d.get("name") for d in c.get("decks") or []],
            "layer": u.get("inspected_layer"), "tab": u.get("inspector_tab"), "clip": u.get("inspected_clip")}


def label():
    """The top text line, read on the MESSAGE thread (/api/debug/ui_text): safe to poll while a staged load runs."""
    try:
        return str(get("/api/debug/ui_text").get("file_label", ""))
    except Exception:  # noqa: BLE001
        return ""


def settle(step, want, limit=6.0, done_label=None, label_limit=15.0):
    """The step's wait: [a staged command: until the top text line holds `done_label`], then 1.0 s, GET /api/health ==
    200 and no report file; then the facts. A dead app or a report is RED before any VALID clause is judged.
    GET /api/composition is served on the http thread and reads the deck list unlocked, so it is NEVER polled while a
    command may still be changing the model (a poll that met Duplicate Deck's push_back was an ASan
    container-overflow in ApiServer::handleComposition, 2026-10-03): it is read once after the 1.0 s, and again
    only at 1 s steps (<= limit) when `want` does not hold yet."""
    if done_label:
        t0 = time.time()
        while time.time() - t0 < label_limit and done_label not in label():
            if asan_lines()[0] or not alive():
                break
            time.sleep(0.25)
    t0 = time.time()
    while True:
        time.sleep(1.0)
        if asan_lines()[0]:
            red(step, "an AddressSanitizer report was written")
        if not alive():
            red(step, "the app does not answer /api/health (dead app)")
        try:
            f = facts()
        except Exception as e:  # noqa: BLE001
            red(step, f"the app stopped answering ({e})")
        if asan_lines()[0]:
            red(step, "an AddressSanitizer report was written")
        if want(f) or time.time() - t0 >= limit:
            return f


def need(step, f, **want):
    for k, v in want.items():
        if f.get(k) != v:
            invalid(step, f"{k} == {json.dumps(v)} (got {json.dumps(f.get(k))})")


def bound(step, f):
    need(step, f, layer="Layer 2", tab="Layer")


# L0 -- the lever ran: the Layer tab is the active tab and is bound to layer index 1
f = settle("L0", lambda x: x["layer"] == "Layer 2" and x["tab"] == "Layer")
bound("L0", f); need("L0", f, layers=3, numDecks=1)
say(f"PASS  L0: inspected_layer \"Layer 2\", inspector_tab \"Layer\", default show (layers 3, numDecks 1)")

# L1 -- Load Deck of a 9-row deck: the layer stack grows 3 -> 9 (its storage moves)
post("/api/debug/load_deck", {"path": NINE})
f = settle("L1", lambda x: x["layers"] == 9 and x["numDecks"] == 2, done_label="Loaded deck:")
need("L1", f, layers=9, numDecks=2); bound("L1", f)
say(f"PASS  L1: load_deck nine-rows.json -> layers 9, numDecks 2, inspected_layer \"Layer 2\", decks {f['decks']}")

# L2 -- Undo: 9 -> 3
post("/api/debug/undo", {"redo": False})
f = settle("L2", lambda x: x["layers"] == 3 and x["numDecks"] == 1)
need("L2", f, layers=3, numDecks=1); bound("L2", f)
say("PASS  L2: undo -> layers 3, numDecks 1, inspected_layer \"Layer 2\"")

# L3 -- Redo: 3 -> 9
post("/api/debug/undo", {"redo": True})
f = settle("L3", lambda x: x["layers"] == 9 and x["numDecks"] == 2)
need("L3", f, layers=9, numDecks=2); bound("L3", f)
say("PASS  L3: redo -> layers 9, numDecks 2, inspected_layer \"Layer 2\"")

# L4 -- Duplicate Deck 0; undo; redo
post("/api/debug/duplicate_deck", {"deck": 0})
f = settle("L4", lambda x: x["numDecks"] == 3, done_label="Duplicated deck:"); need("L4", f, numDecks=3)
post("/api/debug/undo", {"redo": False})
f = settle("L4", lambda x: x["numDecks"] == 2); need("L4", f, numDecks=2)
post("/api/debug/undo", {"redo": True})
f = settle("L4", lambda x: x["numDecks"] == 3); need("L4", f, numDecks=3)
say(f"PASS  L4: duplicate_deck 0 / undo / redo -> numDecks 3 -> 2 -> 3, decks {f['decks']}")

# L5 -- deck 1 (the loaded nine-rows deck): its row-1 clip plays, deck 0 is shown, Remove Deck retires it; Undo
# restores it
nine = 1
post("/api/switch_deck", {"deck": nine})
settle("L5", lambda x: x["active"] == nine)
post("/api/trigger_clip", {"layer": 1, "column": 0})
f = settle("L5", lambda x: bool(x["l1"]) and x["l1"].get("column") == 0)
if not (f["l1"] and f["l1"].get("column") == 0 and f["l1"].get("retired") is False):
    invalid("L5", f"layers[1] plays deck 1's row-1 clip before the removal (activeClip {json.dumps(f['l1'])})")
post("/api/switch_deck", {"deck": 0})
settle("L5", lambda x: x["active"] == 0)
post("/api/debug/remove_deck", {"deck": nine})
f = settle("L5", lambda x: x["numDecks"] == 2 and x["retired"] == 1)
need("L5", f, numDecks=2, retired=1)
if f["l1"].get("retired") is not True:
    invalid("L5", f"layers[1].activeClip.retired == true (activeClip {json.dumps(f['l1'])})")
post("/api/debug/undo", {"redo": False})
f = settle("L5", lambda x: x["numDecks"] == 3 and x["retired"] == 0)
need("L5", f, numDecks=3, retired=0)
if f["l1"].get("retired") is not False or f["l1"].get("column") != 0:
    invalid("L5", f"layers[1].activeClip not retired after the undo (activeClip {json.dumps(f['l1'])})")
say(f"PASS  L5: remove_deck {nine} (its row-1 clip playing) -> numDecks 2, retiredDeckCount 1, "
    f"layers[1].activeClip.retired true; undo -> numDecks 3, retiredDeckCount 0, not retired")

# L6 -- the model swap (unbinds the Layer tab)
code = post("/api/load_composition", {"path": SHOW})
f = settle("L6", lambda x: x["numDecks"] == 20 and x["layer"] == "", done_label="Loaded: bf9b-check", label_limit=40.0)
if code != 200:
    invalid("L6", f"POST /api/load_composition == 200 (got {code})")
need("L6", f, numDecks=20, layers=3, layer="")
say("PASS  L6: load_composition bf9b-check.json -> 20 decks, 3 layers, inspected_layer \"\"")
time.sleep(1.0)
if not alive():
    red("L6", "the app does not answer /api/health at the end (dead app)")
say("VERDICT GREEN")
PYEOF
PRC=${PIPESTATUS[0]}
ALIVE=0; ours_running && [ "$(curl -s -o /dev/null -w '%{http_code}' --max-time 3 -H 'Connection: close' "$A/api/health")" = "200" ] && ALIVE=1
NREP="$(reports)"
quit_ours; QRC=$?
if ours_running; then echo "FAIL  app still running (pid $OURPID)"; else echo "      app terminated (quit_ours rc $QRC)"; fi
NREP_AFTER="$(reports)"
for f in "$OUT"/asan.*; do [ -f "$f" ] && { echo "      report file $f:"; grep -m 3 -E 'ERROR: AddressSanitizer|^ *#[0-3] |freed by|is located' "$f" | cut -c1-220 | sed 's/^/        /'; }; done
V="$(sed -n 's/^VERDICT //p' "$OUT/py.log" | tail -1)"
echo
case "$V" in
  GREEN)
    if [ "$PRC" -eq 0 ] && [ "$ALIVE" -eq 1 ] && [ "$NREP" -eq 0 ] && [ "$NREP_AFTER" -eq 0 ] && ! ours_running; then
      echo 'PROBE-ASAN-LIVE GREEN (7 steps, 0 INVALID, 0 "ERROR: AddressSanitizer", app alive at the end)'; exit 0
    fi
    echo "      after the last step: alive $ALIVE, report lines $NREP, after the quit $NREP_AFTER"
    echo "PROBE-ASAN-LIVE RED (step L6)"; exit 1 ;;
  RED\ L[0-6]) echo "PROBE-ASAN-LIVE RED (step ${V#RED })"; exit 1 ;;
  INVALID\ L[0-6]:*) echo "PROBE-ASAN-LIVE INVALID (step ${V#INVALID })"; exit 2 ;;
  *) # the step script died without a verdict: never a pass
    echo "      no verdict from the step script (rc $PRC); report lines $NREP_AFTER"
    echo "PROBE-ASAN-LIVE RED (step L?)"; exit 1 ;;
esac
