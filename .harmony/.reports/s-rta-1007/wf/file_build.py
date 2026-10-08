import sys,re,os
raw=open(sys.argv[1],encoding='utf-8').read(); L=raw.split('\n'); S=sys.argv[3]
ents=[]
for row in open(sys.argv[2],encoding='utf-8').read().split('\n'):
    if not row.strip(): continue
    p=row.split('|'); assert len(p)==5,row[:60]
    spec,key,tag,title,gist=p; same=spec.endswith('^'); spec=spec.rstrip('^')
    idx=[]
    for part in spec.split(','):
        if '-' in part: a,b=part.split('-'); idx+=list(range(int(a),int(b)+1))
        else: idx.append(int(part))
    for i in idx: assert L[i].strip(), ('empty line',i)
    quote=' / '.join('"%s"'%L[i].strip() for i in idx) if len(idx)<3 else '"%s"'%'\n'.join(L[i] for i in idx)
    for i in idx: assert L[i] in raw
    ents.append(dict(idx=idx,key=key,tag=tag,title=title,gist=gist,same=same,quote=quote))
# coverage: every non-empty content line of his message is filed by some entry
used=set(i for e in ents for i in e['idx'])
skip=lambda l: (not l.strip()) or l.startswith('<pasted_content') or l.startswith('</pasted_content') or l.strip().startswith('[Image #') or l.strip()=='General Questions:'
missing=[i for i,l in enumerate(L) if not skip(l) and i not in used]
assert not missing, missing
qnums=sorted(set(int(e['key']) for e in ents if e['key'].isdigit()))
rnums=[e['key'] for e in ents if e['key'].startswith('R')]; rn=sorted(set(rnums),key=lambda x:int(x[1:]))
allq=list(range(172,217)); unnamed=[n for n in allq if n not in qnums]
HEAD="Boris's answers to the page of ALL open questions (172-216; readings R127-R228): all defaults good except the %d questions and %d readings he names; seven general points (a codec of our own? copy / paste and Option-drag for clips; a low-resolution show recording in chunks; ignore actions on a layer; layout freedom until the big UI redesign); his pages get shorter: focused questions on assumptions, no \"today\"; a list of what everything is called; a MilkDrop document"%(len(qnums),len(rn))
META="session record line 10, type user, 2026-10-08T02:19:09.680Z UTC = 2026-10-07 22:19:09 local; byte-exact copy .harmony/.reports/s-rta-1007/boris-msg-raw-1.txt (18065 chars, sha256 89e3deffbb80b509...; the whole pasted block with the birth prompt he pasted first: boris-msg-raw-1-full.txt)"
PICS=["[Image #4] (with R206 g; boris-images/img-04-7dc96e027a08.png, 608 x 892): Resolume's window \"Manage Presets\": a heading \"Presets\", a list of three names -- Blue, Green, Red -- and two buttons, Cancel and Save (Save in mint).",
"[Image #6] (with R154; boris-images/img-06-ccaf044f8203.png, 872 x 986): Resolume's effects tab, list \"VIDEO EFFECTS\": Acuarela; Add Subtract opened with its presets Blue, Green, Red (Red selected); Auto Mask; Bendoscope; Bloom; Blow opened with Bright Lines, Solid; Blur; Bright.Contrast. Below the list, the properties of the double-clicked entry: a header bar \"Add Subtract\" with a small \"P\" at its right end; rows Blend Mode (a drop-down reading \"Add\", a small \"P\" at the right), Opacity 100 %, R 0 %, G -100 %, B -100 %, each with \"-\" \"+\" and a slider bar.",
"[Image #8] (with R134; boris-images/img-08-85be4ebfacaf.png, 276 x 326): one clip cell of Audio-DNA with a cyan border: the thumbnail of \"Plasma Burst\" with a \"SOURCE\" badge top right and a \"3 FX\" badge at the thumbnail's lower right; the name \"Plasma Burst\" under it; under the name a wide, low button row reading \"Retrigger\"; empty space below."]
bf=154; bl=[]; bd=[]; an=[]
bl.append("## %s (recorded @@STAMP@@, session s-rta-1007) — questions and readings as asked: .harmony/.reports/s-rta-1005/boris-clarify-all.md"%HEAD)
bl.append("VERBATIM (whole message, between the two marker lines; %s):"%META)
bl.append("<<<BORIS"); bl.append(raw.rstrip('\n')); bl.append(">>>BORIS")
bl.append("THE THREE PICTURES (extracted from the session record, sized with sips; .harmony/.reports/s-rta-1007/):")
for p in PICS: bl.append("- "+p)
bl.append("BACKLOG ITEMS (each restates his words only; how an answer changes the item it answers -- the question's letter, the reading's lines -- is this session's application ledger: .harmony/.reports/s-rta-1007/, appended to binding-decisions.md when ruled):")
bd.append("## 2026-10-07 (s-rta-1007) — %s (recorded @@STAMP@@; whole message verbatim + three pictures described: boris-feedback-backlog.md, same stamp; questions and readings as asked: .harmony/.reports/s-rta-1005/boris-clarify-all.md)"%HEAD)
bd.append("- HOW TO READ THIS SECTION: his words are in quotes, pulled by line from the byte-exact message (never re-typed). The text after \"->\" restates his words only. How each answer changes the item it answers (the question's letter, the reading's lines) is ruled in this session's application ledger and appended below this section when ruled.")
for e in ents:
    lab=e['key'] if e['key'] not in('page','general') else ('his opening lines' if e['key']=='page' else 'general')
    bl.append("BF%d [%s%s] %s"%(bf,e['tag'],'' if e['key'] in('page','general') else ', '+e['key'],e['gist']))
    q='(the same words as the entry above)' if e['same'] else e['quote']
    bd.append("- %s (%s; BF%d) — Boris: %s -> %s"%(e['title'],lab,bf,q,e['gist']))
    bf+=1
