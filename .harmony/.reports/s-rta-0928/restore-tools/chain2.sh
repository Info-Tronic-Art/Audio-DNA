R=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/restore
bash $R/run1.sh cardB ease,jump,norestore,loop,loopjump 5
FIXVAR=source bash $R/run1.sh sourceA ease,jump,norestore,loop 5
FIXVAR=big bash $R/run1.sh bigA ease,jump,norestore,loop 5
echo CHAIN2 DONE $(date +%T)
