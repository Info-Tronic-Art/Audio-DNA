# source me: acquire_lock / release_lock / wait_quiet
acquire_lock(){
  local n=0
  while ! mkdir /tmp/audiodna-live.lock 2>/dev/null; do
    n=$((n+1)); [ $n -gt 360 ] && { echo "LOCK TIMEOUT"; return 1; }
    [ $((n % 6)) -eq 1 ] && echo "$(date +%T) lock held by: $(cat /tmp/audiodna-live.lock/owner 2>/dev/null) -- waiting"
    sleep 20
  done
  echo "routines-timing $$ $(date +%s)" > /tmp/audiodna-live.lock/owner
  echo "$(date +%T) lock acquired"
}
release_lock(){
  [ "$(cut -d' ' -f1 /tmp/audiodna-live.lock/owner 2>/dev/null)" = "routines-timing" ] && rm -rf /tmp/audiodna-live.lock && echo "$(date +%T) lock released"
}
wait_quiet(){
  local n=0
  while [ -n "$(pgrep -x clang; pgrep -x 'clang\+\+')" ]; do
    n=$((n+1)); [ $n -gt 90 ] && { echo "$(date +%T) compilers still running after 30 min -- proceeding, flagged"; return 1; }
    [ $((n % 6)) -eq 1 ] && echo "$(date +%T) compiler running ($(pgrep -x clang | wc -l | tr -d ' ') clang, $(pgrep -x 'clang\+\+' | wc -l | tr -d ' ') clang++) -- waiting"
    sleep 20
  done
  return 0
}
