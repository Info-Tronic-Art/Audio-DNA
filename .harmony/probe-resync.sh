#!/bin/bash
# probe-resync.sh -- live witness for s-rta-0925 call 9: a MANUAL Resync (TopBar button /
# bound key or pad / REST POST /api/resync / OSC /audiodna/resync / take replay) restarts every
# tempo-synced shape at the new downbeat, while AUTOMATIC phrase resets (drop entry / breakdown
# exit) keep flowing exactly as before (ruling 25, not drivable live -- see V6). Recipe:
# .harmony/.reports/s-rta-0925/plan-resync.md section 3.8.
# Written by the Builder, NOT run by the Builder (Harmony runs it after merge, like
# probe-downbeat-level.sh). Validate: bash -n; shellcheck (if installed).
#
# ORACLE: /api/set_bpm puts BPMTracker in manual mode (predicted phase wrap, no audio content
# needed). The fixture layer's opacity is driven by a default-mode (resetPhaseOnStructural=false)
# 16-beat SawUp Lfo -- one full ramp every 8 seconds at 120 BPM. Before the fix, this shape folds
# across raw totalBarCount (never reset by a Resync), so a click mid-transport snaps opacity to
# wherever (totalBarCount mod 4)/4 happens to be -- NOT to 0. After the fix, opacity reads ~0 in
# the first sample after the click and ramps 0->1 over the following 8 seconds from there.
#
# RIG FACTS (same as probe-downbeat-level.sh / probe-lane3.sh -- do not re-derive):
#   * ApiServer = http://127.0.0.1:7070 (IPv4 ONLY), PRODUCTION mode, NO --test-mode (test mode
#     never starts AnalysisThread -- requestResync() would never be applied; probe-tempo-silence.sh's
#     lesson, cited in the downbeat probe header, applies here too -- see plan section 3.0).
#   * Launch via `open`; every pgrep uses the bracket trick 'MacOS/Audio-DN[A]'.
#   * SCREEN-SAFETY LAW: never open the Output window; graceful osascript quit first, pkill last.
set -u
OUT="${1:-/tmp/audiodna-resync}"; mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${RESYNC_BUILD_DIR:-build-lane}"
APPBUNDLE="$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app"
VENV_PY="${RESYNC_VENV_PY:-$ROOT/.venv/bin/python}"
FIXTURE="$ROOT/.harmony/probe-resync-layer.json"
A='http://127.0.0.1:7070'
BPM=120
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
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

# --- 1. preconditions + production launch (probe-downbeat-level.sh section 1, verbatim) --------
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE (set RESYNC_BUILD_DIR to override)"; exit 64; }
: > "$OUT/adna-out.log"; : > "$OUT/adna-err.log"      # open --stdout/--stderr APPEND: clear first
open -g --stdout "$OUT/adna-out.log" --stderr "$OUT/adna-err.log" "$APPBUNDLE"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: health never came up on $A (TCC mic prompt? screencapture -x and LOOK)"; exit 1; }
PID="$(pgrep -f 'MacOS/Audio-DN[A]' | head -1)"
[ -n "$PID" ] || { echo "FAIL: health answered but no Audio-DNA process was found"; exit 1; }
ok "app launched (production), /api/health answered, PID=$PID"

# --- 2. manual tempo: predicted beats, no audio content needed ------------------------------------
curl -s --max-time 6 -X POST "$A/api/set_bpm" -H 'Content-Type: application/json' -d "{\"bpm\":$BPM}" >/dev/null
sleep 1
B="$(jget "$A/api/bpm" "round(d['bpm'], 1)")"
[ "$B" = "$BPM.0" ] && ok "manual tempo took (bpm=$B)" || no "manual tempo did not take (bpm=$B)"

# --- 3. load fixture, trigger the clip, confirm the deck is engaged ------------------------------
curl -s --max-time 6 -X POST "$A/api/load_composition" -H 'Content-Type: application/json' \
  -d "{\"path\":\"$FIXTURE\"}" | grep -q '"ok":[[:space:]]*true' \
  && ok "load_composition accepted the resync fixture" || no "load_composition rejected the resync fixture"
