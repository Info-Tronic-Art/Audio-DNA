#!/usr/bin/env python3
"""s-rta-1002 bt2 GATE-3 (ruling-bt2.md AM5): build a scratch test TU from a LANE test file -- its fixture prelude
(everything before the first TEST_CASE / `#if JUCE_MAC` line) plus the TEST_CASEs whose names start with the given IDs,
each cut VERBATIM (brace-matched, string / char literals and comments skipped). A case that sat inside `#if JUCE_MAC`
in the lane file is re-wrapped the same way.

usage: extract_cases.py <test.cpp> <out.cpp | --dump DIR> ID [ID ...]
Prints one line per case and exits non-zero when an ID is missing or ambiguous. --dump DIR writes the prelude and each
case to DIR/prelude.txt / DIR/<ID>.txt instead: run it on the lane file and on the built TU, then `diff -r` the two
dirs -- empty = every case (and the prelude) in the TU is byte-identical to the lane's.
"""
import re
import sys


def match_brace(src, open_at):
    depth = 0
    i = open_at
    n = len(src)
    while i < n:
        c = src[i]
        if src.startswith('//', i):
            i = src.index('\n', i)
            continue
        if src.startswith('/*', i):
            i = src.index('*/', i) + 2
            continue
        if src.startswith('R"', i):   # raw string R"delim( ... )delim"
            m = re.match(r'R"([^(]*)\(', src[i:])
            end = src.index(')' + m.group(1) + '"', i)
            i = end + len(m.group(1)) + 2
            continue
        if c == '"' or c == "'":
            j = i + 1
            while src[j] != c:
                j += 2 if src[j] == '\\' else 1
            i = j + 1
            continue
        if c == '{':
            depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise SystemExit('unbalanced braces')


def main():
    lane, out, ids = sys.argv[1], sys.argv[2], sys.argv[3:]
    ids_dir = None
    if out == '--dump':
        ids_dir, ids = ids[0], ids[1:]
    src = open(lane).read()
    lines = src.split('\n')
    first = next(k for k, l in enumerate(lines) if l.startswith('TEST_CASE(') or l.startswith('#if JUCE_MAC'))
    prelude = '\n'.join(lines[:first]) + '\n'
    # the `#if JUCE_MAC` ... `#endif` spans of the lane file (offsets)
    mac_spans = []
    for m in re.finditer(r'^#if JUCE_MAC\n', src, re.M):
        e = src.index('\n#endif', m.end())
        mac_spans.append((m.start(), e))
    pieces = []
    for cid in ids:
        hits = [m for m in re.finditer(r'^TEST_CASE\("' + re.escape(cid) + r' ', src, re.M)]
        if len(hits) != 1:
            raise SystemExit('case %s: %d matches in %s' % (cid, len(hits), lane))
        start = hits[0].start()
        body_open = src.index('\n{', start) + 1   # the body's brace opens its own line (this file's style)
        end = match_brace(src, body_open) + 1
        text = src[start:end]
        mac = any(a <= start < b for a, b in mac_spans)
        pieces.append(('#if JUCE_MAC\n' + text + '\n#endif' if mac else text))
        print('extracted %-4s %4d lines%s  %s' % (cid, text.count('\n') + 1, ' (#if JUCE_MAC)' if mac else '',
                                                 text.split('\n')[0][:100]))
    if out == '--dump':
        import os
        d = ids_dir
        os.makedirs(d, exist_ok=True)
        open(os.path.join(d, 'prelude.txt'), 'w').write(prelude)
        for cid, p in zip(ids, pieces):
            open(os.path.join(d, cid + '.txt'), 'w').write(p)
        return
    open(out, 'w').write(prelude + '\n\n'.join(pieces) + '\n')


if __name__ == '__main__':
    main()
