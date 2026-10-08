# usage: python3 merge.py <RS dir>   -- overlays each topic's ruling (rule-<T>.md) on its paper (apply-<T>.md) and writes the digests.
# Writes: spec-<T>.md (the ruled blocks), assume-all.md, answers-all.md, names-all.md, today-notes.md, ledger.md. Prints a report.
import sys, os, re, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from blocks import parse, dump, KINDS, TOPICS
RS = sys.argv[1]
# ruling-page-2.md (its line on R151): the first page ruling's replacement of R151 is turned back -- the topic ruling's block stands (a clip previewed by its name waits for the next 1, his L36)
VOID_BY_RULING_2 = {'R151'}
ids = json.load(open(os.path.join(RS, 'slice-ids.json')))
rd = lambda p: open(p, encoding='utf-8').read() if os.path.exists(p) else None
def section(text, head):
    m = re.search(r'^## ' + re.escape(head) + r'.*?$', text, re.M)
    if not m: return ''
    rest = text[m.end():]; n = re.search(r'^## ', rest, re.M)
    return (rest[:n.start()] if n else rest).strip()
allA, allAns, allNames, today, ledger, report = [], [], [], [], [], []
for T in 'ABCDEFGHIJKX':
    a = rd(os.path.join(RS, 'apply-%s.md' % T))
    if a is None: report.append('%s: NO apply paper' % T); continue
    blocks, prob = parse(a)
    r = rd(os.path.join(RS, 'rule-%s.md' % T)); nrep = ndrop = nnew = 0
    if r is not None:
        rb, rp = parse(r); prob += ['rule: ' + p for p in rp]
        key = lambda b: (b['kind'], b['id'])
        for b in rb:
            if b['kind'] == 'DROP':
                parts = b['id'].split(None, 1)
                k = (parts[0], parts[1]) if len(parts) == 2 else ('ASSUME', b['id'])
                n0 = len(blocks); blocks = [x for x in blocks if key(x) != k]; ndrop += n0 - len(blocks)
                if n0 == len(blocks): prob.append('rule: DROP %s names no block of the paper' % b['id'])
            elif b['kind'] in ('ITEM', 'ASSUME', 'ANSWER', 'NAME'):
                hit = [i for i, x in enumerate(blocks) if key(x) == key(b)]
                if hit: blocks[hit[0]] = b; nrep += 1
                else: blocks.append(b); nnew += 1
    # the ruling over all topics (ruling-page.md, then ruling-page-2.md) may replace an item's block: laid over last, by item id
    npage = 0
    for pf in ('ruling-page.md', 'ruling-page-2.md'):
        pr = rd(os.path.join(RS, pf))
        if pr is None: continue
        for b in parse(pr)[0]:
            if pf == 'ruling-page.md' and b['id'] in VOID_BY_RULING_2: continue
            if b['kind'] == 'ITEM' and b['id'] in ids[T] and all(b['fields'].get(f, '').strip() for f in KINDS['ITEM']):
                hit = [i for i, x in enumerate(blocks) if x['kind'] == 'ITEM' and x['id'] == b['id']]
                if hit: blocks[hit[0]] = b; npage += 1
    have = [b['id'] for b in blocks if b['kind'] == 'ITEM']
    miss = [i for i in ids[T] if i not in have]
    out = ['# SPEC %s -- %s (s-rta-1007): the paper apply-%s.md with the ruling rule-%s.md laid over it by merge.py. This file wins over both.' % (T, TOPICS[T], T, T), '']
    for kind, head in (('ITEM', 'ITEMS'), ('ASSUME', 'ASSUMPTIONS'), ('ANSWER', 'QUESTIONS BACK'), ('NAME', 'NAMES')):
        out.append('## ' + head)
        for b in blocks:
            if b['kind'] == kind: out.append(dump(b)); out.append('')
    for head in ('CONFLICTS', 'NOT DONE / UNSURE'):
        s = section(a, head)
        if s: out += ['## ' + head + ' (from the paper, unruled)', s, '']
    if r is not None:
        s = section(r, 'FOR THE PAGE RULING')
        if s: out += ['## FOR THE PAGE RULING (from the ruling)', s, '']
    open(os.path.join(RS, 'spec-%s.md' % T), 'w', encoding='utf-8').write('\n'.join(out) + '\n')
    titles = {b['id']: b['fields'].get('TITLE', '') for b in blocks if b['kind'] == 'ITEM'}
    stat = {b['id']: b['fields'].get('STATUS', '').split(' ')[0] for b in blocks if b['kind'] == 'ITEM'}
    A = [b for b in blocks if b['kind'] == 'ASSUME']
    allA.append('## TOPIC %s -- %s' % (T, TOPICS[T]))
    if r is not None:
        s = section(r, 'FOR THE PAGE RULING')
        if s: allA += ['(the topic ruling\'s notes for the page ruling:)', s, '']
    for b in A:
        ab = re.findall(r'\b(?:R\d{3}|[DPGCNU]\d{1,2}|\d{3})\b', b['fields'].get('ABOUT', ''))
        allA.append('(items: ' + '; '.join('%s "%s" [%s]' % (i, titles.get(i, '?'), stat.get(i, '?')) for i in ab) + ')')
        allA.append(dump(b)); allA.append('')
    for b in blocks:
        if b['kind'] == 'ANSWER': allAns += ['(topic %s)' % T, dump(b), '']
        if b['kind'] == 'NAME': allNames += ['(topic %s)' % T, dump(b), '']
        if b['kind'] == 'ITEM':
            f = b['fields']
            today.append('- [%s] %s (%s; %s): %s' % (T, b['id'], f.get('TITLE', ''), f.get('STATUS', '').split(' ')[0], f.get('TODAY', '')))
            ledger.append('- %s | %s | %s | his lines: %s | %s' % (b['id'], f.get('STATUS', '').split(' ')[0], f.get('TITLE', ''), f.get('HIS', ''), f.get('RULE', '')))
    c = lambda k: sum(1 for b in blocks if b['kind'] == k)
    ask = lambda w: sum(1 for b in A if b['fields'].get('ASK', '').startswith(w))
    sc = {}
    for b in blocks:
        if b['kind'] == 'ITEM': s = b['fields'].get('STATUS', '').split(' ')[0]; sc[s] = sc.get(s, 0) + 1
    report.append('%s: %d/%d items (missing %s) %s | ASSUME %d (YES %d, LINE %d, NO %d) | ANSWER %d | NAME %d | ruling: %s (replaced %d, new %d, dropped %d; by the page ruling %d) | format problems %d' % (T, len(have), len(ids[T]), miss or 'none', ' '.join('%s=%d' % kv for kv in sorted(sc.items())), len(A), ask('YES'), ask('LINE'), ask('NO'), c('ANSWER'), c('NAME'), 'yes' if r is not None else 'NO FILE', nrep, nnew, ndrop, npage, len(prob)))
    for p in prob[:6]: report.append('    ! ' + p)
