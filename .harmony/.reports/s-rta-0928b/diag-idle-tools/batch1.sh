S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-idle
R=$S/tools/run_arm.sh
echo "BATCH1 start $(date +%T)"
bash $R base card 5
bash $R metal card 5 ADNA_DIAG_METAL=1
bash $R notop card 5 ADNA_DIAG_NO_TOPBAR_REPAINT=1
bash $R nols card 5 ADNA_DIAG_NO_LSREPAINT=1
bash $R nowave card 5 ADNA_DIAG_NO_WAVE_REPAINT=1
bash $R nosig card 5 ADNA_DIAG_NO_SIGBAR_REPAINT=1
bash $R noanim card 5 ADNA_DIAG_NO_ANIM=1
bash $R activity card 5 ADNA_DIAG_ACTIVITY=1
echo "BATCH1 done $(date +%T)"
