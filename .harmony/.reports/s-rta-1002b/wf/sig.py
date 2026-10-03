import json,glob,sys
n=0
for j in glob.glob(sys.argv[1]+'/*/journal.jsonl'):
    st={}
    for l in open(j):
        try:o=json.loads(l)
        except:continue
        if o.get('type')=='started': st[o['key']]=o.get('label','')
        if o.get('type')=='result' and not st.get(o['key'],'').startswith(('recon:','attack:')): n+=1
print(n)
