# usage: python3 merge3.py <s-rta-1007 dir> <s-rta-1009 dir> [--dry]
# Lays this round's ruled papers over the old spec: apply-<T>.md, then rule-<T>.md (its REPLACEMENT BLOCKS), then answer-hold.md (topic J).
# Writes (unless --dry): spec-<T>.md (the truth after page 2), assume3-all.md, answers3-all.md, names3-all.md, ledger3.md, today3-notes.md. Prints a report.
import sys, os, re, json
S7, S9 = sys.argv[1:3]; DRY = '--dry' in sys.argv
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from blocks3 import parse, dump, KINDS, TOPICS, section
rd = lambda p: open(p, encoding='utf-8').read() if os.path.exists(p) else None
OLDK = {k: KINDS[k] for k in ('ITEM', 'ASSUME', 'ANSWER', 'NAME')}
ids = json.load(open(os.path.join(S9, 'slice3-ids.json'))); old = json.load(open(os.path.join(S9, 'old-index.json')))
key = lambda b: (b['kind'], b['id'])
report = []; papers = {}; notes = {}
for T in TOPICS:
    a = rd(os.path.join(S9, 'apply-%s.md' % T))
    if a is None: report.append('%s: NO PAPER' % T); continue
    blocks, prob = parse(a)
    for b in blocks: b['src'] = 'paper'
    r = rd(os.path.join(S9, 'rule-%s.md' % T)); nrep = nnew = ndrop = 0
    over = [(r, 'ruling')] if r is not None else []
    if T == 'J':
        h = rd(os.path.join(S9, 'answer-hold.md'))
        if h is not None: over.append((h, 'hold answer'))
        else: report.append('J: answer-hold.md is NOT there (item 246 stays as the paper filed it)')
    for text, who in over:
        rb, rp = parse(section(text, 'REPLACEMENT BLOCKS') if who == 'ruling' else section(text, 'BLOCKS')); prob += ['%s: %s' % (who, p) for p in rp]
        for b in rb:
            b['src'] = who
            if b['kind'] == 'DROP':
                parts = b['id'].split(None, 1)
                if len(parts) == 2 and (parts[0], parts[1]) in set(key(x) for x in blocks):
                    blocks = [x for x in blocks if key(x) != (parts[0], parts[1])]; ndrop += 1; continue
                if len(parts) == 2 and parts[0] == 'ASSUME' and parts[1] in old['assumes']:
                    if key(b) not in set(key(x) for x in blocks): blocks.append(b); nnew += 1
                    continue
                prob.append('%s: DROP %s names no block' % (who, b['id'])); continue
            hit = [i for i, x in enumerate(blocks) if key(x) == key(b)]
            if hit: blocks[hit[0]] = b; nrep += 1
            else: blocks.append(b); nnew += 1
            if b['kind'] == 'ASSUME':   # a ruling that writes an old assumption again keeps it: its DROP goes
                blocks = [x for x in blocks if not (x['kind'] == 'DROP' and x['id'] == 'ASSUME ' + b['id'])]
    papers[T] = blocks
    notes[T] = dict(conf=section(a, 'CONFLICTS'), reach=section(a, 'REACHES OTHER TOPICS'), unsure=section(a, 'NOT DONE / UNSURE'), page=section(r, 'FOR THE PAGE RULING') if r else '', ruled=r is not None)
    have = [b['id'] for b in blocks if b['kind'] == 'ITEM']
    miss = [i for i in ids[T]['page_items'] if have.count(i) != 1]
    c = lambda k: sum(1 for b in blocks if b['kind'] == k); ask = lambda w: sum(1 for b in blocks if b['kind'] == 'ASSUME' and b['fields'].get('ASK', '').startswith(w))
    report.append('%s: ITEM %d/%d%s | AMEND %d | ASSUME %d (YES %d, LINE %d, NO %d) | DROP %d | ANSWER %d | NAME %d | ruling %s (replaced %d, new %d, dropped %d) | format problems %d' % (
        T, len(have), len(ids[T]['page_items']), (' MISSING/TWICE ' + ','.join(miss)) if miss else '', c('AMEND'), c('ASSUME'), ask('YES'), ask('LINE'), ask('NO'), c('DROP'), c('ANSWER'), c('NAME'), 'yes' if r is not None else 'NO FILE', nrep, nnew, ndrop, len(prob)))
    for p in prob[:5]: report.append('    ! ' + p)
