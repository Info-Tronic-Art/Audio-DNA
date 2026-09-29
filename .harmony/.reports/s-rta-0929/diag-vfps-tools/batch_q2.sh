# batch_q2.sh -- the Q2 arms (w2: a 4 x 4K STILLS window + 2 loads of 4 x 4K video per launch, 5 launches each).
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/diag-vfps
N=${N:-5}
run(){ local tag=$1; shift; ARM_ENV="$*" bash $S/tools/run_arm.sh $tag w2 $N 2 > $S/runs/$tag.log 2>&1; tail -1 $S/runs/$tag.log; }
run base_w2
run noup_w2 ADNA_VFPS_NOUPLOAD=1
run bgra1_w2 ADNA_VFPS_BGRA=1
run bgra2_w2 ADNA_VFPS_BGRA=2
run client_w2 ADNA_VFPS_CLIENT=1
run iosurf_w2 ADNA_VFPS_IOSURF=1
run pbo_w2 ADNA_VFPS_PBO=1
run qos_w2 ADNA_VFPS_QOS=1
run cap1_w2 ADNA_VFPS_UPCAP=1
echo "batch_q2 done $(date +%T)"
