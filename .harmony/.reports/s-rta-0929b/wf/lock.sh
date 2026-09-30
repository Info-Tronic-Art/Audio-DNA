# source me with LANE=<name> set: acquire_lock / release_lock / wait_quiet / adna / quit_app / outwins / start_app
: "${LANE:?set LANE=<lane-name> before sourcing}"
# NOTE: bash drops a VAR=x prefix on "." after sourcing, so pin the name in a real assignment here.
LOCK_LANE="$LANE"; export LOCK_LANE; export AUDIODNA_LOCK_OWNER="$LOCK_LANE"
SPL=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/bd9a1c96-8ac1-42b0-a088-3d8fb87be738/scratchpad/lib
LOCKREL=$SPL/.last-release-$LOCK_LANE
acquire_lock(){
  local n=0
  if [ -f $LOCKREL ]; then
    local since=$(( $(date +%s) - $(cat $LOCKREL) ))
    [ $since -lt 45 ] && { echo "$(date +%T) re-acquire cooldown $((45-since)) s"; sleep $((45-since)); }
  fi
  while true; do
    mkdir /tmp/audiodna-live.lock 2>/dev/null && break
    n=$((n+1)); [ $n -gt 360 ] && { echo "$(date +%T) LOCK TIMEOUT (120 min)"; return 1; }
    [ $((n % 6)) -eq 1 ] && echo "$(date +%T) lock held by: $(cat /tmp/audiodna-live.lock/owner 2>/dev/null) -- waiting"
    sleep 20
  done
  echo "$LOCK_LANE $$ $(date +%s)" > /tmp/audiodna-live.lock/owner
  echo "$(date +%T) lock acquired"
}
release_lock(){
  if [ "$(cut -d' ' -f1 /tmp/audiodna-live.lock/owner 2>/dev/null)" = "$LOCK_LANE" ] && [ -n "$LOCK_LANE" ]; then
    rm -rf /tmp/audiodna-live.lock && date +%s > $LOCKREL && echo "$(date +%T) lock released"
  fi
}
wait_quiet(){
  local n=0
  while [ -n "$(pgrep -x clang; pgrep -x 'clang\+\+')" ]; do
    n=$((n+1)); [ $n -gt 90 ] && { echo "$(date +%T) compilers still running after 30 min -- NOT quiet"; return 1; }
    [ $((n % 6)) -eq 1 ] && echo "$(date +%T) compiler running -- waiting"
    sleep 20
  done
  return 0
}
adna(){ ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
quit_app(){
  osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
  for _ in $(seq 1 30); do [ -z "$(adna)" ] && break; sleep 1; done
  [ -n "$(adna)" ] && { echo "pkill last resort"; kill $(adna); sleep 3; }
  echo "app running after quit: $([ -n "$(adna)" ] && echo YES || echo no)"
}
outwins(){
  /Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
a=[w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName', ''))]
print('audio-dna windows %d, Output-named %d' % (len(a), len([w for w in a if 'Output' in str(w.get('kCGWindowName', ''))])))"
}
start_app(){  # start_app APP OUTDIR [test]  -- refuses if any Audio-DNA or listener exists
  local APP=$1 OUT=$2 MODE=$3
  [ -n "$(adna)" ] && { echo "REFUSE: Audio-DNA already running"; return 1; }
  for p in 7070 8080; do lsof -nP -iTCP:$p -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: $p has a listener"; return 1; }; done
  mkdir -p $OUT
  if [ "$MODE" = "test" ]; then
    open -g --stdout $OUT/app-out.log --stderr $OUT/app-err.log "$APP" --args --test-mode
    for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 http://localhost:8080/api/health)" ] && break; sleep 1; done
  else
    open -g --stdout $OUT/app-out.log --stderr $OUT/app-err.log "$APP"
    for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 http://127.0.0.1:7070/api/health)" ] && break; sleep 1; done
  fi
  sleep 2
  echo "$(date +%T) app up: $(adna | tr '\n' ' ')"
}
# acquire_quiet_lock: wait for no compiler BEFORE taking the lock (never hold the lock while waiting for quiet);
# after acquiring, re-check once: a compiler that started meanwhile -> release, cool down, retry. Tries up to 12 times.
acquire_quiet_lock(){
  local tries=0
  while [ $tries -lt 12 ]; do
    tries=$((tries+1))
    wait_quiet || { echo "$(date +%T) not quiet after 30 min (try $tries)"; continue; }
    acquire_lock || return 1
    if [ -z "$(pgrep -x clang; pgrep -x 'clang\+\+')" ]; then return 0; fi
    echo "$(date +%T) compiler started while acquiring -- releasing"; release_lock
  done
  return 1
}
