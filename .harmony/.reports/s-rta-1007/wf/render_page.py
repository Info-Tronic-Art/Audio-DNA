# usage: python3 render_page.py <RS dir>  -> boris-page-2.html from page2-items.md; names.html from names-draft.md; milkdrop.html from milkdrop-current.md (when they exist)
import sys, os, re, html
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from blocks import parse, TOPICS
RS = sys.argv[1]
E = lambda s: html.escape(s, quote=False)
CSS = """body{font:17px/1.55 -apple-system,Helvetica,Arial,sans-serif;max-width:820px;margin:32px auto;padding:0 20px;background:#14161a;color:#e6e6e6}
h1{font-size:27px;margin-bottom:4px}h2{font-size:21px;margin-top:48px;border-bottom:1px solid #333;padding-bottom:6px}h3{font-size:16px;margin:30px 0 6px;color:#9aa0a6;font-weight:600;text-transform:uppercase;letter-spacing:.04em}
a{color:#8fb8e8}.dim{color:#9aa0a6}.sub{color:#9aa0a6;margin-top:0}
.box{background:#1c1f25;border:1px solid #2c3038;border-radius:8px;padding:12px 16px;margin:12px 0}.box p{margin:6px 0}
.n{font-weight:700;font-size:24px;color:#d9c58a;margin-right:10px}
.t{margin:2px 0 4px}.a{margin:6px 0 0 4px;color:#cfd3d8}.a b{color:#d9c58a;margin-right:6px}
.ti{font-weight:600;font-size:18px;margin:6px 0 4px}
.hw{border-left:3px solid #d9c58a;padding:2px 0 2px 12px;color:#d9d2b8;font-style:italic;font-size:15px}
ul.l{padding-left:0;list-style:none;margin:6px 0}ul.l li{background:#1c1f25;border:1px solid #2c3038;border-radius:8px;padding:8px 14px;margin:6px 0}
.r{font-weight:700;color:#d9c58a;margin-right:8px}.alt{display:block;color:#aeb4bb;font-size:15px;margin-top:2px}.alt b{color:#d9c58a;margin-right:4px}
table{border-collapse:collapse;width:100%;margin:10px 0}td,th{border:1px solid #2c3038;padding:6px 10px;text-align:left;vertical-align:top;font-size:15px}th{background:#1c1f25}
code{font-family:ui-monospace,Menlo,monospace;font-size:14px;color:#d9c58a}
@media(max-width:600px){body{margin:16px auto}}"""
def page(title, body):
    return '<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>%s</title><style>%s</style></head><body>\n%s\n</body></html>\n' % (E(title), CSS, body)
def inline(s):
    s = E(s)
    s = re.sub(r'`([^`]+)`', r'<code>\1</code>', s)
    s = re.sub(r'\*\*([^*]+)\*\*', r'<b>\1</b>', s)
    s = re.sub(r'\[([^\]]+)\]\(([^)]+)\)', r'<a href="\2">\1</a>', s)
    return s
def md2html(md):
    out, i, L = [], 0, md.split('\n')
    while i < len(L):
        l = L[i]
        m = re.match(r'^(#{1,4})\s+(.*)$', l)
        if m: out.append('<h%d>%s</h%d>' % (len(m.group(1)), inline(m.group(2)), len(m.group(1)))); i += 1; continue
        if l.startswith('|'):
            rows = []
            while i < len(L) and L[i].startswith('|'):
                cells = [c.strip() for c in L[i].strip().strip('|').split('|')]
                if not all(re.match(r'^:?-{2,}:?$', c) for c in cells): rows.append(cells)
                i += 1
            if rows:
                out.append('<table><tr>%s</tr>' % ''.join('<th>%s</th>' % inline(c) for c in rows[0]))
                out += ['<tr>%s</tr>' % ''.join('<td>%s</td>' % inline(c) for c in r) for r in rows[1:]]
                out.append('</table>')
            continue
        if re.match(r'^\s*[-*]\s+', l):
            out.append('<ul>')
            while i < len(L) and re.match(r'^\s*[-*]\s+', L[i]): out.append('<li>%s</li>' % inline(re.sub(r'^\s*[-*]\s+', '', L[i]))); i += 1
            out.append('</ul>'); continue
        if l.strip():
            buf = []
            while i < len(L) and L[i].strip() and not re.match(r'^(#{1,4}\s|\||\s*[-*]\s)', L[i]): buf.append(L[i].strip()); i += 1
            out.append('<p>%s</p>' % inline(' '.join(buf))); continue
        i += 1
    return '\n'.join(out)
made = []
for src, dst, title in (('names-draft.md', 'names.html', 'Audio-DNA — what everything is called'), ('milkdrop-current.md', 'milkdrop.html', 'Audio-DNA — MilkDrop, how it works now')):
    p = os.path.join(RS, src)
    if os.path.exists(p):
        md = open(p, encoding='utf-8').read()
        if src == 'names-draft.md': md = md.split('\n## Notes for Harmony')[0]   # his copy ends before Harmony's own notes and the re-check record
        open(os.path.join(RS, dst), 'w', encoding='utf-8').write(page(title, md2html(md))); made.append(dst)