# --- coverage of his boxes and of the answers he is owed
allb = [b for T in papers for b in papers[T]]
cited = set(re.findall(r'BF\d{3}', ' '.join(' '.join(b['fields'].get(f, '') for f in ('HIS', 'SOURCE', 'WHY')) for b in allb)))
nb = ['BF%d' % n for n in range(240, 273)]
report.append('BOXES cited: %d of 33%s' % (len([x for x in nb if x in cited]), (' -- NOT CITED: ' + ' '.join(x for x in nb if x not in cited)) if any(x not in cited for x in nb) else ''))
owed = [a for T in TOPICS for a in ids[T]['owed']] + ['246-hold']; havea = set(b['id'] for b in allb if b['kind'] == 'ANSWER')
report.append('ANSWERS owed: %s | missing: %s' % (' '.join(owed), ' '.join(a for a in owed if a not in havea) or 'none'))
# --- amendments laid over the old RULE lines
rules = {i: v['rule'] for i, v in old['items'].items()}; amended = {}; unapplied = []
am = sorted([(T, b) for T in papers for b in papers[T] if b['kind'] == 'AMEND'], key=lambda x: (x[1]['id'].split()[0], int(x[1]['id'].split()[1]) if x[1]['id'].split()[1:] and x[1]['id'].split()[1].isdigit() else 0, x[0]))
for T, b in am:
    oid = b['id'].split()[0]; f = b['fields']
    if oid not in rules: unapplied.append((T, b, 'no such old item')); continue
    o = f.get('OLD', '').strip(); mark = ' [page 2, %s: %s]' % (T, re.sub(r'\s+', ' ', f.get('HIS', ''))[:160])
    if o == 'APPEND': rules[oid] = rules[oid].rstrip() + ' ' + f.get('NEW', '').strip() + mark
    elif rules[oid].count(o) == 1: rules[oid] = rules[oid].replace(o, f.get('NEW', '').strip() + mark)
    else: unapplied.append((T, b, 'OLD found %d times in the rule as it stands after earlier amendments' % rules[oid].count(o))); continue
    amended.setdefault(oid, []).append((T, b['id']))
multi = {i: v for i, v in amended.items() if len(set(t for t, _ in v)) > 1}
report.append('AMENDMENTS: %d applied to %d old items; %d NOT applied; old items amended by more than one topic: %s' % (sum(len(v) for v in amended.values()), len(amended), len(unapplied), ', '.join('%s(%s)' % (i, '+'.join(sorted(set(t for t, _ in v)))) for i, v in multi.items()) or 'none'))
for T, b, why in unapplied: report.append('    ! %s AMEND %s: %s' % (T, b['id'], why))
# --- old assumptions: closed, dropped, replaced, still open
dropped = {}; replaced = {}
for T in papers:
    for b in papers[T]:
        if b['kind'] == 'DROP' and b['id'].startswith('ASSUME '): dropped[b['id'][7:]] = (T, b['fields'].get('WHY', ''))
        if b['kind'] == 'ASSUME' and b['id'] in old['assumes']: replaced[b['id']] = (T, b)
both = sorted(set(dropped) & set(replaced))
if both: report.append('    ! dropped AND rewritten: ' + ' '.join(both))
allA, allAns, allNames, ledger, today = [], [], [], [], []
for T in TOPICS:
    if T not in papers: continue
    ob, _ = parse(rd(os.path.join(S7, 'spec-%s.md' % T)), OLDK); B = papers[T]; n = notes[T]
    out = ['# SPEC %s -- %s (s-rta-1009): the s-rta-1007 spec with Boris\'s answers to page 2 laid over it by wf/merge3.py (paper apply-%s.md, ruling rule-%s.md%s). This file wins over every older one.' % (T, TOPICS[T], T, T, '' if n['ruled'] else ' -- NOT RULED'), '']
    out.append('## PAGE 2 ITEMS (the rule now for each item of page 2 in this topic)')
    for b in B:
        if b['kind'] == 'ITEM':
            out += [dump(b), '']; f = b['fields']
            ledger.append('- %s | %s | %s | %s | %s' % (b['id'], f.get('STATUS', '').split(' ')[0], f.get('TITLE', ''), f.get('HIS', ''), f.get('RULE', '')))
            today.append('- [%s] %s (%s): %s' % (T, b['id'], f.get('TITLE', ''), f.get('TODAY', '')))
    out.append('## ITEMS (the items of the first page; a RULE that page 2 changed carries "[page 2, ...]" marks where it changed)')
    for b in ob:
        if b['kind'] == 'ITEM':
            if b['id'] in amended:
                b['fields']['RULE'] = rules[b['id']]; b['fields']['STATUS'] = b['fields']['STATUS'] + ' -- AMENDED after page 2 (%s)' % ', '.join('%s by %s' % (i, t) for t, i in amended[b['id']])
            out += [dump(b, OLDK), '']
    out.append('## ASSUMPTIONS STILL OPEN (new after page 2: ids with a 3; old ones never shown to him keep their ids)')
    newA = [b for b in B if b['kind'] == 'ASSUME' and b['id'] not in old['assumes']]
    for b in newA: out += [dump(b), '']
    closed = []
    for b in ob:
        if b['kind'] != 'ASSUME': continue
        t = old['assumes'][b['id']]['to']
        if b['id'] in dropped and b['id'] not in replaced: closed.append('- %s -> CLOSED by page 2 (%s): %s' % (b['id'], dropped[b['id']][0], dropped[b['id']][1]))
        elif b['id'] in replaced: out += [dump(replaced[b['id']][1]), '']
        elif t.startswith('INTERNAL'): out += [dump(b, OLDK), '']
        else: closed.append('- %s -> %s' % (b['id'], ('page 2, item ' + t) if t.isdigit() else t))
    out += ['## CLOSED ASSUMPTIONS (one line each)'] + closed + ['']
    out.append('## QUESTIONS BACK')
    for b in B:
        if b['kind'] == 'ANSWER': out += [dump(b), '']; allAns += ['(topic %s)' % T, dump(b), '']
    out.append('## NAMES')
    newN = {b['id']: b for b in B if b['kind'] == 'NAME'}
    for b in ob:
        if b['kind'] == 'NAME' and b['id'] not in newN: out += [dump(b, OLDK), '']
    for b in newN.values(): out += [dump(b), '']; allNames += ['(topic %s)' % T, dump(b), '']
    for head, k in (('REACHES OTHER TOPICS (from the paper)', 'reach'), ('CONFLICTS (from the paper)', 'conf'), ('NOT DONE / UNSURE (from the paper)', 'unsure'), ('FOR THE PAGE RULING (from the ruling)', 'page')):
        if n[k]: out += ['## ' + head, n[k], '']
    if not DRY: open(os.path.join(S9, 'spec-%s.md' % T), 'w', encoding='utf-8').write('\n'.join(out) + '\n')
    allA.append('## TOPIC %s -- %s' % (T, TOPICS[T]))
    for head, k in (('The topic ruling\'s notes for the page ruling', 'page'), ('CONFLICTS (his newest words against earlier ones)', 'conf'), ('REACHES OTHER TOPICS', 'reach'), ('NOT DONE / UNSURE', 'unsure')):
        if n[k]: allA += ['### ' + head, n[k], '']
    allA.append('### The assumptions of this topic after the ruling (YES first, then LINE, then NO)')
    order = {'YES': 0, 'LINE': 1, 'NO': 2}
    for b in sorted([b for b in B if b['kind'] == 'ASSUME'], key=lambda b: order.get(b['fields'].get('ASK', 'NO').split(' ')[0].strip('.,;:'), 3)): allA += [dump(b), '']
    opn = [b for b in B if b['kind'] == 'ITEM' and b['fields'].get('STATUS', '').startswith('OPEN')]
    if opn: allA += ['### Items of page 2 that are still OPEN in this topic: ' + ', '.join(b['id'] for b in opn), '']
