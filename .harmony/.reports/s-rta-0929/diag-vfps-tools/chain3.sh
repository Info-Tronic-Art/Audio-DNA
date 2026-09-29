# chain3.sh -- the true zero-copy arm (IOSURF=2) interleaved with base / iosurf blit / noup (w2, 5 rounds, a capture per
# launch), then the decode pricing bench (vtbench.sh), then the unmodified MAIN app on the proposed Q1 row scene (w1col,
# 5 launches, no diag env) and on w1 (5 launches).
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/diag-vfps
DRIVE_CAPTURE=1 bash $S/tools/run_mix.sh q2b w2 2 5 "base=" "iosurf2=ADNA_VFPS_IOSURF=2" "iosurf1=ADNA_VFPS_IOSURF=1" "noup=ADNA_VFPS_NOUPLOAD=1" > $S/runs/q2bmix.log 2>&1
echo "q2bmix finished $(date +%T)" >> $S/runs/q2bmix.log
bash $S/tools/vtbench.sh > $S/runs/vtbench.log 2>&1
echo "vtbench finished $(date +%T)" >> $S/runs/vtbench.log
MAINAPP=/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app
APP=$MAINAPP DIAG_OFF=1 bash $S/tools/run_mix.sh main w1col 3 5 "col=" > $S/runs/mainmix.log 2>&1
APP=$MAINAPP DIAG_OFF=1 bash $S/tools/run_mix.sh main w1 3 5 "w1=" >> $S/runs/mainmix.log 2>&1
echo "mainmix finished $(date +%T)" >> $S/runs/mainmix.log
