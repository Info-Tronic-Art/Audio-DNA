# chain2.sh -- after q1mix: the Q2 interleaved arms (w2, 5 rounds), then the Q1 second set (w1col, 10 rounds).
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/diag-vfps
until grep -q "q1mix finished" $S/runs/q1mix.log 2>/dev/null; do sleep 10; done
bash $S/tools/run_mix.sh q2 w2 2 5 "base=" "noup=ADNA_VFPS_NOUPLOAD=1" "iosurf=ADNA_VFPS_IOSURF=1" "cap1=ADNA_VFPS_UPCAP=1" \
  "bgra1=ADNA_VFPS_BGRA=1" "bgra2=ADNA_VFPS_BGRA=2" "pbo=ADNA_VFPS_PBO=1" "client=ADNA_VFPS_CLIENT=1" "qos1=ADNA_VFPS_QOS=1" > $S/runs/q2mix.log 2>&1
echo "q2mix finished $(date +%T)" >> $S/runs/q2mix.log
bash $S/tools/run_mix.sh q1b w1col 3 10 "base=" "noupqos4=ADNA_VFPS_NOUPLOAD=1,ADNA_VFPS_QOS=4" "qos4=ADNA_VFPS_QOS=4" \
  "dstagqos1=ADNA_VFPS_DECSTAGGER=8,ADNA_VFPS_QOS=1" "cap2=ADNA_VFPS_UPCAP=2" > $S/runs/q1bmix.log 2>&1
echo "q1bmix finished $(date +%T)" >> $S/runs/q1bmix.log
