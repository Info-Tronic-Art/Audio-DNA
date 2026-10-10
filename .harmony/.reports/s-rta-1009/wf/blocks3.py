# The block format of s-rta-1009 papers (page 2 applied): "@@KIND id" ... one "FIELD: text" per line ... "@@END".
# ITEM   = one item of page 2 (217-273) with the rule now.         AMEND = a change to the RULE line of an OLD item of the
# s-rta-1007 spec: OLD is an exact piece of that RULE (found exactly once) or the word APPEND; NEW replaces it / is appended.
import re
KINDS = {
    'ITEM': ['TITLE', 'STATUS', 'HIS', 'RULE', 'CHANGED', 'TODAY'],
    'AMEND': ['OLD', 'NEW', 'HIS', 'WHY'],
    'ASSUME': ['ABOUT', 'TEXT', 'WHY', 'ALT', 'IF-WRONG', 'ASK'],
    'ANSWER': ['HIS', 'ANSWER'],
    'NAME': ['MEANS', 'SOURCE'],
    'DROP': ['WHY'],
    'PAGE-ANSWER': ['ASKED', 'TITLE', 'TEXT', 'ITEM'],
    'PAGE-ITEM': ['TOPIC', 'KIND', 'TEXT', 'B', 'C', 'FROM'],
    'TRIAGE': ['TO', 'WHY'],
}
TOPICS = {'A': 'Triggering clips and the tempo bar', 'B': 'The cue system', 'C': 'Presets', 'D': 'Actions in a show', 'E': 'Studio and the recordings', 'F': 'The show file, decks and saving', 'G': 'Output screens', 'H': 'How a clip plays', 'I': 'Effects, signals and what moves a slider by itself', 'J': 'The keyboard and MIDI mapping, menus and messages', 'K': 'Sources and the automatic features', 'X': 'The loose lists'}

def parse(text, kinds=KINDS):
    blocks, problems, cur = [], [], None
    for ln, line in enumerate(text.split('\n'), 1):
        m = re.match(r'^@@([A-Z][A-Z-]*)(?:\s+(.+?))?\s*$', line)
        if m and m.group(1) in kinds:
            if cur is not None:
                problems.append('line %d: %s %s not closed with @@END' % (ln, cur['kind'], cur['id'])); blocks.append(cur)
            cur = dict(kind=m.group(1), id=(m.group(2) or '').strip(), fields={}, line=ln, last=None)
            continue
        if line.strip() == '@@END':
            if cur is None: problems.append('line %d: @@END with no open block' % ln)
            else: blocks.append(cur); cur = None
            continue
        if cur is not None:
            m = re.match(r'^([A-Z][A-Z-]*):\s?(.*)$', line)
            if m and m.group(1) in kinds[cur['kind']]:
                if m.group(1) in cur['fields']: problems.append('line %d: field %s twice in %s %s' % (ln, m.group(1), cur['kind'], cur['id']))
                cur['fields'][m.group(1)] = m.group(2).strip(); cur['last'] = m.group(1)
            elif line.strip():
                if cur['last']:
                    cur['fields'][cur['last']] += ' ' + line.strip()
                    problems.append('line %d: a field of %s %s runs over more than one line (%s) -- keep each field on ONE physical line' % (ln, cur['kind'], cur['id'], cur['last']))
                else: problems.append('line %d: text inside %s %s before any field' % (ln, cur['kind'], cur['id']))
    if cur is not None:
        problems.append('end of file: %s %s not closed with @@END' % (cur['kind'], cur['id'])); blocks.append(cur)
    for b in blocks:
        for f in kinds[b['kind']]:
            if not b['fields'].get(f, '').strip(): problems.append('%s %s: field %s missing or empty' % (b['kind'], b['id'], f))
    return blocks, problems

def dump(b, kinds=KINDS):
    return '\n'.join(['@@%s %s' % (b['kind'], b['id'])] + ['%s: %s' % (f, b['fields'].get(f, '')) for f in kinds[b['kind']]] + ['@@END'])

def section(text, head):
    m = re.search(r'^## ' + re.escape(head) + r'.*?$', text, re.M)
    if not m: return ''
    rest = text[m.end():]; n = re.search(r'^## ', rest, re.M)
    return (rest[:n.start()] if n else rest).strip()
