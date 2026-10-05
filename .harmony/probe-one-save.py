#!/usr/bin/env python3
"""probe-one-save.py -- lane one-save (ruling-one-save.md section 5 "LIVE ROWS"): the rows of .harmony/probe-one-save.sh.
RUN ONLY BY HARMONY (a builder never launches the app). Self-test without the app: .harmony/probe-one-save-selftest.sh
(a stub server stands in for Audio-DNA; it proves each row passes on the ruled behaviour and FAILS on its mutant).

usage: probe-one-save.py <root> <out-dir> [row,row,...]
  ONESAVE_API       base URL (default http://127.0.0.1:7070; the self-test points it at its stub)
  ONESAVE_FULLDISK  os_l21 only: the mount point of the small disk image Harmony made (ruling M-1)
  ONESAVE_NOFILL    os_l21, SELF-TEST ONLY: do not write the filler file (the stub models the full disk)

Every request is sent with "Connection: close". Every file this probe writes is under <out-dir> (or ONESAVE_FULLDISK);
no fixture is a file of the user's and no row reads or writes one of his folders. (The APP the .sh launches still
reads his library folder until stage S4b -- see the .sh header.)

A row prints exactly ONE verdict line: "PASS  OS-L<n>: <facts>" or "FAIL  OS-L<n>: <facts>" (os_l21 on a rig where
the measurement cannot be made: "INFO  OS-L21: <why>"), plus "INFO  OS-L<n>: ..." lines for measured facts (M-2: the
time of writeShow). The facts name every clause with the value read; a clause that did not hold starts with "NOT ".
Last line: "PY <p> PASS, <f> FAIL, <i> INFO-only (rows <list>)"; exit 0 iff no row FAILED.

STAGE S1 rows (gate G-OS1):
  os_l13   OS-L13: F-OLD copied to <out>/s/old.json; load it; save_composition {path: old.json} -> show_file.lastSave
           result "saved", backup "made"; s/backups/old.v0.json has the ORIGINAL's sha256; old.json parses, its first
           key is "version", value 2; save_composition {"plain": true} -> backup "not_needed"; the backups folder holds
           exactly 1 file. RED: MU-OS-4 (no copy); the 185147b app (no backups folder; it has no show_file route and
           no "version").
  os_l14   OS-L14: the same with a plain FILE named "backups" in <out>/s2 -> old.json's sha256 unchanged; lastSave
           result "failed", backup "failed". RED: MU-OS-5 (old.json rewritten).
  os_l14b  OS-L14b: <out>/s3/x.json = F-GARBAGE (the 10 bytes "not json!!"); load F-A; save_composition {path: x.json}
           -> s3/backups/x.other.json holds exactly those 10 bytes; x.json is a version-2 show. RED: MU-OS-23 (no copy).
  os_l21   OS-L21 = M-1, the full-disk run: on ONESAVE_FULLDISK a version-2 show of about 200 KB; a filler file leaves
           LESS free space than the show's size; load the show; save_composition {"plain": true} -> lastSave result
           "failed" and the show's sha256 unchanged. On an app without the show_file route (the 185147b arm) the row
           saves with {path: the show's own path} and prints what the file became (cut off / unchanged): its verdict
           line is FAIL when the file changed. RED: MU-OS-21; the 185147b app (outcome O1).
"""
import hashlib
import json
import os
import struct
import sys
import time
import zlib
import urllib.error
import urllib.request

ROOT = sys.argv[1] if len(sys.argv) > 1 else "."
OUT = sys.argv[2] if len(sys.argv) > 2 else "/tmp"
ONLY = [r for r in sys.argv[3].split(",") if r] if len(sys.argv) > 3 and sys.argv[3] else None
A = os.environ.get("ONESAVE_API", "http://127.0.0.1:7070")
GARBAGE = b"not json!!"   # F-GARBAGE, 10 bytes
NPASS = NFAIL = NINFO = 0


