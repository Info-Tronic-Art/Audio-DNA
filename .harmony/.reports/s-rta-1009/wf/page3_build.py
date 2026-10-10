# usage: python3 page3_build.py --lint  <list.md>                         -- lint a list with provisional ids (P1, P2 ...)
#        python3 page3_build.py --check <list.md> <edits.md> [more edits]  -- lay the edit blocks over the list in memory, then lint
#        python3 page3_build.py --build <list.md> <edits.md> [more edits] --frame <frame.md> --out <items.md>
#           -- the same, then number the items from 274 (ASK items topic by topic, then LINE items), remap every reference, write the list boris-page.py renders
# Prints counts, problems ("!"), notes ("~"), and ONE last line that begins "LINT:". Exit 0 always.
import sys, os, re, json
here = os.path.dirname(os.path.abspath(__file__)); RS = os.path.join(here, '..'); H = os.path.join(RS, '..', '..')
sys.path.insert(0, here)
from blocks3 import parse, dump
K = {'PAGE-ANSWER': ['ASKED', 'TITLE', 'TEXT', 'ITEM'], 'PAGE-ITEM': ['TOPIC', 'KIND', 'TEXT', 'B', 'C', 'FROM'], 'TRIAGE': ['TO', 'WHY'], 'DROP': ['WHY'],
     'PAGE-INTRO': [], 'PAGE-TOPIC': ['NAME'], 'PAGE-LINK': ['LABEL', 'HREF', 'TEXT']}
FIRST = 274
argv = sys.argv[1:]; mode = argv[0]
frame = out = None
if '--frame' in argv: frame = argv[argv.index('--frame') + 1]
if '--out' in argv: out = argv[argv.index('--out') + 1]
files = [a for i, a in enumerate(argv[1:], 1) if not a.startswith('--') and argv[i - 1] not in ('--frame', '--out')]
rd = lambda p: open(p, encoding='utf-8').read()
blocks, problems = parse(rd(files[0]), K); soft = []
key = lambda b: (b['kind'], b['id'])
nrep = nnew = ndrop = 0
for ef in files[1:]:
    eb, ep = parse(rd(ef), K); problems += ['edits: ' + p for p in ep]
    for b in eb:
        if b['kind'] == 'DROP':
            parts = b['id'].split(None, 1)
            if len(parts) != 2: problems.append('edits: DROP "%s": the id is "<KIND> <id>"' % b['id']); continue
            n0 = len(blocks); blocks = [x for x in blocks if key(x) != (parts[0], parts[1])]
            if len(blocks) == n0: soft.append('edits: DROP %s names no block of the list (already gone?)' % b['id'])
            else: ndrop += 1
            continue
        hit = [i for i, x in enumerate(blocks) if key(x) == key(b)]
        if hit: blocks[hit[0]] = b; nrep += 1
        else: blocks.append(b); nnew += 1
items = [b for b in blocks if b['kind'] == 'PAGE-ITEM']; answers = [b for b in blocks if b['kind'] == 'PAGE-ANSWER']; triage = [b for b in blocks if b['kind'] == 'TRIAGE']
seen = {}
for b in blocks:
    seen[key(b)] = seen.get(key(b), 0) + 1
for k, n in seen.items():
    if n > 1: problems.append('TWICE: %d blocks @@%s %s' % (n, k[0], k[1]))