bd.append("- THE QUESTIONS HE DID NOT NAME take their defaults by his first line (%d of 45): %s. Question 189 is NOT among the answered: he asks Harmony back (BF172)."%(len(unnamed),', '.join(str(n) for n in unnamed)))
bd.append("- THE READINGS HE DID NOT NAME (%d of 102) stand as shown: INFERRED consent. Evidence: his corrections run in the page's order through every topic, A to K (the last are R201, R202 and 215); he writes that he did not read the list \"Decided without asking you\" and what is below it. Where a reading he did not name contradicts words he wrote in this message, his words win (his rule on repeats)."%(102-len(rn)))
an.append("## ANSWERS")
an.append("")
an.append("Recorded @@STAMP@@ (session s-rta-1007). His message of 2026-10-07 22:19:09 local, byte-exact: .harmony/.reports/s-rta-1007/boris-msg-raw-1.txt; three pictures: .harmony/.reports/s-rta-1007/boris-images/. Filed: boris-feedback-backlog.md BF154-BF%d; binding-decisions.md, the section \"2026-10-07 (s-rta-1007)\". Each line below is his own words, pulled by line from the message."%(bf-1))
an.append("")
an.append("- His opening lines: \"%s\""%L[1])
an.append("- General points (new; none answers a numbered item): "+' / '.join('"%s"'%L[i] for i in range(3,10)))
done=set()
for e in ents:
    if e['key'] in('page','general') or e['same']: continue
    if (e['key'],tuple(e['idx'])) in done: continue
    done.add((e['key'],tuple(e['idx'])))
    an.append("- %s: %s"%(e['key'], e['quote'].replace('\n',' // ')))
an.append("- After the last item: \"%s\" / \"%s\""%(L[130],L[133]))
an.append("- Questions he did not name, at their defaults by his first line: %s."%', '.join(str(n) for n in unnamed))
an.append("- Question 189: no letter; he asks Harmony back. Readings he did not name: stand as shown (INFERRED consent), except where his words in this message say otherwise.")
open(os.path.join(S,'file_backlog.txt'),'w',encoding='utf-8').write('\n'.join(bl)+'\n')
open(os.path.join(S,'file_bd.txt'),'w',encoding='utf-8').write('\n'.join(bd)+'\n')
open(os.path.join(S,'file_answers.txt'),'w',encoding='utf-8').write('\n'.join(an)+'\n')
print('entries',len(ents),'BF154..BF%d'%(bf-1)); print('questions named',len(qnums),qnums); print('readings named',len(rn)); print('unnamed q',len(unnamed),unnamed)
print('sizes',len('\n'.join(bl)),len('\n'.join(bd)),len('\n'.join(an)))