for name, key in (('codec', 'answer-codec.md'), ('lowres', 'answer-lowres-rec.md'), ('tempo-auto', 'answer-tempo-auto.md')):
    a = rd(os.path.join(RS, key))
    if a is None: report.append('answer %s: NO FILE' % name); continue
    allAns += ['## THE ANSWER PAPER %s (%s) -- its two sections for him, as written (the topic ruling may have replaced them: see the @@ANSWER block above with the key %s)' % (name, key, {'codec': 'codec', 'lowres': 'lowres', 'tempo-auto': '189'}[name]), '### ANSWER FOR BORIS', section(a, 'ANSWER FOR BORIS'), '### QUESTION FOR HIM', section(a, 'QUESTION FOR HIM'), '']
    c = rd(os.path.join(RS, 'check-%s.md' % name))
    if c:
        s = section(c, 'A BETTER')
        if s: allAns += ['### THE CHECKER\'S REPLACEMENT', s, '']
w = lambda n, head, lines: open(os.path.join(RS, n), 'w', encoding='utf-8').write(head + '\n\n' + '\n'.join(lines) + '\n')
w('assume-all.md', '# EVERY ASSUMPTION LEFT after Boris\'s answers of 2026-10-07, topic by topic (the ruled blocks; made by merge.py). The line in brackets above a block names the items it belongs to, with each item\'s title and status.', allA)
w('answers-all.md', '# THE ANSWERS TO THE QUESTIONS BORIS ASKED HARMONY (ruled @@ANSWER blocks of the topic papers, then the three answer papers\' own sections; made by merge.py)', allAns)
w('names-all.md', '# EVERY NAME the topic papers fix or pick (ruled @@NAME blocks; made by merge.py)', allNames)
w('today-notes.md', '# HARMONY\'S OWN NOTES: what the app does now, item by item (the TODAY lines of the ruled papers). His rule (2026-10-07): "Keep the ‘today’ in your own notes so you know what to change." Never shown to him.', today)
w('ledger.md', '# THE APPLICATION LEDGER: every item of the page of 2026-10-05 after Boris\'s answers of 2026-10-07 (id | status | title | his lines | the rule now). Made by merge.py from the ruled papers spec-<topic>.md, which hold the full blocks.', ledger)
print('\n'.join(report))
print('TOTAL assumptions: %d blocks in assume-all.md' % sum(1 for l in allA if l.startswith('@@ASSUME')))
