SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/renderperf
bash $SP/fix/batch.sh fix1 $SP/fix/steps-fix1.sh > $SP/fix/batch-fix1.out 2>&1
bash $SP/fix/batch.sh fix2 $SP/fix/steps-fix2.sh > $SP/fix/batch-fix2.out 2>&1
bash $SP/fix/batch.sh tier1 $SP/fix/steps-tier1.sh > $SP/fix/batch-tier1.out 2>&1
echo all done $(date +%T)
