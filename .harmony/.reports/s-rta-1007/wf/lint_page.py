# usage: python3 lint_page.py <RS dir>  -> checks page2-items.md (PAGE-ANSWER, PAGE-ITEM and TRIAGE blocks); the last line reads OK when it can be shown
import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from blocks import parse
RS = sys.argv[1]
blocks, prob = parse(open(os.path.join(RS, 'page2-items.md'), encoding='utf-8').read())
raw = ' '.join(open(os.path.join(RS, 'boris-msg-raw-1.txt'), encoding='utf-8').read().split())
ab, _ = parse(open(os.path.join(RS, 'assume-all.md'), encoding='utf-8').read())
assume_ids = [b['id'] for b in ab if b['kind'] == 'ASSUME']
OWED = ['codec', 'lowres', '189', 'R129-4', 'R184', 'R188-snapshot', 'R221-b', 'review-name', '207-hold']
NOW = re.compile(r'\b(today|currently|at present|at the moment|right now|as it is now|as it stands)\b', re.I)
items = [b for b in blocks if b['kind'] == 'PAGE-ITEM']; ans = [b for b in blocks if b['kind'] == 'PAGE-ANSWER']; tri = [b for b in blocks if b['kind'] == 'TRIAGE']
nums = []
for b in items:
    if not b['id'].isdigit(): prob.append('PAGE-ITEM %s: the id must be a number' % b['id']); continue
    nums.append(int(b['id'])); f = b['fields']
    if f.get('KIND') not in ('ASK', 'LINE'): prob.append('PAGE-ITEM %s: KIND must be ASK or LINE' % b['id'])
    if f.get('TOPIC') not in list('ABCDEFGHIJKX'): prob.append('PAGE-ITEM %s: TOPIC must be one letter A-K or X' % b['id'])
    for k, lim in (('TEXT', 55), ('B', 40), ('C', 40)):
        t = f.get(k, '')
        if len(t.split()) > lim: prob.append('PAGE-ITEM %s: %s is %d words (at most %d)' % (b['id'], k, len(t.split()), lim))
        if NOW.search(t): prob.append('PAGE-ITEM %s: %s speaks of the app as it is now ("%s")' % (b['id'], k, NOW.search(t).group(0)))
        if re.search(r'\bR\d{3}\b', t): prob.append('PAGE-ITEM %s: %s carries a reading number' % (b['id'], k))
    if f.get('KIND') == 'LINE' and len(f.get('TEXT', '').split()) > 32: prob.append('PAGE-ITEM %s: a LINE item is at most 30 words (%d)' % (b['id'], len(f.get('TEXT', '').split())))
if nums:
    if nums[0] != 217: prob.append('the first PAGE-ITEM must be 217 (it is %d)' % nums[0])
    if nums != list(range(nums[0], nums[0] + len(nums))): prob.append('PAGE-ITEM numbers must run without a gap in the order of the file')
order = 'ABCDEFGHIJKX'; last = (0, 0)
for b in items:
    f = b['fields']
    if f.get('TOPIC') in order and f.get('KIND') in ('ASK', 'LINE'):
        cur = (0 if f['KIND'] == 'ASK' else 1, order.index(f['TOPIC']))
        if cur < last: prob.append('PAGE-ITEM %s: out of order (first every ASK item, topics A to K then X; then every LINE item, topics A to K then X)' % b['id'])
        last = cur
keys = [b['id'] for b in ans]
for k in OWED:
    if k not in keys: prob.append('PAGE-ANSWER %s is missing (he asked; an answer is owed)' % k)
for b in ans:
    f = b['fields']
    if len(f.get('TEXT', '').split()) > 175: prob.append('PAGE-ANSWER %s: TEXT is %d words (at most 150)' % (b['id'], len(f.get('TEXT', '').split())))
    q = re.findall(r'"([^"]{12,})"', f.get('ASKED', ''))
    if not q: prob.append('PAGE-ANSWER %s: ASKED holds no quoted words of his' % b['id'])
    for s in q:
        if ' '.join(s.split()) not in raw: prob.append('PAGE-ANSWER %s: ASKED is not verbatim: "%s..."' % (b['id'], s[:60]))
    it = f.get('ITEM', 'none').strip()
    if it != 'none' and not all(x.strip().isdigit() and int(x) in nums for x in it.split(',')): prob.append('PAGE-ANSWER %s: ITEM must be "none" or page item numbers that exist' % b['id'])
tid = [b['id'] for b in tri]
for a in assume_ids:
    if tid.count(a) != 1: prob.append('TRIAGE: assumption %s has %d TRIAGE blocks (exactly one is needed)' % (a, tid.count(a)))
for b in tri:
    to = b['fields'].get('TO', '').split(' ')[0].strip('.,;')
    if to not in ('INTERNAL', 'SETTLED', 'MERGED') and not (to.isdigit() and int(to) in nums): prob.append('TRIAGE %s: TO must begin with a page item number that exists, or INTERNAL, SETTLED or MERGED <number>' % b['id'])
    if b['id'] not in assume_ids: prob.append('TRIAGE %s: no such assumption in assume-all.md' % b['id'])
words = sum(len(b['fields'].get(k, '').split()) for b in items for k in ('TEXT', 'B', 'C') if b['fields'].get(k, '') != 'none') + sum(len(b['fields'].get('TEXT', '').split()) + len(b['fields'].get('ASKED', '').split()) for b in ans)
na = sum(1 for b in items if b['fields'].get('KIND') == 'ASK')
print('page: %d answers, %d items (%d ASK, %d LINE), %d TRIAGE blocks for %d assumptions, about %d words = about %d minutes' % (len(ans), len(items), na, len(items) - na, len(tri), len(assume_ids), words, round(words / 130.0)))
if prob:
    print('%d problem(s):' % len(prob))
    for p in prob[:80]: print(' -', p)
else: print('OK')