p252 = rd(os.path.join(S9, 'answer-252.md')); ph = rd(os.path.join(S9, 'answer-hold.md'))
if p252: allAns += ['## THE TECHNICAL PAPER ON ITEM 252 (answer-252.md): its recommendation, as written (the topic-B ruling\'s @@ANSWER 252 above is the settled text)', section(p252, 'RECOMMENDATION'), '### UNKNOWN UNTIL MEASURED', section(p252, 'UNKNOWN UNTIL MEASURED'), '']
if ph: allAns += ['## THE HOLD SETTING (answer-hold.md): what people do and whether it is worth building, as written', '### WHAT PEOPLE DO', section(ph, 'WHAT PEOPLE DO'), '### IS IT WORTH BUILDING', section(ph, 'IS IT WORTH BUILDING'), '']
if not DRY:
    w = lambda nme, head, lines: open(os.path.join(S9, nme), 'w', encoding='utf-8').write(head + '\n\n' + '\n'.join(lines) + '\n')
    w('assume3-all.md', '# EVERY ASSUMPTION THAT IS OPEN after Boris\'s answers to page 2, topic by topic (the ruled blocks; made by wf/merge3.py). ASK: YES = to be put to him; LINE = one line he can strike; NO = internal.', allA)
    w('answers3-all.md', '# THE ANSWERS BORIS IS OWED after page 2 (the ruled @@ANSWER blocks, then the two answer papers\' own sections; made by wf/merge3.py)', allAns)
    w('names3-all.md', '# EVERY NAME this round fixes or picks (ruled @@NAME blocks; made by wf/merge3.py)', allNames)
    w('ledger3.md', '# THE LEDGER OF PAGE 2: every item 217-273 after his answers (id | status | title | his words | the rule now). Made by wf/merge3.py from the ruled papers; spec-<topic>.md holds the full blocks.', ledger)
    w('today3-notes.md', '# HARMONY\'S OWN NOTES after page 2: what the app does now, item by item (the TODAY lines). Never shown to him.', today)
print('\n'.join(report))
na = [b for T in papers for b in papers[T] if b['kind'] == 'ASSUME']; a2 = lambda w: sum(1 for b in na if b['fields'].get('ASK', '').startswith(w))
print('TOTAL: page-2 items %d of 57 | assumptions %d (YES %d, LINE %d, NO %d) | old assumptions closed by this round %d, rewritten %d | answers %d | %s' % (sum(1 for b in allb if b['kind'] == 'ITEM'), len(na), a2('YES'), a2('LINE'), a2('NO'), len(dropped), len(replaced), sum(1 for b in allb if b['kind'] == 'ANSWER'), 'DRY RUN: nothing written' if DRY else 'written'))
