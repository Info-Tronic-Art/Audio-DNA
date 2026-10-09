# usage: python3 read_answers.py [--dir <folder, default ~/Downloads>] [--page <the page's html, default ../boris-page-2.html>] [--copy-to <folder>] [--quiet]
# Finds the NEWEST audio-dna-page2-answers*.txt that is not older than the page itself (the page's "Save my answers for Claude"
# button writes it), checks it against the page's build, parses it and prints every comment. Exit 0 = found and well-formed,
# 3 = no file yet, 4 = a file is there but it is not well-formed (print it, do not guess).
# The file's format: header lines; then for each box a line that begins "=== " at the left edge and names the box
# ("answer <key>", an item number, or "general"), his words each indented by two spaces; "=== END" at the left edge ends it.
import sys, os, re, glob, hashlib, shutil, time
here = os.path.dirname(os.path.abspath(__file__))
arg = lambda n, d=None: sys.argv[sys.argv.index(n) + 1] if n in sys.argv else d
folder = os.path.expanduser(arg('--dir', '~/Downloads')); page = arg('--page', os.path.join(here, '..', 'boris-page-2.html')); quiet = '--quiet' in sys.argv
since = os.path.getmtime(page)
m = re.search(r"BUILD='([0-9a-f]+)'", open(page, encoding='utf-8').read()); build = m.group(1) if m else '?'
c = [f for f in glob.glob(os.path.join(folder, 'audio-dna-page2-answers*.txt')) if os.path.getmtime(f) >= since]
if not c:
    if not quiet: print('NO FILE YET: no audio-dna-page2-answers*.txt in %s that is newer than the page (%s)' % (folder, time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(since))))
    sys.exit(3)
f = max(c, key=os.path.getmtime); raw = open(f, 'rb').read(); text = raw.decode('utf-8')
lines = text.split('\n'); blocks, cur, ended = [], None, False
for l in lines:
    if ended:
        if l.strip(): cur = 'TEXT AFTER END'
        continue
    if l == '=== END': ended = True; cur = None; continue
    if l.startswith('=== '): cur = [l[4:], []]; blocks.append(cur); continue
    if cur is not None: cur[1].append(l[2:] if l.startswith('  ') else l)
ok = lines[0].startswith("AUDIO-DNA PAGE 2 -- BORIS'S ANSWERS") and ended and cur != 'TEXT AFTER END'
fb = re.search(r'build ([0-9a-f]+)\)', text); head = [l for l in lines[:6] if l.strip()]
print('FILE: %s\n  written %s; %d bytes; sha256 %s; others not older than the page: %d' % (f, time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(os.path.getmtime(f))), len(raw), hashlib.sha256(raw).hexdigest()[:16], len(c) - 1))
for l in head[1:4]: print('  ' + l)
print('  build of the file: %s; build of the page: %s -> %s' % (fb.group(1) if fb else '?', build, 'SAME' if fb and fb.group(1) == build else 'DIFFERENT: the page was re-made after he saved; show him and ask'))
if not ok:
    print('NOT WELL-FORMED (no header, no "=== END", or text after it): read the file itself, do not guess'); sys.exit(4)
if arg('--copy-to'):
    dst = os.path.join(arg('--copy-to'), 'boris-answers-page2.txt'); shutil.copyfile(f, dst); print('  copied byte-exact to %s' % dst)
print('COMMENTS: %d (every other box was left empty = accepted as written)' % len(blocks))
for k, v in blocks: print('--- %s\n%s' % (k, '\n'.join(v).strip('\n')))
