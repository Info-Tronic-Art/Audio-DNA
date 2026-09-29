# batch_smoke2.sh -- one w2v launch per upload arm with a canvas capture (correctness look before the Q2 batch)
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/diag-vfps
export DRIVE_CAPTURE=1
for arm in "base" "iosurf ADNA_VFPS_IOSURF=1" "client ADNA_VFPS_CLIENT=1" "bgra1 ADNA_VFPS_BGRA=1" "pbo ADNA_VFPS_PBO=1"; do
  set -- $arm; tag=smoke2_$1; shift
  ARM_ENV="$*" bash $S/tools/run_arm.sh $tag w2v 1 1 > $S/runs/$tag.log 2>&1
  echo "$tag: $(grep -h 'WINDOW\|CAPTURE' $S/runs/$tag/r1/drive.out | tr '\n' ' ')"; grep -h "DIAG-VFPS" $S/runs/$tag/r1/app-err.log | head -3
done
echo "smoke2 done $(date +%T)"
