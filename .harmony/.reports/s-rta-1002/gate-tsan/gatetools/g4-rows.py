#!/usr/bin/env python3
"""g4-rows.py -- helper of g4-parity.sh / g6-perf.sh (lane tsan-gatetools). Stdlib only.

  g4-rows.py judge  <OUT_DIR> <probe:keymode> [<probe:keymode> ...]   rows.tsv + the pre-registered G4 verdicts
  g4-rows.py extract <keymode> <logfile>                              print the parsed rows of one log (debug)
  g4-rows.py patch  <SRC> <DST> --replace OLD NEW [--replace OLD NEW ...]   exact-substring patch copy of a probe

Row extraction (G4: a probe's rows are its printed "PASS  <text>" / "FAIL  <text>" lines; the python probes also print
"--- <row>" section headers; the .json registries only hold fixtures, so the SECTION / TAG / LABEL of a printed line is
the row name, and a PASS and a FAIL of one row never share their wording, so the key is built from the label, never the
text):
  keymode agg     (probe-crossfade, -render-state, -deck-clock, -async-load, -media-open, -video):
                  row = the "--- name" section a line is in; outside a section = the text before the first ':' (the
                  case tag, e.g. a_both_effected). All lines of a row are ONE verdict (FAIL if any line FAILed).
  keymode seq     (step3, routines, routine-display, mastersignal, deck-tabs): row = LABEL#k, LABEL = a leading id token
                  (R1, d9, 5h, B1 [+ @signal=..]) else the text before the first ':' (<= 4 words) else the first word;
                  k = the nth line with that label in the log.
  keymode ordinal (deck-path, lane3 -- a linear one-line-per-check script whose FAIL wording shares nothing with its
                  PASS wording): row = oNN = the nth PASS/FAIL line of the log.
  Teardown / rig lines (app terminated, 0 windows, foreign REST traffic, crash report, UNC windows ...) outside a section
  = row _hygiene; launch failures (health never came up, port 7070 answered by ..., fixtures) = row _launch.
  A log with a REFUSE line, a G4-TIMEOUT marker or no parsed row is an INVALID run: every row of it is '-' (missing).
"""
import os
import re
import sys

HYG = re.compile(r"app terminated|app quit|windows in the FULL|Output window|APP STILL RUNNING|app still running|"
                 r"still running|crash report|UserNotificationCenter|foreign (REST traffic|render_frame)|"
                 r"Output-window check", re.I)
LAUNCH = re.compile(r"^(app never answered|port 7070 is answered|fixtures|health never)", re.I)
ID_RE = re.compile(r"^(?:[A-Za-z]{1,3}\d+[a-z]?|\d+[a-z]?)$")
LINE_RE = re.compile(r"^(PASS|FAIL)  (.*)$")
SECT_RE = re.compile(r"^--- (\S+)")

RULE = ("G4 pre-registered verdict (verbatim): FAIL iff the row PASSED on PRE in >= 1 of 2 AND FAILED on POST in 2 of 2; "
        "a row red on both arms = PRE-EXISTING (listed); a row missing on an arm = MISSING (listed, never PASS).")


def label_of(text):
    t = text.strip()
    toks = t.split()
    if not toks:
        return "?"
    first = toks[0].rstrip(":,")
    if ID_RE.match(first):
        lab = first
        if len(toks) > 1 and toks[1].startswith("@"):
            lab += " " + toks[1].rstrip(":")
        return lab
    m = re.match(r"^([^:(]{1,40}?):\s", t)
    if m and len(m.group(1).split()) <= 4:
        return m.group(1).strip()
    return first


def extract(path, mode):
    """-> (rows: dict key -> {"v": "PASS"|"FAIL", "p": n, "f": n, "hint": str}, order: [keys], invalid: str|None)"""
    try:
        txt = open(path, errors="replace").read().splitlines()
    except OSError as e:
        return {}, [], "no log (%s)" % e
    rows, order, seen = {}, [], {}
    invalid = None
    section, ordinal = None, 0

    def add(key, verdict, hint):
        if key not in rows:
            rows[key] = {"p": 0, "f": 0, "hint": hint[:90]}
            order.append(key)
        rows[key]["p" if verdict == "PASS" else "f"] += 1
        if verdict == "FAIL" and rows[key]["f"] == 1:
            rows[key]["hint"] = hint[:90]
    for ln in txt:
        if ln.startswith("REFUSE"):
            invalid = ln[:100]
        if ln.startswith("G4-TIMEOUT"):
            invalid = ln
        if ln.startswith("=== phase"):
            section = None
        ms = SECT_RE.match(ln)
        if ms:
            section = ms.group(1)
            continue
        if ln.startswith("PY ") and " PASS / " in ln:
            section = None
            continue
        if ln.startswith("FAIL: "):
            add("_launch", "FAIL", ln[6:])
            continue
        m = LINE_RE.match(ln)
        if not m:
            continue
        verdict, text = m.group(1), m.group(2)
        if section is None and LAUNCH.match(text):
            add("_launch", verdict, text)
            continue
        if section is None and HYG.search(text):
            add("_hygiene", verdict, text)
            continue
        if mode == "ordinal":
            ordinal += 1
            add("o%02d" % ordinal, verdict, text)
        elif mode == "seq":
            lab = label_of(text)
            seen[lab] = seen.get(lab, 0) + 1
            add("%s#%d" % (lab, seen[lab]), verdict, text)
        else:  # agg
            add(section if section else label_of(text), verdict, text)
    if not rows and not invalid:
        invalid = "no PASS/FAIL row parsed"
    return rows, order, invalid


