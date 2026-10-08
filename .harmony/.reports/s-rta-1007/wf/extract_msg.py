import json,sys,base64,hashlib,os
p='/Users/boriskarpman/.claude/projects/-Users-boriskarpman-projects-RealTimeAudio/dc0022aa-41a3-4de2-a8fb-bc5cbd289eab.jsonl'
D=sys.argv[1]
line=None
for i,l in enumerate(open(p),1):
    if i==10: line=l; break
d=json.loads(line)
assert d['type']=='user', d['type']
c=d['message']['content']
texts=[b for b in c if b['type']=='text']; imgs=[b for b in c if b['type']=='image']
assert len(texts)==1 and len(imgs)==3,(len(texts),len(imgs))
full=texts[0]['text']
assert full.lstrip().startswith('<pasted_content id="1ea8">\nYou are Harmony, SECONDARY lane'), full[:80]
k=full.index('All defaults good except for these.')
# his own message = from the opening tag of the block that holds his first words
start=full.rfind('<pasted_content', 0, k)
own=full[start:]
assert own.startswith('<pasted_content id="1ea8">\nAll defaults good except for these.'), own[:90]
open(os.path.join(D,'boris-msg-raw-1-full.txt'),'w',encoding='utf-8').write(full)
open(os.path.join(D,'boris-msg-raw-1.txt'),'w',encoding='utf-8').write(own)
print('timestamp',d.get('timestamp'))
print('full chars',len(full),'sha',hashlib.sha256(full.encode()).hexdigest()[:16])
print('own chars',len(own),'sha',hashlib.sha256(own.encode()).hexdigest()[:16],'start offset',start)
print('pasted blocks in own:',own.count('<pasted_content'),' image marks:',[own[j:j+10] for j in range(len(own)) if own.startswith('[Image #',j)])
names=['img-04','img-06','img-08']
for n,b in zip(names,imgs):
    raw=base64.b64decode(b['source']['data'])
    h=hashlib.sha256(raw).hexdigest()[:12]
    ext=b['source']['media_type'].split('/')[1]
    fn=os.path.join(D,'boris-images','%s-%s.%s'%(n,h,ext))
    open(fn,'wb').write(raw); print(fn,len(raw))
