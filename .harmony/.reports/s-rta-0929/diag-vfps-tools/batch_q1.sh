# batch_q1.sh -- the Q1 arms (w1: 10 launches x 3 loads each), one after another (run_arm.sh handles the lock).
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/diag-vfps
ARM_ENV="ADNA_VFPS_UPCAP=1" bash $S/tools/run_arm.sh cap1_w1 w1 10 3 > $S/runs/cap1_w1.log 2>&1
ARM_ENV="ADNA_VFPS_NOUPLOAD=1" bash $S/tools/run_arm.sh noup_w1 w1 10 3 > $S/runs/noup_w1.log 2>&1
bash $S/tools/run_arm.sh col_w1 w1col 10 3 > $S/runs/col_w1.log 2>&1
ARM_ENV="ADNA_VFPS_UPCAP=1" bash $S/tools/run_arm.sh colcap1_w1 w1col 10 3 > $S/runs/colcap1_w1.log 2>&1
ARM_ENV="ADNA_VFPS_QOS=1" bash $S/tools/run_arm.sh qos_w1 w1 10 3 > $S/runs/qos_w1.log 2>&1
echo "batch_q1 done $(date +%T)"
