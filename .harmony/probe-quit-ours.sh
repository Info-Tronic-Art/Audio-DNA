# probe-quit-ours.sh -- QUIT ONLY WHAT YOU LAUNCHED (rig safety; bf9 Stage P, s-rta-1002b, ruling-bf9 amendment 12(e);
# every .harmony probe since the bf9b merge-in, s-rta-1003, Harmony ruling R-N1).
# Sourced (never run) by every .harmony probe that launches Audio-DNA, AFTER it defines adna_pids (the kernel-ucomm
# "Audio-DNA" pid list). Boris uses this machine and this app: an Audio-DNA the probe did not launch is never quit by
# name and never killed (the s-rta-1002b incident: a gate script quit his running app).
#   refuse_foreign_start  call before the launch: any Audio-DNA running now was not started by this run -> prints
#                  "REFUSE: Audio-DNA already running (pid <p>) ..." and returns 1 (the probe exits non-zero; it never
#                  quits "the stale one" first).
#   record_ourpid  call right after the launch, BEFORE the health wait (a slow app that never answers /api/health is
#                  still ours and still quit): waits up to 10 s for the process, then OURPID = the one ucomm
#                  "Audio-DNA" pid running. The pre-launch refusal guarantees it is ours. Two or more pids: OURPID
#                  stays empty and every one of them is treated as foreign.
#   ours_running   OURPID is set, alive, and its ucomm is still "Audio-DNA" (a recycled pid is never ours).
#   quit_ours      no pid recorded -> says so; quits nothing.
#                  only OURPID running -> osascript quit by name, wait up to 30 s, then kill OURPID only.
#                  any OTHER Audio-DNA pid running -> never osascript by name; kill -TERM OURPID (-KILL after 10 s);
#                  print "FOREIGN Audio-DNA pid <p> running -- untouched"; return non-zero.
#                  Returns 0 iff no foreign pid ran and OURPID is gone.
#   ask_ours_to_quit / kill_ours  the two halves of quit_ours for a row that times the quit itself (probe-async-load,
#                  probe-btguard, probe-tsan, gate-s165, ...): the by-name Apple event is sent only when OURPID is the
#                  ONLY Audio-DNA running (else the FOREIGN line, return 1, nothing sent); kill_ours signals OURPID
#                  only (default TERM; kill_ours -9).
# Selftest (no Audio-DNA involved: dummy processes through the seam below): .harmony/probe-quit-ours-selftest.sh.
# SEAM (selftest only): the caller's adna_pids decides which pids count; QUIT_OURS_UCOMM is the ucomm a pid must
# still carry to be ours; QUIT_OURS_RECORD_TRIES bounds record_ourpid's wait (0.5 s steps).
OURPID=""
QUIT_OURS_UCOMM="${QUIT_OURS_UCOMM:-Audio-DNA}"

refuse_foreign_start() {
  local p
  p="$(adna_pids | tr '\n' ' ' | sed 's/ *$//')"
  [ -z "$p" ] && return 0
  echo "REFUSE: Audio-DNA already running (pid $p) -- this run did not start it (it may be Boris's): never quit, kill or touch it"
  return 1
}

record_ourpid() {
  local p n _
  for _ in $(seq 1 "${QUIT_OURS_RECORD_TRIES:-20}"); do p="$(adna_pids)"; [ -n "$p" ] && break; sleep 0.5; done
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
    && [ "$(ps -o ucomm= -p "$OURPID" 2>/dev/null | tr -d ' ')" = "$QUIT_OURS_UCOMM" ]
}

quit_ours() {
  local others p
  [ -z "$OURPID" ] && echo "quit_ours: no pid recorded by this run -- nothing is quit"
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

ask_ours_to_quit() {
  local others p
  others="$(adna_pids | grep -vx "${OURPID:-none}")"
  if [ -n "$others" ]; then
    for p in $others; do echo "FOREIGN Audio-DNA pid $p running -- untouched (no quit event sent)"; done
    return 1
  fi
  ours_running || return 0
  osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
}

kill_ours() {   # kill_ours [-SIGNAL]
  ours_running && kill ${1:+"$1"} "$OURPID" 2>/dev/null
}
