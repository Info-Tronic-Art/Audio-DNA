# fixtures.sh -- encode the w1/w2 fixtures once into $S/media (probe-video.py --make-fixtures with VIDEO_FIXTURES)
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/diag-vfps
W=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-vfps
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
echo "$(date +%T) fixtures start load $(sysctl -n vm.loadavg)"
VIDEO_FIXTURES=$S/media $PY $W/.harmony/probe-video.py $W $S/fixout --make-fixtures w1_steady_1080x4,w2_steady_4kx4,w8_prores_steady
echo "rc $? $(date +%T)"
# 1080p still (frame 100 of a1080) for a 1080p stills control
[ -f $S/media/still1080_f100.png ] || nice -n 10 ffmpeg -y -loglevel error -threads 2 -i $S/media/a1080_g250.mp4 -vf "select='eq(n\,100)'" -frames:v 1 $S/media/still1080_f100.png
ls -la $S/media
