import re, subprocess, sys
W="/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w10"; BASE="2ee1013"
def show(rev, path):
    if rev is None: return open(W+"/"+path).read()
    return subprocess.run(["git","-C",W,"show",f"{rev}:{path}"],capture_output=True,text=True,check=True).stdout
def blocks(src):
    out={}
    for m in re.finditer(r'(?:inline|static)(?:\s+constexpr)?\s+const\s+char\s*\*\s*(\w+)\s*=\s*R"(\w*)\(', src):
        sym,d=m.group(1),m.group(2); end=src.index(")"+d+'"', m.end()); out[sym]=src[m.end():end]
    return out
def sources(reg):
    # id -> (shaderKey, [(uniform, value)])  expanding the wireframe + torus helpers
    wf=re.search(r'auto registerWireframe = \[this\]\(const std::string& id, const std::string& name, float shapeVal\) \{(.*?)\n    \};', reg, re.S)
    wfp=re.findall(r'addParam\("([^"]+)",\s*"(\w+)",\s*([0-9.]+f|shapeVal)\)', wf.group(1))
    tor=re.search(r'auto addTorusControls = \[\]\(ProceduralSource\* s\) \{(.*?)\n    \};', reg, re.S)
    torp=re.findall(r'addParam\("([^"]+)",\s*"(\w+)",\s*([0-9.]+)f\)', tor.group(1)) if tor else []
    out={}
    for m in re.finditer(r'registerWireframe\("(\w+)",\s*"[^"]+",\s*([0-9.]+)f\)', reg):
        out[m.group(1)]=("source_wireframe_3d",[(u, float(m.group(2)) if v=="shapeVal" else float(v[:-1])) for n,u,v in wfp])
    body=reg
    if wf: body=body.replace(wf.group(0),"")
    if tor: body=body.replace(tor.group(0),"")
    starts=[m.start() for m in re.finditer(r'registerSource\("', body)]+[len(body)]
    for a,b in zip(starts,starts[1:]):
        ch=body[a:b]; sid=re.match(r'registerSource\("(\w+)"',ch).group(1)
        k=re.search(r'"(source_\w+)"',ch)
        if not k: continue
        ps=[(u,float(v)) for n,u,v in re.findall(r'addParam\("([^"]+)",\s*"(\w+)",\s*([0-9.]+)f\)',ch)]
        if "addTorusControls(s.get())" in ch: ps+=[(u,float(v)) for n,u,v in torp]
        out[sid]=(k.group(1),ps)
    return out
def effects(lib):
    out={}
    for m in re.finditer(r'registerEffect\(\{"([^"]+)",\s*"(\w+)",\s*"(\w+)",\s*\{(.*?)\}\s*(?:,\s*(?:true|false))?\s*\}\);', lib, re.S):
        out[m.group(1)]=(m.group(3),[(u,float(v)) for n,u,v in re.findall(r'\{"([^"]+)",\s*"(\w+)",\s*(-?[0-9.]+)f\}',m.group(4))])
    return out
bb=blocks(show(BASE,"src/render/EmbeddedShaders.h")); lb=blocks(show(None,"src/render/EmbeddedShaders.h"))
key2sym=dict(re.findall(r'compile\("(\w+)",\s*EmbeddedShaders::(\w+)\)', show(None,"src/render/Renderer.cpp")))
key2symB=dict(re.findall(r'compile\("(\w+)",\s*EmbeddedShaders::(\w+)\)', show(BASE,"src/render/Renderer.cpp")))
assert key2sym==key2symB
sB=sources(show(BASE,"src/sources/SourceRegistry.cpp")); sL=sources(show(None,"src/sources/SourceRegistry.cpp"))
eB=effects(show(BASE,"src/effects/EffectLibrary.cpp")); eL=effects(show(None,"src/effects/EffectLibrary.cpp"))
assert set(sB)==set(sL) and set(eB)==set(eL), (set(sB)^set(sL), set(eB)^set(eL))
print("sources", len(sB), "effects", len(eB), "blocks base", len(bb), "lane", len(lb), file=sys.stderr)
changed_text=[s for s in lb if bb.get(s)!=lb[s]]; print("shader blocks whose TEXT changed:", changed_text, file=sys.stderr)
head=open(W+"/tests/test_source_defaults_gl.cpp").read(); head=head[:head.index("// A1.1 --")]
o=[head.replace('#include <vector>','#include <vector>\n#include <iostream>',1),"namespace Base {\n"]
used=set(key2sym[v[0]] for v in list(sB.values())+list(eB.values()) if v[0] in key2sym)
for s in sorted(used):
    assert ")B7Q\"" not in bb[s]; o.append(f'const char* {s} = R"B7Q({bb[s]})B7Q";\n')
o.append("}\nstruct Item { const char* kind; const char* id; const char* base; const char* lane; std::vector<Param> pb, pl; };\n")
o.append("std::vector<Item> items() { std::vector<Item> v;\n")
def plist(ps): return "{"+",".join('{"","%s",%sf}'%(u,repr(float(x))) for u,x in ps)+"}"
for kind,B_,L_ in (("src",sB,sL),("fx",eB,eL)):
    for i in sorted(B_):
        k=B_[i][0]
        if k not in key2sym: print("no compile entry for", kind, i, k, file=sys.stderr); continue
        s=key2sym[k]
        o.append(f'v.push_back({{"{kind}","{i}",Base::{s},EmbeddedShaders::{s},{plist(B_[i][1])},{plist(L_[i][1])}}});\n')
o.append("return v; }\n")
o.append('''TEST_CASE("FULL: every source and effect block, pre-change vs lane, at the registered defaults", "[full]")
{
    Rig rig; REQUIRE_GL(rig);
    const GLuint tex = rampTexture(512, 512);
    for (const auto& it : items())
        for (auto [w, h] : kSizes)
            for (float t : { 0.0f, 1.13f })
            {
                const bool fx = std::string(it.kind) == "fx";
                const Pixels a = rig.render(it.base, w, h, t, it.pb, fx ? tex : 0);
                const Pixels b = rig.render(it.lane, w, h, t, it.pl, fx ? tex : 0);
                const Pixels a2 = rig.render(it.base, w, h, t, it.pb, fx ? tex : 0);
                std::cout << "FULL " << it.kind << " " << it.id << " " << w << "x" << h << " t=" << t
                          << " psnr_lane=" << psnr(a, b) << " psnr_repeat=" << psnr(a, a2) << std::endl;
            }
    glDeleteTextures(1, &tex);
}
''')
src="".join(o)
# rampTexture lives after the A5 case in the test file: pull its definition in
t=open(W+"/tests/test_source_defaults_gl.cpp").read()
rt=t[t.index("namespace\n{\nGLuint rampTexture"):]; rt=rt[:rt.index("} // namespace")+len("} // namespace")]
src=src.replace("namespace Base {\n", rt+"\nnamespace Base {\n",1)
open(sys.argv[1],"w").write(src)
