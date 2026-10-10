# usage: python3 lint3.py <paper.md> <topic letter>            -- an architect's paper apply-<T>.md
#        python3 lint3.py <ruling.md> <topic letter> --rule     -- a ruling's replacement blocks rule-<T>.md (checked against the paper beside it)
# Prints counts, then the problems, then ONE last line that begins "LINT:" (return that line verbatim). Exit 0 always.
import sys, os, re, json
here = os.path.dirname(os.path.abspath(__file__)); RS = os.path.join(here, '..')
sys.path.insert(0, here)
from blocks3 import parse, KINDS
path, T = sys.argv[1], sys.argv[2]; RULE = '--rule' in sys.argv
ids = json.load(open(os.path.join(RS, 'slice3-ids.json')))[T]
old = json.load(open(os.path.join(RS, 'old-index.json')))
text = open(path, encoding='utf-8').read()
blocks, problems = parse(text)
soft = []
paper = []
if RULE:
    pp = os.path.join(RS, 'apply-%s.md' % T)
    if os.path.exists(pp): paper = parse(open(pp, encoding='utf-8').read())[0]
    else: problems.append('the paper apply-%s.md is not there' % T)
pk = set((b['kind'], b['id']) for b in paper)
STAT = {'ANSWERED', 'ACCEPTED', 'OPEN', 'REPLACED'}
IFW = {'STAGE', 'REBUILD', 'SMALL'}; ASK = {'YES', 'LINE', 'NO'}
seen = {}; cited = set(); keys = {}
for b in blocks:
    k = (b['kind'], b['id']); keys[k] = keys.get(k, 0) + 1
    f = b['fields']
    for fld in ('HIS', 'SOURCE', 'WHY'):
        cited |= set(re.findall(r'BF\d{3}', f.get(fld, '')))
    if b['kind'] == 'ITEM':
        seen[b['id']] = seen.get(b['id'], 0) + 1
        if b['id'] not in ids['page_items']:
            problems.append('ITEM %s is not an item of page 2 in topic %s -- an OLD item is changed with @@AMEND blocks, never re-typed as @@ITEM' % (b['id'], T))
        st = (f.get('STATUS', '').split() or [''])[0].strip('.,;:')
        if st not in STAT: problems.append('ITEM %s: STATUS must begin with one of %s' % (b['id'], ' '.join(sorted(STAT))))
        if b['id'] in ids['empty'] and st == 'ANSWERED': problems.append('ITEM %s: he left its box empty -- STATUS is ACCEPTED (or REPLACED when words of his in another box say otherwise, or OPEN), not ANSWERED' % b['id'])
        if b['id'] in ids['page_items'] and b['id'] not in ids['empty'] and st == 'ACCEPTED': problems.append('ITEM %s: he wrote in its box -- STATUS is ANSWERED or OPEN, not ACCEPTED' % b['id'])
        if re.search(r'\btoday\b', f.get('RULE', ''), re.I): problems.append('ITEM %s: the RULE says "today" -- what the app does now belongs in TODAY only' % b['id'])
        if re.search(r'\bReview\b', f.get('RULE', '')): soft.append('ITEM %s: the RULE says "Review" -- the screen is called Studio now, unless you quote him' % b['id'])
    elif b['kind'] == 'AMEND':
        m = re.match(r'^(\S+)\s+(\d+)$', b['id'])
        if not m: problems.append('AMEND "%s": the id is "<old item id> <running number>", for example "R188 1"' % b['id']); continue
        oid = m.group(1)
        if oid not in old['items']: problems.append('AMEND %s: there is no @@ITEM %s in the s-rta-1007 specs' % (b['id'], oid)); continue
        o = f.get('OLD', '').strip(); rule = old['items'][oid]['rule']
        if o != 'APPEND':
            n = rule.count(o)
            if n != 1: problems.append('AMEND %s: OLD must be an exact piece of the RULE line of @@ITEM %s (spec-%s.md) found exactly ONCE -- found %d times. Copy it from the file (grep -n -A 7 "^@@ITEM %s"), or write OLD: APPEND' % (b['id'], oid, old['items'][oid]['T'], n, oid))
        if re.search(r'\btoday\b', f.get('NEW', ''), re.I): problems.append('AMEND %s: NEW says "today"' % b['id'])
        if not re.search(r'BF\d{3}', f.get('HIS', '')): problems.append('AMEND %s: HIS names no BF number (the box of his that makes this change), or the page item he accepted as "item 2xx accepted"' % b['id']) if 'accepted' not in f.get('HIS', '') else None
    elif b['kind'] == 'ASSUME':
        new = re.match(r'^%s3-\d+$' % T, b['id'])
        if not new and b['id'] not in old['assumes'] and (('ASSUME', b['id']) not in pk):
            problems.append('ASSUME %s: a NEW assumption is numbered %s3-<n>; an old one keeps its old id (and must exist in the s-rta-1007 specs)' % (b['id'], T))
        if (f.get('IF-WRONG', '').split(' ') or [''])[0].strip('.,;:') not in IFW: problems.append('ASSUME %s: IF-WRONG must begin with STAGE, REBUILD or SMALL' % b['id'])
        if (f.get('ASK', '').split(' ') or [''])[0].strip('.,;:') not in ASK: problems.append('ASSUME %s: ASK must begin with YES, LINE or NO' % b['id'])
        t = f.get('TEXT', '')
        if not t.startswith('I assume'): problems.append('ASSUME %s: TEXT must begin "I assume"' % b['id'])
        if re.search(r'\btoday\b|\bcurrently\b|\bright now\b|\bat the moment\b', t + ' ' + f.get('ALT', ''), re.I): problems.append('ASSUME %s: TEXT or ALT speaks of how the app is now ("today", "currently", "right now")' % b['id'])
        if re.search(r'\bR\d{3}\b|\b[A-KX]3?-\d+\b', t): problems.append('ASSUME %s: TEXT carries a reading number or a block id -- he does not read those' % b['id'])
        if len(t.split()) > 60: problems.append('ASSUME %s: TEXT is %d words (at most 45, hard stop 60)' % (b['id'], len(t.split())))
        elif len(t.split()) > 45: soft.append('ASSUME %s: TEXT is %d words (aim: at most 45)' % (b['id'], len(t.split())))
        if re.search(r'\bReview\b', t + ' ' + f.get('ALT', '')): soft.append('ASSUME %s: says "Review" -- the screen is called Studio now' % b['id'])
    elif b['kind'] == 'ANSWER':
        if len(f.get('ANSWER', '').split()) > 170: problems.append('ANSWER %s: %d words (at most 150)' % (b['id'], len(f.get('ANSWER', '').split())))
        if re.search(r'\bReview\b', f.get('ANSWER', '')): soft.append('ANSWER %s: says "Review" -- the screen is called Studio now' % b['id'])
    elif b['kind'] == 'DROP':
        parts = b['id'].split(None, 1)
        if len(parts) != 2 or parts[0] not in ('ASSUME', 'ITEM', 'AMEND', 'ANSWER', 'NAME'): problems.append('DROP "%s": the id is "<KIND> <id>", for example "ASSUME F-16"' % b['id']); continue
        if parts[0] == 'ASSUME' and parts[1] not in old['assumes'] and ('ASSUME', parts[1]) not in pk: problems.append('DROP %s: no such @@ASSUME in the s-rta-1007 specs%s' % (b['id'], ' or in the paper' if RULE else ''))
        if parts[0] in ('ITEM', 'AMEND', 'ANSWER', 'NAME') and not RULE: problems.append('DROP %s: a paper drops only old assumptions (DROP ASSUME <id>)' % b['id'])
        if parts[0] in ('ITEM', 'AMEND', 'ANSWER', 'NAME') and RULE and (parts[0], parts[1]) not in pk: problems.append('DROP %s: the paper has no such block' % b['id'])