his = '\n'.join(rd(p) for p in (os.path.join(RS, 'boris-answers-page2.txt'), os.path.join(RS, '..', 's-rta-1007', 'boris-msg-raw-1.txt')) if os.path.exists(p))
NOW = re.compile(r'\b(today|currently|right now|at the moment|at present|as it is now)\b', re.I)
IDS = re.compile(r'\bR\d{3}\b|\b[A-KX]3?-\d+\b|\bBF\d{3}\b|\bspec-[A-KX]\b')
wc = lambda s: len(s.split())
iid = set(b['id'] for b in items)
for b in items:
    f = b['fields']; i = b['id']
    if not re.match(r'^(P\d+|NEW-\d+|\d{3})$', i): problems.append('PAGE-ITEM "%s": the id is P<n> (or NEW-<k> in an edit block)' % i)
    if f.get('TOPIC', '') not in list('ABCDEFGHIJK'): problems.append('PAGE-ITEM %s: TOPIC must be one letter A to K' % i)
    kind = f.get('KIND', '')
    if kind not in ('ASK', 'LINE'): problems.append('PAGE-ITEM %s: KIND must be ASK or LINE' % i)
    t = f.get('TEXT', ''); alt = [f.get('B', ''), f.get('C', '')]
    if not t.startswith('I assume'): problems.append('PAGE-ITEM %s: TEXT must begin "I assume"' % i)
    lim, hard = (45, 55) if kind == 'ASK' else (30, 38)
    if wc(t) > hard: problems.append('PAGE-ITEM %s: TEXT is %d words (%s: at most %d)' % (i, wc(t), kind, lim))
    elif wc(t) > lim: soft.append('PAGE-ITEM %s: TEXT is %d words (aim: %d)' % (i, wc(t), lim))
    for nm, a in zip('BC', alt):
        if a.strip().lower() != 'none' and wc(a) > 40: problems.append('PAGE-ITEM %s: %s is %d words (at most 30)' % (i, nm, wc(a)))
    if kind == 'LINE' and alt[1].strip().lower() != 'none': soft.append('PAGE-ITEM %s: a LINE item with a third way -- is it a card?' % i)
    whole = ' '.join([t] + [a for a in alt if a.strip().lower() != 'none'])
    if NOW.search(whole): problems.append('PAGE-ITEM %s: speaks of how the app is now ("%s")' % (i, NOW.search(whole).group(0)))
    if IDS.search(whole): problems.append('PAGE-ITEM %s: carries a reading number, a block id or a file name ("%s")' % (i, IDS.search(whole).group(0)))
    if re.search(r'\bReview\b', re.sub(r'"[^"]*"', '', whole)): problems.append('PAGE-ITEM %s: says "Review" -- the screen is called Studio' % i)
    if re.search(r'\b(see|as in|item) (2[1-7]\d)\b', whole): problems.append('PAGE-ITEM %s: points back to an item of page 2 -- say the thing itself' % i)
    if '?' in t and kind != 'ASK': problems.append('PAGE-ITEM %s: a "?" in TEXT makes it an item silence cannot accept: KIND must be ASK' % i)
for b in answers:
    f = b['fields']; i = b['id']
    spans = re.findall(r'"([^"]+)"', f.get('ASKED', ''))
    if not spans: problems.append('PAGE-ANSWER %s: ASKED holds no words of his in quotes' % i)
    for s in spans:
        if s not in his: problems.append('PAGE-ANSWER %s: ASKED is not verbatim: "%s"' % (i, s[:70]))
    lim, hard = (150, 175) if 'hold' in i or i == '246' else (130, 150)
    plain = re.sub(r'\[([^\]]+)\]\([^)]+\)', r'\1', f.get('TEXT', ''))
    if wc(plain) > hard: problems.append('PAGE-ANSWER %s: TEXT is %d words (at most %d)' % (i, wc(plain), lim))
    elif wc(plain) > lim: soft.append('PAGE-ANSWER %s: TEXT is %d words (aim: %d)' % (i, wc(plain), lim))
    if NOW.search(plain) and i not in ('general',): soft.append('PAGE-ANSWER %s: says "%s" -- only if how the app is now is what he asked' % (i, NOW.search(plain).group(0)))
    if IDS.search(plain): problems.append('PAGE-ANSWER %s: carries a reading number, a block id or a file name ("%s")' % (i, IDS.search(plain).group(0)))
    if re.search(r'\bReview\b', re.sub(r'"[^"]*"', '', plain)): problems.append('PAGE-ANSWER %s: says "Review" -- the screen is called Studio' % i)
    it = f.get('ITEM', '').strip()
    if it.lower() != 'none':
        for x in re.split(r'[,\s]+', it):
            if x and x not in iid: problems.append('PAGE-ANSWER %s: ITEM names "%s", which is not an item of the list' % (i, x))
