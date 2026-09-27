import sys
p = sys.argv[1]
s = open(p).read()
def rep(old, new, count=1):
    global s
    n = s.count(old)
    assert n == count, (old, n)
    s = s.replace(old, new)
rep('ROOT="$(cd "$(dirname "$0")/.." && pwd)"', 'ROOT="/Users/boriskarpman/projects/RealTimeAudio"   # TIMING COPY: main checkout (read-only)\nexport TIMING_LOG="$OUT/timing.jsonl"; : > "$TIMING_LOG"')
# rt.py: tlog helper
rep("A = 'http://127.0.0.1:7070'\n\ndef get(", "A = 'http://127.0.0.1:7070'\nTLOG = os.environ.get('TIMING_LOG')\ndef tlog(**kw):\n    if TLOG:\n        with open(TLOG, 'a') as f:\n            f.write(json.dumps(kw) + '\\n')\n\ndef get(")
# snap: timestamps after each GET + clockBeat
rep("    c = get('/api/composition')\n", "    c = get('/api/composition')\n    row['tc'] = time.time()\n")
rep("    s = get('/api/routine/status')\n    if isinstance(s.get('bank'), list):", "    s = get('/api/routine/status')\n    row['ts'] = time.time()\n    row['cb'] = s.get('clockBeat')\n    if isinstance(s.get('bank'), list):")
# wait: log
rep("            if s['bank'][slot]['state'] == want:\n                print('%.3f' % time.time()); sys.exit(0)",
    "            if s['bank'][slot]['state'] == want:\n                tw = time.time()\n                tlog(ev='wait', slot=slot, state=want, t=tw, cb=s.get('clockBeat'), pos=s['bank'][slot].get('position'))\n                print('%.3f' % tw); sys.exit(0)")
# edge: log
rep("                print('%.3f' % (t + (4.0 - d['beatInBar'] - d['beatPhase']) * 60.0 / d['bpm'])); sys.exit(0)",
    "                tlog(ev='edge', t=t, t_after=time.time(), beatInBar=d['beatInBar'], beatPhase=d['beatPhase'], bpm=d['bpm'], edge=t + (4.0 - d['beatInBar'] - d['beatPhase']) * 60.0 / d['bpm'])\n                print('%.3f' % (t + (4.0 - d['beatInBar'] - d['beatPhase']) * 60.0 / d['bpm'])); sys.exit(0)")
# frame(): timing
rep('''    local resp
    resp="$(curl -s --max-time 20 -X POST "$A/api/render_frame"''', '''    local resp t0 t1
    t0="$(now)"
    resp="$(curl -s --max-time 20 -X POST "$A/api/render_frame"''')
rep('''-d "{\\"output_path\\":\\"$OUT/$1\\"}")"
    echo "$resp"''', '''-d "{\\"output_path\\":\\"$OUT/$1\\"}")"
    t1="$(now)"; echo "{\\"ev\\":\\"capture\\",\\"name\\":\\"$1\\",\\"t0\\":$t0,\\"t1\\":$t1}" >> "$TIMING_LOG"
    echo "$resp"''')
# mid / jmid shots
rep('''      T_MID="$(now)"
      curl -s --max-time 20 -X POST "$A/api/render_frame" -H 'Content-Type: application/json' \\
           -d "{\\"output_path\\":\\"$OUT/mid.png\\"}" > "$OUT/mid.json"
''', '''      T_MID="$(now)"
      curl -s --max-time 20 -X POST "$A/api/render_frame" -H 'Content-Type: application/json' \\
           -d "{\\"output_path\\":\\"$OUT/mid.png\\"}" > "$OUT/mid.json"
      echo "{\\"ev\\":\\"capture\\",\\"name\\":\\"mid.png\\",\\"t0\\":$T_MID,\\"t1\\":$(now)}" >> "$TIMING_LOG"
''')
rep('''    ( sleep "$(perl -e "my \\$d = ($T_EDGEJ - 0.30) - $(now); printf '%.3f', \\$d > 0 ? \\$d : 0")"
      curl -s --max-time 20 -X POST "$A/api/render_frame" -H 'Content-Type: application/json' \\
           -d "{\\"output_path\\":\\"$OUT/jmid.png\\"}" > "$OUT/jmid.json" ) &''',
'''    ( sleep "$(perl -e "my \\$d = ($T_EDGEJ - 0.30) - $(now); printf '%.3f', \\$d > 0 ? \\$d : 0")"
      TJM0="$(now)"
      curl -s --max-time 20 -X POST "$A/api/render_frame" -H 'Content-Type: application/json' \\
           -d "{\\"output_path\\":\\"$OUT/jmid.png\\"}" > "$OUT/jmid.json"
      echo "{\\"ev\\":\\"capture\\",\\"name\\":\\"jmid.png\\",\\"t0\\":$TJM0,\\"t1\\":$(now)}" >> "$TIMING_LOG" ) &''')
# record anchors: log T values for analysis
rep('echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"',
    'echo "{\\"ev\\":\\"anchors\\",\\"T1\\":${T1:-0},\\"T2\\":${T2:-0},\\"T3\\":${T3:-0},\\"T4\\":${T4:-0},\\"T5\\":${T5:-0},\\"T6\\":\\"${T6:-NA}\\",\\"TJ\\":${TJ:-0},\\"T_EDGE\\":\\"${T_EDGE:-NA}\\",\\"T_EDGEJ\\":\\"${T_EDGEJ:-NA}\\",\\"T_EDGE2\\":\\"${T_EDGE2:-NA}\\",\\"take\\":\\"$TAKE_FOLDER\\"}" >> "$TIMING_LOG"\necho; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"')
open(p, 'w').write(s)
print('patched')
