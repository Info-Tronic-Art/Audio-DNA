"""
Vision Check — Image comparison for the Eyes visual testing harness.

Compares rendered frames against golden reference images using PSNR and SSIM
metrics. Saves amplified diff images on failure for debugging.
"""

import os
import cv2
import numpy as np


def compute_psnr(img1: np.ndarray, img2: np.ndarray) -> float:
    """Compute Peak Signal-to-Noise Ratio between two images.

    Returns float('inf') for identical images, higher = more similar.
    """
    mse = np.mean((img1.astype(np.float64) - img2.astype(np.float64)) ** 2)
    if mse == 0:
        return float("inf")
    return 10.0 * np.log10(255.0**2 / mse)


def compute_ssim(img1: np.ndarray, img2: np.ndarray) -> float:
    """Compute Structural Similarity Index between two images.

    Uses scikit-image's implementation. Returns 1.0 for identical images.
    Falls back to manual SSIM if scikit-image is not available.
    """
    try:
        from skimage.metrics import structural_similarity as ssim

        return ssim(img1, img2, channel_axis=2)
    except ImportError:
        # Manual SSIM fallback (Wang et al. 2004)
        c1 = (0.01 * 255) ** 2
        c2 = (0.03 * 255) ** 2

        i1 = img1.astype(np.float64)
        i2 = img2.astype(np.float64)

        mu1 = cv2.GaussianBlur(i1, (11, 11), 1.5)
        mu2 = cv2.GaussianBlur(i2, (11, 11), 1.5)

        mu1_sq = mu1**2
        mu2_sq = mu2**2
        mu1_mu2 = mu1 * mu2

        sigma1_sq = cv2.GaussianBlur(i1**2, (11, 11), 1.5) - mu1_sq
        sigma2_sq = cv2.GaussianBlur(i2**2, (11, 11), 1.5) - mu2_sq
        sigma12 = cv2.GaussianBlur(i1 * i2, (11, 11), 1.5) - mu1_mu2

        ssim_map = ((2 * mu1_mu2 + c1) * (2 * sigma12 + c2)) / (
            (mu1_sq + mu2_sq + c1) * (sigma1_sq + sigma2_sq + c2)
        )
        return float(np.mean(ssim_map))


def verify_frame(
    rendered_path: str,
    golden_path: str,
    psnr_threshold: float = 50.0,
    ssim_threshold: float = 0.99,
    save_diff: bool = True,
    diff_dir: str = "tests/visual/diffs",
) -> tuple:
    """Compare a rendered frame against a golden reference.

    Args:
        rendered_path: Path to the rendered PNG from the test.
        golden_path: Path to the golden reference PNG.
        psnr_threshold: Minimum acceptable PSNR (dB). Higher = stricter.
        ssim_threshold: Minimum acceptable SSIM [0-1]. Higher = stricter.
        save_diff: Whether to save an amplified diff image on failure.
        diff_dir: Directory for diff images.

    Returns:
        (passed: bool, metrics: dict) where metrics contains:
            - psnr: float
            - ssim: float
            - max_pixel_diff: int (0-255)
            - error: str (if load failed)
    """
    rendered = cv2.imread(rendered_path)
    golden = cv2.imread(golden_path)

    if rendered is None:
        return False, {"error": f"Could not load rendered: {rendered_path}"}
    if golden is None:
        return False, {"error": f"Could not load golden: {golden_path}"}
    if rendered.shape != golden.shape:
        return False, {
            "error": f"Resolution mismatch: rendered={rendered.shape} golden={golden.shape}"
        }

    # Compute metrics
    psnr_val = compute_psnr(rendered, golden)
    ssim_val = compute_ssim(rendered, golden)

    # Max per-pixel difference
    diff = cv2.absdiff(rendered, golden)
    max_diff = int(np.max(diff))

    passed = psnr_val >= psnr_threshold and ssim_val >= ssim_threshold

    metrics = {
        "psnr": psnr_val,
        "ssim": ssim_val,
        "max_pixel_diff": max_diff,
    }

    if not passed and save_diff:
        os.makedirs(diff_dir, exist_ok=True)
        name = os.path.basename(rendered_path)
        # Amplify diff for visibility (normalize to full 0-255 range)
        if max_diff > 0:
            diff_amplified = (diff.astype(np.float32) / max_diff * 255).astype(np.uint8)
        else:
            diff_amplified = diff
        cv2.imwrite(os.path.join(diff_dir, f"diff_{name}"), diff_amplified)

    return passed, metrics