sleep 1
curl -s --max-time 6 -X POST "$A/api/trigger_clip" -H 'Content-Type: application/json' -d '{"layer":0,"column":0}' >/dev/null
sleep 1
CONNECTED="$(jget "$A/api/composition" "'yes' if 'opacity' in d['decks'][0]['layers'][0].get('connected', []) else 'no'")"
[ "$CONNECTED" = "yes" ] && ok "layers[0].connected contains 'opacity' (deck engaged)" \
  || no "layers[0].connected does NOT contain 'opacity' -- the fixture did not load, everything below is void"

# --- 4. FIELD GATE: /api/bpm exposes resyncBarOrigin as an integer --------------------------------
ORIGIN_TYPE="$(jget "$A/api/bpm" "isinstance(d.get('resyncBarOrigin'), int)")"
[ "$ORIGIN_TYPE" = "True" ] && ok "/api/bpm exposes resyncBarOrigin (int)" \
  || no "/api/bpm has no int resyncBarOrigin ($ORIGIN_TYPE) -- pre-fix build?"

# --- 5. ENDPOINT GATE: POST /api/resync answers ok:true within 1s --------------------------------
RESYNC_OK="$(curl -s --max-time 1 -X POST "$A/api/resync" | grep -o '"ok"[[:space:]]*:[[:space:]]*true')"
[ -n "$RESYNC_OK" ] && ok "POST /api/resync answers ok:true within 1s" \
  || no "POST /api/resync did not answer ok:true within 1s (404 on a pre-fix build?)"

if [ "$CONNECTED" != "yes" ] || [ "$ORIGIN_TYPE" != "True" ]; then
    no "steps 6-7 skipped -- a prerequisite gate above failed"
else

# --- 6. poll /api/bpm at 10Hz until totalBarCount % 4 == 2 && beatInBar == 2 ----------------------
python3 - "$A" > "$OUT/wait1.txt" <<'PY'
import json, sys, time, urllib.request
url = sys.argv[1] + '/api/bpm'
def get():
    with urllib.request.urlopen(url, timeout=2) as r:
        return json.load(r)
t_end = time.monotonic() + 8.5
while time.monotonic() < t_end:
    d = get()
    if d['totalBarCount'] % 4 == 2 and d['beatInBar'] == 2:
        print('PASS  reached totalBarCount%4==2, beatInBar==2 within 8.5s '
              '(16-beat SawUp sits mid-cycle, ~0.625 pre-fix)')
        sys.exit(0)
    time.sleep(0.1)
print('FAIL  never reached totalBarCount%4==2, beatInBar==2 within 8.5s')
PY
cat "$OUT/wait1.txt"
PASS=$((PASS + $(grep -c '^PASS ' "$OUT/wait1.txt"))); FAIL=$((FAIL + $(grep -c '^FAIL ' "$OUT/wait1.txt")))

# --- 7. POST /api/resync; sample live.opacity + /api/bpm every 40ms for 4.5s; verdicts V1-V4 ------
python3 - "$A" "$OUT" > "$OUT/verdicts.txt" <<'PY'
import json, sys, time, urllib.request

A, OUT = sys.argv[1], sys.argv[2]

def post_resync():
    req = urllib.request.Request(A + '/api/resync', method='POST')
    with urllib.request.urlopen(req, timeout=6) as r:
        return json.load(r)

def sample():
    with urllib.request.urlopen(A + '/api/bpm', timeout=2) as r:
        bpm = json.load(r)
    with urllib.request.urlopen(A + '/api/composition', timeout=2) as r:
        comp = json.load(r)
    opacity = comp['decks'][0]['layers'][0]['live']['opacity']
    return bpm, opacity

t_post = time.time()
post_resync()

samples = []
t_end = time.monotonic() + 4.5
while time.monotonic() < t_end:
    ts = time.time()
    try:
        bpm, opacity = sample()
        samples.append({'ts': ts, 'elapsed_ms': (ts - t_post) * 1000.0, 'opacity': opacity, **bpm})
    except Exception as e:
        samples.append({'ts': ts, 'elapsed_ms': (ts - t_post) * 1000.0, 'error': str(e)})
    time.sleep(0.04)

