# usage: python3 file_answers.py <his answers file (byte-exact copy)> <the page's text copy> <entries table> <out dir>
# Files Boris's page answers word for word. His words are pulled BY BOX from the byte-exact file (never re-typed);
# the table carries only key|tag|title|gist. Asserts: every box of his has exactly one row; every content line is filed;
# every option text quoted from the page is a substring of the page's text copy.
# Writes: <out>/boris-answers-numbered.txt, file_backlog.txt, file_bd.txt, answers-by-item.md (the workflow's input).
import sys, re, os, hashlib
src, pagep, entp, out = sys.argv[1:5]
rawb = open(src, 'rb').read(); raw = rawb.decode('utf-8'); L = raw.split('\n')
page = open(pagep, encoding='utf-8').read()
open(os.path.join(out, 'boris-answers-numbered.txt'), 'w', encoding='utf-8').write(
    ''.join('%3d: %s\n' % (i + 1, l) for i, l in enumerate(L[:-1] if L[-1] == '' else L)))
# --- boxes: "=== key" opens, lines indented by two spaces follow, "=== END" closes
boxes = []; cur = None; end = False
for i, l in enumerate(L):
    if l.startswith('=== '):
        k = l[4:].strip()
        if k == 'END': end = True; cur = None; continue
        cur = dict(key=k, line=i + 1, text=[], lines=[]); boxes.append(cur)
    elif cur is not None and l.strip():
        assert l.startswith('  '), ('not indented', i + 1); cur['text'].append(l[2:]); cur['lines'].append(i + 1)
assert end, 'no === END'
head = [l for l in L[:6] if l.strip()]
m = re.search(r'comments: (\d+) of (\d+) boxes', raw); assert m and int(m.group(1)) == len(boxes), (m and m.group(0), len(boxes))
NB, NALL = int(m.group(1)), int(m.group(2))
for b in boxes: assert b['text'], ('empty box', b['key'])
# --- the page's items (number -> text, options)
def item(n):
    mm = re.search(r'^\[%d\] (.*?)(?=^\s*$)' % n, page, re.M | re.S); assert mm, n
    t = mm.group(1).rstrip('\n').replace('\n   ', '   ')
    parts = re.split(r'   ([bc])\) ', t); d = {'a': parts[0].strip()}
    for j in range(1, len(parts), 2): d[parts[j]] = parts[j + 1].strip()
    for v in d.values(): assert v in page, v[:50]
    return d
nums = [int(x) for x in re.findall(r'^\[(\d+)\] ', page, re.M)]
# --- entries
ents = {}
for row in open(entp, encoding='utf-8').read().split('\n'):
    if not row.strip() or row.startswith('#'): continue
    p = row.split('|'); assert len(p) == 4, row[:70]
    key, tag, title, gist = [x.strip() for x in p]; assert key not in ents, key; ents[key] = dict(tag=tag, title=title, gist=gist)
bk = [b['key'] for b in boxes]
assert sorted(bk) == sorted(ents), (sorted(set(bk) - set(ents)), sorted(set(ents) - set(bk)))
filed = set(n for b in boxes for n in b['lines'])
content = set(i + 1 for i, l in enumerate(L) if l.startswith('  ') and l.strip())
assert filed == content, sorted(content - filed)
answered = [int(k) for k in bk if k.isdigit()]; empty = [n for n in nums if n not in answered]
ansboxes = [k for k in bk if k.startswith('answer ')]
sha = hashlib.sha256(rawb).hexdigest()
HEAD = ("Boris's answers to page 2 (9 answers to what he asked; assumptions 217-273): %d comments in %d boxes -- %d on the numbered items, "
        "%d under Harmony's answers, 1 general; every box left empty accepted as written (%d items)" % (NB, NALL, len(answered), len(ansboxes), len(empty)))
META = ("the file his page's Save button wrote, ~/Downloads/audio-dna-page2-answers.txt, %s; %d bytes, sha256 %s...; build of the file = build of the page; "
        "byte-exact copy .harmony/.reports/s-rta-1009/boris-answers-page2.txt, numbered by line boris-answers-numbered.txt; it came with the session's first chat message, "
        "which carried the birth prompt and the file's path and no other words of his" % (head[1], len(rawb), sha[:16]))
