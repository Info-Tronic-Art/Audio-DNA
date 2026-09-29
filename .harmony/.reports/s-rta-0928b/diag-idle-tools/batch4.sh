S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-idle
R=$S/tools/run_arm.sh
echo "BATCH4 start $(date +%T)"
bash $R grid16 many16 3 ADNA_DIAG_DIRTYGRID=1
bash $R many16stat many16 5
bash $R many16noanim many16 5 ADNA_DIAG_NO_ANIM=1
echo "BATCH4 done $(date +%T)"
