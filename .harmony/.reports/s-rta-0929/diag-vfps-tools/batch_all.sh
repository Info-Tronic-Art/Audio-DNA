# batch_all.sh -- after the smoke: the Q1 arms on the w1col scene (ONE trigger_column: every load is bunch pattern 4),
# then the Q2 arms (w2). Serial (one lock owner at a time); run_arm.sh takes / releases the lock per <= 11 min.
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/diag-vfps
until grep -q "smoke2 done" /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/tasks/b28y5kwz4.output 2>/dev/null; do sleep 10; done
q1(){ local tag=$1 n=$2; shift 2; ARM_ENV="$*" bash $S/tools/run_arm.sh $tag w1col $n 3 > $S/runs/$tag.log 2>&1; echo "$(date +%T) $(tail -1 $S/runs/$tag.log | head -1) $tag"; }
q2(){ local tag=$1; shift; ARM_ENV="$*" bash $S/tools/run_arm.sh $tag w2 5 2 > $S/runs/$tag.log 2>&1; echo "$(date +%T) $tag done"; }
q1 colbase 10
q1 colcap1 10 ADNA_VFPS_UPCAP=1
q1 coldstag 10 ADNA_VFPS_DECSTAGGER=8
q1 colqos1 10 ADNA_VFPS_QOS=1
q2 base_w2
q2 noup_w2 ADNA_VFPS_NOUPLOAD=1
q2 iosurf_w2 ADNA_VFPS_IOSURF=1
q2 cap1_w2 ADNA_VFPS_UPCAP=1
q2 bgra1_w2 ADNA_VFPS_BGRA=1
q2 client_w2 ADNA_VFPS_CLIENT=1
q2 pbo_w2 ADNA_VFPS_PBO=1
q2 bgra2_w2 ADNA_VFPS_BGRA=2
q2 qos_w2 ADNA_VFPS_QOS=1
q1 colqos4 5 ADNA_VFPS_QOS=4
q1 colqos3 5 ADNA_VFPS_QOS=3
q1 colnoupdstag 5 ADNA_VFPS_NOUPLOAD=1 ADNA_VFPS_DECSTAGGER=8
echo "batch_all done $(date +%T)"
