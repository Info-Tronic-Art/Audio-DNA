# source me: acquire_lock / release_lock / wait_quiet / adna / quit_app
echo "ARCHIVED RECORD (R-N1, s-rta-1003): this script quits Audio-DNA by name -- never run or source it; use .harmony/probe-quit-ours.sh" >&2; return 64 2>/dev/null || exit 64
export AUDIODNA_LOCK_OWNER=renderperf
LOCKREL=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/renderperf/.last-release
acquire_lock(){
  local n=0
  if [ -f $LOCKREL ]; then
    local since=$(( $(date +%s) - $(cat $LOCKREL) ))
    [ $since -lt 45 ] && { echo "$(date +%T) re-acquire cooldown $((45-since)) s"; sleep $((45-since)); }
  fi
  while true; do
    if [ -f $LOCKREL ] && [ $(( $(date +%s) - $(cat $LOCKREL) )) -lt 45 ]; then sleep 5; continue; fi
    mkdir /tmp/audiodna-live.lock 2>/dev/null && break
    n=$((n+1)); [ $n -gt 360 ] && { echo "LOCK TIMEOUT"; return 1; }
    [ $((n % 6)) -eq 1 ] && echo "$(date +%T) lock held by: $(cat /tmp/audiodna-live.lock/owner 2>/dev/null) -- waiting"
    sleep 20
  done
  echo "renderperf $$ $(date +%s)" > /tmp/audiodna-live.lock/owner
  echo "$(date +%T) lock acquired"
}
release_lock(){
  if [ "$(cut -d' ' -f1 /tmp/audiodna-live.lock/owner 2>/dev/null)" = "renderperf" ]; then
    rm -rf /tmp/audiodna-live.lock && date +%s > $LOCKREL && echo "$(date +%T) lock released"
  fi
}
wait_quiet(){
  local n=0
  while [ -n "$(pgrep -x clang; pgrep -x 'clang\+\+')" ]; do
    n=$((n+1)); [ $n -gt 90 ] && { echo "$(date +%T) compilers still running after 30 min -- NOT quiet"; return 1; }
    [ $((n % 6)) -eq 1 ] && echo "$(date +%T) compiler running ($(pgrep -x clang | wc -l | tr -d ' ') clang, $(pgrep -x 'clang\+\+' | wc -l | tr -d ' ') clang++) -- waiting"
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
