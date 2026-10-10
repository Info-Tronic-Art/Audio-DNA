# usage: python3 html_text.py <page.html> <out.txt>  -- the rendered page as the plain text he reads (scripts and styles left out), one line per element
import sys, re, html
h = open(sys.argv[1], encoding='utf-8').read()
body = h.split('<body', 1)[1]
body = re.sub(r'<script.*?</script>', '', body, flags=re.S); body = re.sub(r'<style.*?</style>', '', body, flags=re.S)
body = re.sub(r'<textarea[^>]*data-k="([^"]*)"[^>]*>.*?</textarea>', r'\n[comment box: \1]\n', body, flags=re.S)
body = re.sub(r'<(/?)(b|i|em|strong|a|code|span)(\s[^>]*)?>', '', body)
t = html.unescape(re.sub(r'<[^>]+>', '\n', body)); t = re.sub(r'[ \t]+\n', '\n', t); t = re.sub(r'\n\s*\n+', '\n', t).strip()
open(sys.argv[2], 'w', encoding='utf-8').write(t + '\n')
print('written %s: %d lines, %d words, %d comment boxes' % (sys.argv[2], t.count('\n') + 1, len(t.split()), t.count('[comment box: ')))