# ------------------------------------------------------------------ REST (Connection: close on every request)
def http(method, path, body=None, timeout=20):
    """-> (status, parsed JSON or {}); status 0 = no answer."""
    data = json.dumps(body).encode() if body is not None else (b"" if method == "POST" else None)
    req = urllib.request.Request(A + path, data=data, method=method,
                                 headers={"Connection": "close", "Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            raw, code = r.read(), r.status
    except urllib.error.HTTPError as e:
        raw, code = e.read(), e.code
    except Exception as e:  # noqa: BLE001
        return 0, {"error": str(e)}
    try:
        return code, json.loads(raw.decode() or "{}")
    except Exception:  # noqa: BLE001
        return code, {}


def show_file():
    """GET /api/debug/show_file -> (status, {path, loadedVersion, lastSave{seq, result, path, backup, ms}})."""
    return http("GET", "/api/debug/show_file")


def load(path):
    """POST /api/load_composition answers once the staged swap is done."""
    code, body = http("POST", "/api/load_composition", {"path": path}, timeout=30)
    return code == 200 and body.get("ok") is True, f"HTTP {code} {json.dumps(body)[:120]}"


def save(body, target):
    """POST /api/debug/save_composition, then wait for the save to be DONE. The route answers before the write: the
    result is show_file.lastSave with a seq above the one read before the post. On an app without show_file (404) the
    wait is a settle of the target file (unchanged size for 1 s, 5 s at most).
    -> (lastSave dict or None, note)"""
    c0, s0 = show_file()
    seq0 = (s0.get("lastSave") or {}).get("seq", 0) if c0 == 200 else None
    code, ans = http("POST", "/api/debug/save_composition", body)
    if code != 200 or ans.get("ok") is not True:
        return None, f"save_composition HTTP {code} {json.dumps(ans)[:120]}"
    if seq0 is None:
        t0, last, since = time.time(), -2, time.time()
        while time.time() - t0 < 5.0:
            size = os.path.getsize(target) if os.path.exists(target) else -1
            if size != last:
                last, since = size, time.time()
            elif time.time() - since >= 1.0:
                break
            time.sleep(0.1)
        return None, f"show_file HTTP {c0} (no lastSave on this app)"
    t0 = time.time()
    while time.time() - t0 < 10.0:
        c, s = show_file()
        ls = s.get("lastSave") or {}
        if c == 200 and ls.get("seq", 0) > seq0:
            return ls, "ok"
        time.sleep(0.05)
    return None, f"lastSave.seq stayed {seq0} for 10 s"


# ------------------------------------------------------------------ files
def sha(path):
    try:
        return hashlib.sha256(open(path, "rb").read()).hexdigest()
    except OSError:
        return None


def write(path, data):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as f:
        f.write(data if isinstance(data, bytes) else data.encode())
    return path


def first_key_and_version(path):
    """-> (first key, version value, has a "decks" list) of a JSON object file; (None, None, False) if unreadable."""
    try:
        pairs = json.loads(open(path, "rb").read().decode(), object_pairs_hook=list)
    except Exception:  # noqa: BLE001
        return None, None, False
    if not isinstance(pairs, list) or not pairs or not isinstance(pairs[0], (list, tuple)):
        return None, None, False
    d = dict(pairs)
    return pairs[0][0], d.get("version"), isinstance(d.get("decks"), list)


def is_v2_show(path):
    k, v, decks = first_key_and_version(path)
    return k == "version" and type(v) is int and v == 2 and decks


def listing(folder):
    return sorted(os.listdir(folder)) if os.path.isdir(folder) else None


# ------------------------------------------------------------------ fixtures (written by the probe; none is his)
def picture():
    """A 16 x 16 grey PNG in <out-dir> (stdlib only) -- the media of every fixture clip."""
    path = os.path.join(OUT, "fixture.png")
    if not os.path.exists(path):
        def chunk(kind, data):
            return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)
        raw = b"".join(b"\0" + b"\x80\x80\x80" * 16 for _ in range(16))
        write(path, b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", 16, 16, 8, 2, 0, 0, 0))
              + chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b""))
    return path


