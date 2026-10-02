#!/bin/bash
# probe-btguard.sh -- s-rta-0929b btguard (.harmony/.reports/s-rta-0929b/plan-btguard.md 4.10 + HARMONY ADOPTION BG1-BG9).
# Live witness that the app opens only allowed (wired / built-in) audio devices, that the no-input and no-device states
# launch, render, record and switch sources with a persistent plain-words notice, and that nothing crashes. No Bluetooth
# device is ever connected and no system setting is touched: the TEST-ONLY env ADNA_AUDIO_DENY_DEVICES makes the app
# treat the rig's own devices as denied -- it drives the guard's HIDING branch (lists, default, states), never the
# transport-classification branch (that one is unit-only: tests/test_device_policy.cpp).
#
# Arms (one launch each, production mode, no --test-mode, TEST_SERVER build):
#   A  no env               A0-A5, A7-A9; A6 / A6b opens == 1 after launch and after file -> input (s-rta-0930 bt2 C3);
#                           A10-A13 (bt2): the open mic denied at runtime -> input-lost re-apply (A10), allowed again ->
#                           adopted, never within the 10 s bound (A11a / A11), two list changes inside one settle -> nothing
#                           (A12, INFO A6c), a fresh scan -> nothing (A13a), audio_stop -> device-stopped re-apply (A13)
#   D  deny the opened OUTPUT only, when a second allowed output exists (BG8) -- SKIP otherwise (never counted)
#   B  deny the opened INPUT                         B0-B6 (B3b: a label-writing action keeps the notice, BG6);
#                                                    B7-B8 (bt2): allowed again at runtime -> adopted once (B7b opens 2)
#   C  deny the opened INPUT and OUTPUT (no device)  C0-C6 (C5: a take arms / stops with no device, C5b: it is saved without audio, BG7);
#                                                    C7-C8 (bt2): the output allowed -> opened (no-device re-apply), then
#                                                    the mic -> adopted after the 10 s bound (C8a / C8)
# s-rta-0930 bt2: TEST-ONLY POST /api/debug/audio_deny {"names": [...]} REPLACES the denied set at runtime and runs the
# guard's device-list-change path (inner rescan -> rebuild -> JUCE's listeners -> the reconciler); audio_stop stops the
# open device as JUCE's combiner does when its input dies; the device stays alive, so the dead-device close (R2) is not
# reproduced. The live rows = the POLICY path (the guard's rebuild -> JUCE's list handler -> the reconciler) plus A13's
# stop stand-in; the HAL side -- a dead input's combiner shutdown, its up-to-2-s close, per-device detail timers -- is
# covered by JUCE source reading (CA:1118-1127, CA:1735-1752, CA:734-757) and the mock cases (R8, R14, R16, R17) only;
# Boris check B2 is its hardware check.
# B / C take the device names from arm A's opened{} (vj S4); if arm A has no endpoint (the pre-change app) they fall
# back to BTGUARD_INPUT / BTGUARD_OUTPUT (default: the MacBook Pro built-in names).
#
# probe-async-load.sh's scaffolding: live-lock gate, REFUSE if Audio-DNA runs or 7070 has a listener, open -g with
# --stdout/--stderr/--env, health wait <= 60 s, graceful osascript quit with a 30 s clock (kill = FAIL), no new
# ~/Library/Logs/DiagnosticReports/Audio-DNA*.ips, 0 on-screen UserNotificationCenter windows. Screen-safe: no Output
# window, no synthetic input; window-only captures by Quartz window id only when BTGUARD_SHOTS=1.
#
# usage: probe-btguard.sh [out-base]
#   BTGUARD_APP   app bundle (default: <root>/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app)
#   BTGUARD_PY    python with requests-free stdlib + pyobjc Quartz (+ PIL for C3) (default: <root>/.venv, else main's)
#   BTGUARD_SHOTS 1 = save window-only PNGs of the main window (A normal, B, B after the label write, C) into <out>
# PROBE RIG GATE: refuses (exit 64) unless /tmp/audiodna-live.lock/owner exists; if AUDIODNA_LOCK_OWNER is set, it
# must match the owner file's first field.
set -u
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held"; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
fi
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
adna_kill() { local p; p="$(adna_pids)"; [ -n "$p" ] && kill $p 2>/dev/null; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${BTGUARD_APP:-$ROOT/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${BTGUARD_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set BTGUARD_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import Quartz, PIL' 2>/dev/null || { echo "REFUSE: no python with pyobjc Quartz + PIL (set BTGUARD_PY)"; exit 64; }
adna_running && { echo "REFUSE: Audio-DNA already running"; exit 64; }
lsof -nP -iTCP:7070 -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port 7070 already has a listener"; exit 64; }
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/btguard.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
touch "$OUT/start.stamp"
RESULTS="$OUT/results.txt"; : > "$RESULTS"
row() { echo "$1  $2" | tee -a "$RESULTS"; }   # row PASS|FAIL|SKIP|INFO "<id> <text>"

unc_count() {
  "$PY" -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName', ''))]))" 2>/dev/null || echo "?"
}
launch() {  # launch <arm> [env]
  if [ -n "${2:-}" ]; then
    open -g --stdout "$OUT/out-$1.log" --stderr "$OUT/err-$1.log" --env "$2" "$APP"
  else
    open -g --stdout "$OUT/out-$1.log" --stderr "$OUT/err-$1.log" "$APP"
  fi
  local up=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 -H 'Connection: close' "$A/api/health")" ] && { up=1; break; }; sleep 1; done
  sleep 2
  local l7070; l7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
  if [ "$up" -eq 1 ] && [ "$l7070" != "Audio-DNA" ]; then echo "port 7070 is answered by '$l7070', not Audio-DNA"; up=0; fi
  [ "$up" -eq 1 ]
}
quit_check() {  # quit_check <row-id>: graceful quit within 30 s, no new .ips, no dialog on screen
  local t0; t0=$(date +%s)
  osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1 &
  local osa=$!
  for _ in $(seq 1 30); do adna_running || break; sleep 1; done
  kill "$osa" 2>/dev/null; wait "$osa" 2>/dev/null
  local dt=$(( $(date +%s) - t0 )) bad=""
  if adna_running; then bad="still running $dt s after the quit (killed)"; adna_kill; sleep 3; fi
  sleep 2
  local ips; ips="$(find "$HOME/Library/Logs/DiagnosticReports" -name 'Audio-DNA*' -newer "$OUT/start.stamp" 2>/dev/null | head -3)"
  [ -n "$ips" ] && bad="$bad new crash report(s): $ips"
  local u; u="$(unc_count)"; [ "$u" = "0" ] || bad="$bad UserNotificationCenter windows on screen: $u"
  if [ -z "$bad" ]; then row PASS "$1 quit clean ($dt s, no .ips, 0 dialogs)"; else row FAIL "$1 quit:$bad"; fi
}
shot() {  # shot <name>: window-only capture of the main window by Quartz window id (BTGUARD_SHOTS=1)
  [ "${BTGUARD_SHOTS:-0}" = "1" ] || return 0
  "$PY" - "$OUT/shot-$1.png" <<'PYEOF'
import sys, Quartz
from Foundation import NSURL
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
cands = [w for w in wl if w.get('kCGWindowOwnerName') == 'Audio-DNA' and w.get('kCGWindowLayer') == 0
         and 'Output' not in str(w.get('kCGWindowName', ''))]
cands.sort(key=lambda w: -(w['kCGWindowBounds']['Width'] * w['kCGWindowBounds']['Height']))
if not cands:
    print('shot: no main window'); sys.exit(0)
wid = cands[0]['kCGWindowNumber']
img = Quartz.CGWindowListCreateImage(Quartz.CGRectNull, Quartz.kCGWindowListOptionIncludingWindow, wid,
                                     Quartz.kCGWindowImageBoundsIgnoreFraming)
url = NSURL.fileURLWithPath_(sys.argv[1])
dest = Quartz.CGImageDestinationCreateWithURL(url, 'public.png', 1, None)
Quartz.CGImageDestinationAddImage(dest, img, None)
Quartz.CGImageDestinationFinalize(dest)
print('shot: window %d -> %s' % (wid, sys.argv[1]))
PYEOF
}
rows() {  # rows <arm> [deny-input] [deny-output]: the HTTP rows of one arm (python; fresh connection per request)
  "$PY" - "$1" "$OUT" "${2:-}" "${3:-}" <<'PYEOF'
import json, os, re, sys, time, urllib.request
arm, out, deny_in, deny_out = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
A = 'http://127.0.0.1:7070'
res = open(os.path.join(out, 'results.txt'), 'a')
def row(v, text):
    line = '%s  %s' % (v, text); print(line); res.write(line + '\n'); res.flush()
def call(method, path, body=None, timeout=10):
    data = json.dumps(body).encode() if body is not None else (b'' if method == 'POST' else None)
    req = urllib.request.Request(A + path, data=data, method=method,
                                 headers={'Connection': 'close', 'Content-Type': 'application/json'})
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            return r.status, json.loads(r.read().decode() or 'null')
    except urllib.error.HTTPError as e:
        return e.code, None
    except Exception as e:
        return -1, str(e)
def devices():
    st, d = call('GET', '/api/debug/audio_devices')
    return (d if st == 200 and isinstance(d, dict) else None), st
def ui():
    st, d = call('GET', '/api/debug/ui_text')
    return d if st == 200 and isinstance(d, dict) else {}
def dev_row(d, name, key):
    return [r for r in d.get('devices', []) if r.get(key) == name]
def check(rid, cond, text, detail=''):
    row('PASS' if cond else 'FAIL', '%s %s%s' % (rid, text, '' if cond else '  [' + detail + ']'))
    return cond
NOTICE_IN = 'No wired mic found - plug one in. Bluetooth is never used.'
NOTICE_DEV = 'No audio device found - plug one in. Bluetooth is never used.'
WIRELESS = {'blue', 'blea', 'airp', 'ccwl', 'ccap'}
NOMIC = 'Mic: no wired mic (Bluetooth is never used)'
def mic(name, rate):
    return 'Mic: %s @ %dHz' % (name, int(rate or 0))
# s-rta-0930 bt2: the TEST-ONLY runtime deny (replaces the denied set; [] = none) and the device stop. Fresh connection each.
def deny(names):
    st, _ = call('POST', '/api/debug/audio_deny', {'names': names})
    return st
def poll(pred, seconds):   # devices() every 0.25 s until pred(d) or the deadline; the last answer either way
    t0 = time.time(); d = None
    while True:
        d, _ = devices()
        if d is not None and pred(d):
            return d, True
        if time.time() - t0 >= seconds:
            return d or {}, False
        time.sleep(0.25)
def brief(d):
    d = d or {}
    return 'reapplies %s last_reapply "%s" state %s opened %s lost_input "%s" opens %s' % (
        d.get('reapplies'), d.get('last_reapply', ''), d.get('state'), json.dumps(d.get('opened', {}), sort_keys=True),
        d.get('lost_input', ''), d.get('opens'))

if arm == 'A':
    d, st = devices()
    check('A1', d is not None and d.get('ok') is True, 'GET /api/debug/audio_devices 200 ok:true', 'HTTP %s' % st)
    d = d or {}
    op = d.get('opened', {}) or {}
    json.dump(d, open(os.path.join(out, 'arm-A-devices.json'), 'w'), indent=1)
    for rid, key, name_key in (('A2', 'input', 'name_in'), ('A3', 'output', 'name_out')):
        name = op.get(key, '')
        rows_ = dev_row(d, name, name_key) if name else []
        r = rows_[0] if rows_ else {}
        ok = bool(name) and len(rows_) == 1 and r.get('allowed') is True and r.get('transport_read_ok') is True \
             and r.get('transport') == 'bltn' and r.get('transport') not in WIRELESS
        check(rid, ok, 'opened %s "%s" is allowed, its transport read OK and is exactly bltn' % (key, name),
              'row %s' % json.dumps(r))
    sk = d.get('skipped', None)
    ok = isinstance(sk, list) and all(s.get('reason') and s.get('name') not in (op.get('input'), op.get('output')) for s in sk) \
         and d.get('unmapped') == []
    check('A4', ok, 'every skipped device has a reason and is not opened; unmapped == []',
          'skipped %s unmapped %s' % (sk, d.get('unmapped')))
    st, f = call('GET', '/api/features')
    ssr = (f or {}).get('sourceSampleRate') if isinstance(f, dict) else None
    check('A5', ssr is not None and op.get('sample_rate', 0) > 0 and abs(ssr - op.get('sample_rate', -1)) < 0.5,
          '/api/features sourceSampleRate == opened.sample_rate (%s)' % op.get('sample_rate'), 'features %s' % ssr)
    check('A6', d.get('opens') == 1, 'opens == 1 after launch (one device open per launch; reads %s, reapplies %s)'
          % (d.get('opens'), d.get('reapplies')), 'opens %s' % d.get('opens'))
    before = json.dumps(op, sort_keys=True)
    s1, _ = call('POST', '/api/audio/source', {'mode': 'file'}); time.sleep(1)
    s2, _ = call('POST', '/api/audio/source', {'mode': 'input'}); time.sleep(1)
    d2, _ = devices()
    after = json.dumps((d2 or {}).get('opened', {}), sort_keys=True)
    check('A7', s1 == 200 and s2 == 200 and d2 is not None and before == after,
          'source file -> input: opened{} byte-identical', 'before %s after %s' % (before, after))
    check('A6b', (d2 or {}).get('opens') == 1, 'opens == 1 after the source switches file -> input (reads %s)'
          % (d2 or {}).get('opens'), 'opens %s' % (d2 or {}).get('opens'))
    open(os.path.join(out, 'names.txt'), 'w').write('%s\n%s\n%d\n' % (op.get('input', ''), op.get('output', ''),
                                                                     len((d.get('lists') or {}).get('outputs', []))))
    # s-rta-0930 bt2: the reconciler rows. IN / OUTN = this arm's opened{}; the source is the mic (A7 left it on input).
    IN, OUTN = op.get('input', ''), op.get('output', '')
    if not IN:
        for rid in ('A10', 'A11a', 'A11', 'A12'):
            row('SKIP', '%s the open mic is denied / returns -- arm A opened no input' % rid)
    else:
        # A10: the open mic vanishes (denied at runtime) -> input-lost re-apply -> output-only + the notice + the label.
        st = deny([IN])
        d, ok = poll(lambda x: x.get('reapplies') == 1 and x.get('state') == 'no-input', 3)
        u = ui(); op10 = d.get('opened', {}) or {}
        check('A10', st == 200 and d.get('reapplies') == 1 and d.get('last_reapply', '') == 'input-lost'
              and d.get('state') == 'no-input' and op10.get('input') == '' and op10.get('output') == OUTN
              and d.get('lost_input', '') == '' and u.get('audio_notice') == NOTICE_IN and u.get('file_label') == NOMIC,
              'deny [IN] -> within 3 s one input-lost re-apply: output-only on "%s", the no-input notice, label "%s"'
              % (OUTN, NOMIC), 'HTTP %s %s ui %s' % (st, brief(d), json.dumps(u)))
        # A11a / A11: the mic returns -> adopt-input, but never within the 10 s bound of A10's re-apply.
        st = deny([]); t_post = time.time()
        time.sleep(2)
        d, _ = devices(); d = d or {}
        check('A11a', st == 200 and d.get('reapplies') == 1, 'deny [] -> at +2 s still 1 re-apply (the 10 s bound)',
              'HTTP %s %s' % (st, brief(d)))
        d, ok = poll(lambda x: x.get('reapplies') == 2, max(0.0, 13 - (time.time() - t_post)))
        t_ref = time.time()
        u = ui(); op11 = d.get('opened', {}) or {}
        st, f = call('GET', '/api/features')
        ssr = (f or {}).get('sourceSampleRate') if isinstance(f, dict) else None
        rate = op11.get('sample_rate', 0)
        check('A11', d.get('reapplies') == 2 and d.get('last_reapply', '') == 'adopt-input' and d.get('state') == 'ok'
              and op11.get('input') == IN and op11.get('output') == OUTN and d.get('lost_input', '') == ''
              and u.get('audio_notice') == '' and u.get('file_label') == mic(IN, rate)
              and ssr is not None and rate > 0 and abs(ssr - rate) < 0.5,
              'within 13 s: adopt-input re-apply onto "%s" / "%s", notice gone, label "%s", sourceSampleRate %s'
              % (IN, OUTN, mic(IN, rate), ssr), '%s ui %s features %s' % (brief(d), json.dumps(u), ssr))
        # A12 (AM4): two list changes inside one settle (the mic gone and back) -> nothing, measured against r0 / o0.
        a12 = None
        for attempt in (1, 2):
            time.sleep(max(0.0, 10.5 - (time.time() - t_ref)))
            d, _ = devices(); d = d or {}
            r0, o0 = d.get('reapplies'), json.dumps(d.get('opened', {}), sort_keys=True)
            s1 = deny([IN]); t1 = time.time()
            s2 = deny([]); t2 = time.time()
            gap = t2 - t1
            time.sleep(1.5)
            d, _ = devices(); d = d or {}
            o1 = json.dumps(d.get('opened', {}), sort_keys=True)
            a12 = (s1, s2, gap, r0, o0, d, o1)
            t_ref = time.time()
            if gap <= 0.15:
                break
            row('INFO', 'A12 attempt %d: the two POSTs were %.3f s apart (> 0.15 s) -- repeating once' % (attempt, gap))
        s1, s2, gap, r0, o0, d, o1 = a12
        check('A12', s1 == 200 and s2 == 200 and gap <= 0.15 and d.get('reapplies') == r0 and o1 == o0,
              'deny [IN] + deny [] %.3f s apart -> at +1.5 s still %s re-applies, opened{} byte-identical' % (gap, r0),
              'gap %.3f (> 0.15 = the two list changes did not land inside the 250 ms settle) r0 %s o0 %s now %s'
              % (gap, r0, o0, brief(d)))
        row('INFO', 'A6c opens after A12 = %s (expected 3: launch + A10 + A11)' % d.get('opens'))
    # A13a / A13 (AM14): the open combiner stops (the dead-input end state without the death) -> device-stopped re-apply.
    if not IN or IN == OUTN:
        row('SKIP', 'A13a a fresh scan with nothing changed -- arm A has no combined input + output device')
        row('SKIP', 'A13 audio_stop -> device-stopped re-apply -- arm A has no combined input + output device')
    else:
        d, _ = devices(); d = d or {}
        r0, o0 = d.get('reapplies'), d.get('opens')
        st = deny([])
        time.sleep(1)
        d, _ = devices(); d = d or {}
        check('A13a', st == 200 and d.get('reapplies') == r0, 'deny [] (a fresh scan, nothing changes) -> at +1 s still %s '
              're-applies' % r0, 'HTTP %s %s' % (st, brief(d)))
        st, _ = call('POST', '/api/debug/audio_stop')
        d, ok = poll(lambda x: x.get('reapplies') == (r0 or 0) + 1 and x.get('opens') == (o0 or 0) + 1, 3)
        op13 = d.get('opened', {}) or {}
        check('A13', st == 200 and d.get('reapplies') == (r0 or 0) + 1 and d.get('last_reapply', '') == 'device-stopped'
              and d.get('state') == 'ok' and op13.get('input') == IN and op13.get('output') == OUTN
              and d.get('lost_input', '') == '' and d.get('opens') == (o0 or 0) + 1,
              'audio_stop -> within 3 s one device-stopped re-apply back onto "%s" / "%s", opens %s -> %s'
              % (IN, OUTN, o0, d.get('opens')), 'HTTP %s %s' % (st, brief(d)))

elif arm == 'D':
    d, st = devices(); d = d or {}
    op = d.get('opened', {}) or {}
    check('D1', op.get('output') and op.get('output') != deny_out and d.get('state') in ('ok', 'no-input'),
          'default output "%s" denied -> another allowed output opened ("%s")' % (deny_out, op.get('output')), json.dumps(op))
    rows_ = dev_row(d, op.get('output', ''), 'name_out')
    check('D2', len(rows_) == 1 and rows_[0].get('allowed') is True, 'the opened output row is allowed', json.dumps(rows_))

elif arm == 'B':
    d, st = devices(); d = d or {}
    op = d.get('opened', {}) or {}
    r = dev_row(d, deny_in, 'name_in')
    check('B1', len(r) == 1 and r[0].get('allowed') is False and r[0].get('reason') == 'test-denied'
          and deny_in in d.get('test_denied', []), 'the mic "%s" is hidden: allowed:false reason test-denied' % deny_in,
          'HTTP %s row %s test_denied %s' % (st, r, d.get('test_denied')))
    check('B2', op.get('input') == '' and bool(op.get('output')) and d.get('state') == 'no-input'
          and op.get('input_channels') == 0, 'opened output-only: input "", output "%s", state no-input, 0 input channels'
          % op.get('output'), json.dumps(op) + ' state %s' % d.get('state'))
    u = ui()
    check('B3', u.get('audio_notice') == NOTICE_IN, 'the notice reads "%s"' % NOTICE_IN, json.dumps(u))
    open(os.path.join(out, 'b3-before-write.txt'), 'w').write(json.dumps(u))
elif arm == 'B3b':
    before = ui()
    s1, _ = call('POST', '/api/audio/source', {'mode': 'file'}); time.sleep(0.7)
    s2, _ = call('POST', '/api/audio/source', {'mode': 'input'}); time.sleep(0.7)
    u = ui()
    check('B3b', s1 == 200 and s2 == 200 and u.get('file_label', '').startswith('Mic:') and u.get('audio_notice') == NOTICE_IN,
          'a label-writing action (source file -> input: file label "%s") leaves the notice shown' % u.get('file_label'),
          'before %s after %s' % (json.dumps(before), json.dumps(u)))
elif arm == 'B4':
    t0 = time.time(); alive = True
    while time.time() - t0 < 10:
        st, b = call('GET', '/api/bpm')
        alive = alive and st == 200 and isinstance(b, dict)
        time.sleep(1)
    check('B4', alive, '/api/bpm readable every second for 10 s (app alive, output-only)')
    d, st = devices(); d = d or {}
    op = d.get('opened', {}) or {}
    st, f = call('GET', '/api/features')
    ssr = (f or {}).get('sourceSampleRate') if isinstance(f, dict) else None
    check('B5', ssr is not None and op.get('sample_rate', 0) > 0 and abs(ssr - op.get('sample_rate', -1)) < 0.5,
          'output-only device still clocks the analysis: sourceSampleRate == opened.sample_rate (%s)' % op.get('sample_rate'),
          'features %s opened %s' % (ssr, json.dumps(op)))
elif arm == 'B7':
    # s-rta-0930 bt2: the denied mic is allowed again (plugged in after launch) -> adopted (the source is the mic: B3b).
    st = deny([])
    d, ok = poll(lambda x: x.get('reapplies') == 1 and x.get('state') == 'ok', 3)
    u = ui(); op = d.get('opened', {}) or {}
    rate = op.get('sample_rate', 0)
    check('B7', st == 200 and d.get('reapplies') == 1 and d.get('last_reapply', '') == 'adopt-input' and d.get('state') == 'ok'
          and op.get('input') == deny_in and op.get('input_channels', 0) >= 1 and u.get('audio_notice') == ''
          and u.get('file_label') == mic(deny_in, rate),
          'deny [] -> within 3 s the mic "%s" is adopted (one adopt-input re-apply), notice gone, label "%s"'
          % (deny_in, mic(deny_in, rate)), 'HTTP %s %s ui %s' % (st, brief(d), json.dumps(u)))
    check('B7b', d.get('opens') == 2, 'opens == 2 (launch + the adoption; reads %s)' % d.get('opens'), brief(d))
    time.sleep(2)
    d, _ = devices(); d = d or {}
    check('B8', d.get('reapplies') == 1, 'after 2 more s still 1 re-apply', brief(d))

elif arm == 'C':
    d, st = devices(); d = d or {}
    op = d.get('opened', {}) or {}
    check('C1', d.get('state') == 'no-device' and op.get('input') == '' and op.get('output') == ''
          and deny_in in d.get('test_denied', []) and deny_out in d.get('test_denied', []),
          'nothing allowed: state no-device, both opened names empty', 'HTTP %s %s state %s' % (st, json.dumps(op), d.get('state')))
    u = ui()
    check('C2', u.get('audio_notice') == NOTICE_DEV, 'the notice reads "%s"' % NOTICE_DEV, json.dumps(u))
    png = os.path.join(out, 'c3-frame.png')
    sl, _ = call('POST', '/api/load_source', {'source_type': 'plasma'})   # a picture to decode (the standalone-source path)
    time.sleep(1.5)
    st, r = call('POST', '/api/render_frame', {'output_path': png}, timeout=30)
    info = ''
    ok = sl == 200 and st == 200 and isinstance(r, dict) and r.get('error') is None and os.path.exists(png)
    if ok:
        from PIL import Image
        im = Image.open(png).convert('RGB'); w, h = im.size
        b = im.resize((32, 18)).tobytes(); mean = sum(b) / float(len(b))
        info = '%dx%d mean %.1f' % (w, h, mean); ok = w > 0 and h > 0 and mean > 8.0
    check('C3', ok, 'POST /api/render_frame renders plasma with no audio device, decoded non-black (%s)' % info,
          'HTTP load %s render %s %s' % (sl, st, r))
    s1, _ = call('POST', '/api/audio/source', {'mode': 'file'}); time.sleep(0.7)
    s2, _ = call('POST', '/api/audio/source', {'mode': 'input'}); time.sleep(0.7)
    sh, _ = call('GET', '/api/health')
    check('C4', s1 == 200 and s2 == 200 and sh == 200, 'source file -> input answered 200, app alive', '%s %s %s' % (s1, s2, sh))
    name = 'btguard-probe-%d' % int(time.time())
    s1, r1 = call('POST', '/api/perf/record', {'name': name})
    time.sleep(2)
    s2, r2 = call('POST', '/api/perf/stop')
    time.sleep(2)
    s3, ps = call('GET', '/api/perf/status')
    sh, _ = call('GET', '/api/health')
    check('C5', s1 == 200 and s2 == 200 and s3 == 200 and sh == 200,
          'a take arms and stops with no audio device (HTTP %s / %s / status %s), app alive' % (s1, s2, s3),
          'record %s stop %s status %s' % (r1, r2, ps))
    open(os.path.join(out, 'c5-take.txt'), 'w').write(name + '\n' + json.dumps(ps) + '\n')
    # C5b (btguard-fix, BG7 load-bearing): /api/perf/record answers 200 before the message thread arms, so C5 alone
    # cannot see a refused or audio-carrying take. The saved take.json must exist, carry NO audio (mode "", no
    # segments, "audio" not in features) and the recorder must report no error.
    tj = os.path.join(os.path.expanduser('~/Documents/Audio-DNA/Takes'), name + '.adna-take', 'take.json')
    try:
        t = json.load(open(tj)); au = t.get('audio') or {}
        ok = au.get('mode', '') == '' and au.get('segments', []) == [] and 'audio' not in (t.get('features') or []) \
             and isinstance(ps, dict) and ps.get('lastError', '') == ''
        info = 'audio %s features %s lastError "%s"' % (json.dumps(au)[:160], t.get('features'), (ps or {}).get('lastError'))
    except Exception as e:
        ok, info = False, 'no take.json (%s); status %s' % (e, json.dumps(ps)[:200])
    check('C5b', ok, 'the no-device take was saved WITHOUT audio and the recorder reports no error', info)
    # s-rta-0930 bt2: nothing was allowed at launch; the output is plugged in after launch -> opened (no-device re-apply);
    # then the mic too -> adopted, never within the 10 s bound of the first re-apply.
    st = deny([deny_in])
    d, ok = poll(lambda x: x.get('reapplies') == 1 and x.get('state') == 'no-input', 3)
    u = ui(); op = d.get('opened', {}) or {}
    check('C7', st == 200 and d.get('reapplies') == 1 and d.get('last_reapply', '') == 'no-device'
          and d.get('state') == 'no-input' and op.get('output') == deny_out and op.get('input') == ''
          and u.get('audio_notice') == NOTICE_IN,
          'deny [IN] -> within 3 s one no-device re-apply opens "%s" output-only, the no-input notice' % deny_out,
          'HTTP %s %s ui %s' % (st, brief(d), json.dumps(u)))
    st = deny([]); t_post = time.time()
    time.sleep(2)
    d, _ = devices(); d = d or {}
    check('C8a', st == 200 and d.get('reapplies') == 1, 'deny [] -> at +2 s still 1 re-apply (the 10 s bound)',
          'HTTP %s %s' % (st, brief(d)))
    d, ok = poll(lambda x: x.get('reapplies') == 2, max(0.0, 13 - (time.time() - t_post)))
    u = ui(); op = d.get('opened', {}) or {}
    check('C8', d.get('reapplies') == 2 and d.get('last_reapply', '') == 'adopt-input' and d.get('state') == 'ok'
          and op.get('input') == deny_in and u.get('audio_notice') == '',
          'within 13 s: the mic "%s" is adopted (adopt-input), notice gone' % deny_in, '%s ui %s' % (brief(d), json.dumps(u)))
PYEOF
}

names_from_a() { sed -n "${1}p" "$OUT/names.txt" 2>/dev/null; }

echo "=== arm A: no env ($(date +%T))"
if launch A; then
  row PASS "A0 launch + /api/health"
  rows A
  shot A-normal
  n="$(grep -c '\[AudioEngine\] Switched to mic input mode' "$OUT/err-A.log" 2>/dev/null)"
  if [ "${n:-0}" -eq 2 ]; then row PASS "A8 stderr '[AudioEngine] Switched to mic input mode' x2 (launch + A7)"
  else row FAIL "A8 stderr '[AudioEngine] Switched to mic input mode' x${n:-0} (want 2)"; fi
else row FAIL "A0 app never answered /api/health"; fi
quit_check A9

IN="$(names_from_a 1)"; OUTN="$(names_from_a 2)"; NOUT="$(names_from_a 3)"
[ -n "$IN" ] || IN="${BTGUARD_INPUT:-MacBook Pro Microphone}"
[ -n "$OUTN" ] || OUTN="${BTGUARD_OUTPUT:-MacBook Pro Speakers}"
echo "device names for B / C / D: input '$IN', output '$OUTN' (allowed outputs in arm A: ${NOUT:-?})"

if [ "${NOUT:-0}" -ge 2 ]; then
  sleep 2; echo "=== arm D: deny the default output ($(date +%T))"
  if launch D "ADNA_AUDIO_DENY_DEVICES=$OUTN"; then rows D "" "$OUTN"; else row FAIL "D0 app never answered /api/health"; fi
  quit_check D9
else
  row SKIP "D1 deny only the default output while a second allowed output exists -- the rig lists ${NOUT:-0} allowed output(s)"
  row SKIP "D2 the opened output row is allowed -- no second output"
fi

sleep 2; echo "=== arm B: deny the input '$IN' ($(date +%T))"
if launch B "ADNA_AUDIO_DENY_DEVICES=$IN"; then
  row PASS "B0 launch + /api/health"
  rows B "$IN" ""
  shot B-no-input
  rows B3b "$IN" ""
  shot B-after-label-write
  rows B4 "$IN" ""
  rows B7 "$IN" ""
else row FAIL "B0 app never answered /api/health"; fi
quit_check B6

sleep 2; echo "=== arm C: deny '$IN' and '$OUTN' ($(date +%T))"
if launch C "ADNA_AUDIO_DENY_DEVICES=$IN;$OUTN"; then
  row PASS "C0 launch + /api/health"
  rows C "$IN" "$OUTN"
  shot C-no-device
else row FAIL "C0 app never answered /api/health"; fi
quit_check C6
# the take C5 made (no audio: nothing in the Audio Store) -- only this probe's own folder
TAKE="$(head -1 "$OUT/c5-take.txt" 2>/dev/null)"
[ -n "$TAKE" ] && [ -d "$HOME/Documents/Audio-DNA/Takes/$TAKE.adna-take" ] && rm -rf "$HOME/Documents/Audio-DNA/Takes/$TAKE.adna-take"

P=$(grep -c '^PASS' "$RESULTS"); F=$(grep -c '^FAIL' "$RESULTS"); S=$(grep -c '^SKIP' "$RESULTS")
echo; echo "PROBE-BTGUARD: $P PASS / $F FAIL / $S SKIP (SKIP never counts)"
[ "$F" -eq 0 ] && echo "PROBE-BTGUARD GREEN" || echo "PROBE-BTGUARD RED"
[ "$F" -eq 0 ]
