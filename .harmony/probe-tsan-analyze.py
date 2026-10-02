#!/usr/bin/env python3
"""probe-tsan-analyze.py -- keys the TSan reports of a .harmony/probe-tsan.sh sweep (lane tsan, s-rta-1002; plan
plan-tsan.md T8, ruling ruling-tsan.md G3.2; adapted from .harmony/.reports/s-rta-0930/tsan-harness/analyze.py).

usage: probe-tsan-analyze.py <sweep-dir> [--src ARM=PATH|ARM=git:REV ...] [--repo DIR] [--deps DIR]
                             [--default-arm NAME] [--json FILE]
  <sweep-dir>     holds runs/tsan-<N>/tsan.* and launches.tsv (probe-tsan.sh output, or the s-rta-0929b archive
                  format, whose rows carry no arm: those launches get --default-arm, default "main")
  --src ARM=...   the source tree of that arm, for the printed source lines and the src/ basenames: a checkout root
                  (its src/) or git:<rev> read from --repo (default: this tree). Default: every arm = this tree.
  --deps DIR      FetchContent sources whose basenames join src/'s in the KEY (default <main checkout>/build/_deps)

Key (G3.2): kind + the top-4 frames (module Audio-DNA, basename under src/ or the deps) of both access stacks.
Classes (G3.2):
  APP           a src/ frame in either access stack, in the location's heap-allocation stack, or in either access
                thread's creation stack;
  UNATTRIBUTED  not APP, and an access stack is empty or "[failed to restore the stack]";
  JUCE-SYSTEM   everything else.
FAMILY keys (APP reports), judged by the racing source line (and, only when the line names no field, the function)
of the top src/ frame of each access stack -- printed for every unique so each key is checkable; a deeper src/ frame
is used only when the top ones name nothing (marked "deep"):
  A  the location is std::__1::cerr, or an access stack runs in a libc++ ostream frame, with at least one side (its
     top src access frame, else its thread's creation frame) in a file of T7's converted list. Both sides in the R8
     residual files (audio/AudioEngine.cpp, audio/DeviceGuard.cpp; adoption H3) -> PRE-EXISTING-R8 (listed, not a
     lane failure); any other cerr race -> A-UNLISTED (review).
  B  the Layer trigger tuple ONLY (the fields LayerRuntimeCell holds: activeClipColumn / previousClipColumn /
     crossfadeProgress / pendingTriggerColumn + its snap override, and the trigger / undo / fade functions that write
     them) plus clip playing / hasBeenTriggered / beatsPlayed. A config scalar read or written inside one of those
     functions is NOT B: the racing field decides, not the function that holds the line (s-rta-1002b).
  C  clip playheadPosition.
  D  the 23 manualRef scalars, manualWriteCore's write, or eff().
  E  Composition::activeDeckIndex.
Out-of-lane classes (ruling-tsan.md amendment 1, follow-up lane "tsan-r5"; listed, never a lane-family failure):
  R5 config scalar: Layer visible / bypassed / solo / muted / autopilotEnabled / transitionSpeed / blendMode / keying /
     feedback ..., Clip speed / reverse / loopMode / inPoint / outPoint / beatSnapMode, effect paramValues / dryWet,
     source params, macros, Composition outputWidth / outputHeight / globalTransitionSpeed / quantizeMode.
  R7 httplib reader: an access stack, or the creation stack of its (non-main) thread, with an ApiServer / TestServer /
     httplib frame (the unfenced /api reads). The main thread never counts: it runs ApiServer's callAsync lambdas. It ranks BELOW E C D B: an httplib read of a lane-fixed field is still that family.
  UNCLASSIFIED  an APP report none of the above names: printed with its top src frames, never folded into a family.
Priority when a unique names several: E C D B R7 R5 (the rest print as "also ...").
`--self-test` runs the synthetic classification cases and exits.
"""
import argparse, collections, glob, json, os, re, subprocess, sys