for k, n in keys.items():
    if n > 1: problems.append('TWICE: %d blocks @@%s %s' % (n, k[0], k[1]))
if not RULE:
    for i in ids['page_items']:
        if seen.get(i, 0) == 0: problems.append('MISSING: no @@ITEM block for page item %s' % i)
    for bf in ids['boxes']:
        if bf not in cited: problems.append('NOT CITED: his box %s lands in this topic and no HIS / SOURCE / WHY line names it' % bf)
    have = set(b['id'] for b in blocks if b['kind'] == 'ANSWER')
    for a in ids['owed']:
        if a not in have: problems.append('MISSING: no @@ANSWER %s (he asks back; this topic owes the answer)' % a)
    for h in ('SUMMARY', 'ITEMS', 'AMENDMENTS', 'ASSUMPTIONS', 'QUESTIONS BACK', 'NAMES', 'REACHES OTHER TOPICS', 'CONFLICTS', 'NOT DONE / UNSURE'):
        if not re.search(r'^## ' + re.escape(h), text, re.M): problems.append('no heading "## %s"' % h)
else:
    if not re.search(r'^## VERDICTS', text, re.M): problems.append('no heading "## VERDICTS"')
    if not re.search(r'^## FOR THE PAGE RULING', text, re.M): problems.append('no heading "## FOR THE PAGE RULING"')
c = lambda k: sum(1 for b in blocks if b['kind'] == k)
ask = lambda w: sum(1 for b in blocks if b['kind'] == 'ASSUME' and b['fields'].get('ASK', '').startswith(w))
print('blocks: %d ITEM (page 2 has %d here), %d AMEND, %d ASSUME (%d YES, %d LINE, %d NO), %d ANSWER, %d NAME, %d DROP' % (c('ITEM'), len(ids['page_items']), c('AMEND'), c('ASSUME'), ask('YES'), ask('LINE'), ask('NO'), c('ANSWER'), c('NAME'), c('DROP')))
for p in problems[:80]: print(' ! ' + p)
for p in soft[:30]: print(' ~ ' + p)
print('LINT: %s -- %d ITEM, %d AMEND, %d ASSUME (%d YES, %d LINE, %d NO), %d ANSWER, %d NAME, %d DROP; %d problem(s) that must be fixed, %d soft note(s)' % ('OK' if not problems else 'NOT OK', c('ITEM'), c('AMEND'), c('ASSUME'), ask('YES'), ask('LINE'), ask('NO'), c('ANSWER'), c('NAME'), c('DROP'), len(problems), len(soft)))
