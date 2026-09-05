#!/usr/bin/env bash
# gate-s165.sh — the app-level gates session s165 could not run.
#
# WHY THIS FILE EXISTS
# s165 shipped five lanes (L1-FU, SIGRACE, L3 steps 1-3). Every one got a build gate
# (exit code captured), ctest 213/213, and an independent source review. NONE of them
# got an app-level gate, for one reason: Audio-DNA enforces a single running instance,
# and a user-owned instance was up for the whole session. `open` on either bundle just
# ACTIVATES the running window (same bundle id) and `open -n` self-quits. Quitting a
# user's own app is not the agent's call (SCREEN-SAFETY LAW).
#
# So this is the deferred half, packaged. Run it when no Audio-DNA is running.
#
# WHAT IT DOES NOT DO
#   * It NEVER opens the output window. That window is a real fullscreen surface on the
#     user's actual monitors and driving it in a gate has burned this project before.
#   * It never pkills anything. Every quit is a graceful AppleScript quit.
#   * It refuses to run at all if an instance is already up, rather than fighting it.
#
# WHAT IT CANNOT DO (stated so nobody reads a green run as more than it is)
#   The L3 composition round trip needs media dropped into cells, and there is no REST
#   path that loads a video into a cell. That check is in the handoff's human list.
#
# Usage:  bash .harmony/gate-s165.sh
set -uo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REL="$REPO/build/AudioDNA_artefacts/Release/Audio-DNA.app"
TSAN="$REPO/build-tsan/AudioDNA_artefacts/Debug/Audio-DNA.app"
OUT="/tmp/gate-s165"
mkdir -p "$OUT"
PASS=0; FAIL=0
ok()   { echo "PASS  $*"; PASS=$((PASS+1)); }
bad()  { echo "FAIL  $*"; FAIL=$((FAIL+1)); }
note() { echo "      $*"; }

echo "=== gate-s165 — the app-level gates s165 deferred ==="

# ---------------------------------------------------------------- preflight
if pgrep -f 'MacOS/Audio-DNA' >/dev/null 2>&1; then
    echo "REFUSING TO RUN: an Audio-DNA instance is already running."
    pgrep -fl 'MacOS/Audio-DNA'
    echo
    echo "This app is single-instance. A second launch does not happen — 'open' silently"
    echo "activates the running window and reports success, which is exactly the false"
    echo "green this script exists to avoid. Quit that instance, then re-run."
    exit 64
fi
ok "preflight: no Audio-DNA running"

quit_gracefully() {   # $1 = pid
    # NOTE: the System Events per-process form ("first process whose unix id is N")
    # was tried first and DID NOT WORK against this app -- the process stayed alive
    # through a 20s wait. The app-level form does. Kept in this order so the targeted
    # form is attempted first when several instances could exist, with the working
    # form as the real quit.
    osascript -e "tell application \"System Events\" to tell (first process whose unix id is $1) to quit" >/dev/null 2>&1
    sleep 2
    kill -0 "$1" 2>/dev/null && osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
    for _ in $(seq 1 20); do
        kill -0 "$1" 2>/dev/null || return 0
        sleep 1
    done
    return 1
}

launch() {            # $1 = bundle, $2 = tag, rest = args ; echoes pid
    local bundle="$1" tag="$2"; shift 2
    rm -f "$OUT/$tag.out" "$OUT/$tag.err"
    # A PREVIOUS RUN OF THIS SCRIPT REPORTED "TSan app started (pid N)" WHERE N WAS THE
    # STILL-RUNNING RELEASE APP -- because the quit above had failed, `open -n` self-quit
    # (single instance), and `pgrep | head -1` cheerfully returned the stale pid. That is
    # precisely the false green this whole script exists to prevent, committed by the
    # script itself. So: refuse to launch while anything is alive, and verify the pid we
    # return is one that did NOT exist beforehand.
    if pgrep -f 'MacOS/Audio-DNA' >/dev/null 2>&1; then
        echo ""   # caller treats empty as "did not start"
        return 0
    fi
    open -n --stdout "$OUT/$tag.out" --stderr "$OUT/$tag.err" "$bundle" ${1+--args "$@"}
    sleep 12
    pgrep -f 'MacOS/Audio-DNA' | head -1
}

# ---------------------------------------------------------- 1. RELEASE / API
echo
echo "--- 1. Release build: launch, health, MilkDrop, graceful quit ---"
PID="$(launch "$REL" release --test-mode)"
if [ -z "$PID" ]; then
    bad "Release app did not start (see $OUT/release.err)"