blocks, prob = parse(open(os.path.join(RS, 'page2-items.md'), encoding='utf-8').read())
items = [b for b in blocks if b['kind'] == 'PAGE-ITEM']; ans = [b for b in blocks if b['kind'] == 'PAGE-ANSWER']
ask = [b for b in items if b['fields'].get('KIND') == 'ASK']; line = [b for b in items if b['fields'].get('KIND') == 'LINE']
alts = lambda f: [(k.lower(), f[k]) for k in ('B', 'C') if f.get(k, 'none').strip().lower() not in ('none', '')]
words = sum(len(b['fields'].get(k, '').split()) for b in items for k in ('TEXT', 'B', 'C') if b['fields'].get(k, '') != 'none') + sum(len(b['fields'].get('TEXT', '').split()) + len(b['fields'].get('ASKED', '').split()) for b in ans)
mins = int(round(words / 130.0 / 5.0) * 5) or 5
o = []
o.append('<h1>Audio-DNA — what I still assume</h1><p class="sub">Page 2, after your answers of 7 October. Nothing is built.</p>')
o.append('<div class="box">')
o.append('<p>You wrote: <span class="hw" style="border:0;padding:0">“It would be best if you just asked me focused questions on any assumption that you\'re making.”</span> This page is that: one list, each line one thing I assume. No readings, and nothing about how the app works now; that stays in my notes.</p>')
o.append('<p>Your answers are filed word for word and applied everywhere, also where you left a repeat unanswered. The list you did not read, “Decided without asking you”, is folded in: what your answers settled is settled, and what they did not settle is below.</p>')
o.append('<p><b>How to answer:</b> write only what is wrong: “%d b”, or “%d no: …”. One line, “all good”, accepts everything else.</p>' % ((int(ask[0]['id']) if ask else 217), (int(ask[0]['id']) if ask else 217)))
o.append('<p>First, %d answers to what you asked me. Then %d assumptions that matter: you or the audience would see a wrong guess, or it would be costly to change once built. Then %d smaller ones, one line each. About %d minutes in all.</p>' % (len(ans), len(ask), len(line), mins))
o.append('<p>Where new things sit on screen is not asked: I lay them out where they fit, as you said, until the big UI redesign.</p>')
o.append('</div>')
o.append('<h2>1 · What you asked me</h2>')
for b in ans:
    f = b['fields']; q = re.sub(r'\s*\[L[\d, L-]+\]\s*$', '', f.get('ASKED', '')).strip()
    it = f.get('ITEM', 'none').strip()
    o.append('<div class="box"><div class="hw">%s</div><div class="ti">%s</div><p>%s</p>%s</div>' % (E(q), E(f.get('TITLE', '')), E(f.get('TEXT', '')), '' if it == 'none' else '<p class="dim">%s below.</p>' % (('Your choice is number %s' if len(it.split(',')) == 1 else 'Your choices are numbers %s') % ' and '.join('<a href="#i%s">%s</a>' % (x.strip(), x.strip()) for x in it.split(',')))))
def by_topic(bs):
    for T in 'ABCDEFGHIJKX':
        sel = [b for b in bs if b['fields'].get('TOPIC') == T]
        if sel: yield T, sel
o.append('<h2>2 · What I assume, where a wrong guess would matter</h2><p class="dim">Each one is what I will build unless you say otherwise. Where another way is possible it is given as b or c.</p>')
for T, sel in by_topic(ask):
    o.append('<h3>%s</h3>' % E(TOPICS[T]))
    for b in sel:
        f = b['fields']
        o.append('<div class="box" id="i%s"><p class="t"><span class="n">%s</span>%s</p>%s</div>' % (b['id'], b['id'], E(f.get('TEXT', '')), ''.join('<p class="a"><b>%s</b>%s</p>' % (k, E(t)) for k, t in alts(f))))
o.append('<h2>3 · Smaller assumptions, one line each</h2><p class="dim">Read them at a glance; write a number only if it is wrong.</p>')
for T, sel in by_topic(line):
    o.append('<h3>%s</h3><ul class="l">' % E(TOPICS[T]))
    for b in sel:
        f = b['fields']
        o.append('<li id="i%s"><span class="r">%s</span>%s%s</li>' % (b['id'], b['id'], E(f.get('TEXT', '')), ''.join('<span class="alt"><b>%s</b>%s</span>' % (k, E(t)) for k, t in alts(f))))
    o.append('</ul>')
o.append('<h2>4 · Made as you asked</h2><ul class="l">')
if 'names.html' in made: o.append('<li><a href="names.html">The list of what everything is called</a>. I keep it, and it is the truth for names from now on: in the app, in the menus, when we talk, and later in the manual. My own picks are marked; say so if one is wrong. Nothing in it has to be read now.</li>')
if 'milkdrop.html' in made: o.append('<li><a href="milkdrop.html">The MilkDrop document</a>: how MilkDrop works in the app now. It is for the session in which you design the new MilkDrop; nothing in it has to be read now.</li>')
o.append('</ul>')
o.append('<p class="dim" style="margin-top:40px">“All good” answers this page. I then tell you whether anything is still open. Nothing is built until you say that all is clear; the build starts in the session after that.</p>')
open(os.path.join(RS, 'boris-page-2.html'), 'w', encoding='utf-8').write(page('Audio-DNA — what I still assume (page 2)', '\n'.join(o)))
txt = '\n'.join(o)
txt = re.sub(r'</(p|div|li|h1|h2|h3|ul)>', '\n', txt); txt = re.sub(r'<h[123][^>]*>', '\n\n== ', txt); txt = re.sub(r'<span class="n">(\d+)</span>', r'[\1] ', txt); txt = re.sub(r'<span class="r">(\d+)</span>', r'[\1] ', txt); txt = re.sub(r'<b>([bc])</b>', r'   \1) ', txt)
txt = html.unescape(re.sub(r'<[^>]+>', '', txt)); txt = re.sub(r'\n{3,}', '\n\n', txt)
open(os.path.join(RS, 'boris-page-2.txt'), 'w', encoding='utf-8').write(txt.strip() + '\n')
print('rendered boris-page-2.html: %d answers, %d ASK, %d LINE, about %d words, about %d minutes; also made: %s; format problems in the items file: %d' % (len(ans), len(ask), len(line), words, mins, ', '.join(made) or 'nothing', len(prob)))
