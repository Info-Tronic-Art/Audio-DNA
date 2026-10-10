# usage: python3 check_quotes.py <workflow.js> <file with his words> [more files ...]
# Every value of the script's "const Q = {...}" / "const Q3 = {...}" tables must be a verbatim piece of one of his messages.
import sys, re
js = open(sys.argv[1], encoding='utf-8').read()
src = '\n'.join(open(p, encoding='utf-8').read() for p in sys.argv[2:])
tabs = re.findall(r'^const (Q\d?) = \{\n(.*?)^\}', js, re.M | re.S); assert tabs, 'no Q table'
bad = 0; n = 0; keys = set()
for name, body in tabs:
    for line in body.split('\n'):
        if not line.strip(): continue
        mm = re.match(r'^\s+(\w+): "(.*)",?\s*$', line); assert mm, line[:60]
        n += 1; keys.add(name + '.' + mm.group(1))
        if mm.group(2) not in src: bad += 1; print('NOT VERBATIM:', name, mm.group(1), '|', mm.group(2)[:90])
used = set(re.findall(r'\$\{(Q\d?\.\w+)\}', js))
print('quotes', n, 'not verbatim', bad, '| used but not in a table:', sorted(used - keys), '| in a table, unused:', sorted(keys - used))
