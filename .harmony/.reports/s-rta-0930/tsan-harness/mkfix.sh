#!/bin/bash
# fixtures for the sanitizer sweep (no app involved)
M=${SWEEP_MEDIA:-/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/sweep/media}
F=/opt/homebrew/bin/ffmpeg
set -e
cd $M
$F -y -loglevel error -f lavfi -i testsrc2=s=1920x1080:r=30:d=6 -g 60 -c:v libx264 -pix_fmt yuv420p -preset ultrafast v1080.mp4
$F -y -loglevel error -f lavfi -i testsrc2=s=3840x2160:r=30:d=4 -g 60 -c:v libx264 -pix_fmt yuv420p -preset ultrafast v4k.mp4
for k in $(seq 0 15); do cp -c v1080.mp4 v1080_$(printf %02d $k).mp4; done
for k in $(seq 0 3); do cp -c v4k.mp4 v4k_$k.mp4; done
for k in 0 1 2 3; do $F -y -loglevel error -f lavfi -i "testsrc2=s=1920x1080:r=1:d=1,hue=h=$((k*70))" -frames:v 1 img_$k.png; done
ls -la $M | head -40
