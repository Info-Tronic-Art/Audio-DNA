# usage: python3 lint_apply.py <paper.md> <slice letter>      -> prints OK or the problems (exit 0 either way; read the output)
#        python3 lint_apply.py --parse <paper.md>              -> prints counts only
# The paper format (s-rta-1007): blocks that open with a marker line at column 0 and close with a line "@@END".
#   @@ITEM <id>      fields TITLE, STATUS, HIS, RULE, CHANGED, TODAY
#   @@ASSUME <id>    fields ABOUT, TEXT, WHY, ALT, IF-WRONG, ASK
#   @@ANSWER <key>   fields HIS, ANSWER
#   @@NAME <term>    fields MEANS, SOURCE
# Every field is ONE physical line: "FIELD: text".
import sys, re, json, os

KINDS = {
    'ITEM': ['TITLE', 'STATUS', 'HIS', 'RULE', 'CHANGED', 'TODAY'],
    'ASSUME': ['ABOUT', 'TEXT', 'WHY', 'ALT', 'IF-WRONG', 'ASK'],
    'ANSWER': ['HIS', 'ANSWER'],
    'NAME': ['MEANS', 'SOURCE'],
}
STATUS = {'ANSWERED', 'DEFAULT', 'STANDS', 'CORRECTED', 'REPLACED', 'SETTLED', 'DROPPED', 'OPEN'}
IFW = {'STAGE', 'REBUILD', 'SMALL'}
ASK = {'YES', 'LINE', 'NO'}


def parse(text):
    """returns (blocks, problems); a block = dict(kind, id, fields{}, line)"""
    blocks, problems, cur = [], [], None
    for ln, line in enumerate(text.split('\n'), 1):
        m = re.match(r'^@@(ITEM|ASSUME|ANSWER|NAME)\s+(.+?)\s*$', line)
        if m:
            if cur is not None:
                problems.append('line %d: block %s %s was not closed with @@END before a new block' % (ln, cur['kind'], cur['id']))
                blocks.append(cur)
            cur = dict(kind=m.group(1), id=m.group(2).strip(), fields={}, line=ln, last=None)
            continue
        if line.strip() == '@@END':
            if cur is None:
                problems.append('line %d: @@END with no open block' % ln)
            else:
                blocks.append(cur)
                cur = None
            continue
        if cur is not None:
            m = re.match(r'^([A-Z][A-Z-]*):\s?(.*)$', line)
            if m and m.group(1) in KINDS[cur['kind']]:
                if m.group(1) in cur['fields']:
                    problems.append('line %d: field %s twice in %s %s' % (ln, m.group(1), cur['kind'], cur['id']))
                cur['fields'][m.group(1)] = m.group(2).strip()
                cur['last'] = m.group(1)
            elif line.strip():
                # a continuation line: tolerated (joined), but reported
                if cur['last']:
                    cur['fields'][cur['last']] += ' ' + line.strip()
                    problems.append('line %d: a field of %s %s runs over more than one line (%s) -- joined; keep each field on ONE line' % (ln, cur['kind'], cur['id'], cur['last']))
                else:
                    problems.append('line %d: text inside %s %s before any field' % (ln, cur['kind'], cur['id']))
    if cur is not None:
        problems.append('end of file: block %s %s was not closed with @@END' % (cur['kind'], cur['id']))
        blocks.append(cur)
    return blocks, problems


def lint(path, letter, here):
    text = open(path, encoding='utf-8').read()
    blocks, problems = parse(text)
    ids = json.load(open(os.path.join(here, '..', 'slice-ids.json')))[letter]
    nlines = len(open(os.path.join(here, '..', 'boris-msg-raw-1.txt'), encoding='utf-8').read().split('\n'))
    seen = {}
    for b in blocks:
        for f in KINDS[b['kind']]:
            if not b['fields'].get(f, '').strip():
                problems.append('%s %s: field %s is missing or empty' % (b['kind'], b['id'], f))
        fl = b['fields']
        if b['kind'] == 'ITEM':
            seen[b['id']] = seen.get(b['id'], 0) + 1
            st = fl.get('STATUS', '').split()[0] if fl.get('STATUS') else ''
            if st not in STATUS:
                problems.append('ITEM %s: STATUS "%s" is not one of %s (the first word must be the status)' % (b['id'], fl.get('STATUS', ''), ' '.join(sorted(STATUS))))
            if re.search(r'\btoday\b', fl.get('RULE', ''), re.I):
                problems.append('ITEM %s: the RULE line says "today" -- what the app does now belongs in TODAY only' % b['id'])
        if b['kind'] == 'ASSUME':
            if fl.get('IF-WRONG', '').split(' ')[0].strip('.,;') not in IFW:
                problems.append('ASSUME %s: IF-WRONG must begin with STAGE, REBUILD or SMALL' % b['id'])
            if fl.get('ASK', '').split(' ')[0].strip('.,;') not in ASK:
                problems.append('ASSUME %s: ASK must begin with YES, LINE or NO' % b['id'])
            t = fl.get('TEXT', '')
            if re.search(r'\btoday\b', t, re.I):
                problems.append('ASSUME %s: TEXT says "today"' % b['id'])
            if re.search(r'\bR\d{3}\b', t):
                problems.append('ASSUME %s: TEXT carries a reading number (R...) -- he does not want them' % b['id'])
            if len(t.split()) > 60:
                problems.append('ASSUME %s: TEXT is %d words (keep it under 45)' % (b['id'], len(t.split())))
        for f in ('HIS',):
            for m in re.finditer(r'\bL(\d+)\b', fl.get(f, '')):
                if int(m.group(1)) >= nlines:
                    problems.append('%s %s: HIS names L%s, which is past the end of his message' % (b['kind'], b['id'], m.group(1)))
    for i in ids:
        if seen.get(i, 0) == 0:
            problems.append('MISSING: no @@ITEM block for %s' % i)
        elif seen[i] > 1:
            problems.append('TWICE: %d @@ITEM blocks for %s' % (seen[i], i))
    for i in seen:
        if i not in ids:
            problems.append('NOT IN THE SLICE: @@ITEM %s (allowed only for a NEW item you had to add; say so in its TITLE)' % i)
    c = lambda k: sum(1 for b in blocks if b['kind'] == k)
    ay = sum(1 for b in blocks if b['kind'] == 'ASSUME' and b['fields'].get('ASK', '').startswith('YES'))
    al = sum(1 for b in blocks if b['kind'] == 'ASSUME' and b['fields'].get('ASK', '').startswith('LINE'))
    print('blocks: %d ITEM (slice has %d ids), %d ASSUME (%d ask YES, %d LINE), %d ANSWER, %d NAME' % (c('ITEM'), len(ids), c('ASSUME'), ay, al, c('ANSWER'), c('NAME')))
    hard = [p for p in problems if not p.startswith('NOT IN THE SLICE') and 'runs over more than one line' not in p and 'words (keep' not in p]
    if not problems:
        print('OK')
    else:
        print('%d problem(s), %d of them must be fixed:' % (len(problems), len(hard)))
        for p in problems[:80]:
            print(' -', p)
        if not hard:
            print('OK (only soft notes left)')


if __name__ == '__main__':
    here = os.path.dirname(os.path.abspath(__file__))
    if sys.argv[1] == '--parse':
        b, p = parse(open(sys.argv[2], encoding='utf-8').read())
        print(len(b), 'blocks;', len(p), 'problems')
    else:
        lint(sys.argv[1], sys.argv[2], here)
