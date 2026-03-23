#!/usr/bin/env python3
"""
Scan ALL MilkDrop presets, render each one, classify by vibe.

Vibes:
  - Energetic / Rave: high contrast, bright, saturated, complex edges
  - Chill / Ambient: soft, muted, low contrast, smooth
  - Psychedelic / Trippy: high saturation, kaleidoscopic, fractal
  - Dark / Industrial: low brightness, high contrast, monochrome/red/blue

Outputs:
  /tmp/milkdrop_scan/results.json — full results
  /tmp/milkdrop_scan/vibes.json — preset → vibe mapping
  /tmp/milkdrop_scan/energetic.json, chill.json, psychedelic.json, dark.json
"""

import os
import json
import requests
import numpy as np
from PIL import Image
import time
import sys
import signal

BASE = "http://localhost:8080"
PRESET_DIR = "/tmp/milkdrop-presets"
OUTPUT_DIR = "/tmp/milkdrop_scan"
RENDERS_DIR = os.path.join(OUTPUT_DIR, "renders")

os.makedirs(OUTPUT_DIR, exist_ok=True)
os.makedirs(RENDERS_DIR, exist_ok=True)

# Track progress for resume
PROGRESS_FILE = os.path.join(OUTPUT_DIR, "progress.json")

stop_requested = False
def signal_handler(sig, frame):
    global stop_requested
    print("\n[CTRL+C] Stopping after current preset...")
    stop_requested = True
signal.signal(signal.SIGINT, signal_handler)


def classify_vibe(img_path):
    """Analyze a rendered frame and classify into a vibe category.
    Returns (vibe, score_dict) where vibe is the primary classification
    and score_dict has scores for each category.
    """
    try:
        img = Image.open(img_path)
        arr = np.array(img).astype(float)
    except Exception:
        return "unknown", {}

    if arr.ndim < 3:
        return "unknown", {}

    r, g, b = arr[:,:,0], arr[:,:,1], arr[:,:,2]
    brightness = np.mean(arr, axis=2)

    # Key metrics
    avg_bright = np.mean(brightness)
    contrast = np.std(brightness)
    max_rgb = np.maximum(np.maximum(r, g), b)
    min_rgb = np.minimum(np.minimum(r, g), b)
    saturation = np.where(max_rgb > 0, (max_rgb - min_rgb) / (max_rgb + 1e-6), 0)
    avg_sat = np.mean(saturation)

    # Edge density (complexity)
    gray = brightness / 255.0
    dx = np.abs(np.diff(gray, axis=1))
    dy = np.abs(np.diff(gray, axis=0))
    edge_density = (np.mean(dx) + np.mean(dy)) / 2

    # Color distribution
    pct_dark = np.mean(brightness < 40) * 100
    pct_bright = np.mean(brightness > 200) * 100

    # Dominant hue analysis
    hue_r = np.mean(r) / 255
    hue_g = np.mean(g) / 255
    hue_b = np.mean(b) / 255

    # Score each vibe
    scores = {}

    # Energetic: bright, high contrast, saturated, complex
    scores["energetic"] = (
        min(avg_bright / 2, 30) +
        min(contrast / 2, 25) +
        min(avg_sat * 25, 25) +
        min(edge_density * 300, 20)
    )

    # Chill: moderate brightness, low contrast, low edges, soft
    scores["chill"] = (
        (30 - min(abs(avg_bright - 100) / 3, 30)) +
        (25 - min(contrast / 3, 25)) +
        min((1 - edge_density) * 20, 25) +
        (20 - min(avg_sat * 20, 20))
    )

    # Psychedelic: high saturation, complex, moderate+ brightness
    scores["psychedelic"] = (
        min(avg_sat * 40, 35) +
        min(edge_density * 400, 25) +
        min(avg_bright / 4, 20) +
        min(contrast / 4, 20)
    )

    # Dark: low brightness, can be high contrast, low saturation OK
    scores["dark"] = (
        min(pct_dark, 35) +
        min(contrast / 2, 25) +
        (20 - min(avg_bright / 8, 20)) +
        min(edge_density * 200, 20)
    )

    # Pick highest score
    vibe = max(scores, key=scores.get)

    return vibe, {
        "scores": {k: round(v, 1) for k, v in scores.items()},
        "metrics": {
            "brightness": round(avg_bright, 1),
            "contrast": round(contrast, 1),
            "saturation": round(avg_sat, 3),
            "edge_density": round(edge_density, 4),
            "pct_dark": round(pct_dark, 1),
            "pct_bright": round(pct_bright, 1),
        }
    }


def load_progress():
    """Load progress from previous run."""
    if os.path.exists(PROGRESS_FILE):
        with open(PROGRESS_FILE) as f:
            return json.load(f)
    return {"completed": [], "results": []}


def save_progress(progress):
    """Save progress for resume."""
    with open(PROGRESS_FILE, "w") as f:
        json.dump(progress, f)