def clip(cid, name):
    return {"name": name, "id": cid, "mediaType": 1, "mediaFile": picture(), "effects": []}


def binding(**kw):
    """One key / MIDI binding with all 22 fields BindingManager::toVar writes (enums as ints)."""
    b = {"inputType": 0, "keyCode": 0, "keyModShift": False, "keyModCmd": False, "keyModAlt": False, "midiChannel": 0,
         "midiNote": 0, "midiCC": 0, "action": 0, "triggerMode": 0, "ccMode": 0, "ccStepSize": 0.01, "targetMode": 0,
         "targetClipId": 0, "velocityToOpacity": False, "targetLayerIndex": 0, "targetColumn": 0, "targetDeckIndex": 0,
         "targetEffectIndex": 0, "targetMacroIndex": 0, "targetRoutineSlot": 0, "enabled": True}
    b.update(kw)
    return b


def row_settings(lid, opacity=1.0, blend=0, ltype=0):
    return {"name": f"Layer {lid + 1}", "id": lid, "opacity": opacity, "visible": True, "blendMode": blend,
            "type": ltype, "transitionSpeed": 0.0, "layerEffects": []}


def f_old():
    """F-OLD: old shape as files written before decks became boxes -- no "version", no top-level "layers"; decks
    "Deck 1" (id 0) and "Deck 2" (id 100); every row carries its layer settings; "persistent"; "globalTransitionSpeed";
    "outputDisplay"."""
    def row(lid, clips, **kw):
        r = row_settings(lid, **kw)
        r["clips"] = clips
        return r
    d1 = [row(0, [clip(1, "d1r1")]), dict(row(1, [clip(2, "d1r2")], opacity=0.8), persistent=True),
          row(2, [clip(3, "d1r3")], ltype=1)]
    d2 = [row(0, []), row(1, [clip(4, "d2r2a"), clip(5, "d2r2b")], opacity=0.8), row(2, [], opacity=0.4, blend=1, ltype=1)]
    return {"name": "one-save-old", "activeDeckIndex": 0, "masterOpacity": 1.0, "bpmMultiplier": 1, "quantizeMode": 0,
            "outputWidth": 1920, "outputHeight": 1080, "outputDisplay": -1, "globalTransitionSpeed": 0.5,
            "decks": [{"name": "Deck 1", "id": 0, "numColumns": 2, "layers": d1},
                      {"name": "Deck 2", "id": 100, "numColumns": 2, "layers": d2}]}


def f_a(nclips=1, pad=""):
    """F-A: a version-2 show, 2 layers, deck "A1", keys = 3 bindings, layout {300, [0.30, 0.55, 0.80]}. `nclips` /
    `pad` grow the file (os_l21's ~200 KB show)."""
    cells = [clip(100 + i, f"a{i}{pad}") for i in range(nclips)]
    # Keyboard 81 TriggerClip L0 C0; MidiNote 36 ch 1 TriggerColumn 1; MidiCC 7 MasterOpacity (plan-one-save PL:591-592)
    keys = {"bindings": [
        binding(inputType=0, keyCode=81, action=0, targetLayerIndex=0, targetColumn=0),
        binding(inputType=1, midiNote=36, midiChannel=1, action=1, targetColumn=1),
        binding(inputType=2, midiCC=7, midiChannel=1, action=16)], "version": 1}
    return {"version": 2, "name": "one-save-a", "activeDeckIndex": 0, "masterOpacity": 1.0, "bpmMultiplier": 1,
            "quantizeMode": 0, "outputWidth": 1920, "outputHeight": 1080,
            "layers": [row_settings(0), row_settings(1, ltype=1)],
            "decks": [{"name": "A1", "id": 100, "numColumns": max(1, nclips),
                       "layers": [{"clips": cells}, {"clips": []}]}],
            "keys": keys, "layout": {"deckDividerY": 300, "vDividerFrac": [0.30, 0.55, 0.80]}}


