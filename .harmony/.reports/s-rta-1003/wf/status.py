import json,sys,os,glob
for j in sorted(glob.glob(sys.argv[1]+'/*/journal.jsonl')):
    st={};res=set()
    for l in open(j):
        try:o=json.loads(l)
        except:continue
        if o.get('type')=='started': st[o.get('key')]=o.get('label')
        if o.get('type')=='result': res.add(o.get('key'))
    run=[str(v) for k,v in st.items() if k not in res]; done=[str(v) for k,v in st.items() if k in res]
    print(os.path.basename(os.path.dirname(j)), 'done', len(done), '['+', '.join(done)+']', '| running:', ', '.join(run) if run else '-')
