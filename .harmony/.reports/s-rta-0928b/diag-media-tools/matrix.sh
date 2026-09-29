# matrix.sh FIRST LAST -- the measurement matrix, one batch (one lock hold) per repetition, arms interleaved.
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-media
for n in $(seq $1 $2); do
  echo "=== rep $n $(date +%T)"
  bash $SP/tools/batch2.sh "v0_$n:video:;v1_$n:video:DIAG_VID_NODECODE,DIAG_NO_THUMB;v2_$n:video:DIAG_VID_UPLOAD_ONLY_NEW;k0_$n:seek:;i0_$n:images:;i1_$n:images:DIAG_NO_EXISTS,DIAG_NO_PAINTSTAT;q0_$n:seq:;q1_$n:seq:DIAG_NO_THUMB"
done
echo "=== matrix done $(date +%T)"
