import re, glob, os, json, collections, subprocess
import os; SW = os.environ.get('SWEEP_DIR', "/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/sweep")
ROOT = os.environ.get("SWEEP_ROOT", "/Users/boriskarpman/projects/RealTimeAudio")
known = set()
for base in (ROOT + "/src", ROOT + "/build/_deps"):
    for dp, dn, fn in os.walk(base):
        for f in fn:
            if f.endswith((".cpp", ".h", ".mm", ".m", ".c", ".hpp", ".inl")): known.add(f)
def frames_of(lines):
    out = []
    for l in lines:
        m = re.match(r"\s+#(\d+) (.*)", l)
        if not m: continue
        body = m.group(2)
        mm = re.search(r" (\S+?):(\d+) \((\S+?):arm64", body)
        fn = body.split(" ")[0]
        func = re.sub(r"\s\S+:\d+ \(.*$", "", body)[:110]
        if mm: out.append({"func": func, "file": mm.group(1), "line": mm.group(2), "mod": mm.group(3)})
        else: out.append({"func": body[:110], "file": None, "line": None, "mod": None})
    return out
def isapp(fr): return fr["file"] in known and fr["mod"] == "Audio-DNA"
def top4(fr): return [f'{f["file"]}:{f["line"]}' for f in fr if isapp(f)][:4]
def parse(path):
    txt = open(path, errors="replace").read()
    reps = []
    for blk in re.split(r"^==================\n", txt, flags=re.M):
        if "WARNING: ThreadSanitizer" not in blk: continue
        L = blk.split("\n")
        kind = re.search(r"WARNING: ThreadSanitizer: (.*?) \(pid", blk).group(1)
        secs = []; cur = None
        for l in L:
            if re.match(r"  (Read|Write|Atomic read|Atomic write|Previous read|Previous write|Previous atomic read|Previous atomic write)", l):
                mm = re.match(r"  (.*?) of size (\d+) at (0x[0-9a-f]+) by (thread T\d+|main thread)(.*):", l)
                cur = {"hdr": l.strip(), "op": mm.group(1), "thr": mm.group(4).replace("thread ", ""), "lines": [], "mtx": mm.group(5)}; secs.append(cur)
            elif re.match(r"  (Location|Mutex|Thread T\d+|As if)", l) or l.startswith("SUMMARY"): cur = None if not re.match(r"  (Thread T|Mutex|Location)", l) else {"hdr": l.strip(), "lines": []}; 
            elif cur is not None: cur["lines"].append(l)
            if cur is not None and cur not in secs and l.strip().startswith(("Location", "Thread T", "Mutex")): secs.append(cur)
        acc = [s for s in secs if s["hdr"].startswith(("Read", "Write", "Atomic", "Previous"))]
        loc = next((s["hdr"] for s in secs if s["hdr"].startswith("Location")), "")
        # location detail lines
        locl = []
        for i, l in enumerate(L):
            if l.startswith("  Location"):
                j = i + 1
                while j < len(L) and L[j].strip() and not L[j].startswith("  Thread") and not L[j].startswith("  Mutex"): locl.append(L[j]); j += 1
        threads = {}
        for i, l in enumerate(L):
            m = re.match(r"  Thread (T\d+) \((.*?)\) (created by .*?)( at:)?$", l)
            if m:
                j = i + 1; fl = []
                while j < len(L) and L[j].startswith("    #"): fl.append(L[j]); j += 1
                threads[m.group(1)] = {"desc": m.group(2) + " " + m.group(3), "frames": frames_of(fl)}
        summ = re.search(r"^SUMMARY: ThreadSanitizer: (.*)$", blk, flags=re.M)
        r = {"kind": kind, "acc": [], "loc": loc, "locl": locl, "threads": threads, "summary": summ.group(1) if summ else ""}
        for s in acc:
            r["acc"].append({"hdr": s["hdr"], "thr": s["thr"] if "thr" in s else "?", "frames": frames_of(s["lines"])})
        # thread of each access
        for a in r["acc"]:
            m = re.search(r"by (thread T\d+|main thread)", a["hdr"]); a["thr"] = (m.group(1).replace("thread ", "") if m else "?")
        reps.append(r)
    return reps
uniq = collections.OrderedDict()
for f in sorted(glob.glob(SW + "/runs/tsan-*/tsan.*")):
    launch = f.split("/runs/")[1].split("/")[0]
    scen = {ln.split("\t")[0] + "-" + ln.split("\t")[1]: ln.split("\t")[2] for ln in open(SW + "/launches.tsv") if "\t" in ln}.get(launch, "?")
    for r in parse(f):
        key = (r["kind"], tuple(sorted(tuple(top4(a["frames"])) for a in r["acc"][:2])))
        e = uniq.setdefault(key, {"r": r, "hits": collections.OrderedDict()})
        e["hits"].setdefault(launch, scen)
json.dump([{"key": [k[0], [list(x) for x in k[1]]], "hits": list(v["hits"].items()), "acc": v["r"]["acc"], "loc": v["r"]["loc"], "locl": v["r"]["locl"], "threads": v["r"]["threads"], "summary": v["r"]["summary"]} for k, v in uniq.items()], open(SW + "/unique.json", "w"), indent=1)
print(len(uniq), "unique")
for i, (k, v) in enumerate(uniq.items(), 1):
    print("F%d" % i, k[0], "|", " <> ".join("/".join(x) for x in k[1])[:200], "|", ",".join("%s(%s)" % (a, b) for a, b in v["hits"].items()))
