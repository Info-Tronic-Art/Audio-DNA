# usage: python3 mkslices.py <items.json> <raw msg> <entries.txt> <outdir>
# Cuts the page's items (s-rta-1005) into one slice per topic, with the line numbers of Boris's message of 2026-10-07 that name each item.
import json,sys,os,collections
d=json.load(open(sys.argv[1])); raw=open(sys.argv[2],encoding='utf-8').read(); L=raw.split('\n'); out=sys.argv[4]
# numbered copy of his message
with open(os.path.join(out,'boris-msg-numbered.txt'),'w',encoding='utf-8') as f:
    f.write("Boris's message of 2026-10-07 22:19:09 local, one line per line of the byte-exact copy boris-msg-raw-1.txt, each prefixed L<number> (0-based). Empty lines and the paste / picture markers are kept. Quote him ONLY from these lines, verbatim.\n\n")
    for i,l in enumerate(L): f.write('L%d: %s\n'%(i,l))
key2lines=collections.OrderedDict()
for row in open(sys.argv[3],encoding='utf-8').read().split('\n'):
    if not row.strip(): continue
    spec,key=row.split('|')[:2]; spec=spec.rstrip('^'); idx=[]
    for part in spec.split(','):
        if '-' in part: a,b=part.split('-'); idx+=list(range(int(a),int(b)+1))
        else: idx.append(int(part))
    for i in idx:
        if i not in key2lines.setdefault(key,[]): key2lines[key].append(i)
def hl(key):
    ls=key2lines.get(str(key))
    return 'his lines: '+', '.join('L%d'%i for i in ls) if ls else 'NOT NAMED by him'
titles={int(t.split()[0]):t.split('] ',1)[1] for t in d['question_titles']}
DEC=d['decided_not_asked']; DES=d['design_page']
for i,x in enumerate(DEC,1): x['id']='D%d'%i
for i,x in enumerate(DES,1): x['id']='P%d'%i
GEN={'H':[('G1','L3','a codec of our own, like DXV 3.0 (he asks; a researcher answers the question, you file where it lands)')],
     'F':[('G2','L4','copy and paste for any clip'),('G3','L5','Option-drag copies a clip into the cell it is dragged to')],
     'E':[('G4','L6','a very low-resolution show recording in 10 or 20 minute chunks (he asks its cost; a researcher answers that; you file the function)')],
     'D':[('G5','L7','an "ignore actions" toggle on the layer, as well as "ignore column"')]}