else
    ok "Release app started (pid $PID)"
    HEALTH=""
    for _ in $(seq 1 15); do
        HEALTH="$(curl -s -m 3 http://localhost:8080/api/health 2>/dev/null)"
        [ -n "$HEALTH" ] && break
        sleep 2
    done
    # NOTE: probe 'localhost' or '[::1]', never 127.0.0.1 — TestServer binds IPv6 only,
    # and 8080 only exists with --test-mode (7070 binds regardless = a documented false green).
    [ -n "$HEALTH" ] && ok "8080 /api/health answered: $HEALTH" \
                     || bad "8080 never answered (needs --test-mode; check $OUT/release.err)"

    if grep -q 'MilkDrop.*Loaded.*presets' "$OUT/release.err" 2>/dev/null; then
        ok "MilkDrop autoload alive: $(grep -o '\[MilkDrop\] Loaded [0-9]* presets' "$OUT/release.err" | head -1)"
    else
        bad "no '[MilkDrop] Loaded N presets' line — the L0-MD regression may be back"
    fi

    # L3 reached the API surface at all?
    COMP="$(curl -s -m 3 http://localhost:8080/api/composition 2>/dev/null | head -c 200)"
    [ -n "$COMP" ] && note "composition endpoint: $COMP" || note "composition endpoint: (empty)"

    grep -qi 'crash\|assert.*fail\|Fatal' "$OUT/release.err" 2>/dev/null \
        && bad "crash markers in Release stderr" || ok "no crash markers in Release stderr"

    quit_gracefully "$PID" && ok "Release quit gracefully" || bad "Release did not quit within 20s"
fi

# ------------------------------------------------------------- 2. TSAN / RACE
echo
echo "--- 2. TSan build: the SIGRACE gate (the reviewer's falsifier) ---"
note "FAIL means: any WARNING/SUMMARY mentioning SignalRegistry reappears."
sleep 3
PID="$(launch "$TSAN" tsan)"
if [ -z "$PID" ]; then
    bad "TSan app did not start (see $OUT/tsan.err)"
else
    ok "TSan app started (pid $PID) — letting it reach steady state"
    sleep 60
    quit_gracefully "$PID" && ok "TSan app quit gracefully" || bad "TSan app did not quit within 20s"

    # grep -c prints "0" AND exits 1 when there are no matches, so `|| echo 0` appends a
    # SECOND line and every later [ "$X" -eq 0 ] dies with "integer expression expected".
    # Take the first line only.
    RACES="$(grep -c 'WARNING: ThreadSanitizer' "$OUT/tsan.err" 2>/dev/null | head -1)"; RACES="${RACES:-0}"
    SIGR="$(grep -c 'SignalRegistry' "$OUT/tsan.err" 2>/dev/null | head -1)"; SIGR="${SIGR:-0}"
    note "total ThreadSanitizer warnings: $RACES ; lines mentioning SignalRegistry: $SIGR"
    if [ "$SIGR" -eq 0 ]; then
        ok "SIGRACE HOLDS — no SignalRegistry race in this run"
    else
        bad "SignalRegistry still racing — the fix did not take. Offending lines:"
        grep -n -A4 'SignalRegistry' "$OUT/tsan.err" | head -30
    fi
    if [ "$RACES" -gt 0 ]; then
        note "other races present (may be pre-existing and unrelated) — distinct #0 frames:"
        grep -A2 'WARNING: ThreadSanitizer' "$OUT/tsan.err" | grep '#0' | sort -u | head -10
    fi
fi

# ----------------------------------------------------------- 3. SCREEN SAFETY
echo
echo "--- 3. Screen safety (law #3: pgrep empty does NOT mean the screen is clean) ---"
sleep 3
if pgrep -f 'MacOS/Audio-DNA' >/dev/null 2>&1; then
    bad "an Audio-DNA process survived this script"; pgrep -fl 'MacOS/Audio-DNA'
else
    ok "no Audio-DNA process remains"
fi
if [ -x "$REPO/.venv/bin/python" ]; then
    WIN="$("$REPO/.venv/bin/python" -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName',''))]))" 2>/dev/null)"
    [ "$WIN" = "0" ] && ok "CGWindowList (FULL list, not on-screen-only): 0 Audio-DNA windows" \
                     || bad "CGWindowList still shows $WIN Audio-DNA window(s)"
else
    note "no .venv python — skipping the window check (this is the check that catches a"
    note "black overlay or a stuck window, which no process probe can see)"
fi
screencapture -x "$OUT/screen-after.png" 2>/dev/null && note "screenshot: $OUT/screen-after.png — LOOK AT IT, do not just note that it exists"

echo
echo "=== TOTAL: $PASS pass, $FAIL fail ==="
echo "logs: $OUT"
[ "$FAIL" -eq 0 ] || exit 1
