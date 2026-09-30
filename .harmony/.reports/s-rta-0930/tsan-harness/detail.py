import json, re, collections
import os; SW = os.environ.get('SWEEP_DIR', "/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/sweep")
U = json.load(open(SW + "/unique.json"))
known_exclude = ("libc++", "libsystem", "libclang_rt")
def appf(fr):
    return [f for f in fr if f["file"] and f["mod"] == "Audio-DNA" and not f["file"].startswith("__") and f["file"] not in ("thread.h","pad_and_output.h","ostream","memory","atomic")]
out = []
for i, u in enumerate(U, 1):
    out.append("### F%d  hits: %s" % (i, ", ".join("%s(%s)" % tuple(h) for h in u["hits"])))
    for a in u["acc"][:2]:
        fr = a["frames"]
        out.append("  %s [%s]" % (a["hdr"][:90], a["thr"]))
        for f in fr[:7]:
            out.append("      %s %s:%s" % (f["func"][:95], f["file"], f["line"]))
    if u["loc"]: out.append("  " + u["loc"][:200])
    for l in u["locl"][:6]: out.append("   " + l.strip()[:170])
    for t in set(re.findall(r"T\d+", " ".join(a["hdr"] for a in u["acc"][:2]))):
        th = u["threads"].get(t)
        if th:
            fr = [f for f in th["frames"] if f["file"] and f["mod"] == "Audio-DNA"][:3]
            out.append("  thread %s %s created at %s" % (t, th["desc"][:40], " <- ".join("%s:%s" % (f["file"], f["line"]) for f in fr)))
        else: out.append("  thread %s: (main thread / no creation record)" % t)
open(SW + "/detail.txt", "w").write("\n".join(out))
print(len(out))