# ------------------------------------------------------------------ verdict lines
class Row:
    def __init__(self, tag):
        self.tag, self.facts, self.good = tag, [], True

    def clause(self, cond, text):
        self.facts.append(text if cond else "NOT " + text)
        self.good = self.good and bool(cond)
        return bool(cond)

    def info(self, text):
        print(f"INFO  {self.tag}: {text}", flush=True)

    def done(self):
        global NPASS, NFAIL
        if self.good:
            NPASS += 1
        else:
            NFAIL += 1
        print(f"{'PASS' if self.good else 'FAIL'}  {self.tag}: " + "; ".join(self.facts), flush=True)


# ------------------------------------------------------------------ S1 rows
def os_l13():
    r = Row("OS-L13")
    folder = os.path.join(OUT, "s")
    target = write(os.path.join(folder, "old.json"), json.dumps(f_old(), indent=1))
    sha0 = sha(target)
    ok, why = load(target)
    if not r.clause(ok, f"the old-shape show loads ({why})"):
        return r.done()
    ls, note = save({"path": target}, target)
    ls = ls or {}
    r.clause(ls.get("result") == "saved" and ls.get("backup") == "made",
             f"first save: lastSave result '{ls.get('result')}', backup '{ls.get('backup')}' (want saved / made; {note})")
    copy = os.path.join(folder, "backups", "old.v0.json")
    r.clause(sha(copy) == sha0, f"backups/old.v0.json has the ORIGINAL's sha256 (copy {str(sha(copy))[:12]}, "
                                f"original {sha0[:12]}; backups folder: {listing(os.path.join(folder, 'backups'))})")
    k, v, decks = first_key_and_version(target)
    r.clause(k == "version" and type(v) is int and v == 2 and decks,
             f"old.json parses, first key '{k}', version {v!r} (want 'version', 2)")
    ms1 = ls.get("ms")
    ls2, note2 = save({"plain": True}, target)
    ls2 = ls2 or {}
    r.clause(ls2.get("result") == "saved" and ls2.get("backup") == "not_needed",
             f"plain save: lastSave result '{ls2.get('result')}', backup '{ls2.get('backup')}' (want saved / "
             f"not_needed; {note2})")
    files = listing(os.path.join(folder, "backups"))
    r.clause(files is not None and len(files) == 1, f"the backups folder holds exactly 1 file ({files})")
    r.info(f"M-2 the time of writeShow on F-OLD ({os.path.getsize(target)} bytes after the save): first save (target "
           f"read + parse, copy, write, read-back) {ms1} ms; plain save (no copy) {ls2.get('ms')} ms")
    r.done()


def os_l14():
    r = Row("OS-L14")
    folder = os.path.join(OUT, "s2")
    target = write(os.path.join(folder, "old.json"), json.dumps(f_old(), indent=1))
    blocker = write(os.path.join(folder, "backups"), "a plain FILE named backups")
    sha0, shab = sha(target), sha(blocker)
    ok, why = load(target)
    if not r.clause(ok, f"the old-shape show loads ({why})"):
        return r.done()
    ls, note = save({"path": target}, target)
    ls = ls or {}
    r.clause(sha(target) == sha0, f"old.json's sha256 unchanged (now {str(sha(target))[:12]}, before {sha0[:12]})")
    r.clause(ls.get("result") == "failed" and ls.get("backup") == "failed",
             f"lastSave result '{ls.get('result')}', backup '{ls.get('backup')}' (want failed / failed; {note})")
    r.clause(os.path.isfile(blocker) and sha(blocker) == shab, "the plain file 'backups' is as it was")
    r.clause(listing(folder) == ["backups", "old.json"], f"nothing else appeared in the folder ({listing(folder)})")
    r.done()


