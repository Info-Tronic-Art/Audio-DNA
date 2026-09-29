# vtbench.sh -- price the decode half of c4 (zero-copy) OUTSIDE the app: ffmpeg decode of the 4K fixture (300 frames),
# software (libavcodec h264, 2 frame threads, + sws to RGBA like the app) vs VideoToolbox (hardware, frames kept as
# CVPixelBuffers = videotoolbox_vld, and hardware + download to RGBA). 1 stream and 4 concurrent streams. Wall time + CPU
# (user + sys) from /usr/bin/time -l. Holds the live lock (quiet machine) so no lane measures through it.
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/diag-vfps
LANE=diag-vfps
. /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/lib/lock.sh
F=$S/media/a4k_g250.mp4
OUT=$S/runs/vtbench; mkdir -p $OUT
acquire_quiet_lock || exit 1
echo "$(date +%T) load start $(sysctl -n vm.loadavg)" | tee $OUT/bench.txt
one(){ local tag=$1; shift; /usr/bin/time -l ffmpeg -hide_banner -loglevel error -benchmark "$@" 2> $OUT/$tag.err; \
  echo "$tag: $(grep -E 'bench: utime|real|user|sys' $OUT/$tag.err | tr -s ' ' | tr '\n' ' ')" | tee -a $OUT/bench.txt; }
one sw1   -threads 2 -i $F -f null -
one sw1_rgba -threads 2 -i $F -vf format=rgba -f null -
one sw1_bgra -threads 2 -i $F -vf format=bgra -f null -
one vt1_vld -hwaccel videotoolbox -hwaccel_output_format videotoolbox_vld -i $F -f null -
one vt1_rgba -hwaccel videotoolbox -i $F -vf format=rgba -f null -
# 4 concurrent streams (the w2 scene's decode load)
for arm in "sw4 -threads 2 -i $F -vf format=rgba -f null -" "vt4 -hwaccel videotoolbox -hwaccel_output_format videotoolbox_vld -i $F -f null -"; do
  set -- $arm; tag=$1; shift
  t0=$(python3 -c 'import time; print(time.time())')
  for i in 1 2 3 4; do /usr/bin/time -l ffmpeg -hide_banner -loglevel error "$@" 2> $OUT/${tag}_$i.err & done; wait
  t1=$(python3 -c 'import time; print(time.time())')
  echo "$tag: wall $(python3 -c "print(round($t1 - $t0, 2))") s for 4 x 300 frames; per-proc: $(for i in 1 2 3 4; do grep -E 'real' $OUT/${tag}_$i.err | tr -s ' '; done | tr '\n' '|')" | tee -a $OUT/bench.txt
done
echo "$(date +%T) load end $(sysctl -n vm.loadavg)" | tee -a $OUT/bench.txt
release_lock