T7_LIST = {"AnalysisThread.cpp", "ApiServer.cpp", "TestServer.cpp", "Renderer.cpp", "ImageDecode.h",
           "TextureManager.cpp", "LUTLoader.cpp", "VideoRecorder.cpp", "MediaOpener.cpp", "ImageSequence.cpp",
           "SyphonOutput.mm", "SharedFrameSet.cpp", "OutputPresenter.cpp", "VideoPlayer.cpp"}
R8_LIST = {"AudioEngine.cpp", "DeviceGuard.cpp"}
OSTREAM_FILES = {"pad_and_output.h", "put_character_sequence.h", "ostream", "ios", "streambuf", "locale",
                 "__locale", "sstream", "string"}
MANUAL_FIELDS = ("opacity", "positionX", "positionY", "layerScale", "layerRotation", "layerAnchorX", "layerAnchorY",
                 "clipOpacity", "scale", "rotation", "anchorX", "anchorY", "masterOpacity", "masterSpeed",
                 "masterSignal", "compPositionX", "compPositionY", "compScale", "compRotation", "compAnchorX",
                 "compAnchorY")
FAMILY_RULES = [   # (family, regex over the source line, regex over the function name -- used only if no line names a field)
    ("E", r"\bactiveDeckIndex\b", r"$^"),
    ("C", r"\bplayheadPosition\b", r"$^"),
    ("D", r"\bmanualRef\b|\beff\s*\(|\.manual\b|->manual\b|\b(" + "|".join(MANUAL_FIELDS) + r")\b",
     r"manualWriteCore|::eff\(|ManualSlot"),
    ("B", r"\b(activeClipColumn|previousClipColumn|crossfadeProgress|pendingTriggerColumn|pendingTriggerSnapOverride"
          r"|playing|hasBeenTriggered|beatsPlayed|getActiveClip|triggerClip\w*|clearActiveClip|releaseMomentary"
          r"|processPendingTrigger|cancelPendingTriggers|\w*LayerRuntime\w*|runtime|setRuntime|casRuntime"
          r"|updateRuntime|advanceCrossfade|triggerColumn)\b",
     r"TriggerClipCmd|ClearActiveClipCmd|Layer::trigger|Layer::clearActiveClip|Layer::releaseMomentary"
     r"|processPendingTrigger|LayerClock|applyLayerRuntime|captureLayerRuntime|cancelPendingTriggers|Autopilot::"
     r"|incomingImagePending|CrossfadeStartDetector|ClipTransportSync|Deck::triggerColumn"),
    ("R5", r"\b(visible|bypassed|solo|muted|autopilotEnabled|autopilotEndOfVideo|autopilotLoops|transitionSpeed"
           r"|blendMode|keyingMode|feedback|speed|reverse|loopMode|inPoint|outPoint|startOffset|beatSnapMode|beatSnap"
           r"|paramValues|dryWet|effDryWet|paramLive|sourceParams|presetBeatsPlayed|macros?|outputWidth|outputHeight"
           r"|globalTransitionSpeed|quantizeMode)\b|\blayer(\.|->)type\b",
     r"applyLayerFlag"),
]
PRIORITY = ["E", "C", "D", "B", "R7", "R5"]
FAMILY_ORDER = ["A", "B", "C", "D", "E", "R5", "R7"]   # per-arm summary order; the rest follow
HTTP_FILES = {"ApiServer.cpp", "TestServer.cpp", "httplib.h"}


def is_http_frame(fr):
    return fr["file"] in HTTP_FILES or re.match(r"(httplib|ApiServer|TestServer)::", fr["func"]) is not None


