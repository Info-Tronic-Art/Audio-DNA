# usage: python3 mkslices3.py <s-rta-1007 dir> <s-rta-1009 dir>
# Cuts page 2 and Boris's answers per topic: slice3-<T>.md (what an architect, a checker and a ruling read first),
# slice3-ids.json (ids per topic, for the lint) and old-index.json (every block of the s-rta-1007 specs by id).
import sys, os, re, json
S7, S9 = sys.argv[1:3]
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from blocks3 import parse, TOPICS
rd = lambda p: open(p, encoding='utf-8').read()
OLDK = {'ITEM': ['TITLE', 'STATUS', 'HIS', 'RULE', 'CHANGED', 'TODAY'], 'ASSUME': ['ABOUT', 'TEXT', 'WHY', 'ALT', 'IF-WRONG', 'ASK'], 'ANSWER': ['HIS', 'ANSWER'], 'NAME': ['MEANS', 'SOURCE']}
PK = {'PAGE-ANSWER': ['ASKED', 'TITLE', 'TEXT', 'ITEM'], 'PAGE-ITEM': ['TOPIC', 'KIND', 'TEXT', 'B', 'C', 'FROM'], 'TRIAGE': ['TO', 'WHY']}
BOXK = {'BOX': ['BF', 'FILE-LINE', 'PAGE-ITEM', 'PAGE-WAY-b', 'PAGE-WAY-c', 'HIS-WORDS'], 'EMPTY': ['ITEMS-ACCEPTED-AS-WRITTEN']}
# --- his boxes (answers-by-item.md is made by file_answers.py)
boxes = {}; cur = None
for line in rd(os.path.join(S9, 'answers-by-item.md')).split('\n'):
    if line.startswith('@@BOX '): cur = dict(key=line[6:].strip(), raw=[line]); boxes[cur['key']] = cur
    elif cur is not None:
        cur['raw'].append(line)
        if line.startswith('BF: '): cur['bf'] = line[4:].strip()
        if line.startswith('HIS-WORDS: '): cur['his'] = line[11:]
        if line.strip() == '@@END': cur = None
assert len(boxes) == 33, len(boxes)
# --- page 2
pb, pp = parse(rd(os.path.join(S7, 'page2-items.md')), PK); assert not pp, pp[:3]
items = [b for b in pb if b['kind'] == 'PAGE-ITEM']; answers = [b for b in pb if b['kind'] == 'PAGE-ANSWER']; triage = {b['id']: b['fields'] for b in pb if b['kind'] == 'TRIAGE'}
ANS_TOPIC = {'codec': 'H', 'lowres': 'E', '189': 'A', 'R129-4': 'D', 'R184': 'E', 'R188-snapshot': 'F', 'R221-b': 'I', 'review-name': 'J', '207-hold': 'J'}
assert sorted(a['id'] for a in answers) == sorted(ANS_TOPIC), [a['id'] for a in answers]
item_topic = {b['id']: b['fields']['TOPIC'] for b in items}
def box_topic(k):
    if k.isdigit(): return item_topic[k]
    if k.startswith('answer '): return ANS_TOPIC[k[7:]]
    return 'K'   # the box "Anything else?": the audio clips he will bring
# what Boris asks back, and which topic's paper owes the answer (246 is answered by its own seat after the research)
OWED = {'A': ['250'], 'B': ['222', '252'], 'E': ['lowres'], 'H': ['239', 'codec'], 'K': ['general']}
# --- the old specs
old = {'items': {}, 'assumes': {}}; per = {}
for T in TOPICS:
    b, p = parse(rd(os.path.join(S7, 'spec-%s.md' % T)), OLDK); assert not p, (T, p[:3])
    per[T] = b
    for x in b:
        if x['kind'] == 'ITEM': assert x['id'] not in old['items'], x['id']; old['items'][x['id']] = dict(T=T, title=x['fields']['TITLE'], status=x['fields']['STATUS'], rule=x['fields']['RULE'])
        if x['kind'] == 'ASSUME':
            t = triage.get(x['id'], {'TO': 'NO TRIAGE BLOCK', 'WHY': ''})
            old['assumes'][x['id']] = dict(T=T, to=t['TO'], why=t['WHY'], text=x['fields']['TEXT'], about=x['fields']['ABOUT'])