def main():
    global stop_requested

    # Check app is ready
    try:
        r = requests.get(f"{BASE}/api/health", timeout=5)
        print(f"App ready: {r.json()['status']}")
    except Exception as e:
        print(f"App not ready: {e}")
        print("Start the app with: Audio-DNA --test-mode --test-port=8080")
        sys.exit(1)

    # Find all presets
    all_presets = []
    for root, dirs, files in os.walk(PRESET_DIR):
        for f in files:
            if f.endswith('.milk'):
                all_presets.append(os.path.join(root, f))
    all_presets.sort()

    print(f"\nTotal presets found: {len(all_presets)}")

    # Load progress
    progress = load_progress()
    completed_set = set(progress["completed"])
    results = progress["results"]

    remaining = [p for p in all_presets if p not in completed_set]
    print(f"Already completed: {len(completed_set)}")
    print(f"Remaining: {len(remaining)}")
    print()

    start_time = time.time()

    for i, preset_path in enumerate(remaining):
        if stop_requested:
            print(f"\n[STOPPED] Saving progress at {len(completed_set)} presets...")
            break

        preset_name = os.path.basename(preset_path).replace('.milk', '')
        render_path = os.path.join(RENDERS_DIR, f"{len(completed_set):05d}.png")

        # Load preset (this feeds synthetic audio and waits ~1s)
        try:
            resp = requests.post(f"{BASE}/api/load_milkdrop_preset",
                                json={"preset_path": preset_path}, timeout=15)
            if resp.status_code != 200:
                results.append({
                    "path": preset_path,
                    "name": preset_name,
                    "vibe": "unknown",
                    "error": "load failed"
                })
                completed_set.add(preset_path)
                continue
        except Exception as e:
            print(f"  ERROR loading preset: {e}")
            # App might have crashed — try to recover
            time.sleep(2)
            try:
                requests.get(f"{BASE}/api/health", timeout=3)
            except:
                print("App is down. Saving progress and exiting.")
                break
            continue

        # Capture frame
        try:
            requests.post(f"{BASE}/api/render_frame",
                         json={"output_path": render_path, "time": 5.0}, timeout=10)
        except:
            completed_set.add(preset_path)
            continue

        # Classify
        vibe, analysis = classify_vibe(render_path)

        entry = {
            "path": preset_path,
            "name": preset_name,
            "vibe": vibe,
        }
        if analysis:
            entry.update(analysis)
        results.append(entry)
        completed_set.add(preset_path)

        # Delete render to save disk (keep every 100th for review)
        if len(completed_set) % 100 != 0 and os.path.exists(render_path):
            os.remove(render_path)

        # Progress logging
        total_done = len(completed_set)
        elapsed = time.time() - start_time
        rate = (i + 1) / elapsed if elapsed > 0 else 0
        eta = (len(remaining) - i - 1) / rate if rate > 0 else 0

        if total_done % 50 == 0 or total_done < 10:
            print(f"  [{total_done:5d}/{len(all_presets)}] {vibe:12s}  {preset_name[:50]}"
                  f"  ({elapsed/60:.0f}m elapsed, {eta/60:.0f}m remaining)")

        # Save progress every 100 presets
        if total_done % 100 == 0:
            progress["completed"] = list(completed_set)
            progress["results"] = results
            save_progress(progress)

    # Final save
    progress["completed"] = list(completed_set)
    progress["results"] = results
    save_progress(progress)

    # Generate vibe reports
    print(f"\n{'='*60}")
    print(f"SCAN COMPLETE: {len(completed_set)} presets classified")
    print(f"{'='*60}")

    vibe_counts = {}
    vibe_lists = {"energetic": [], "chill": [], "psychedelic": [], "dark": [], "unknown": []}
    for r in results:
        v = r.get("vibe", "unknown")
        vibe_counts[v] = vibe_counts.get(v, 0) + 1
        if v in vibe_lists:
            vibe_lists[v].append(r)

    for v, count in sorted(vibe_counts.items()):
        print(f"  {v:15s}: {count}")

    # Save per-vibe files
    for vibe_name, entries in vibe_lists.items():
        if not entries:
            continue
        # Sort by top score within this vibe
        entries.sort(key=lambda x: -x.get("scores", {}).get(vibe_name, 0))
        with open(os.path.join(OUTPUT_DIR, f"{vibe_name}.json"), "w") as f:
            json.dump(entries, f, indent=2)

    # Save master vibe mapping (preset path → vibe)
    vibe_map = {r["path"]: r["vibe"] for r in results}
    with open(os.path.join(OUTPUT_DIR, "vibes.json"), "w") as f:
        json.dump(vibe_map, f, indent=2)

    # Save full results
    with open(os.path.join(OUTPUT_DIR, "results.json"), "w") as f:
        json.dump(results, f, indent=2)

    print(f"\nResults saved to {OUTPUT_DIR}/")
    print(f"  vibes.json — preset → vibe mapping")
    print(f"  energetic.json, chill.json, psychedelic.json, dark.json")
    print(f"  results.json — full analysis data")


if __name__ == "__main__":
    main()
