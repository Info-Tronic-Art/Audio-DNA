# sourced: acquire_lock / release_lock for the shared live app (rig rules)
LOCKLOG=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/beatclock/lock.log
acquire_lock() {
  local start=$(date +%s)
  while ! mkdir /tmp/audiodna-live.lock 2>/dev/null; do
    if [ $(( $(date +%s) - start )) -gt 7200 ]; then echo "LOCK TIMEOUT after 120 min ($(cat /tmp/audiodna-live.lock/owner 2>/dev/null))" | tee -a $LOCKLOG; return 1; fi
    sleep 20
  done
  echo "beatclock $$ $(date +%s)" > /tmp/audiodna-live.lock/owner
  echo "$(date '+%F %T') ACQUIRED (waited $(( $(date +%s) - start )) s)" | tee -a $LOCKLOG
  export AUDIODNA_LOCK_OWNER=beatclock
}
release_lock() {
  if [ "$(cut -d' ' -f1 /tmp/audiodna-live.lock/owner 2>/dev/null)" = "beatclock" ]; then
    rm -rf /tmp/audiodna-live.lock
    echo "$(date '+%F %T') RELEASED" | tee -a $LOCKLOG
  fi
}