ids = {}
for T in TOPICS:
    its = [b for b in items if b['fields']['TOPIC'] == T]
    bx = [k for k in boxes if box_topic(k) == T]
    o = ['# SLICE %s -- %s: page 2 as Boris saw it, and what he did with it (made by wf/mkslices3.py; never edit by hand)' % (T, TOPICS[T]), '']
    o += ['## HIS BOXES THAT LAND IN THIS TOPIC (%d; his words verbatim, with the BF number of the filing and the line of his file)' % len(bx), '']
    for k in bx: o += boxes[k]['raw'] + ['']
    if not bx: o += ['(none: he left every box of this topic empty)', '']
    o += ['## THE PAGE-2 ITEMS OF THIS TOPIC (%d), each as he read it' % len(its), '']
    for b in its:
        f = b['fields']; k = b['id']
        state = ('HE WROTE IN ITS BOX (%s): "%s"' % (boxes[k]['bf'], boxes[k]['his'])) if k in boxes else 'HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN'
        o += ['### [%s] (%s)' % (k, 'a card of its own' if f['KIND'] == 'ASK' else 'one line in the small list'), 'TEXT: ' + f['TEXT'], 'B: ' + f['B'], 'C: ' + f['C'],
              'MADE FROM (blocks of the s-rta-1007 specs; an id like %s-4 is an @@ASSUME, any other id is an @@ITEM): %s' % (T, f['FROM']), state, '']
    an = [a for a in answers if ANS_TOPIC[a['id']] == T]
    o += ['## HARMONY\'S ANSWERS ON PAGE 2 THAT BELONG TO THIS TOPIC (%d), each as he read it' % len(an), '']
    for a in an:
        f = a['fields']; k = 'answer ' + a['id']
        state = ('HE WROTE IN ITS BOX (%s): "%s"' % (boxes[k]['bf'], boxes[k]['his'])) if k in boxes else 'HE LEFT ITS BOX EMPTY (no comment)'
        o += ['### answer %s' % a['id'], 'HE HAD ASKED: ' + f['ASKED'], 'TITLE: ' + f['TITLE'], 'TEXT: ' + f['TEXT'], 'LEADS TO ITEM: ' + f['ITEM'], state, '']
    if not an: o += ['(none)', '']
    oa = [x for x in per[T] if x['kind'] == 'ASSUME']
    o += ['## THE OLD ASSUMPTIONS OF THIS TOPIC (%d @@ASSUME blocks in spec-%s.md of s-rta-1007) AND WHERE EACH WENT' % (len(oa), T),
          'A number = it became that item of page 2 (now answered or accepted: closed). SETTLED / MERGED = closed before page 2. INTERNAL = never shown to him: still Harmony\'s own, and STILL OPEN -- test each against his new words.', '']
    for x in oa:
        t = old['assumes'][x['id']]; o.append('- %s -> %s%s' % (x['id'], t['to'], (': ' + t['why']) if t['to'].split()[0] in ('SETTLED', 'MERGED', 'INTERNAL') and t['why'] else ''))
    o += ['', '## IDS', 'PAGE-2 ITEMS: ' + (' '.join(b['id'] for b in its) or 'none'), 'BOXES: ' + (' '.join('%s(%s)' % (boxes[k]['bf'], k) for k in bx) or 'none'),
          'ANSWERS OWED BY THIS TOPIC\'S PAPER (an @@ANSWER block each): ' + (' '.join(OWED.get(T, [])) or 'none'),
          'OLD @@ITEM IDS IN spec-%s.md: %s' % (T, ' '.join(x['id'] for x in per[T] if x['kind'] == 'ITEM')),
          'OLD @@ASSUME IDS STILL OPEN (INTERNAL): ' + (' '.join(x['id'] for x in oa if old['assumes'][x['id']]['to'].startswith('INTERNAL')) or 'none'), '']
    open(os.path.join(S9, 'slice3-%s.md' % T), 'w', encoding='utf-8').write('\n'.join(o))
    ids[T] = dict(page_items=[b['id'] for b in its], empty=[b['id'] for b in its if b['id'] not in boxes], boxes=[boxes[k]['bf'] for k in bx], box_keys=bx, owed=OWED.get(T, []),
                  old_items=[x['id'] for x in per[T] if x['kind'] == 'ITEM'], old_assumes=[x['id'] for x in oa],
                  old_internal=[x['id'] for x in oa if old['assumes'][x['id']]['to'].startswith('INTERNAL')])
    print(T, 'items', len(its), 'empty', len(ids[T]['empty']), 'boxes', len(bx), 'answers', len(an), 'old items', len(ids[T]['old_items']), 'old assumes', len(oa), 'internal', len(ids[T]['old_internal']), 'bytes', len('\n'.join(o)))
assert sum(len(v['boxes']) for v in ids.values()) == 33 and sum(len(v['page_items']) for v in ids.values()) == 57
json.dump(ids, open(os.path.join(S9, 'slice3-ids.json'), 'w'), indent=0)
json.dump(old, open(os.path.join(S9, 'old-index.json'), 'w'))
print('old items', len(old['items']), 'old assumes', len(old['assumes']), 'no triage:', [k for k, v in old['assumes'].items() if v['to'] == 'NO TRIAGE BLOCK'])