def os_l14b():
    r = Row("OS-L14b")
    folder = os.path.join(OUT, "s3")
    target = write(os.path.join(folder, "x.json"), GARBAGE)
    fa = write(os.path.join(OUT, "fa", "F-A.json"), json.dumps(f_a(), indent=1))
    ok, why = load(fa)
    if not r.clause(ok, f"F-A loads ({why})"):
        return r.done()
    ls, note = save({"path": target}, target)
    ls = ls or {}
    copy = os.path.join(folder, "backups", "x.other.json")
    got = open(copy, "rb").read() if os.path.isfile(copy) else None
    r.clause(got == GARBAGE, f"backups/x.other.json holds exactly the 10 bytes ({got!r}; backups folder: "
                             f"{listing(os.path.join(folder, 'backups'))})")
    k, v, _ = first_key_and_version(target)
    r.clause(is_v2_show(target), f"x.json is a version-2 show (first key '{k}', version {v!r})")
    r.clause(ls.get("result") == "saved" and ls.get("backup") == "made",
             f"lastSave result '{ls.get('result')}', backup '{ls.get('backup')}' (want saved / made; {note})")
    r.done()


def os_l21():
    global NINFO
    r = Row("OS-L21")
    disk = os.environ.get("ONESAVE_FULLDISK", "")
    if not disk or not os.path.isdir(disk):
        NINFO += 1
        print("INFO  OS-L21: not run -- ONESAVE_FULLDISK is not a folder (ruling M-1 outcome O3 unless Harmony mounts "
              "the 2 MB image and re-runs: a blocked measurement, never a pass)", flush=True)
        return
    target = write(os.path.join(disk, "show.json"), json.dumps(f_a(nclips=48, pad="-" + "x" * 4000), indent=1))
    size = os.path.getsize(target)
    ok, why = load(target)
    if not r.clause(ok, f"the {size}-byte version-2 show loads from the small disk ({why})"):
        return r.done()
    filler = os.path.join(disk, "filler.bin")
    if not os.environ.get("ONESAVE_NOFILL"):
        st = os.statvfs(disk)
        free = st.f_bavail * st.f_frsize
        want = max(0, free - size // 2)   # leave about half the show's size free
        try:
            with open(filler, "wb") as f:
                f.write(b"\0" * want)
                f.flush()
                os.fsync(f.fileno())
        except OSError as e:
            r.info(f"filler write stopped early: {e}")
        st = os.statvfs(disk)
        left = st.f_bavail * st.f_frsize
        if not r.clause(left < size, f"free space {left} bytes is LESS than the show's {size} bytes"):
            return r.done()
    sha0 = sha(target)
    c, _ = show_file()
    body = {"plain": True} if c == 200 else {"path": target}   # the 185147b arm has only {path}
    ls, note = save(body, target)
    after = sha(target)
    now = os.path.getsize(target) if os.path.exists(target) else -1
    parses = first_key_and_version(target)[0] is not None
    r.clause(after == sha0, f"the show's sha256 is unchanged (size before {size}, after {now}; still parses: {parses})")
    if c == 200:
        ls = ls or {}
        r.clause(ls.get("result") == "failed", f"lastSave result '{ls.get('result')}' (want failed; {note})")
    else:
        r.info(f"this app has no show_file route (HTTP {c}): the 185147b arm. Outcome "
               f"{'O1 (a cut-off or changed file, nothing reported)' if after != sha0 else 'O2 (the file is unchanged)'}")
    leftovers = [n for n in (listing(disk) or []) if n not in ("show.json", "filler.bin") and not n.startswith(".")]
    r.clause(not leftovers, f"no stray file beside the show ({leftovers})")
    r.done()


ROWS = [("os_l13", os_l13), ("os_l14", os_l14), ("os_l14b", os_l14b), ("os_l21", os_l21)]


def main():
    names = [n for n, _ in ROWS]
    todo = ONLY if ONLY else names
    unknown = [n for n in todo if n not in names]
    if unknown:
        print(f"FAIL  unknown row(s) {unknown}; rows: {names}")
        return 64
    print(f"PY-ROWS registered {len(ROWS)}")
    os.makedirs(OUT, exist_ok=True)
    for name, fn in ROWS:
        if name in todo:
            print(f"--- {name}", flush=True)
            fn()
    print(f"PY {NPASS} PASS, {NFAIL} FAIL, {NINFO} INFO-only (rows {','.join(todo)})")
    return 0 if NFAIL == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
