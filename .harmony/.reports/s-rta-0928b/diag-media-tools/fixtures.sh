# fixtures.sh -- the diag-media media fixtures (ffmpeg testsrc2), nice'd, 2 threads.
M=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-media/media
mkdir -p $M/seq_png $M/seq_jpg $M/img4k $M/vid
F="nice -n 10 ffmpeg -hide_banner -loglevel error -y -threads 2"
date +%T
[ -f $M/vid/v1080_g250.mp4 ] || $F -f lavfi -i testsrc2=size=1920x1080:rate=30:duration=10 -c:v libx264 -pix_fmt yuv420p -preset medium -crf 20 $M/vid/v1080_g250.mp4
[ -f $M/vid/v1080_g30.mp4 ] || $F -f lavfi -i testsrc2=size=1920x1080:rate=30:duration=10 -c:v libx264 -pix_fmt yuv420p -preset medium -crf 20 -g 30 -keyint_min 30 $M/vid/v1080_g30.mp4
[ -f $M/vid/v4k_g250.mp4 ] || $F -f lavfi -i testsrc2=size=3840x2160:rate=30:duration=10 -c:v libx264 -pix_fmt yuv420p -preset medium -crf 20 $M/vid/v4k_g250.mp4
[ -f $M/vid/prores1080.mov ] || $F -f lavfi -i testsrc2=size=1920x1080:rate=30:duration=10 -c:v prores_ks -profile:v 2 -pix_fmt yuv422p10le $M/vid/prores1080.mov
date +%T
[ -f $M/seq_png/frame_0300.png ] || $F -f lavfi -i testsrc2=size=1920x1080:rate=30:duration=10 -vf noise=alls=12:allf=t -frames:v 300 -compression_level 3 $M/seq_png/frame_%04d.png
[ -f $M/seq_jpg/frame_0300.jpg ] || $F -f lavfi -i testsrc2=size=1920x1080:rate=30:duration=10 -vf noise=alls=12:allf=t -frames:v 300 -q:v 3 $M/seq_jpg/frame_%04d.jpg
date +%T
for i in $(seq -w 1 16); do
  [ -f $M/img4k/img_$i.jpg ] || $F -f lavfi -i "testsrc2=size=3840x2160:rate=1:duration=40" -vf "select=eq(n\,$((10#$i+1))),noise=alls=12:allf=t" -frames:v 1 -q:v 3 $M/img4k/img_$i.jpg
done
# 16 video cells: APFS clones of the encoded files (distinct paths, no extra space)
for i in $(seq -w 1 16); do
  [ -f $M/vid/c1080_$i.mp4 ] || cp -c $M/vid/v1080_g250.mp4 $M/vid/c1080_$i.mp4
  [ -f $M/vid/c4k_$i.mp4 ] || cp -c $M/vid/v4k_g250.mp4 $M/vid/c4k_$i.mp4
done
date +%T
ls -la $M/vid | head -8; du -sh $M/*; ffprobe -v error -select_streams v -show_entries stream=codec_name,width,height,r_frame_rate,pix_fmt -of csv=p=0 $M/vid/v4k_g250.mp4 $M/vid/prores1080.mov
for f in v1080_g250 v1080_g30 v4k_g250; do echo "$f keyframes: $(ffprobe -v error -select_streams v -skip_frame nokey -show_entries frame=pts_time -of csv=p=0 $M/vid/$f.mp4 | tr '\n' ' ')"; done