def frames_of(lines):
    out = []
    for l in lines:
        m = re.match(r"\s+#(\d+) (.*) \(([^()]*?)\+0x[0-9a-f]+\)\s*$", l)
        if not m:
            if re.match(r"\s+#\d+ ", l):
                out.append({"func": l.strip()[:120], "file": None, "line": None, "mod": None})
            continue
        body, mod = m.group(2), m.group(3).split(":")[0]
        func, file, line = body, None, None
        if " " in body:
            head, last = body.rsplit(" ", 1)
            mm = re.match(r"^([^:\s]+\.[A-Za-z0-9]+|ostream|ios|string|locale|sstream|streambuf|__locale|atomic"
                          r"|memory|thread|vector)(?::(\d+))?(?::\d+)?$", last)
            if mm:
                func, file, line = head, mm.group(1), mm.group(2)
        out.append({"func": func[:160], "file": file, "line": line, "mod": mod})
    return out


def parse(path):
    txt = open(path, errors="replace").read()
    reps = []
    for blk in re.split(r"^==================\n", txt, flags=re.M):
        if "WARNING: ThreadSanitizer" not in blk:
            continue
        L = blk.split("\n")
        km = re.search(r"WARNING: ThreadSanitizer: (.*?) \(pid", blk)
        kind = km.group(1) if km else "?"
        secs, cur = [], None
        for l in L:
            if re.match(r"  \S", l):                     # a section header (2-space indent)
                cur = {"hdr": l.strip(), "lines": []}
                secs.append(cur)
            elif cur is not None and l.startswith("    "):
                cur["lines"].append(l)
        acc, loc, threads = [], None, {}
        for s in secs:
            h = s["hdr"]
            if re.match(r"(Previous )?([Aa]tomic )?(Read|Write|read|write) of size", h):
                m = re.search(r"by (thread T\d+|main thread)", h)
                acc.append({"hdr": h, "thr": m.group(1).replace("thread ", "") if m else "?",
                            "frames": frames_of(s["lines"]),
                            "failed": any("failed to restore" in x for x in s["lines"]) or not s["lines"]})
            elif h.startswith("Location is"):
                loc = {"hdr": h, "frames": frames_of(s["lines"])}
            else:
                m = re.match(r"Thread (T\d+) \((.*?)\) (created by .*?)( at:)?$", h)
                if m:
                    threads[m.group(1)] = {"desc": m.group(2) + " " + m.group(3), "frames": frames_of(s["lines"])}
        summ = re.search(r"^SUMMARY: ThreadSanitizer: (.*)$", blk, flags=re.M)
        reps.append({"kind": kind, "acc": acc[:2], "loc": loc, "threads": threads,
                     "summary": summ.group(1) if summ else ""})
    return reps


class Tree:
    """One arm's source tree: basename -> src-relative paths, and line lookup."""

    def __init__(self, spec, repo):
        self.spec, self.repo, self.cache = spec, repo, {}
        if spec.startswith("git:"):
            self.rev = spec[4:]
            names = subprocess.run(["git", "-C", repo, "ls-tree", "-r", "--name-only", self.rev, "src"],
                                   capture_output=True, text=True).stdout.split()
            if not names:
                sys.exit("probe-tsan-analyze: no src/ at %s in %s" % (self.rev, repo))
            self.paths = names
        else:
            self.rev = None
            self.paths = []
            for dp, dn, fn in os.walk(os.path.join(spec, "src")):
                for f in fn:
                    self.paths.append(os.path.relpath(os.path.join(dp, f), spec))
        self.by_base = collections.defaultdict(list)
        for p in self.paths:
            self.by_base[os.path.basename(p)].append(p)

    def text(self, rel):
        if rel not in self.cache:
            if self.rev:
                r = subprocess.run(["git", "-C", self.repo, "show", "%s:%s" % (self.rev, rel)], capture_output=True,
                                   text=True, errors="replace")
                self.cache[rel] = r.stdout.split("\n")
            else:
                try:
                    self.cache[rel] = open(os.path.join(self.spec, rel), errors="replace").read().split("\n")
                except OSError:
                    self.cache[rel] = []
        return self.cache[rel]

    def line(self, base, n):
        if not n:
            return ""
        out = []
        for rel in self.by_base.get(base, []):
            t = self.text(rel)
            k = int(n)
            if 0 < k <= len(t):
                out.append(t[k - 1].strip())
        return " || ".join(out)


