S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-idle
R=$S/tools/run_arm.sh
echo "BATCH5 start $(date +%T)"
bash $R ctx card 3
bash $R ctx16 many16 3
echo "BATCH5 done $(date +%T)"