bf = 240; bl = []; bd = []; ai = []
bl.append("## %s (recorded @@STAMP@@, session s-rta-1009) — the page as shown: .harmony/.reports/s-rta-1007/boris-page-2.txt" % HEAD)
bl.append("VERBATIM (the whole file, between the two marker lines; %s):" % META)
bl.append("<<<BORIS"); bl.append(raw.rstrip('\n')); bl.append(">>>BORIS")
bl.append("BACKLOG ITEMS (each restates his words only; a bracket \"read as\" marks a word of his that Harmony reads as a slip of dictation: INFERRED; how an answer changes the item it answers is this session's ruled application, .harmony/.reports/s-rta-1009/, appended to binding-decisions.md when ruled):")
bd.append("## 2026-10-09 (s-rta-1009) — %s (recorded @@STAMP@@; the whole file verbatim: boris-feedback-backlog.md, same stamp; the page as shown: .harmony/.reports/s-rta-1007/boris-page-2.txt)" % HEAD)
bd.append("- HOW TO READ THIS SECTION: his words are in quotes, pulled by box from the byte-exact file his page saved (never re-typed). The text after \"->\" restates his words only; where it quotes a way of the page (b, c), the words are the page's. How each answer changes its item and the spec blocks behind it is ruled in this session's application and appended below this section when ruled.")
ai.append("# Boris's answers to page 2, box by box (made by wf/file_answers.py from the byte-exact file; never edit by hand)")
ai.append("Each block: the box, the line of his file, the page's item as he saw it, HIS WORDS verbatim. A box that is not listed was left empty = accepted as written.")
ai.append("")
for b in boxes:
    e = ents[b['key']]; k = b['key']; q = ' '.join(b['text']); assert all(t in raw for t in b['text'])
    lab = ('item %s' % k) if k.isdigit() else ('his box under Harmony\'s answer "%s"' % k[7:] if k.startswith('answer ') else 'the box "Anything else?"')
    bl.append("BF%d [%s%s] %s" % (bf, e['tag'], (', ' + k) if k.isdigit() else '', e['gist']))
    bd.append("- %s (%s; file line %d; BF%d) — Boris: \"%s\" -> %s" % (e['title'], lab, b['lines'][0], bf, q, e['gist']))
    ai.append("@@BOX %s" % k); ai.append("BF: BF%d" % bf); ai.append("FILE-LINE: %d" % b['lines'][0])
    if k.isdigit():
        d = item(int(k)); ai.append("PAGE-ITEM: [%s] %s" % (k, d['a']))
        for o in ('b', 'c'):
            if o in d: ai.append("PAGE-WAY-%s: %s" % (o, d[o]))
    ai.append("HIS-WORDS: %s" % q); ai.append("@@END"); ai.append("")
    bf += 1
bd.append("- THE BOXES HE LEFT EMPTY ARE ACCEPTED AS WRITTEN (%d of %d), by the page's own rule, which he read and used (the page: \"Leave the others empty: an empty box counts as accepted.\"): items %s; and %d of Harmony's nine answers carry no comment. Where an item he left empty contradicts words he wrote in this file, his words win (his rule on repeats)." % (NALL - NB, NALL, ', '.join(str(n) for n in empty), 9 - len(ansboxes)))
assert "Leave the others empty: an empty box counts as accepted." in page
ai.append("@@EMPTY"); ai.append("ITEMS-ACCEPTED-AS-WRITTEN: %s" % ', '.join(str(n) for n in empty)); ai.append("@@END")
open(os.path.join(out, 'file_backlog.txt'), 'w', encoding='utf-8').write('\n'.join(bl) + '\n')
open(os.path.join(out, 'file_bd.txt'), 'w', encoding='utf-8').write('\n'.join(bd) + '\n')
open(os.path.join(out, 'answers-by-item.md'), 'w', encoding='utf-8').write('\n'.join(ai) + '\n')
print('boxes', len(boxes), 'of', NALL, '| items answered', len(answered), '| answer boxes', len(ansboxes), '| items left empty', len(empty))
print('BF240..BF%d' % (bf - 1), '| content lines filed', len(filed), 'of', len(content))
print('sizes', len('\n'.join(bl)), len('\n'.join(bd)), len('\n'.join(ai)))
