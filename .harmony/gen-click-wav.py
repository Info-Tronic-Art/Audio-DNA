#!/usr/bin/env python3
"""gen-click-wav.py -- s-rta-0923 step3 lane S3-G. python3 stdlib only (no
numpy/PIL) so probe-step3.sh can generate the click WAV without the .venv.

Writes a deterministic click-track WAV: silence with a single 1-sample,
full-scale impulse every `--interval` frames -- the D10.3 T1 alignment
signal (24000 frames @ 48kHz = 0.5s = 120 BPM equivalent, plan s-rta-0923
step3 section 4). The alignment check in probe-step3.sh locates these
impulses in the captured take audio and compares against the take's
`markers[]` (onsetMarkers:true) to measure record/analysis latency.

Usage: gen-click-wav.py [output.wav] [--duration-s N] [--rate N]
                         [--interval N] [--channels N] [--amplitude N]
Defaults match the plan exactly: 120s, 48000 Hz, stereo, impulse every
24000 frames, amplitude 30000 (comfortably below int16 full scale to avoid
any downstream clipping in the tap's re-encode).
"""
import argparse
import struct
import wave


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("output", nargs="?", default="/tmp/click_48k.wav")
    p.add_argument("--duration-s", type=float, default=120.0)
    p.add_argument("--rate", type=int, default=48000)
    p.add_argument("--interval", type=int, default=24000)
    p.add_argument("--channels", type=int, default=2)
    p.add_argument("--amplitude", type=int, default=30000)
    args = p.parse_args()

    total_frames = int(round(args.duration_s * args.rate))
    silence_frame = struct.pack("<" + "h" * args.channels, *([0] * args.channels))
    click_frame = struct.pack("<" + "h" * args.channels, *([args.amplitude] * args.channels))

    n_clicks = 0
    with wave.open(args.output, "wb") as w:
        w.setnchannels(args.channels)
        w.setsampwidth(2)  # int16
        w.setframerate(args.rate)
        for i in range(total_frames):
            if i % args.interval == 0:
                w.writeframesraw(click_frame)
                n_clicks += 1
            else:
                w.writeframesraw(silence_frame)

    print(f"wrote {args.output}: {total_frames} frames @ {args.rate} Hz, "
          f"{args.channels} ch, {n_clicks} impulses every {args.interval} frames "
          f"({args.interval / args.rate:.3f}s = {60.0 * args.rate / args.interval:.1f} BPM equivalent)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
