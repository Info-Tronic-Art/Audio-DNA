#!/usr/bin/env python3
"""Verify that an effect with its default params produces a visible change from the original image."""
import sys, time
import requests
import numpy as np
from PIL import Image

BASE = "http://localhost:8080"
IMG = "/Users/boriskarpman/Documents/RealTimeAudio/tests/fixtures/test_card.png"

def psnr(path1, path2):
    img1 = np.array(Image.open(path1).convert("RGB")).astype(np.float64)
    img2 = np.array(Image.open(path2).convert("RGB")).astype(np.float64)
    h = min(img1.shape[0], img2.shape[0])
    w = min(img1.shape[1], img2.shape[1])
    img1 = img1[:h, :w]
    img2 = img2[:h, :w]
    mse = np.mean((img1 - img2) ** 2)
    if mse == 0:
        return 999.0
    return 10 * np.log10(255.0**2 / mse)

def test_effect(name):
    """Test if an effect with defaults produces visible change. Returns PSNR."""
    # Reset all effects, load image
    requests.post(f"{BASE}/api/reset", json={})
    requests.post(f"{BASE}/api/load_image", json={"filepath": IMG})
    time.sleep(0.1)

    # Render baseline
    base_path = "/tmp/audit_base.png"
    r = requests.post(f"{BASE}/api/render_frame", json={"output_path": base_path, "time": 1.0})
    if not r.ok:
        return -1, f"render fail: {r.status_code}"

    # Enable effect with defaults (no params override = use registered defaults)
    requests.post(f"{BASE}/api/set_effect", json={"name": name, "enabled": True})
    time.sleep(0.1)

    # Render with effect
    fx_path = "/tmp/audit_fx.png"
    r = requests.post(f"{BASE}/api/render_frame", json={"output_path": fx_path, "time": 1.0})
    if not r.ok:
        return -1, f"render fail: {r.status_code}"

    try:
        p = psnr(base_path, fx_path)
    except Exception as e:
        return -1, str(e)

    return p, "OK"

if __name__ == "__main__":
    effects = sys.argv[1:] if len(sys.argv) > 1 else []
    if not effects:
        r = requests.get("http://localhost:7070/api/effects")
        effects = [e["name"] for e in r.json().get("effects", [])]

    print(f"Testing {len(effects)} effects...")
    visible = []
    invisible = []
    for name in effects:
        p, status = test_effect(name)
        if p < 40:
            visible.append((name, p))
            tag = "VISIBLE"
        else:
            invisible.append((name, p))
            tag = "INVISIBLE"
        print(f"  {tag:10s} PSNR={p:6.1f}  {name}")

    print(f"\nRESULT: {len(visible)} VISIBLE, {len(invisible)} INVISIBLE out of {len(effects)}")
    if invisible:
        print(f"\n{len(invisible)} effects INVISIBLE with defaults:")
        for name, p in invisible:
            print(f"  {name}")