def cell(rows, key):
    r = rows.get(key)
    if r is None:
        return "-"
    return "FAIL" if r["f"] else "PASS"


def judge(out_dir, specs):
    print(RULE)
    tsv = [("probe", "row", "pre1", "pre2", "post1", "post2", "verdict")]
    fails, preex, missing, flaky, partial, invalid_runs = [], [], [], [], [], []
    for spec in specs:
        probe, mode = spec.split(":")
        runs = {}
        for arm in ("pre", "post"):
            for n in (1, 2):
                lp = os.path.join(out_dir, probe, "%s-%d.log" % (arm, n))
                runs[(arm, n)] = extract(lp, mode)
                if runs[(arm, n)][2]:
                    invalid_runs.append("%s/%s-%d (%s)" % (probe, arm, n, runs[(arm, n)][2]))
        keys = []
        for k in (("pre", 1), ("pre", 2), ("post", 1), ("post", 2)):
            rows, order, inv = runs[k]
            if inv and not rows:
                continue
            for key in order:
                if key not in keys:
                    keys.append(key)
        for key in keys:
            cs = {}
            for k, (rows, order, inv) in runs.items():
                cs[k] = "-" if (inv and inv.startswith(("REFUSE", "G4-TIMEOUT"))) else cell(rows, key)
            pre = [cs[("pre", 1)], cs[("pre", 2)]]
            post = [cs[("post", 1)], cs[("post", 2)]]
            if all(c == "-" for c in pre) or all(c == "-" for c in post):
                verdict = "MISSING(%s)" % ("pre" if all(c == "-" for c in pre) else "post")
                missing.append("%s/%s [%s]" % (probe, key, verdict))
            elif "PASS" in pre and post == ["FAIL", "FAIL"]:
                verdict = "FAIL"
                fails.append("%s/%s" % (probe, key))
            elif "PASS" not in pre and "FAIL" in pre and "PASS" not in post and "FAIL" in post:
                verdict = "PRE-EXISTING"
                preex.append("%s/%s" % (probe, key))
            else:
                verdict = "ok"
                if len(set(c for c in pre + post if c != "-")) > 1:
                    verdict = "ok(flaky/mixed)"
                    flaky.append("%s/%s" % (probe, key))
            if "-" in pre + post and not verdict.startswith("MISSING"):
                partial.append("%s/%s" % (probe, key))
            tsv.append((probe, key, pre[0], pre[1], post[0], post[1], verdict))
    with open(os.path.join(out_dir, "rows.tsv"), "w") as f:
        for r in tsv:
            f.write("\t".join(r) + "\n")
    print("rows: %d   (%s/rows.tsv; cols: probe row pre1 pre2 post1 post2 verdict)" % (len(tsv) - 1, out_dir))
    for r in tsv[1:]:
        if r[6] != "ok":
            print("  %-34s %-34s pre=%s,%s post=%s,%s  %s" % (r[0], r[1][:34], r[2], r[3], r[4], r[5], r[6]))
    print("PRE-EXISTING (red on both arms): %s" % (", ".join(preex) or "none"))
    print("MISSING (never PASS): %s" % (", ".join(missing) or "none"))
    print("flaky/mixed (INFO): %s" % (", ".join(flaky) or "none"))
    print("rows with a missing run on one side (INFO): %s" % (", ".join(partial) or "none"))
    print("INVALID runs (REFUSE / timeout / no row): %s" % (", ".join(invalid_runs) or "none"))
    return fails


def patch(src, dst, pairs):
    txt = open(src).read()
    for old, new in pairs:
        c = txt.count(old)
        if c != 1:
            sys.exit("patch: expected exactly 1 occurrence of %r in %s, found %d" % (old, src, c))
        txt = txt.replace(old, new)
    lines = txt.split("\n")
    banner = "# PATCHED COPY (gatetools) of %s -- only the substring replacements below differ from it" % src
    lines.insert(1, banner)
    for old, new in pairs:
        lines.insert(2, "#   replaced: %s  =>  %s" % (old.replace("\n", " ")[:90], new.replace("\n", " ")[:90]))
    open(dst, "w").write("\n".join(lines))
    os.chmod(dst, 0o755)


def main():
    a = sys.argv[1:]
    if not a:
        sys.exit(__doc__)
    if a[0] == "judge":
        fails = judge(a[1], a[2:])
        final = "G4: PASS" if not fails else "G4: FAIL " + " ".join(fails)
        with open(os.path.join(a[1], "g4-final.txt"), "w") as f:   # the shell prints it LAST, after the window checks
            f.write(final + "\n")
        sys.exit(0 if not fails else 1)
    elif a[0] == "extract":
        rows, order, inv = extract(a[2], a[1])
        for k in order:
            print("%-40s %s p=%d f=%d  %s" % (k, "FAIL" if rows[k]["f"] else "PASS", rows[k]["p"], rows[k]["f"], rows[k]["hint"]))
        print("invalid: %s" % inv)
    elif a[0] == "patch":
        pairs, i = [], 3
        while i < len(a):
            if a[i] != "--replace" or i + 2 >= len(a):
                sys.exit("patch: bad args (want: patch SRC DST --replace OLD NEW ...)")
            pairs.append((a[i + 1], a[i + 2]))
            i += 3
        patch(a[1], a[2], pairs)
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