have = set(b['id'] for b in answers)
for a in ('222', '250', '252', '246-hold', 'general'):
    if a not in have: problems.append('MISSING: no @@PAGE-ANSWER %s' % a)
for a in ('239', 'codec', 'lowres'):
    if a not in have: soft.append('no @@PAGE-ANSWER %s (merged into another answer?)' % a)
a3 = os.path.join(RS, 'assume3-all.md'); must = {}
if os.path.exists(a3):
    ab, _ = parse(rd(a3))
    must = {b['id']: b['fields'].get('ASK', '').split(' ')[0].strip('.,;:') for b in ab if b['kind'] == 'ASSUME'}
tid = {}
for b in triage:
    tid[b['id']] = b; to = b['fields'].get('TO', '').split(' ')[0].strip('.,;:')
    if must and b['id'] not in must: soft.append('TRIAGE %s: no such assumption in assume3-all.md' % b['id'])
    if to not in ('MERGED', 'SETTLED', 'INTERNAL') and to not in iid: problems.append('TRIAGE %s: TO must be an item of the list, MERGED, SETTLED or INTERNAL (it reads "%s")' % (b['id'], to))
for a, ask in must.items():
    if ask in ('YES', 'LINE') and a not in tid: problems.append('NO TRIAGE: the assumption %s (ASK: %s) has no @@TRIAGE block' % (a, ask))
ref = set(b['fields'].get('TO', '').split(' ')[0].strip('.,;:') for b in triage)
for b in items:
    if b['id'] not in ref and 'again' not in b['fields'].get('FROM', '') and 'newest' not in b['fields'].get('FROM', ''): soft.append('PAGE-ITEM %s: no @@TRIAGE points to it and FROM does not say "again: <number>" or "newest words"' % b['id'])
na = sum(1 for b in items if b['fields'].get('KIND') == 'ASK'); nl = len(items) - na
words = sum(wc(' '.join(b['fields'].get(x, '') for x in ('TEXT', 'B', 'C'))) for b in items) + sum(wc(b['fields'].get('TEXT', '') + ' ' + b['fields'].get('ASKED', '')) for b in answers)
print('list: %d answers, %d ASK items, %d LINE items, %d triage blocks; about %d words to read (%d min at 200 a minute, %d min at 130)%s' % (len(answers), na, nl, len(triage), words, round(words / 200), round(words / 130), ('; edits: replaced %d, new %d, dropped %d' % (nrep, nnew, ndrop)) if len(files) > 1 else ''))
for p in problems[:80]: print(' ! ' + p)
for p in soft[:40]: print(' ~ ' + p)
if mode == '--build' and not problems:
    order = [b for T in 'ABCDEFGHIJK' for b in items if b['fields']['TOPIC'] == T and b['fields']['KIND'] == 'ASK'] + [b for T in 'ABCDEFGHIJK' for b in items if b['fields']['TOPIC'] == T and b['fields']['KIND'] == 'LINE']
    num = {b['id']: str(FIRST + n) for n, b in enumerate(order)}
    o = [rd(frame).rstrip('\n'), ''] if frame else []
    for b in answers:
        it = b['fields']['ITEM'].strip()
        if it.lower() != 'none': b['fields']['ITEM'] = ', '.join(num[x] for x in re.split(r'[,\s]+', it) if x)
        o += [dump(b, K), '']
    for b in order:
        b = dict(b, id=num[b['id']]); o += [dump(b, K), '']
    for b in triage:
        to = b['fields']['TO'].split(' ')[0].strip('.,;:')
        if to in num: b['fields']['TO'] = num[to]
        o += [dump(b, K), '']
    open(out, 'w', encoding='utf-8').write('\n'.join(o) + '\n')
    json.dump(num, open(out + '.map.json', 'w'))
    print('built: %s (items %d-%d; the map of provisional ids: %s.map.json)' % (out, FIRST, FIRST + len(order) - 1, out))
print('LINT: %s -- %d answers, %d ASK, %d LINE, %d triage; %d problem(s) that must be fixed, %d note(s)' % ('OK' if not problems else 'NOT OK', len(answers), na, nl, len(triage), len(problems), len(soft)))
