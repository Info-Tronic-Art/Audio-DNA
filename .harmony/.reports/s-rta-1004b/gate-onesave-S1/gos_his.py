"""G-OS-HIS (Harmony's own row, ruling-one-save.md section 5): on COPIES of Boris's show, never his file.
usage: gos_his.py A <copy1> <outdir>          (the 185147b app is running: load copy 1 -> A.json)
       gos_his.py B <copy2> <outdir> <sha>    (the new app is running: load copy 2 -> B.json; plain save; backup + first key)
       gos_his.py C <saved> <outdir>          (a fresh new app: load the saved file -> C.json; compare)
REST on 7070, stdlib only, Connection: close. /api/composition is read only after the load answered and a settle."""
import sys, json, time, os, hashlib, urllib.request, urllib.error
A = 'http://127.0.0.1:7070'

def http(method, path, body=None, timeout=30):
    data = json.dumps(body).encode() if body is not None else (b'' if method == 'POST' else None)
    req = urllib.request.Request(A + path, data=data, method=method, headers={'Connection': 'close', 'Content-Type': 'application/json'})
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r: raw, code = r.read(), r.status
    except urllib.error.HTTPError as e: raw, code = e.read(), e.code
    except Exception as e: return 0, {'error': str(e)}
    try: return code, json.loads(raw.decode() or '{}')
    except Exception: return code, {}

def sha(p):
    return hashlib.sha256(open(p, 'rb').read()).hexdigest() if os.path.isfile(p) else None

def load_and_read(path, out):
    c, b = http('POST', '/api/load_composition', {'path': path})
    print('load: HTTP', c, json.dumps(b)[:160])
    if c != 200 or b.get('ok') is not True: return None
    time.sleep(3.0)                      # the staged swap has answered; settle before the one unlocked read
    c1, j1 = http('GET', '/api/composition')
    time.sleep(1.0)
    c2, j2 = http('GET', '/api/composition')
    same = diff(j1, j2)
    print('composition: HTTP', c1, c2, '| top-level keys', len(j1) if isinstance(j1, dict) else '?', '| two reads 1 s apart differ at', len(same), 'path(s)', same[:6])
    json.dump(j1, open(out, 'w'), indent=1, sort_keys=True)
    return j1

def diff(a, b, path=''):
    """-> list of paths where a and b differ (a key on one side only, or unequal leaves)."""
    out = []
    if isinstance(a, dict) and isinstance(b, dict):
        for k in sorted(set(a) | set(b)):
            p = path + '/' + str(k)
            if k not in a: out.append(p + ' (only in second)')
            elif k not in b: out.append(p + ' (only in first)')
            else: out += diff(a[k], b[k], p)
    elif isinstance(a, list) and isinstance(b, list):
        if len(a) != len(b): out.append(path + ' (list length %d vs %d)' % (len(a), len(b)))
        for i, (x, y) in enumerate(zip(a, b)): out += diff(x, y, path + '/' + str(i))
    elif a != b:
        out.append(path)
    return out

step, src, outdir = sys.argv[1], sys.argv[2], sys.argv[3]
if step == 'A':
    j = load_and_read(src, outdir + '/A.json'); sys.exit(0 if j else 1)
if step == 'B':
    want = sys.argv[4]
    print('copy 2 sha before:', sha(src), '== copy-time sha:', sha(src) == want)
    j = load_and_read(src, outdir + '/B.json')
    if not j: sys.exit(1)
    c, s = http('GET', '/api/debug/show_file'); print('show_file after load: HTTP', c, json.dumps(s)[:300])
    a = json.load(open(outdir + '/A.json'))
    d = diff(a, j)
    allowed = {'/outputDisplay (only in first)', '/version (only in second)'}
    extra = [x for x in d if x not in allowed]
    print('A vs B: %d differing path(s); beyond {outputDisplay in A, version in B}: %d' % (len(d), len(extra)), extra[:12])
    print(('PASS' if not extra and '/version (only in second)' in d else 'FAIL') + '  G-OS-HIS (a): the show loaded by the 185147b app and by the new app is the same, apart from outputDisplay (A) and version (B)')
    seq0 = (s.get('lastSave') or {}).get('seq', 0)
    c, ans = http('POST', '/api/debug/save_composition', {'plain': True}); print('save_composition plain: HTTP', c, json.dumps(ans)[:160])
    ls = None; t0 = time.time()
    while time.time() - t0 < 15:
        c, s = http('GET', '/api/debug/show_file'); ls = s.get('lastSave') or {}
        if c == 200 and ls.get('seq', 0) > seq0: break
        time.sleep(0.05)
    print('lastSave:', json.dumps(ls)[:300])
    folder = os.path.dirname(src); name = os.path.basename(src)[:-5]
    bk = os.path.join(folder, 'backups', name + '.v0.json')
    print('folder now:', sorted(os.listdir(folder)), '| backups:', sorted(os.listdir(os.path.join(folder, 'backups'))) if os.path.isdir(os.path.join(folder, 'backups')) else None)
    pairs = json.load(open(src), object_pairs_hook=lambda p: p)
    first = pairs[0] if pairs else (None, None)
    okb = sha(bk) == want
    okv = first[0] == 'version' and first[1] == 2
    print('backup sha:', sha(bk), '== copy-time sha:', okb, '| saved file first key:', first[0], '=', first[1] if not isinstance(first[1], list) else '...', '| saved size', os.path.getsize(src))
    print(('PASS' if okb and okv and ls.get('result') not in (None, 'failed', 'cancelled') else 'FAIL') + "  G-OS-HIS (b): a plain Save left backups/%s.v0.json with the copy-time checksum; the saved file's first key is version = 2; lastSave.result %r backup %r ms %r" % (name, ls.get('result'), ls.get('backup'), ls.get('ms')))
    sys.exit(0)
if step == 'C':
    j = load_and_read(src, outdir + '/C.json')
    if not j: sys.exit(1)
    c, s = http('GET', '/api/debug/show_file'); print('show_file after load: HTTP', c, json.dumps(s)[:200])
    b = json.load(open(outdir + '/B.json'))
    d = diff(b, j)
    clip_ids = [x for x in d if x.endswith('/id') and '/clips/' in x]
    other = [x for x in d if x not in clip_ids]
    print('B vs C: %d differing path(s): %d are a clip id; other: %d' % (len(d), len(clip_ids), len(other)), other[:12], '| id examples', clip_ids[:3])
    print(('PASS' if not other else 'FAIL') + '  G-OS-HIS (c): the saved version-2 file, loaded by a fresh new app, is the show that was saved (only clip ids differ)')
    sys.exit(0)
