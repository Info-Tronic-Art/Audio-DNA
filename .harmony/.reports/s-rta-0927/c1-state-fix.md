## BUILDER REPORT -- lane c1-state-fix (s-rta-0927)

STATUS: DONE
RESULT: The d_return_hitch `/api/state` failure is a race in the probe's HTTP client, not a C1 regression. The probe's s0 GET goes out 5.0 s after its previous request, on a reused (keep-alive) connection. The app's cpp-httplib server closes connections idle for 5 s, and drains and discards a request that arrives in that instant. This failed about 1 run in 3 on the pre-C1 app and on the C1 app alike. The fix is in the probe: every request now uses its own connection (`Connection: close`). Fix commit 8654b74 on branch fix/c1-state-0927.
FACTS: server-side proof: `.harmony/.reports/s-rta-0927/c1-state-fix-evidence/04-server-instrumented-kai.txt` (the ERR attempt: keepalive-timeout 0.66 ms after the client sent, then `drain_and_close ... drained=160`, where 160 bytes = the GET /api/state request); both apps fail the same way: `.harmony/.reports/s-rta-0927/c1-state-fix-evidence/01-ab-main-vs-pre.txt` (main 1/3, pre-C1 1/3, both on request #106 after 5.002-5.005 s idle); traces: `.harmony/.reports/s-rta-0927/c1-state-fix-evidence/02-hitch-traces-main1-pre1.txt`; sweep of the race window on both apps: `.harmony/.reports/s-rta-0927/c1-state-fix-evidence/03-sweep-main-vs-pre.txt`; fix teeth: `.harmony/.reports/s-rta-0927/c1-state-fix-evidence/05-fix-teeth-sweep.txt` (2/40 ERR with a reused connection vs 0/40 with `Connection: close`, same build, same session); GREEN: `.harmony/.reports/s-rta-0927/c1-state-fix-evidence/06-green-battery-lane.txt`; ctest: `.harmony/.reports/s-rta-0927/c1-state-fix-evidence/07-ctest.txt`; the fix: `.harmony/probe-deck-clock.py:61` (`S.headers["Connection"] = "close"`, comment above it).
METHOD: Diagnosed first, fixed afterwards. (1) I ran an instrumented COPY of the original probe (python timestamps and urllib3 connection events only) 3 full runs on each app, alternating. (2) A sweep repeated the s0 pattern (POST switch_deck, idle 5.0 s + x, GET /api/state on the same connection) on both apps. (3) I temporarily added "[KAI]" fprintf lines to the fetched `httplib.h` in build-lane/_deps (keep-alive timeout, readable, served, drain_and_close with drained byte count, conn) and joined the server events to the client sends by wall clock. Then I reverted the header (sha256 identical to the pre-patch copy, `git status` of the dep clean), rebuilt, and checked `strings` of the app binary: 0 "[KAI]" matches. (4) Fix, then a teeth sweep and the GREEN battery on the clean lane build.
CONFIDENCE+VERIFY: High on the mechanism: a server log shows it. High that C1 is not involved: the pre-C1 app fails the same way in the probe and in the sweep. Harmony can re-run it: `DCLOCK_APP=<WT>/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app bash <WT>/.harmony/probe-deck-clock.sh <out>` → expect `PY 10 PASS / 0 FAIL`, `PROBE-DECK-CLOCK GREEN`. The fixed probe also runs against the main app, whose code is unchanged. The race teeth: `XS=-1,0,1,2,3 REPS=8 bash <scratch>/c1fix/sweep-ab.sh keepalive-x=<app> close-x=<app>` → ERR > 0 is possible only in the keepalive run; the close run is always 0.
UNKNOWNS/NOT-DONE: (a) One sweep put the C1 app's close edge about 2 ms earlier than the pre-C1 app's (RESET from x=0 on main vs x=+2 on pre). The same code (the lane build) moves by the same amount from run to run, so C1 changing the odds is NOT shown and NOT ruled out. The mechanism is the same on both apps either way. (b) I did not audit the other 4 probes that use a pooled `requests.Session` (probe-canvas/render-state/fitmode/outputs) for idle gaps near 5 s. None has a literal 4.8-5.0 s sleep, and config-driven gaps were not checked. (c) Row-level RED on the main app with the ORIGINAL probe: Harmony's 2 + my main1 (3 of 5 runs). It is a race, so no deterministic per-run RED exists. The deterministic teeth are the sweep plus the source check (see NUANCE).
NUANCE: Why the pre-C1 app passed 10/0 for Harmony: that was luck. The race fires in about 1 of 3 full runs on EITHER app (my runs: pre1 RED, pre2/pre3 GREEN; main1 RED, main2/main3 GREEN). Harmony had 1 pre-C1 run (GREEN) and 2 main runs (both RED); 3/5 vs 1/4 is not a significant difference. Harmony's keep-alive "refutation" (ka.py) was underpowered, not wrong: at the 5.0 s edge a single reused-connection attempt fails about 2-6% of the time (main 2/33, pre 1/33, KAI build 1/30, lane 2/40). Three 5.0 s attempts that all come back OK is therefore the expected outcome. The window is narrow because two cases are harmless. If the server's FIN arrives before urllib3's `is_connected` check, urllib3 reconnects ("Resetting dropped connection", RESET). If the request lands while the server is still waiting (checking every 10 ms), the server reads it. The request fails only when it arrives after the server's last 10 ms wait timed out but before the FIN reaches the client; `drain_and_close_socket` then reads and throws it away. Why the fix cannot race: with `Connection: close`, http.client closes the connection's socket on a `will_close` response (`http/client.py:1457`, sets `sock = None`), and urllib3's `is_connected` returns False when `sock is None` (`urllib3/connection.py:352`). Every request therefore opens a fresh socket, whatever the timing. Other alternatives ruled out: the handler throwing, blocking or crashing (the failing request never reached a handler: no "served" line, its 160 bytes were drained; s1, the same handler with the same C1 fields, answered 200 in every run, including right after the failure; the app quit cleanly). `CPPHTTPLIB_KEEPALIVE_MAX_COUNT=100` (#106 was the 8th request on a connection opened at #99; a count-limit close is also announced with `Connection: close`, so it is clean).
HANDOFF-NEEDS: none

### SUMMARY
The failure is not a C1 regression. d_return_hitch's s0 GET is sent 5.0 s after the previous request, which lands on cpp-httplib's 5 s keep-alive close. With the instrumented header, the server's log shows it timing out the connection and draining (discarding) the 160-byte GET that arrived in that instant. The pre-C1 app fails the same way. The probe now uses a fresh connection for every request; the app is unchanged.

### DIAGNOSIS (instrumented; every claim below comes from a run)
1. **Which GET fails: s0** (VERIFIED, `02-hitch-traces-main1-pre1.txt`). main1: `#105 POST /api/switch_deck` answered at .823314, then `[HITCH] s0`, then `#106 GET /api/state` sent 5.0052 s later, `RemoteDisconnected` after 1.00 ms, with no "Resetting dropped connection" first. s1 (#108) went out on a new connection and got 200. pre1 has the same pattern: idle 5.0022 s, failure after 2.66 ms.
2. **Chance, not C1** (VERIFIED, `01-ab-main-vs-pre.txt`). I ran the full probe 3x per app, alternating main/pre, with the same instrumented client. main: RED, GREEN, GREEN. pre-C1: RED, GREEN, GREEN. In every run the only failure is #106, after 5.002-5.005 s idle. Gaps of 5.5 s and 10-11 s were always RESET (clean reconnect); 4.0 s gaps were always OK.
3. **The race window, on both apps** (VERIFIED, `03-sweep-main-vs-pre.txt`). Pattern: POST switch_deck, idle 5.0 s + x (x = -4 to +12 ms), GET /api/state, 3 reps. main: ERR 2/33 (at x = -4 and +2 ms). pre-C1: ERR 1/33 (at x = +3 ms). From x ≥ +4 ms, both are always RESET.
4. **Server side** (VERIFIED, `04-server-instrumented-kai.txt`, temporary [KAI] build). The ERR attempt, server lines in time order:
   - `served` at 131.971560 (the switch_deck response)
   - `keepalive-timeout` at 136.980996 (5.0094 s later; the client sent at 136.980338, 0.66 ms earlier)
   - `drain_and_close ... drained=160` at 136.981214

   160 bytes is exactly the prepared `GET /api/state` from requests 2.34.2. Every RESET attempt shows `drained=0`, and every OK attempt shows `readable`.

   Row-only runs of the original-logic probe on that build were 4/4 GREEN, all RESET: idle 5.008-5.019 s, past the edge. That is the "most runs pass" side of the race.

### FIX
- `.harmony/probe-deck-clock.py`: `S.headers["Connection"] = "close"` right after `S = requests.Session()`, with a 5-line comment naming the mechanism and pointing to this report. Nothing else changed: no row logic, no timings, no thresholds.
- I chose this over a GET-only urllib3 `Retry`. A retry would still leave the same race on POSTs (e.g. a trigger or switch sent about 5 s after the previous request), and retrying a POST is not safe. `Connection: close` removes connection reuse altogether, so neither can race. Loopback connect costs about 0.25 ms (`NEWCONN ... connect took 0.26 ms`).
- App side: no change. Closing idle connections is correct server behaviour. RFC 9112 §9.3.1/9.8 leaves recovery from an asynchronous close to the client. Changing the keep-alive timeout would only move the race to a different idle gap.

### FILES CHANGED
- `.harmony/probe-deck-clock.py`: every request uses a fresh connection (fix commit 8654b74).
- `.harmony/notebook.md`: a note on the keep-alive race and how to check it (report commit).
- `.harmony/.reports/s-rta-0927/c1-state-fix.md` + `c1-state-fix-evidence/01..07`: this report and the evidence (report commit, `git add -f`).

### TESTS
- RED (original probe, main app): Harmony's green-o and rerun-o (2/2), plus my main1 (instrumented copy; client logic unchanged). That is 3 of 5 runs.
- RED on the pre-C1 app, same probe: my pre1 (1 of 3). This shows the pre-C1 pass was luck.
- Teeth (same clean lane build, same session): reused connection ERR 2/40, `Connection: close` ERR 0/40 (`05-fix-teeth-sweep.txt`).
- GREEN, fixed probe-deck-clock on the lane build, 3 runs: `PY 10 PASS / 0 FAIL` each; d_return_hitch peak 21.41 / 19.31 / 18.76 ms (FAIL above 50).
- probe-outputs GREEN (13 PASS), probe-canvas GREEN (17 PASS), probe-render-state GREEN (33 PASS): 0 FAIL on the lane build (`06-green-battery-lane.txt`).
- ctest, full, serial (`-j1`): 691/691 passed (`07-ctest.txt`).
- Load: the A/B and first sweep ran at load 5-11.7 with 6 clang jobs (my lane build). The KAI sweep, teeth sweep and GREEN battery ran at load 2.6-6.6 with 0 clang jobs. Per-block values are in each evidence file ("### env" lines).

### SLIM CHECK
Nothing to cut. The diff is one header line plus a comment. Instrumentation was never committed. The scratch copies (`c1fix/probe/`, `kai_patch.py`, `sweep*.py`) live only in the scratchpad.

### ISSUES
- Harmony's FYI said lane routines-timing was instrumenting the 7070 server under "[KAI]". At 13:05 its scratch dir (`scratchpad/timing/`) had no keep-alive or [KAI] data, so I cited nothing from it. The "[KAI]" build and sweep described in this report are this lane's own (`scratchpad/c1fix/`).
- The fetched `httplib.h` in build-lane/_deps was patched temporarily and restored from my own pre-patch copy (sha256 9bd590b0…c23171 before and after). Only the AudioDNA target was built with the patch. The final full rebuild recompiled everything against the restored header.

### SKILL_PROPOSALS
- **http-flake-triage**: triggers on a lone `RemoteDisconnected` / `ConnectionResetError` in a probe that uses a `requests.Session`. Steps:
  1. Log timestamps and urllib3 connection events in a probe copy.
  2. Tabulate the idle gap before every request.
  3. Sweep idle = server keep-alive timeout + x.
  4. Instrument the server's keep-alive close and drain count temporarily.
  5. Join server and client events by wall clock.
  6. Fix the client with `Connection: close` (or a retry, if you can prove the requests are idempotent).

  Existing skills (root-cause) have no HTTP keep-alive specifics.

### RISKS
- Low: the other 4 probes that use a pooled `requests.Session` may carry the same latent race wherever a gap is close to 5.0 s (UNKNOWNS b). Mitigation: the same one-liner, or a shared probe HTTP helper. Out of scope here.
- Low: the scratch probe wrapper hard-codes ROOT to the scratch dir. It is evidence tooling only and is not committed.

### METRICS
- Self-check: lane build EXIT 0; `strings` "[KAI]" = 0; ctest 691/691; 4 probes GREEN (deck-clock x3).
- Tool calls: ~60. Files read: ~14.

### KNOWLEDGE CONTEXT
- Tools used: grep plus reading the httplib / http.client / urllib3 source. Impact authority: none needed (the change is in a probe, no app code).
- Risk level: NORMAL.

### PACKET QUALITY
- Clarity: CLEAR.
- Missing context: "passes 10/0" meant 10 rows in ONE run, not 10 runs. Only one pre-C1 run existed, so "pre-C1 passes" rested on n=1.
- Unused context: none.
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files: CLAUDE.md (project) and the notebook heading list were useful. The outputs-c1 report was not needed: the diff was enough.

### STATUS
DONE. The mechanism was established with a server-side log and a pre-C1 vs C1 A/B, the probe is fixed, and the battery is GREEN. Nothing was merged or pushed.

### NEXT ACTION
Harmony: gate (probe-deck-clock on your chosen app), review the one-line fix, and merge fix/c1-state-0927 if accepted. Optionally apply the same `Connection: close` line to the other Session-based probes.

INBOX-RECHECK: 1 addenda folded (Harmony FYI re routines-timing [KAI] data: checked, nothing applicable, noted in ISSUES)
