"""Live client for the 8080 test server: a FRESH connection per request (Connection: close), decoded-pixel metrics."""
import os, time, requests, cv2, numpy as np
B = "http://localhost:8080"
H = {"Connection": "close"}
def post(p, d, timeout=60):
    with requests.Session() as s:
        r = s.post(B + p, json=d, timeout=timeout, headers=H); r.raise_for_status(); return r.json()
def get(p, timeout=60):
    with requests.Session() as s:
        r = s.get(B + p, timeout=timeout, headers=H); r.raise_for_status(); return r.json()
def sources():
    return {s["id"]: s for s in get("/api/sources")["sources"]}
def defaults(srcs, sid):
    return {p["uniform"]: p["default"] for p in srcs[sid]["params"]}
def render(path, t=1.13, w=256, h=256):
    if os.path.exists(path): os.remove(path)
    post("/api/render_frame", {"output_path": path, "time": t, "width": w, "height": h})
    im = cv2.imread(path)
    if im is None: raise RuntimeError("no image at " + path)
    return im
def metrics(im):
    m = im.max(axis=2)
    return {"mean": float(im.mean()), "p995": float(np.percentile(m, 99.5)), "lit": float((m > 16).mean()), "sd": float(m.std())}
def fmt(mt):
    return f"mean={mt['mean']:.2f} p99.5={mt['p995']:.0f} lit={mt['lit']*100:.2f}% sd={mt['sd']:.1f}"
def visible(mt):
    return mt["p995"] >= 16 and mt["lit"] >= 0.05
def psnr(a, b):
    a = a.astype(np.float64); b = b.astype(np.float64)
    if a.shape != b.shape: return -1.0
    mse = ((a - b) ** 2).mean()
    return float("inf") if mse == 0 else 10 * np.log10(255 * 255 / mse)
def log(*a): print(*a, flush=True)
