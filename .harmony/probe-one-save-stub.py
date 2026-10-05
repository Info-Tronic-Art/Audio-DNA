#!/usr/bin/env python3
"""probe-one-save-stub.py -- a STAND-IN for Audio-DNA's REST routes, used ONLY by .harmony/probe-one-save-selftest.sh
(no app is launched, nothing of the user's is read). It models what ruling-one-save.md rules for a show save -- and,
per mode, one MUTANT of it -- so the self-test can show that each row of probe-one-save.py passes on the ruled
behaviour and FAILS on its mutant.

usage: probe-one-save-stub.py <port> <mode>
  good   the ruled behaviour: any existing target that is not a version-2 show is copied to backups/<name>.<tag>.json
         first (fail-closed); the file starts with "version": 2; the write is verified (a save onto a "full disk" --
         the folder holds a file named filler.bin -- fails and leaves the target as it was)
  mu4    MU-OS-4: no copy is ever made (backup "not_needed")
  mu5    MU-OS-5: a failed copy is ignored and the write goes on
  mu23   MU-OS-23: a target that does not parse needs no copy
  mu21   MU-OS-21: the write is not verified: on the "full disk" half the bytes are stored and "saved" is reported
  old    the app before the lane: no show_file route (404), {"plain": true} refused (400), no "version", no copy, and
         on the "full disk" a cut-off file
Routes: GET /api/health; POST /api/load_composition {path}; POST /api/debug/save_composition {path} | {"plain": true};
GET /api/debug/show_file. Binds 127.0.0.1 only.
"""
import json
import os
import sys
from http.server import BaseHTTPRequestHandler, HTTPServer

PORT, MODE = int(sys.argv[1]), sys.argv[2]
STATE = {"path": "", "loadedVersion": 2, "show": {},
         "lastSave": {"seq": 0, "result": "", "path": "", "backup": "not_needed", "ms": 0.0}}


def version_of(root):
    v = root.get("version") if isinstance(root, dict) else None
    if type(v) is int and v >= 2:
        return v
    return 0 if isinstance(root, dict) and not isinstance(root.get("layers"), list) else 1


def backup_tag(raw):
    try:
        root = json.loads(raw.decode())
    except Exception:  # noqa: BLE001
        return None if MODE == "mu23" else "other"
    if not isinstance(root, dict) or not isinstance(root.get("decks"), list):
        return "other"
    v = version_of(root)
    return None if v == 2 else f"v{v}"


def backup(target):
    """-> "not_needed" | "made" | "already_there" | "failed" """
    if MODE in ("mu4", "old") or not os.path.isfile(target) or os.path.getsize(target) == 0:
        return "not_needed"
    raw = open(target, "rb").read()
    tag = backup_tag(raw)
    if tag is None:
        return "not_needed"
    folder = os.path.join(os.path.dirname(target), "backups")
    if not os.path.isdir(folder):
        try:
            os.mkdir(folder)
        except OSError:
            return "failed"
    base = os.path.splitext(os.path.basename(target))[0]
    for n in range(1, 1000):
        copy = os.path.join(folder, f"{base}.{tag}{'' if n == 1 else f' ({n})'}.json")
        if not os.path.exists(copy):
            open(copy, "wb").write(raw)
            return "made"
        if open(copy, "rb").read() == raw:
            return "already_there"
    return "failed"


def write_show(target):
    """-> (saved, backup)"""
    b = backup(target)
    if b == "failed" and MODE != "mu5":
        return False, b
    show = dict(STATE["show"])
    for k in ("version", "persistent", "globalTransitionSpeed", "outputDisplay", "keys", "layout"):
        show.pop(k, None)
    if "layers" not in show:   # an old-shape show, converted in memory
        show["layers"] = [{k: v for k, v in row.items() if k not in ("clips", "persistent")}
                          for row in (show.get("decks") or [{}])[0].get("layers", [])]
        show["decks"] = [dict(d, layers=[{"clips": row.get("clips", [])} for row in d.get("layers", [])])
                         for d in show.get("decks", [])]
    out = show if MODE == "old" else dict({"version": 2}, **show, keys={}, layout={})
    data = json.dumps(out, indent=1).encode()
    full = os.path.exists(os.path.join(os.path.dirname(target), "filler.bin"))
    if full and MODE in ("mu21", "old"):
        open(target, "wb").write(data[:len(data) // 2])   # the cut-off file, reported as saved
        return True, b
    if full:
        return False, b                                    # verified: the target is left as it was
    open(target, "wb").write(data)
    return True, b


class H(BaseHTTPRequestHandler):
    def log_message(self, *a):   # quiet
        pass

    def answer(self, code, obj):
        raw = json.dumps(obj).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(raw)))
        self.send_header("Connection", "close")
        self.end_headers()
        self.wfile.write(raw)

    def do_GET(self):
        if self.path == "/api/health":
            return self.answer(200, {"ok": True})
        if self.path == "/api/debug/show_file" and MODE != "old":
            return self.answer(200, {k: STATE[k] for k in ("path", "loadedVersion", "lastSave")})
        self.answer(404, {"error": "no such route"})

    def do_POST(self):
        n = int(self.headers.get("Content-Length") or 0)
        try:
            body = json.loads(self.rfile.read(n).decode() or "{}")
        except Exception:  # noqa: BLE001
            body = {}
        if self.path == "/api/load_composition":
            try:
                STATE["show"] = json.loads(open(body.get("path", ""), "rb").read().decode())
            except Exception:  # noqa: BLE001
                return self.answer(200, {"ok": False, "reason": "could not read/parse file"})
            STATE["path"] = body["path"]
            STATE["loadedVersion"] = version_of(STATE["show"])
            return self.answer(200, {"ok": True})
        if self.path == "/api/debug/save_composition":
            plain = body.get("plain") is True and MODE != "old"
            path = body.get("path", "")
            if not plain and not (path and os.path.isabs(path) and os.path.isdir(os.path.dirname(path))):
                return self.answer(400, {"error": "path required"})
            last = STATE["lastSave"]
            if plain and not STATE["path"]:
                last.update(seq=last["seq"] + 1, result="cancelled", path="", backup="not_needed")
                return self.answer(200, {"ok": True})
            target = STATE["path"] if plain else path
            saved, b = write_show(target)
            last.update(seq=last["seq"] + 1, result="saved" if saved else "failed", path=target, backup=b, ms=1.5)
            if saved:
                STATE["path"] = target
            return self.answer(200, {"ok": True})
        self.answer(404, {"error": "no such route"})


if __name__ == "__main__":
    HTTPServer(("127.0.0.1", PORT), H).serve_forever()
