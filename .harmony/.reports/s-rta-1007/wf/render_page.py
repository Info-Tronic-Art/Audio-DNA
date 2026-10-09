# usage: python3 render_page.py <RS dir>  -> boris-page-2.html from page2-items.md; names.html from names-draft.md; milkdrop.html from milkdrop-current.md (when they exist)
import sys, os, re, html
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from blocks import parse, TOPICS
RS = sys.argv[1]
OUT = sys.argv[2] if len(sys.argv) > 2 else 'boris-page-2.html'
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
textarea.cm{display:block;width:100%;box-sizing:border-box;margin:10px 0 2px;padding:7px 10px;min-height:36px;resize:vertical;overflow:hidden;background:#101216;color:#e6e6e6;border:1px solid #6b7380;border-radius:6px;font:15px/1.4 -apple-system,Helvetica,Arial,sans-serif}
textarea.cm::placeholder{color:#8f96a0}textarea.cm.has{border-color:#9fd3a4}textarea.cm:focus,textarea.cm.has:focus{outline:none;border-color:#d9c58a;box-shadow:0 0 0 2px rgba(217,197,138,.45)}
ul.l li textarea.cm{margin-top:8px}
#sendbar{margin:40px 0 10px;padding:16px;background:#1c1f25;border:1px solid #d9c58a;border-radius:8px}
#send{font:600 17px -apple-system,Helvetica,Arial,sans-serif;padding:10px 22px;border-radius:8px;border:0;background:#d9c58a;color:#14161a;cursor:pointer;margin-top:10px}#send:focus{outline:3px solid #fff}
#sendmsg{margin-top:12px;color:#9fd3a4;font-weight:600}#sendmsg.bad{color:#e3a19a}#count{color:#9aa0a6;margin-left:14px;font-size:15px}
#sent{display:none;width:100%;box-sizing:border-box;height:160px;margin-top:10px;background:#101216;color:#cfd3d8;border:1px solid #3a3f49;border-radius:6px;font:13px/1.4 ui-monospace,Menlo,monospace;padding:8px}
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
def ta(key, hint, asks=False):
    return '<textarea class="cm" rows="1" data-k="%s"%s placeholder="%s" aria-label="Your comment on %s"></textarea>' % (E(key), ' data-q="1"' if asks else '', E(hint), E(key))
def hint(f):
    if '?' in f.get('TEXT', ''): return 'This one asks you something: type your answer.'
    n = len([k for k in ('B', 'C') if f.get(k, 'none').strip().lower() not in ('none', '')])
    return 'Right? Leave empty. Or type ' + ('b, c, or your own words.' if n == 2 else 'b, or your own words.' if n == 1 else 'what is wrong.')
import hashlib
BUILD = hashlib.sha1(open(os.path.join(RS, 'page2-items.md'), 'rb').read()).hexdigest()[:8]
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
o.append('<p><b>How to answer:</b> there is a box under every item. Type in it only where something is wrong or you want to add something: “b”, or your own words. Leave the others empty: an empty box counts as accepted. At the bottom press <b>Save my answers for Claude</b>, then start the next session; Claude reads your answers by itself. What you type is kept in this browser while you work, so you can stop and come back.</p>')
o.append('<p>First, %d answers to what you asked me. Then %d assumptions that matter: you or the audience would see a wrong guess, or it would be costly to change once built. Then %d smaller ones, one line each. About %d minutes in all.</p>' % (len(ans), len(ask), len(line), mins))
o.append('<p>Where new things sit on screen is not asked: I lay them out where they fit, as you said, until the big UI redesign.</p>')
o.append('</div>')
o.append('<h2>1 · What you asked me</h2>')
for b in ans:
    f = b['fields']; q = re.sub(r'\s*\[L[\d, L-]+\]\s*$', '', f.get('ASKED', '')).strip()
    it = f.get('ITEM', 'none').strip()
    o.append('<div class="box"><div class="hw">%s</div><div class="ti">%s</div><p>%s</p>%s</div>' % (E(q), E(f.get('TITLE', '')), E(f.get('TEXT', '')), '' if it == 'none' else '<p class="dim">%s below.</p>' % (('Your choice is number %s' if len(it.split(',')) == 1 else 'Your choices are numbers %s') % ' and '.join('<a href="#i%s">%s</a>' % (x.strip(), x.strip()) for x in it.split(',')))))
    o[-1] = o[-1][:-6] + ta('answer ' + b['id'], 'Fine? Leave empty. Or type a comment.') + '</div>'
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
        o[-1] = o[-1][:-6] + ta(b['id'], hint(f), '?' in f.get('TEXT', '')) + '</div>'
o.append('<h2>3 · Smaller assumptions, one line each</h2><p class="dim">Read them at a glance; write a number only if it is wrong.</p>')
for T, sel in by_topic(line):
    o.append('<h3>%s</h3><ul class="l">' % E(TOPICS[T]))
    for b in sel:
        f = b['fields']
        o.append('<li id="i%s"><span class="r">%s</span>%s%s</li>' % (b['id'], b['id'], E(f.get('TEXT', '')), ''.join('<span class="alt"><b>%s</b>%s</span>' % (k, E(t)) for k, t in alts(f))))
        o[-1] = o[-1][:-5] + ta(b['id'], hint(f), '?' in f.get('TEXT', '')) + '</li>'
    o.append('</ul>')
o.append('<h2>4 · Made as you asked</h2><ul class="l">')
if 'names.html' in made: o.append('<li><a href="names.html">The list of what everything is called</a>. I keep it, and it is the truth for names from now on: in the app, in the menus, when we talk, and later in the manual. My own picks are marked; say so if one is wrong. Nothing in it has to be read now.</li>')
if 'milkdrop.html' in made: o.append('<li><a href="milkdrop.html">The MilkDrop document</a>: how MilkDrop works in the app now. It is for the session in which you design the new MilkDrop; nothing in it has to be read now.</li>')
o.append('</ul>')
o.append('<p class="dim" style="margin-top:40px">Saving with every box empty means “all good”. I then tell you whether anything is still open. Nothing is built until you say that all is clear; the build starts in the session after that.</p>')
SEND = '''<div id="sendbar"><b>Anything else?</b> <span class="dim">A general comment, or something no item covers.</span>
''' + ta('general', 'Optional.') + '''
<p style="margin:12px 0 4px"><b>Every box you left empty counts as accepted.</b></p>
<button id="send" type="button">Save my answers for Claude</button><span id="count"></span>
<div id="sendmsg" role="status"></div>
<div class="dim" style="margin-top:10px;font-size:15px">The button saves your comments as a small text file (audio-dna-page2-answers.txt, usually in your Downloads folder) and copies them to the clipboard. Then start the next session with the prompt: Claude reads the file by itself. You can change a box and press again; the newest file counts. Shortcut here: Cmd+Enter.</div>
<textarea id="sent" readonly aria-label="What was saved"></textarea></div>'''
SELFTEST = r'''if(location.hash==='#selftest'){var t0=boxes[0],t1=document.querySelector('textarea.cm[data-k="217"]'),t2=document.querySelector('textarea.cm[data-k="general"]');
  t0.value='TEST first box';t1.value=' b ';t2.value='two\n=== END\nlines  ';var r=collect();document.body.setAttribute('data-selftest',JSON.stringify({n:r.n,boxes:boxes.length,open:r.open,text:r.text,keys:boxes.map(function(t){return t.getAttribute('data-k')})}))}'''
JS = r'''<script>
(function(){
var KEY='audio-dna-page2-answers-v1', FILE='audio-dna-page2-answers.txt', BUILD='@@BUILD@@';
var $=function(id){return document.getElementById(id)};
var boxes=[].slice.call(document.querySelectorAll('textarea.cm'));
var saved={}; try{saved=JSON.parse(localStorage.getItem(KEY)||'{}')}catch(e){saved={}}
if(!saved||typeof saved!=='object'||Array.isArray(saved))saved={};
var savedOnce=false;
function grow(t){t.style.height='auto';t.style.height=(t.scrollHeight+2)+'px'}
function count(){var n=0;boxes.forEach(function(t){var h=!!t.value.trim();if(h)n++;t.classList.toggle('has',h)});
  $('count').textContent=n?(n+(n===1?' comment':' comments')+'; every other box is empty and counts as accepted'):'No comments yet: saving now accepts everything';return n}
boxes.forEach(function(t){var k=t.getAttribute('data-k');if(typeof saved[k]==='string')t.value=saved[k];grow(t);
  t.addEventListener('input',function(){grow(t);saved[k]=t.value;try{localStorage.setItem(KEY,JSON.stringify(saved))}catch(e){}count();
    if(savedOnce){var m=$('sendmsg');m.className='bad';m.textContent='You changed something after saving. Press the button again.';$('sent').style.display='none'}})});
function collect(){var out=[],n=0,open=[];boxes.forEach(function(t){var v=t.value.replace(/^\s+|\s+$/g,'');
    if(v){n++;out.push('=== '+t.getAttribute('data-k')+'\n'+v.split('\n').map(function(l){return l?'  '+l:l}).join('\n'))}
    else if(t.getAttribute('data-q')==='1'){open.push(t.getAttribute('data-k'))}});
  var d=new Date(),p=function(x){return(x<10?'0':'')+x};
  var stamp=d.getFullYear()+'-'+p(d.getMonth()+1)+'-'+p(d.getDate())+' '+p(d.getHours())+':'+p(d.getMinutes())+':'+p(d.getSeconds());
  var head="AUDIO-DNA PAGE 2 -- BORIS'S ANSWERS\nsaved: "+stamp+" (from the page boris-page-2.html, build "+BUILD+")\ncomments: "+n+" of "+boxes.length+" boxes; every box left empty = accepted as written\nleft empty although the item asks a question: "+(open.length?open.join(', '):'none')+"\nformat: a line that begins \"=== \" at the left edge opens a box and names it; his words follow, each line indented by two spaces; \"=== END\" at the left edge is the last line\n";
  return{text:head+'\n'+out.join('\n\n')+(out.length?'\n\n':'')+'=== END\n',n:n,open:open}}
function oldCopy(text){try{var s=$('sent');s.style.display='block';s.value=text;s.focus();s.select();return document.execCommand('copy')}catch(e){return false}}
function send(){var r=collect(),msg=$('sendmsg'),box=$('sent'),file=false;
  msg.className='';msg.textContent='Saving...';box.style.display='block';box.value=r.text;savedOnce=true;
  try{var a=document.createElement('a');a.href=URL.createObjectURL(new Blob([r.text],{type:'text/plain;charset=utf-8'}));a.download=FILE;document.body.appendChild(a);a.click();a.remove();file=true}catch(e){}
  function done(clip){var what=r.n?(r.n+(r.n===1?' comment':' comments')):'no comments, everything accepted';
    var q=r.open.length?' Still empty although it asks you something: '+r.open.join(', ')+'.':'';
    if(file){msg.className='';msg.textContent='Saved ('+what+') as '+FILE+': your browser shows the download, usually in your Downloads folder.'+(clip?' Also copied to the clipboard.':'')+' Claude reads the file when the next session starts, or at once if a session is waiting for it.'+q;$('send').focus()}
    else if(clip){msg.className='bad';msg.textContent='The file could not be saved, but the text is on the clipboard: paste it to Claude.'+q;$('send').focus()}
    else{msg.className='bad';msg.textContent='The browser blocked both the file and the clipboard. The text below is selected: copy it (Cmd+C) and paste it to Claude.';box.focus();box.select()}}
  try{if(navigator.clipboard&&navigator.clipboard.writeText){navigator.clipboard.writeText(r.text).then(function(){done(true)},function(){done(oldCopy(r.text))})}else{done(oldCopy(r.text))}}catch(e){done(oldCopy(r.text))}}
$('send').addEventListener('click',send);
document.addEventListener('keydown',function(e){if((e.metaKey||e.ctrlKey)&&e.key==='Enter'&&!e.repeat&&$('sendbar').contains(document.activeElement)){e.preventDefault();send()}});
window.addEventListener('resize',function(){boxes.forEach(grow)});
count();
@@SELFTEST@@
})();
</script>'''
open(os.path.join(RS, OUT), 'w', encoding='utf-8').write(page('Audio-DNA — what I still assume (page 2)', '\n'.join(o + [SEND, JS.replace('@@BUILD@@', BUILD).replace('@@SELFTEST@@', SELFTEST if os.environ.get('PAGE_SELFTEST') else '')])))
txt = '\n'.join(o)
txt = re.sub(r'</(p|div|li|h1|h2|h3|ul)>', '\n', txt); txt = re.sub(r'<h[123][^>]*>', '\n\n== ', txt); txt = re.sub(r'<span class="n">(\d+)</span>', r'[\1] ', txt); txt = re.sub(r'<span class="r">(\d+)</span>', r'[\1] ', txt); txt = re.sub(r'<b>([bc])</b>', r'   \1) ', txt)
txt = html.unescape(re.sub(r'<[^>]+>', '', txt)); txt = re.sub(r'\n{3,}', '\n\n', txt)
open(os.path.join(RS, 'boris-page-2.txt'), 'w', encoding='utf-8').write(txt.strip() + '\n')
print('rendered boris-page-2.html: %d answers, %d ASK, %d LINE, about %d words, about %d minutes; also made: %s; format problems in the items file: %d' % (len(ans), len(ask), len(line), words, mins, ', '.join(made) or 'nothing', len(prob)))
