S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-idle
R=$S/tools/run_arm.sh
STOCK=/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app
echo "BATCH3 start $(date +%T)"
PS_SAMPLE=1 bash $R base3 card 5
bash $R nohb card 5 ADNA_DIAG_NOHB=1
PS_SAMPLE=1 DIAGFILE_OFF=1 APP=$STOCK bash $R stock card 5
bash $R floor card 5 ADNA_DIAG_METAL=1 ADNA_DIAG_NO_ANIM=1
bash $R deflt default 5
bash $R many16 many16 5
bash $R many16metal many16 5 ADNA_DIAG_METAL=1
MODE=test bash $R testmode card 5
bash $R grid card 3 ADNA_DIAG_DIRTYGRID=1
echo "BATCH3 done $(date +%T)"