counts={}
for T,name in d['topics'].items():
    qs=[q for q in d['questions'] if q['topic']==T]; rs=[r for r in d['readings'] if r['topic']==T]
    de=[x for x in DEC if x['topic']==T]; ds=[x for x in DES if x['topic']==T]
    o=[]
    o.append('# SLICE %s -- %s'%(T,name))
    o.append('The items of this topic exactly as they stood on the page shown to Boris on 2026-10-05 (source: s-rta-1005/boris-all-items.json), and for each the lines of his message of 2026-10-07 that name it (line numbers of boris-msg-numbered.txt). "NOT NAMED by him" on a QUESTION = its default is taken by his first line (L1). "NOT NAMED by him" on a READING = it stands as shown unless words of his elsewhere in the message say otherwise. The lists "DECIDED WITHOUT ASKING" and "COMES NEXT AS PICTURES" he did NOT read (L130).')
    o.append('')
    o.append('## ITEM IDS OF THIS SLICE (every one needs an @@ITEM block in your paper)')
    ids=[str(q['n']) for q in qs]+[r['r'] for r in rs]+[x['id'] for x in de]+[x['id'] for x in ds]+[g[0] for g in GEN.get(T,[])]
    o.append(' '.join(ids)); counts[T]=ids
    o.append('')
    if GEN.get(T):
        o.append('## GENERAL POINTS OF HIS MESSAGE THAT LAND IN THIS TOPIC (new; they answer no numbered item)')
        for g in GEN[T]: o.append('### %s (%s): %s'%g)
        o.append('')
    o.append('## QUESTIONS (%d)'%len(qs))
    for q in qs:
        o.append('### Question %d -- %s   [%s]'%(q['n'],titles.get(q['n'],''),hl(q['n'])))
        o.append('About: %s'%q.get('about',''))
        o.append('Situation: %s'%q.get('situation',''))
        for op in q['options']:
            o.append('- %s%s%s: %s%s'%(op['k'],' (THE DEFAULT)' if op.get('default') else '',' [%s]'%op['tag'] if op.get('tag') else '',op['text'],('  -- why the default: '+op['why_default']) if op.get('why_default') else ''))
        if q.get('why'): o.append('Why it was asked: %s'%q['why'])
        if q.get('his_words'): o.append('His earlier words it rested on: '+' / '.join('"%s"'%w for w in q['his_words']))
        if q.get('source'): o.append('Source (Harmony-side): %s'%q['source'])
        o.append('')
    o.append('## READINGS (%d)'%len(rs))
    for r in rs:
        o.append('### %s (about: %s)   [%s]'%(r['r'],r.get('about',''),hl(r['r'])))
        o.append(r['text'])
        for k in r:
            if k not in('r','topic','about','text','part','state') and r[k]: o.append('(%s: %s)'%(k,json.dumps(r[k],ensure_ascii=False)))
        o.append('')
    o.append('## DECIDED WITHOUT ASKING (%d) -- he did NOT read these: each is still Harmony\'s assumption until his words settle it'%len(de))
    for x in de:
        o.append('### %s: %s'%(x['id'],x['text'])); o.append('(decided by: %s)'%x.get('by','')); o.append('')
    o.append('## COMES NEXT AS PICTURES (%d) -- he did NOT read these; his new rule on layout is L8'%len(ds))
    for x in ds:
        o.append('### %s: %s'%(x['id'],x['what'])); o.append('(variants that were to be drawn: %s)'%' | '.join(x.get('variants',[]))); o.append('')
    open(os.path.join(out,'slice-%s.md'%T),'w',encoding='utf-8').write('\n'.join(o)+'\n')
# the loose lists
o=['# SLICE X -- the page\'s loose lists (Harmony-side; Boris did not see most of them)','Each entry gets an id: C1.. (where two statements disagreed), N1.. (not established), U1.. (still unsure).','']
ids=[]
o.append('## WHERE TWO STATEMENTS DISAGREED (%d)'%len(d['conflicts']))
for i,c in enumerate(d['conflicts'],1):
    ids.append('C%d'%i); o.append('### C%d  [handled on the page as: %s]'%(i,c.get('handled_as',''))); o.append('a (%s): %s'%(c.get('a_where',''),c.get('a',''))); o.append('b (%s): %s'%(c.get('b_where',''),c.get('b',''))); o.append('')
o.append('## NOT ESTABLISHED (%d)'%len(d['not_established']))
for i,c in enumerate(d['not_established'],1): ids.append('N%d'%i); o.append('### N%d: %s'%(i,c)); o.append('')
o.append('## STILL UNSURE (%d)'%len(d['still_unsure']))
for i,c in enumerate(d['still_unsure'],1): ids.append('U%d'%i); o.append('### U%d: %s'%(i,c)); o.append('')
o.insert(3,'## ITEM IDS OF THIS SLICE (every one needs an @@ITEM block in your paper)\n'+' '.join(ids)+'\n')
open(os.path.join(out,'slice-X.md'),'w',encoding='utf-8').write('\n'.join(o)+'\n'); counts['X']=ids
json.dump(counts,open(os.path.join(out,'slice-ids.json'),'w'),indent=0)
tot=0
for T,ids in counts.items():
    p=os.path.join(out,'slice-%s.md'%T); print(T,len(ids),'ids',os.path.getsize(p),'bytes'); tot+=len(ids)
print('total ids',tot)
named=set(k for k in key2lines if k not in('page','general'))
inslices=set(i for ids in counts.values() for i in ids)
print('named keys not in any slice:',sorted(named-inslices))
