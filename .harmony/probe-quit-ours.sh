# probe-quit-ours.sh -- QUIT ONLY WHAT YOU LAUNCHED (rig safety; bf9 Stage P, s-rta-1002b, ruling-bf9 amendment 12(e)).
# Sourced (never run) by .harmony/probe-render-state.sh and .harmony/probe-boxes.sh, AFTER they define adna_pids
# (the kernel-ucomm "Audio-DNA" pid list). Boris uses this machine and this app: an Audio-DNA the probe did not launch
# is never quit by name and never killed (the s-rta-1002b incident: a gate script quit his running app).
#   record_ourpid  call right after /api/health answers: OURPID = the one ucomm "Audio-DNA" pid then running. The
#                  probe's pre-launch REFUSE (no Audio-DNA running) guarantees it is ours. Two or more pids: OURPID
#                  stays empty and every one of them is treated as foreign.
#   ours_running   OURPID is set, alive, and its ucomm is still "Audio-DNA" (a recycled pid is never ours).
#   quit_ours      only OURPID running -> osascript quit by name, wait up to 30 s, then kill OURPID only.
#                  any OTHER Audio-DNA pid running -> never osascript by name; kill -TERM OURPID (-KILL after 10 s);
#                  print "FOREIGN Audio-DNA pid <p> running -- untouched"; return non-zero.
#                  Returns 0 iff no foreign pid ran and OURPID is gone.
OURPID=""

record_ourpid() {
  local p n
  p="$(adna_pids)"
  n="$(printf '%s\n' "$p" | grep -c .)"
  if [ "$n" -eq 1 ]; then
    OURPID="$(printf '%s' "$p" | tr -d ' \n')"
  else
    OURPID=""
    echo "WARN  $n Audio-DNA pids after launch ($(printf '%s' "$p" | tr '\n' ' ')) -- none is treated as ours"
  fi
}

ours_running() {
  [ -n "$OURPID" ] && kill -0 "$OURPID" 2>/dev/null \
    && [ "$(ps -o ucomm= -p "$OURPID" 2>/dev/null | tr -d ' ')" = "Audio-DNA" ]
}

quit_ours() {
  local others p
  others="$(adna_pids | grep -vx "${OURPID:-none}")"
  if [ -z "$others" ]; then
    if ours_running; then
      osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
      for _ in $(seq 1 30); do ours_running || break; sleep 1; done
      ours_running && { kill "$OURPID" 2>/dev/null; sleep 2; }
    fi
    ours_running && return 1
    return 0
  fi
  for p in $others; do echo "FOREIGN Audio-DNA pid $p running -- untouched"; done
  if ours_running; then
    kill -TERM "$OURPID" 2>/dev/null
    for _ in $(seq 1 10); do ours_running || break; sleep 1; done
    ours_running && kill -KILL "$OURPID" 2>/dev/null
  fi
  return 1
}
