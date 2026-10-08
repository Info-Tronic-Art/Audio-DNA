# usage: python3 lint_rule.py <rule-T.md> <slice letter>  -> the last line reads OK when the ruling's blocks can be laid over the paper
import sys, os, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from blocks import parse
RS = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
path, T = sys.argv[1], sys.argv[2]
blocks, prob = parse(open(path, encoding='utf-8').read())
ids = json.load(open(os.path.join(RS, 'slice-ids.json')))[T]
ab, _ = parse(open(os.path.join(RS, 'apply-%s.md' % T), encoding='utf-8').read())
have = set((b['kind'], b['id']) for b in ab)
ST = {'ANSWERED', 'DEFAULT', 'STANDS', 'CORRECTED', 'REPLACED', 'SETTLED', 'DROPPED', 'OPEN'}
import re
for b in blocks:
    f = b['fields']
    if b['kind'] not in ('ITEM', 'ASSUME', 'ANSWER', 'NAME', 'DROP'): prob.append('%s %s: this kind of block does not belong in a topic ruling' % (b['kind'], b['id']))
    if b['kind'] == 'ITEM':
        if b['id'] not in ids: prob.append('ITEM %s is not an id of slice %s' % (b['id'], T))
        if f.get('STATUS', '').split(' ')[0] not in ST: prob.append('ITEM %s: STATUS must begin with one of %s' % (b['id'], ' '.join(sorted(ST))))
        if re.search(r'\btoday\b', f.get('RULE', ''), re.I): prob.append('ITEM %s: the RULE line says "today"' % b['id'])
    if b['kind'] == 'ASSUME':
        if f.get('ASK', '').split(' ')[0].strip('.,;') not in ('YES', 'LINE', 'NO'): prob.append('ASSUME %s: ASK must begin with YES, LINE or NO' % b['id'])
        if f.get('IF-WRONG', '').split(' ')[0].strip('.,;') not in ('STAGE', 'REBUILD', 'SMALL'): prob.append('ASSUME %s: IF-WRONG must begin with STAGE, REBUILD or SMALL' % b['id'])
        t = f.get('TEXT', '')
        if re.search(r'\btoday\b', t, re.I) or re.search(r'\bR\d{3}\b', t): prob.append('ASSUME %s: TEXT says "today" or carries a reading number' % b['id'])
        if len(t.split()) > 60: prob.append('ASSUME %s: TEXT is %d words (at most 40)' % (b['id'], len(t.split())))
    if b['kind'] == 'DROP':
        parts = b['id'].split(None, 1)
        k = (parts[0], parts[1]) if len(parts) == 2 else ('ASSUME', b['id'])
        if k not in have: prob.append('DROP %s names no block of the paper (write "@@DROP ASSUME <id>")' % b['id'])
c = lambda k: sum(1 for b in blocks if b['kind'] == k)
print('blocks: %d ITEM, %d ASSUME, %d ANSWER, %d NAME, %d DROP' % (c('ITEM'), c('ASSUME'), c('ANSWER'), c('NAME'), c('DROP')))
if prob:
    print('%d problem(s):' % len(prob))
    for p in prob[:60]: print(' -', p)
else: print('OK')