def classes_of(fr, line_of):
    """The family names ONE src frame names: by its source line; by its function only if the line names none."""
    text = line_of(fr["file"], fr["line"])
    got = {name for name, lre, fre in FAMILY_RULES if re.search(lre, text)}
    if not got:
        got = {name for name, lre, fre in FAMILY_RULES if re.search(fre, fr["func"])}
    return got


def family_of(sides, loc_hdr, line_of):
    """(family, note) of an APP report. sides: dicts with top, via, srcframes, ostream, http (see main())."""
    if "'std::__1::cerr'" in loc_hdr or any(s["ostream"] for s in sides):
        files = [s["top"]["file"] if s["top"] else None for s in sides]
        if any(f in T7_LIST for f in files):
            return "A", ""
        if files and all(f in R8_LIST for f in files):
            return "PRE-EXISTING-R8", ""
        return "A-UNLISTED", ""
    got, note = set(), ""
    for s in sides:
        if s["via"] == "access" and s["top"] is not None:
            got |= classes_of(s["top"], line_of)
    if any(s["http"] for s in sides):
        got.add("R7")
    if not got:
        for s in sides:
            for fr in s["srcframes"]:
                got |= classes_of(fr, line_of)
        note = "deep" if got else ""
    if not got:
        tops = ["%s | %s" % (s["hdr"][:40], " <- ".join("%s:%s %s" % (f["file"], f["line"], f["func"][:50])
                for f in s["srcframes"][:4]) or "(no src/ frame)") for s in sides]
        return "UNCLASSIFIED", "top src frames: " + " || ".join(tops)
    fam = next(x for x in PRIORITY if x in got)
    if len(got) > 1:
        note = (note + " " if note else "") + "also " + " ".join(sorted(got - {fam}))
    return fam, note


