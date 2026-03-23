#!/usr/bin/env python3
"""Test 80 MilkDrop presets via Eyes, score each one, pick best 30."""

import os
import json
import requests
import numpy as np
from PIL import Image
import time
import sys

BASE = "http://localhost:8080"
PRESET_DIR = "/tmp/test_presets"
OUTPUT_DIR = "/tmp/milkdrop_renders"

os.makedirs(OUTPUT_DIR, exist_ok=True)

def load_preset(preset_path):
    """Load a MilkDrop preset via the test API."""
    r = requests.post(f"{BASE}/api/load_milkdrop_preset",
                      json={"preset_path": preset_path}, timeout=10)
    return r.json()

def render_frame(output_path, time_val=1.0):
    """Capture a rendered frame."""
    r = requests.post(f"{BASE}/api/render_frame",
                      json={"output_path": output_path, "time": time_val}, timeout=10)
    return r.json()

def reset():
    """Reset the renderer."""
    requests.post(f"{BASE}/api/reset", json={}, timeout=5)
    time.sleep(0.2)

def score_image(img_path):
    """Score a rendered image on visual interest (0-100).

    Criteria:
    - Non-black: more than 5% of pixels above threshold
    - Color variety: number of distinct hue regions
    - Brightness distribution: good spread of light/dark
    - Contrast: difference between brightest and darkest regions
    """
    try:
        img = Image.open(img_path)
        arr = np.array(img).astype(float)
    except Exception:
        return 0, "failed to open"

    if arr.ndim < 3:
        return 0, "grayscale"

    # Check non-black
    brightness = np.mean(arr, axis=2)
    pct_bright = np.mean(brightness > 10) * 100
    if pct_bright < 3:
        return 0, "all black"

    # Average brightness (0-255)
    avg_bright = np.mean(brightness)

    # Contrast: std dev of brightness
    contrast = np.std(brightness)

    # Color variety: sample hues and count unique regions
    r, g, b = arr[:,:,0], arr[:,:,1], arr[:,:,2]
    max_rgb = np.maximum(np.maximum(r, g), b)
    min_rgb = np.minimum(np.minimum(r, g), b)
    saturation = np.where(max_rgb > 0, (max_rgb - min_rgb) / (max_rgb + 1e-6), 0)
    avg_sat = np.mean(saturation)

    # Edge density (proxy for detail/complexity)
    gray = brightness / 255.0
    dx = np.abs(np.diff(gray, axis=1))
    dy = np.abs(np.diff(gray, axis=0))
    edge_density = (np.mean(dx) + np.mean(dy)) / 2

    # Composite score (weighted)
    score = 0
    score += min(pct_bright, 30)  # Up to 30 points for coverage
    score += min(contrast / 3, 25)  # Up to 25 for contrast
    score += min(avg_sat * 30, 20)  # Up to 20 for color
    score += min(edge_density * 500, 15)  # Up to 15 for detail
    score += min(avg_bright / 10, 10)  # Up to 10 for brightness

    reason = f"bright={avg_bright:.0f} contrast={contrast:.0f} sat={avg_sat:.2f} edges={edge_density:.3f} cover={pct_bright:.0f}%"
    return min(score, 100), reason

def main():
    # Check app is ready
    try:
        r = requests.get(f"{BASE}/api/health", timeout=5)
        print(f"App ready: {r.json()['status']}")
    except Exception as e:
        print(f"App not ready: {e}")
        sys.exit(1)

    # Find all presets
    presets = sorted([f for f in os.listdir(PRESET_DIR) if f.endswith('.milk')])
    print(f"\nTesting {len(presets)} presets...\n")

    results = []

    for i, preset_file in enumerate(presets):
        preset_path = os.path.join(PRESET_DIR, preset_file)
        render_path = os.path.join(OUTPUT_DIR, f"{i:03d}.png")

        # Load preset
        try:
            resp = load_preset(preset_path)
            if "error" in str(resp):
                print(f"  [{i+1:3d}/{len(presets)}] SKIP {preset_file[:60]} — {resp}")
                results.append((preset_file, preset_path, 0, "load failed"))
                continue
        except Exception as e:
            print(f"  [{i+1:3d}/{len(presets)}] ERROR {preset_file[:60]} — {e}")
            results.append((preset_file, preset_path, 0, str(e)))
            continue

        # Render multiple frames to let projectM build up its feedback
        for t in [0.5, 1.0, 1.5, 2.0]:
            render_frame("/dev/null", t)

        # Capture the final frame
        render_frame(render_path, 2.5)

        # Score it
        score, reason = score_image(render_path)

        status = "OK" if score > 10 else "LOW"
        print(f"  [{i+1:3d}/{len(presets)}] {status} score={score:5.1f}  {preset_file[:55]}  ({reason})")

        results.append((preset_file, preset_path, score, reason))

        # Reset for next preset
        reset()
        time.sleep(0.1)

    # Summary
    print("\n" + "=" * 80)
    print("RESULTS SUMMARY")
    print("=" * 80)

    # Sort by score descending
    results.sort(key=lambda x: -x[2])

    total = len(results)
    ok = sum(1 for r in results if r[2] > 10)
    black = sum(1 for r in results if r[2] == 0)

    print(f"\nTotal: {total}, Rendered: {ok}, Black/Failed: {black}")

    print(f"\n--- Top 30 Presets ---")
    top30 = results[:30]
    for i, (name, path, score, reason) in enumerate(top30):
        print(f"  {i+1:2d}. score={score:5.1f}  {name}")

    print(f"\n--- Bottom 10 ---")
    for name, path, score, reason in results[-10:]:
        print(f"  score={score:5.1f}  {name}  ({reason})")

    # Save results as JSON
    with open(os.path.join(OUTPUT_DIR, "results.json"), "w") as f:
        json.dump([{
            "name": r[0],
            "path": r[1],
            "score": r[2],
            "reason": r[3]
        } for r in results], f, indent=2)

    # Save top 30 paths for bundling
    with open(os.path.join(OUTPUT_DIR, "top30.json"), "w") as f:
        json.dump([{
            "name": r[0],
            "path": r[1],
            "score": r[2]
        } for r in top30], f, indent=2)

    print(f"\nResults saved to {OUTPUT_DIR}/results.json")
    print(f"Top 30 saved to {OUTPUT_DIR}/top30.json")

if __name__ == "__main__":
    main()
