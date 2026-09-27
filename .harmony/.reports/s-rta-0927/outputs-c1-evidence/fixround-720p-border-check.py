"""Fix round (outputs-c1): is the (15,15,15) edge in f720_render_frame.png a render/capture border, or content?
Decodes pixels only; no app. argv: <evidence png dir> <fixture image B>."""
import sys, numpy as np
from PIL import Image
D, IMG_B = sys.argv[1], sys.argv[2]
GREY = (15, 15, 15)

def rgb(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(int)

def lead_grey(line):
    n = 0
    for px in line:
        if tuple(int(v) for v in px) != GREY:
            break
        n += 1
    return n

def ring(a):
    """outer margin of exact (15,15,15) at the quarter lines (left, top), and its fraction of the dimension."""
    h, w, _ = a.shape
    L, T = lead_grey(a[h // 4]), lead_grey(a[:, w // 4])
    return f"left {L}px ({100.0 * L / w:.2f}% of {w}), top {T}px ({100.0 * T / h:.2f}% of {h})"

b = rgb(IMG_B)
print(f"fixture imageB {IMG_B.split('/')[-1]} {b.shape[1]}x{b.shape[0]}: corner {tuple(int(v) for v in b[0,0])}, "
      f"frac==(15,15,15) {((b == 15).all(axis=2)).mean():.3f}; margin {ring(b)}")
for f in ["f0_render_frame.png", "fB_render_frame.png", "probe_after_B.png",
          "f720_render_frame.png", "probe_1920x1080_of_720p.png"]:
    a = rgb(f"{D}/{f}")
    h, w, _ = a.shape
    stretched = np.asarray(Image.open(IMG_B).convert("RGB").resize((w, h), Image.BILINEAR)).astype(int)
    print(f"{f} {w}x{h}: corner {tuple(int(v) for v in a[0,0])}, frac==(15,15,15) {((a == 15).all(axis=2)).mean():.3f}, "
          f"margin {ring(a)}, d(capture, imageB stretched to {w}x{h}) {np.abs(a - stretched).mean():.3f}")