def self_test():
    """Synthetic classification cases (no TSan data needed). Exit non-zero on the first failure."""
    def fr(file, line, func="f()"):
        return {"file": file, "line": str(line), "func": func, "mod": "Audio-DNA"}

    def side(top, http=False, ostream=False):
        return {"hdr": "x", "thr": "T1", "top": top, "via": "access", "srcframes": [top] if top else [],
                "ostream": ostream, "http": http}
    lines = {("MainComponent.cpp", "1"): 'else if (flag == "autopilot") layer->autopilotEnabled = value;',
             ("Autopilot.cpp", "2"): "if (!layer.autopilotEnabled || !layer.autopilotEndOfVideo)",
             ("Layer.h", "3"): "triggerClipImmediate(column);",
             ("Renderer.cpp", "4"): "if (!layer || layer->activeClipColumn < 0) continue;",
             ("TriggerCommands.h", "5"): "void execute() override { apply(after_, targetPlayingAfter_); }",
             ("ApiServer.cpp", "6"): "j[\"name\"] = clip.name;",
             ("Renderer.cpp", "7"): "DeckClock::tick(other, realDt, cb);",
             ("Odd.cpp", "8"): "doSomething(x);",
             ("Layer.h", "9"): "const float speed = transitionSpeed;"}
    lo = lambda f, n: lines.get((f, str(n)), "")
    cases = [
        ("config scalar read in Autopilot:: is R5, not B (s-rta-1002 F76)",
         [side(fr("Autopilot.cpp", 2, "Autopilot::processFrame")), side(fr("MainComponent.cpp", 1, "MainComponent::applyLayerFlag"))],
         "R5"),
        ("tuple field read vs trigger write is B",
         [side(fr("Renderer.cpp", 4, "Renderer::renderOpenGL")), side(fr("Layer.h", 3, "Layer::triggerClip"))], "B"),
        ("function-only match (undo command) is still B",
         [side(fr("Renderer.cpp", 4, "Renderer::renderOpenGL")), side(fr("TriggerCommands.h", 5, "TriggerClipCmd::execute"))],
         "B"),
        ("config scalar read inside a trigger function is R5",
         [side(fr("Layer.h", 9, "Layer::triggerClip")), side(fr("MainComponent.cpp", 1, "MainComponent::applyLayerFlag"))],
         "R5"),
        ("httplib reader of an unnamed field is R7",
         [side(fr("ApiServer.cpp", 6, "ApiServer::handle"), http=True), side(fr("Odd.cpp", 8, "Odd::run"))], "R7"),
        ("httplib reader of a tuple field stays B (R7 ranks below the lane families)",
         [side(fr("ApiServer.cpp", 6, "ApiServer::handle"), http=True), side(fr("Layer.h", 3, "Layer::triggerClip"))],
         "B"),
        ("nothing named is UNCLASSIFIED",
         [side(fr("Odd.cpp", 8, "Odd::run")), side(fr("Renderer.cpp", 7, "Renderer::renderOpenGL"))], "UNCLASSIFIED"),
    ]
    bad = 0
    for name, frm, want in [
            ("httplib.h frame is http", {"file": "httplib.h", "func": "x"}, True),
            ("ApiServer::start frame is http", {"file": "ApiServer.cpp", "func": "ApiServer::start()"}, True),
            ("callAsync lambda wrapper naming ApiServer in a template arg is not http",
             {"file": "function.h", "func": "std::__1::__function::__func<ApiServer::handlePerfPlay(httplib::Request const&)::$_0>"},
             False)]:
        ok = is_http_frame(dict(frm, line="1", mod="Audio-DNA")) == want
        print("%-4s %s" % ("ok" if ok else "FAIL", name))
        bad += not ok
    for name, sides, want in cases:
        got, note = family_of(sides, "", lo)
        print("%-4s %s -> %s%s" % ("ok" if got == want else "FAIL", name, got, " (%s)" % note if note else ""))
        bad += got != want
    sys.exit(1 if bad else 0)