with open(OUT + '/samples.json', 'w') as f:
    json.dump({'t_post': t_post, 'samples': samples}, f, indent=1)

def at(lo, hi):
    return [s for s in samples if lo <= s['elapsed_ms'] <= hi and 'error' not in s]

def v(cond, msg):
    print(('PASS  ' if cond else 'FAIL  ') + msg)

# V1 tracker: the first sample >= 60ms after the POST has resyncBarOrigin == totalBarCount,
# beatInBar == 0, barCount == 0, downbeatDetected == true, beatPhase < 0.25.
win = at(60, 250)
origin_v1 = None
if not win:
    v(False, "V1 tracker: no sample landed in the 60-250ms window")
else:
    s0 = win[0]
    origin_v1 = s0['resyncBarOrigin']
    v1 = (s0['resyncBarOrigin'] == s0['totalBarCount'] and s0['beatInBar'] == 0
          and s0['barCount'] == 0 and s0['downbeatDetected'] is True and s0['beatPhase'] < 0.25)
    v(v1, "V1 tracker: first sample >=60ms after POST is the new downbeat "
          "(origin=%s totalBarCount=%s beatInBar=%s barCount=%s downbeatDetected=%s beatPhase=%.4f)"
          % (s0['resyncBarOrigin'], s0['totalBarCount'], s0['beatInBar'], s0['barCount'],
             s0['downbeatDetected'], s0['beatPhase']))

# V2 THE RULING: live.opacity in the first sample >=60ms and <=200ms after the POST is < 0.10.
win2 = at(60, 200)
if not win2:
    v(False, "V2 THE RULING: no sample landed in the 60-200ms window")
else:
    s = win2[0]
    v(s['opacity'] < 0.10, "V2 THE RULING: live.opacity at %.0fms after Resync is %.4f "
                            "(< 0.10 expected -- pre-fix reads ~0.6)" % (s['elapsed_ms'], s['opacity']))

