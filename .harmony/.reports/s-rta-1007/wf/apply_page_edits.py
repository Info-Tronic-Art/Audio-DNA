# usage: python3 apply_page_edits.py <RS dir>
# Lays page2-edits.md (the second ruling's edit blocks) over the round-1 list and renumbers. Re-runnable: it always starts from
# page2-items.round1.md (made from page2-items.md on the first run). Blocks in the edits file, the LAST one for an id wins:
#   @@PAGE-ITEM <round-1 number>   replaces that item          @@PAGE-ITEM NEW-<k>   adds an item
#   @@DROP PAGE-ITEM <round-1 number>                          @@PAGE-ANSWER <key>   replaces that answer
#   @@TRIAGE <assumption id>       replaces that triage block (TO may name a round-1 number or NEW-<k>)
import sys, os, re, shutil
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from blocks import parse, dump
RS = sys.argv[1]
base = os.path.join(RS, 'page2-items.round1.md'); cur = os.path.join(RS, 'page2-items.md')
if not os.path.exists(base): shutil.copyfile(cur, base)
blocks, p0 = parse(open(base, encoding='utf-8').read())
ed, p1 = parse(open(os.path.join(RS, 'page2-edits.md'), encoding='utf-8').read())
prob = ['edits: ' + p for p in p1]
ans = [b for b in blocks if b['kind'] == 'PAGE-ANSWER']; items = [b for b in blocks if b['kind'] == 'PAGE-ITEM']; tri = [b for b in blocks if b['kind'] == 'TRIAGE']
order = {b['id']: i for i, b in enumerate(items)}
dropped, nrep, nnew, ntri, nans = set(), 0, 0, 0, 0
for b in ed:
    if b['kind'] == 'DROP':
        m = re.match(r'^PAGE-ITEM\s+(\S+)$', b['id'])
        if not m or m.group(1) not in [x['id'] for x in items]: prob.append('DROP %s: write "@@DROP PAGE-ITEM <a number that is in the list>"' % b['id']); continue
        items = [x for x in items if x['id'] != m.group(1)]; dropped.add(m.group(1))
    elif b['kind'] == 'PAGE-ITEM':
        hit = [i for i, x in enumerate(items) if x['id'] == b['id']]
        if hit: items[hit[0]] = b; nrep += 1
        elif b['id'].startswith('NEW-'): order[b['id']] = 100000 + len(order); items.append(b); nnew += 1
        elif b['id'] in dropped: order.setdefault(b['id'], 100000 + len(order)); items.append(b); dropped.discard(b['id']); nrep += 1
        else: prob.append('PAGE-ITEM %s: not a number of the list and not NEW-<k>' % b['id'])
    elif b['kind'] == 'PAGE-ANSWER':
        hit = [i for i, x in enumerate(ans) if x['id'] == b['id']]
        if hit: ans[hit[0]] = b
        else: ans.append(b)
        nans += 1
    elif b['kind'] == 'TRIAGE':
        hit = [i for i, x in enumerate(tri) if x['id'] == b['id']]
        if hit: tri[hit[0]] = b
        else: tri.append(b)
        ntri += 1
    else: prob.append('%s %s: this kind of block does not belong in the edits file' % (b['kind'], b['id']))
T = 'ABCDEFGHIJKX'
key = lambda b: (0 if b['fields'].get('KIND') == 'ASK' else 1, T.index(b['fields'].get('TOPIC')) if b['fields'].get('TOPIC') in T else 99, order.get(b['id'], 10 ** 6))
items.sort(key=key)
mp = {}
for i, b in enumerate(items): mp[b['id']] = str(217 + i)
def remap(tok):
    t = tok.strip('.,;')
    return mp.get(t)
for b in tri:
    to = b['fields'].get('TO', ''); parts = to.split(' ')
    if parts and (parts[0].strip('.,;').isdigit() or parts[0].startswith('NEW-')):
        n = remap(parts[0])
        if n is None: prob.append('TRIAGE %s points to item %s, which is dropped or unknown: it needs a new @@TRIAGE block' % (b['id'], parts[0])); continue
        parts[0] = n
    elif parts and parts[0].strip('.,;') == 'MERGED' and len(parts) > 1:
        n = remap(parts[1])
        if n is None: prob.append('TRIAGE %s is MERGED into item %s, which is dropped or unknown: it needs a new @@TRIAGE block' % (b['id'], parts[1])); continue
        parts[1] = n
    b['fields']['TO'] = ' '.join(parts)
for b in ans:
    it = b['fields'].get('ITEM', 'none').strip()
    if it != 'none':
        new = []
        for x in it.split(','):
            n = remap(x)
            if n is None: prob.append('PAGE-ANSWER %s names item %s, which is dropped or unknown' % (b['id'], x.strip()))
            else: new.append(n)
        b['fields']['ITEM'] = ', '.join(new) if new else 'none'
# one wording pass (the list of names says "trigger"; "fire" is retired) -- never inside his own quoted words (ASKED)
W = [(r'\bfires\b', 'triggers'), (r'\bfired\b', 'triggered'), (r'\bfiring\b', 'triggering'), (r'\bfire\b', 'trigger'), (r'\bFires\b', 'Triggers'), (r'\bFired\b', 'Triggered'), (r'\bFiring\b', 'Triggering'), (r'\bFire\b', 'Trigger'), (r'\btempo row\b', 'tempo bar')]
nw = 0
for b in items + ans:
    for f in (('TEXT', 'B', 'C') if b['kind'] == 'PAGE-ITEM' else ('TEXT', 'TITLE')):
        s = b['fields'].get(f, '')
        for a, r in W:
            s, k = re.subn(a, r, s); nw += k
        b['fields'][f] = s
out = []
for b in ans: out += [dump(b), '']
for b in items:
    nb = dict(b); nb['id'] = mp[b['id']]; out += [dump(nb), '']
for b in tri: out += [dump(b), '']
open(cur, 'w', encoding='utf-8').write('\n'.join(out))
moved = ['%s->%s' % (k, v) for k, v in mp.items() if k != v]
print('applied: %d items replaced, %d new, %d dropped (%s), %d answers, %d triage blocks; %d words changed to the list of names; %d items now; numbers that moved: %s' % (nrep, nnew, len(dropped), ' '.join(sorted(dropped)) or 'none', nans, ntri, nw, len(items), ' '.join(moved) or 'none'))
if prob:
    print('%d problem(s):' % len(prob))
    for p in prob[:60]: print(' -', p)
else: print('APPLIED OK')