def main():
    if "--self-test" in sys.argv:
        self_test()
    ap = argparse.ArgumentParser()
    ap.add_argument("sweep")
    ap.add_argument("--src", action="append", default=[])
    here = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    ap.add_argument("--repo", default=here)
    common = subprocess.run(["git", "-C", here, "rev-parse", "--path-format=absolute", "--git-common-dir"],
                            capture_output=True, text=True).stdout.strip()
    ap.add_argument("--deps", default=os.path.join(os.path.dirname(common), "build", "_deps") if common else "")
    ap.add_argument("--default-arm", default="main")
    ap.add_argument("--json", default=None)
    a = ap.parse_args()

    # launches.tsv: "tsan N scen arm healthS=..." (probe-tsan.sh) or "tsan N scen healthS=..." (the archive format)
    launches = collections.OrderedDict()
    tsv = os.path.join(a.sweep, "launches.tsv")
    for ln in open(tsv) if os.path.exists(tsv) else []:
        f = ln.rstrip("\n").split("\t")
        if len(f) < 3:
            continue
        arm = f[3] if len(f) > 3 and "=" not in f[3] and f[3] and not f[3].startswith("SKIPPED") else a.default_arm
        launches["%s-%s" % (f[0], f[1])] = {"scen": f[2], "arm": arm, "row": f[3:]}
    arms = sorted({v["arm"] for v in launches.values()}) or [a.default_arm]
    srcspec = dict(s.split("=", 1) for s in a.src)
    trees = {arm: Tree(srcspec.get(arm, here), a.repo) for arm in arms}
    srcbase = set()
    for t in trees.values():
        srcbase |= set(t.by_base)
    depbase = set()
    if a.deps and os.path.isdir(a.deps):
        for dp, dn, fn in os.walk(a.deps):
            for f in fn:
                if f.endswith((".cpp", ".h", ".mm", ".m", ".c", ".hpp", ".inl")):
                    depbase.add(f)

    def is_src(fr):
        return fr["file"] in srcbase and fr["mod"] == "Audio-DNA"

    def is_known(fr):
        return (fr["file"] in srcbase or fr["file"] in depbase) and fr["mod"] == "Audio-DNA"

    def top4(frs):
        return ["%s:%s" % (f["file"], f["line"]) for f in frs if is_known(f)][:4]

    uniq = collections.OrderedDict()
    per_launch = collections.OrderedDict()
    for f in sorted(glob.glob(os.path.join(a.sweep, "runs", "tsan-*", "tsan.*")),
                    key=lambda p: int(re.search(r"tsan-(\d+)", p).group(1))):
        launch = f.split("/runs/")[1].split("/")[0]
        info = launches.get(launch, {"scen": "?", "arm": a.default_arm})
        if info["arm"] not in trees:
            trees[info["arm"]] = Tree(srcspec.get(info["arm"], here), a.repo)
            arms.append(info["arm"])
            srcbase |= set(trees[info["arm"]].by_base)
        pl = per_launch.setdefault(launch, {"scen": info["scen"], "arm": info["arm"], "warnings": 0,
                                            "classes": collections.Counter(), "families": collections.Counter()})
        for r in parse(f):
            key = (r["kind"], tuple(sorted(tuple(top4(x["frames"])) for x in r["acc"])))
            e = uniq.setdefault(key, {"r": r, "hits": collections.OrderedDict(), "arm": info["arm"]})
            e["hits"].setdefault(launch, (info["scen"], info["arm"]))
            pl["warnings"] += 1
            e.setdefault("_launch_reports", []).append(launch)
    for launch in launches:
        per_launch.setdefault(launch, {"scen": launches[launch]["scen"], "arm": launches[launch]["arm"],
                                       "warnings": 0, "classes": collections.Counter(),
                                       "families": collections.Counter()})

    # classify each unique
    rows = []
    for i, (key, e) in enumerate(uniq.items(), 1):
        r = e["r"]
        tree = trees[e["arm"]]
        acc = r["acc"]
        crt = [r["threads"].get(x["thr"], {"frames": [], "desc": "main thread / no creation record"}) for x in acc]
        app = any(is_src(fr) for x in acc for fr in x["frames"]) \
            or (r["loc"] is not None and any(is_src(fr) for fr in r["loc"]["frames"])) \
            or any(is_src(fr) for t in crt for fr in t["frames"])
        unattributed = (not app) and (len(acc) < 2 or any(x["failed"] for x in acc))
        cls = "APP" if app else ("UNATTRIBUTED" if unattributed else "JUCE-SYSTEM")
        sides = []
        for x, t in zip(acc, crt):
            srcfr = [fr for fr in x["frames"] if is_src(fr)]
            top = srcfr[0] if srcfr else None
            via = "access"
            if top is None:
                cf = [fr for fr in t["frames"] if is_src(fr)]
                top, via = (cf[0], "thread-creation") if cf else (None, "-")
            sides.append({"hdr": x["hdr"], "thr": x["thr"], "top": top, "via": via, "srcframes": srcfr,
                          "ostream": any(fr["file"] in OSTREAM_FILES for fr in x["frames"]),
                          "http": x["thr"] != "main thread" and any(is_http_frame(fr) for fr in x["frames"] + t.get("frames", [])),
                          "thread": t.get("desc", "")})
        fam, famnote = "", ""
        if cls == "APP":
            fam, famnote = family_of(sides, r["loc"]["hdr"] if r["loc"] else "", tree.line)
        e["cls"], e["fam"], e["famnote"], e["sides"] = cls, fam, famnote, sides
        for launch in e["_launch_reports"]:
            per_launch[launch]["classes"][cls] += 1
            if fam:
                per_launch[launch]["families"][fam] += 1
        rows.append((i, key, e))

    # ---- output ----
    print("probe-tsan-analyze: %s -- %d launches with reports, %d warnings, %d unique (src trees: %s)"
          % (a.sweep, sum(1 for v in per_launch.values() if v["warnings"]), sum(v["warnings"] for v in
             per_launch.values()), len(rows), ", ".join("%s=%s" % (k, v.spec) for k, v in trees.items())))
    print("\n== launches (warnings by class / family)")
    for launch, v in per_launch.items():
        row = launches.get(launch, {}).get("row", [])
        valid = next((x for x in row if x.startswith("valid=")), "")
        print("  %-9s %s@%-6s warnings %3d  %s  %s  %s" % (launch, v["scen"], v["arm"], v["warnings"],
              dict(v["classes"]) or "{}", dict(v["families"]) or "{}", valid))
    print("\n== unique reports")
    for i, key, e in rows:
        hits = ", ".join("%s(%s@%s)" % (l, s, arm) for l, (s, arm) in e["hits"].items())
        print("F%-3d %-13s family %-16s %s| %s" % (i, e["cls"], e["fam"] or "", ("(" + e["famnote"] + ") ")
              if e["famnote"] else "", hits))
        print("      key: %s | %s" % (key[0], " <> ".join("/".join(x) for x in key[1])[:220]))
        tree = trees[e["arm"]]
        for s in e["sides"]:
            t = s["top"]
            where = ("%s:%s %s" % (t["file"], t["line"], t["func"][:70])) if t else "(no src/ frame)"
            print("      %-58s [%s] top src (%s): %s" % (s["hdr"][:58], s["thr"], s["via"], where))
            if t:
                print("          | %s" % tree.line(t["file"], t["line"])[:200])
        if e["r"]["loc"]:
            print("      %s" % e["r"]["loc"]["hdr"][:150])
    for arm in arms:
        print("\n== arm %s: per family (uniques / launches with >= 1 report, by scenario)" % arm)
        fam_u = collections.defaultdict(set)
        fam_l = collections.defaultdict(set)
        for i, key, e in rows:
            for l, (s, a2) in e["hits"].items():
                if a2 == arm:
                    fam_u[e["fam"] or e["cls"]].add(i)
                    fam_l[e["fam"] or e["cls"]].add((l, s))
        scen_n = collections.Counter(v["scen"] for l, v in launches.items() if v["arm"] == arm)
        for f in sorted(fam_u, key=lambda x: (FAMILY_ORDER.index(x) if x in FAMILY_ORDER else 99, x)):
            by = collections.Counter(s for l, s in fam_l[f])
            print("  %-16s uniques %-3d launches %-3d  %s" % (f, len(fam_u[f]), len(fam_l[f]),
                  " ".join("%s %d/%d" % (s, by[s], scen_n.get(s, 0)) for s in sorted(by))))
        cls_by = collections.Counter()
        for l, v in per_launch.items():
            if v["arm"] == arm:
                for c, n in v["classes"].items():
                    cls_by[(v["scen"], c)] += n
        print("  reports by scenario x class: %s" % (", ".join("%s/%s %d" % (s, c, n) for (s, c), n in
              sorted(cls_by.items())) or "none"))
    if a.json:
        json.dump([{"id": "F%d" % i, "kind": key[0], "key": [list(x) for x in key[1]], "class": e["cls"],
                    "family": e["fam"], "note": e["famnote"],
                    "hits": [[l, s, arm] for l, (s, arm) in e["hits"].items()],
                    "sides": [{"hdr": s["hdr"], "thr": s["thr"], "via": s["via"],
                               "top": s["top"], "line": trees[e["arm"]].line(s["top"]["file"], s["top"]["line"])
                               if s["top"] else ""} for s in e["sides"]],
                    "loc": e["r"]["loc"]["hdr"] if e["r"]["loc"] else "", "summary": e["r"]["summary"]}
                   for i, key, e in rows], open(a.json, "w"), indent=1)


if __name__ == "__main__":
    main()