# V3 runs from the NEW downbeat: at +2.0s (+/-0.1) opacity in [0.22,0.30]; at +4.0s in [0.47,0.55].
win3a = at(1900, 2100)
win3b = at(3900, 4100)
if win3a:
    o = win3a[len(win3a) // 2]['opacity']
    v(0.22 <= o <= 0.30, "V3a runs from the new downbeat: opacity at ~2.0s is %.4f (expect [0.22,0.30])" % o)
else:
    v(False, "V3a: no sample near +2.0s")
if win3b:
    o = win3b[len(win3b) // 2]['opacity']
    v(0.47 <= o <= 0.55, "V3b runs from the new downbeat: opacity at ~4.0s is %.4f (expect [0.47,0.55])" % o)
else:
    v(False, "V3b: no sample near +4.0s")

# V4 no phantom bar: at +1.0s totalBarCount == resyncBarOrigin; at +2.3s totalBarCount ==
# resyncBarOrigin+1 and resyncBarOrigin unchanged from V1.
win4a = at(900, 1100)
win4b = at(2200, 2400)
if win4a and origin_v1 is not None:
    s = win4a[len(win4a) // 2]
    v(s['totalBarCount'] == origin_v1,
      "V4a no phantom bar: totalBarCount at ~1.0s == origin (%s == %s)" % (s['totalBarCount'], origin_v1))
else:
    v(False, "V4a: no sample near +1.0s (or V1 origin unavailable)")
if win4b and origin_v1 is not None:
    s = win4b[len(win4b) // 2]
    v(s['totalBarCount'] == origin_v1 + 1 and s['resyncBarOrigin'] == origin_v1,
      "V4b one bar after Resync: totalBarCount == origin+1, resyncBarOrigin unchanged "
      "(totalBarCount=%s origin=%s, expected origin=%s)" % (s['totalBarCount'], s['resyncBarOrigin'], origin_v1))
else:
    v(False, "V4b: no sample near +2.3s (or V1 origin unavailable)")

print("INFO  %d samples captured over ~4.5s (artifacts in %s/samples.json)" % (len(samples), OUT))
PY
cat "$OUT/verdicts.txt"
PASS=$((PASS + $(grep -c '^PASS ' "$OUT/verdicts.txt"))); FAIL=$((FAIL + $(grep -c '^FAIL ' "$OUT/verdicts.txt")))

# --- V5 parity independence: repeat steps 6-7 once with totalBarCount % 4 == 3 -------------------
python3 - "$A" > "$OUT/verdicts-v5.txt" <<'PY'
import json, sys, time, urllib.request
A = sys.argv[1]

def get(path):
    with urllib.request.urlopen(A + path, timeout=2) as r:
        return json.load(r)

t_end = time.monotonic() + 8.5
reached = False
while time.monotonic() < t_end:
    d = get('/api/bpm')
    if d['totalBarCount'] % 4 == 3:
        reached = True
        break
    time.sleep(0.1)

if not reached:
    print("FAIL  V5 parity independence: never reached totalBarCount%4==3 within 8.5s")
    sys.exit(0)

req = urllib.request.Request(A + '/api/resync', method='POST')
with urllib.request.urlopen(req, timeout=6) as r:
    json.load(r)

t_post = time.time()
samples = []
t_end2 = time.monotonic() + 0.3
while time.monotonic() < t_end2:
    ts = time.time()
    bpm = get('/api/bpm')
    samples.append({'elapsed_ms': (ts - t_post) * 1000.0, **bpm})
    time.sleep(0.04)

win = [s for s in samples if 60 <= s['elapsed_ms'] <= 250]
if not win:
    print("FAIL  V5 parity independence: no sample landed in the 60-250ms window (second Resync)")
else:
    s0 = win[0]
    ok5 = (s0['resyncBarOrigin'] == s0['totalBarCount'] and s0['beatInBar'] == 0
           and s0['barCount'] == 0 and s0['downbeatDetected'] is True)
    tag = 'PASS' if ok5 else 'FAIL'
    print("%s  V5 parity independence: same V1 shape reached from totalBarCount%%4==3 "
          "(origin=%s totalBarCount=%s beatInBar=%s barCount=%s downbeatDetected=%s)"
          % (tag, s0['resyncBarOrigin'], s0['totalBarCount'], s0['beatInBar'], s0['barCount'],
             s0['downbeatDetected']))
PY
cat "$OUT/verdicts-v5.txt"
PASS=$((PASS + $(grep -c '^PASS ' "$OUT/verdicts-v5.txt"))); FAIL=$((FAIL + $(grep -c '^FAIL ' "$OUT/verdicts-v5.txt")))

# V6 (INFO, not a verdict): the automatic-reset half of this ruling is not drivable live in
# production -- structural transitions come from real audio and are suppressed in manual mode
# (BPMTracker.cpp updatePhrase's !predictedBeatRegime_ guard). Unit test T3d
# ("automatic structural reset leaves resyncBarOrigin untouched (real onsets)",
# tests/test_bpm_stabilization.cpp) carries that half.
echo "INFO  V6: automatic-reset half (drop/breakdown never touches resyncBarOrigin) is NOT drivable live in production -- manual mode suppresses structural transitions; unit test T3d carries it."
fi

# --- 8. teardown (probe-downbeat-level.sh section 5, verbatim) -----------------------------------
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
    if [ "$W" = "0" ] || [ -z "$W" ]; then ok "0 Audio-DNA Output windows in the FULL window list (SCREEN-SAFETY LAW held)"
    else no "$W Audio-DNA Output window(s) were open during the run -- screen-safety breach"; fi
else
    no "Output-window check: $VENV_PY (pyobjc/Quartz) unavailable -- set RESYNC_VENV_PY to the main checkout's .venv/bin/python"
fi
echo "no full-screen capture (Boris works on this Mac; s-rta-0925 habit): the Quartz window-list checks above are the screen witness"
echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
