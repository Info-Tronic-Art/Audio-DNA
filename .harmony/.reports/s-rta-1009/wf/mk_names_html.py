# usage: python3 mk_names_html.py <NAMES.md> <out names.html>
#   -- his copy of the list of what everything is called: the same look as page 2's names.html
#      (the style and the markdown reader are taken from s-rta-1007/wf/render_page.py, lines "E = lambda" .. before "def ta(")
import sys, os, re, html
here = os.path.dirname(os.path.abspath(__file__))
old = os.path.join(here, '..', '..', 's-rta-1007', 'wf', 'render_page.py')
src = open(old, encoding='utf-8').read()
i = src.index('E = lambda s:'); j = src.index('\ndef ta(')
ns = {'html': html, 're': re, 'os': os}
exec(src[i:j], ns)
md = open(sys.argv[1], encoding='utf-8').read()
cut = md.split('\n## Notes for Harmony')[0]   # his copy ends before Harmony's own notes
open(sys.argv[2], 'w', encoding='utf-8').write(ns['page']('Audio-DNA — what everything is called', ns['md2html'](cut)))
print('written: %s (%d of %d lines of the list; "Review" left: %d, "Studio": %d)' % (sys.argv[2], cut.count('\n'), md.count('\n'), len(re.findall(r'\bReview\b', cut)), len(re.findall(r'\bStudio\b', cut))))
