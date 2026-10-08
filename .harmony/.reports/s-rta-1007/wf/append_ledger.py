# usage: python3 append_ledger.py <RS dir> <binding-decisions.md> "<stamp>"  -- appends the compact application block (items he named) under today's section
import sys, os, re, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from blocks import parse, TOPICS
RS, BD, NOW = sys.argv[1:4]
ids = json.load(open(os.path.join(RS, 'slice-ids.json')))
s = open(BD, encoding='utf-8').read()
assert 'THE APPLICATION OF HIS ANSWERS (ruled' not in s, 'already appended'
assert s.rstrip().split('\n')[-1].startswith('- THE READINGS HE DID NOT NAME'), 'the last line of binding-decisions.md is not the end of the 2026-10-07 section'
out = ['- THE APPLICATION OF HIS ANSWERS (ruled %s; s-rta-1007). Harmony\'s consequence text, not his words: how each item he NAMED stands after his message, as written by one architect per topic, re-checked blind, ruled per topic and across topics. The full blocks (every one of the 296 items of the last page, with status, the rule now, and Harmony\'s own notes on what the app does now): .harmony/.reports/s-rta-1007/spec-<topic letter>.md; one line per item: ledger.md; what is still assumed and is on his next page: page2-items.md. Status words: ANSWERED (his letter or his words), CORRECTED (a reading he changed in part), REPLACED (his words elsewhere say otherwise), STANDS, DEFAULT, SETTLED, DROPPED, OPEN.' % NOW]
tot = {}
for T in 'ABCDEFGHIJKX':
    blocks, _ = parse(open(os.path.join(RS, 'spec-%s.md' % T), encoding='utf-8').read())
    its = [b for b in blocks if b['kind'] == 'ITEM']
    for b in its:
        st = b['fields'].get('STATUS', '').split(' ')[0]; tot[st] = tot.get(st, 0) + 1
    if T == 'X': continue
    named = [b for b in its if re.search(r'\bL\d+\b', b['fields'].get('HIS', '')) and (b['id'].isdigit() or b['id'].startswith('R') or b['id'].startswith('G'))]
    if not named: continue
    out.append('  - Topic %s, %s:' % (T, TOPICS[T]))
    for b in named:
        f = b['fields']; r = f.get('RULE', '')
        if len(r) > 330: r = r[:330].rsplit(' ', 1)[0] + ' [...]'
        out.append('    - %s (%s; his lines %s) %s: %s' % (b['id'], f.get('STATUS', '').split(' ')[0], f.get('HIS', ''), f.get('TITLE', ''), r))
out.append('  - All 296 items by status: ' + ', '.join('%s %d' % kv for kv in sorted(tot.items())) + '.')
open(BD, 'a', encoding='utf-8').write('\n'.join(out) + '\n')
print('appended %d lines, %d characters; statuses: %s' % (len(out), len('\n'.join(out)), tot))
