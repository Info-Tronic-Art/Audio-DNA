# Plan: STEP3_LONG=1 -- opt-in long take for the T2 drift proof (probe-step3.sh)

Architect (Fable), s-rta-0924b, 2026-09-24. Read-only on source; every claim
below is disk-cited and binned VERIFIED / INFERRED / ASSUMED. Line numbers are
from `.harmony/probe-step3.sh` as of main (717 lines, `wc -l` VERIFIED).

QUESTION: how to add an opt-in 10-minute file-mode click take to
`.harmony/probe-step3.sh` so the T2 drift row can be measured the way spec
D10.3 says (offset at minute 10 minus offset at minute 0), with the default
run left unchanged, thresholds taken from the spec, a cheap dry run, and exact
line ranges for the builder.

APPROACH (recommended, stated first)
1. ADDITIVE section, not a replacement: when `STEP3_LONG=1`, after the overdub
   section (11) and before the crash row (12), record ONE extra take
   (`step3long`) against a second, longer click WAV, for
   `STEP3_LONG_MINUTES` (default 10) minutes + 10 s margin, stop it, and run
   the SAME alignment oracle with a window argument. No replay of the long
   take (replay would cost 2x its length and proves nothing about drift).
   Default run: the block collapses to one `SKIP` line (same shape as the
   crash row, :637-671); nothing before it changes in behaviour.
2. ONE oracle: hoist section 9's inline python (:351-452) into a bash
   function `t2_align WAV TAKE INTERVAL [M [W]]`; with `M` given it appends
   six D10.3 window keys, without `M` its output line is byte-identical
   (VERIFIED below against the run-8 take on disk).
3. Two drift statistics on the long take, both reported with a standard
   error, both judged with the spec's 1 ms constant under the SAME
   "+2*stderr" rule the short-take row already uses (:465-477):
   (a) D10.3 literal: mean offset in asset-time window [0,60) vs
       [(M-1)*60, M*60); (b) least-squares slope x take length (the existing
       `drift_ms`/`drift_stderr_ms`). p95 jitter <= 15 ms, mean offset in
       [0,60] ms, coverage >= 90% as in section 9.
4. Cheap self-test: `STEP3_LONG=1 STEP3_LONG_MINUTES=2` (adds ~2.5 min), plus
   an OFFLINE check of the oracle against the run-8 take already on disk
   (no app launch at all; expected output given verbatim in section 6).

## 1. Facts the design rests on (disk-cited)

| # | Fact | Where | Bin |
|---|------|-------|-----|
| F1 | Spec T2: "play a 10-minute click-track WAV in file mode while recording a take ... PASS = p95 jitter <= 15 ms and drift (offset at minute 10 - offset at minute 0) <= 1 ms"; mean offset expected ~20-30 ms | `.harmony/specs/s167-performance-log-and-routines.md:606-612` | VERIFIED |
| F2 | Wall clock drifts "tens of ppm (up to ~30 ms per 10 min)" -- the drift class the long run must refute | same spec `:566-567` | VERIFIED |
| F3 | Arm over REST: `audioFile` is loaded via `AudioEngine::loadFile` (which calls `stop()`, position 0) and the transport is started right after a successful `recorderHost_.arm` | `src/MainComponent.cpp:2023-2028, 2055-2061`; `src/audio/AudioEngine.cpp:32-52, 64-68`; `applyAudioTransport("play")` `:5161-5164` | VERIFIED |
| F4 | Nothing in the app reacts to the transport reaching end-of-file: `onTransportStateChanged` has no consumer outside AudioEngine (grep over `src/`: 0 hits); no looping API exists (`setLooping`/`isLooping`: 0 hits in `src/audio`) | grep, `src/audio/AudioEngine.cpp:129-133` | VERIFIED |
| F5 | Therefore a WAV shorter than the take does NOT stop the take -- the tap keeps writing what the transport renders (silence), onsets stop, the minute-M window is empty. The WAV must outlast the take | inference from F3/F4 | INFERRED |
| F6 | Onset markers: one per onset EVENT via the monotonic `onsetCount` delta, capped `kMaxOnsetMarkersPerTick = 8`; stamped with `clock_.now().sample` = delivered-sample counter at tick time | `src/recording/RecorderHost.cpp:22, 425-447, 559-574` | VERIFIED |
| F7 | Tick cadence: the unconditional 120 Hz message-thread timer | `src/MainComponent.cpp:3443-3463` | VERIFIED |
| F8 | Periodic `Take::save` every `kCheckpointSeconds = 60` of take clock (10 saves in a 10-minute take, each a full-file TemporaryFile+rename) | `src/recording/RecorderHost.h:214`, `.cpp:454-468` | VERIFIED |
| F9 | Tap self-stop is surfaced only through `status().lastError` and the notify hook; `recording` stays true (the edge detector at `:403-415` sets lastError, never disarms) | `src/recording/RecorderHost.cpp:395-415` | VERIFIED |
| F10 | `perf/status` JSON carries `recording`, `t`, `markers`, `framesWritten`, `lastError`, `rateChangedSinceArm`, `assetId` -- enough to poll progress without any new endpoint | `src/MainComponent.cpp:2110-2157` | VERIFIED |
| F11 | AudioTap refuses to arm when the store volume has under 2 GB free | `src/recording/AudioTap.h:132`, `.cpp:109-110` | VERIFIED |
| F12 | `gen-click-wav.py --duration-s` (default 120) and `--interval` are the only knobs the probe needs; 120 s took 4.7 s wall and 23.0 MB (stereo int16 48 kHz = 192 kB/s) | `.harmony/gen-click-wav.py:63-66`; timed run in the scratchpad | VERIFIED (timing), INFERRED (linear scaling: 630 s ~ 25 s, ~121 MB) |
| F13 | Disk: 18 GiB free on the data volume (98% used); the audio store already holds 113 MB from earlier runs, never auto-deleted (Ruling 28) | `df -h ~/Documents`, `du -sh ~/Documents/Audio-DNA/Audio` | VERIFIED (today) |
| F14 | The run-8 take is on disk: `~/Documents/Audio-DNA/Takes/step3gate1.adna-take/take.json` (firstSample 587776, frames 3194368 = 66.5 s, 135 onset markers) and its asset `~/Documents/Audio-DNA/Audio/87e3305265d5408bb7fcb956ff890cd0.adna-audio/audio.wav` (12.8 MB) | read today | VERIFIED |
| F15 | Section 9's oracle on that take today prints exactly: `mean_offset_ms=33.22 drift_ms=-0.14 drift_stderr_ms=2.26 p95_jitter_ms=12.11 n_markers_raw=135 n_dupes=0 n_matched=135 n_spurious=0 n_grid_in_range=134 pct_matched=100.7 n_peaks=130` | `sed -n 351,452p .harmony/probe-step3.sh \| .venv/bin/python - WAV TAKE 24000` | VERIFIED |
| F16 | Residual noise of that fit: sigma = 7.60 ms (OLS, includes outliers up to +59 ms above the mean); abs-residual p50/p90/p95/p99 = 3.9/8.1/12.1/16.1 ms; 1 of 134 grid slots carried 2 markers | same data, numpy | VERIFIED |
| F17 | Existing section 9 drift rule: PASS iff abs(drift) <= 1 ms + 2*stderr; WARN (not FAIL) if stderr exceeds 0.5 ms | `.harmony/probe-step3.sh:465-477` | VERIFIED |
| F18 | Script is `#!/bin/bash` with `set -u`, macOS bash 3.2 (no namerefs, :519-520); the pgrep bracket trick is the only allowed process match (:43-48); teardown is a graceful osascript quit (:673-683); the only endpoints touched by the new section (`/api/perf/record`, `/api/perf/status`, `/api/perf/stop`) are already in the A6-verified safe set (:49-57) -- no output-window path | script header + body | VERIFIED |

## 2. What the numbers say (sizing the thresholds honestly)

With sigma = 7.6 ms per marker (F16) and 2 markers/s (CLICK_INTERVAL 24000):

| take | matched n | slope-drift stderr (sigma/sqrt(Sxx) x T) | 60 s-window difference stderr (sqrt(2) x sigma/sqrt(120)) |
|------|-----------|-------------------------------------------|------------------------------------------------------------|
| 65 s (today) | ~130 | 2.31 ms (observed 2.26) | n/a (no second full minute) |
| 2 min (dry run) | ~240 | 1.70 ms | 0.98 ms |
| 10 min (spec) | ~1200 | 0.76 ms | 0.98 ms |
| 20 min | ~2400 | 0.54 ms | 0.98 ms |

Consequences (all INFERRED from F16 by the standard OLS formulas the script
already uses at :424-436):
- A HARD 1 ms bound on either statistic is a flaky gate at true drift 0:
  P(FAIL) ~ 31% for the window difference (0.98 ms stderr) and ~19% for the
  10-minute slope (0.76 ms). That is the same flake class lane D removed
  from the short take (:465-471). So the long rows use the same rule:
  PASS iff abs(drift) <= 1 ms + 2*stderr. False-FAIL at true drift 0 drops
  to ~0.25% (window, bound ~2.96 ms) and ~0.1% (slope, bound ~2.5 ms); a real
  drift of 5 ms per 10 min (8 ppm) is caught with ~98% / ~99.9% power. A
  2.5-3 ms bound over 600 s is ~4-5 ppm -- an order of magnitude under the
  "tens of ppm" wall-clock drift class the spec warns about (F2), which is
  exactly what the long run exists to refute.
- The 0.5 ms WARN threshold (F17) will STILL fire on a 10-minute take at this
  jitter (0.76 / 0.98 ms). That is not a defect of the branch: at sigma
  7.6 ms and 2 markers/s, a 0.5 ms slope stderr needs ~23 minutes. The WARN
  text on the long rows therefore prints the minutes that WOULD retire it,
  computed from the run's own sigma (`minutes_for_0p5ms`), so a reader acts
  (`STEP3_LONG_MINUTES=23`) instead of re-diagnosing.
- Denser clicks (interval 12000) would buy only sqrt(2) and shrink the
  pairing tolerance (interval/2) to 125 ms against observed marker outliers
  of +92 ms absolute (F16: mean 33 + 59) -- a mis-pairing risk for a marginal
  gain. Not adopted; CLICK_INTERVAL stays the single source of truth (:103-107).

## 3. Tradeoffs considered

- Replace the gate's own take with a 10-minute one when STEP3_LONG=1 --
  REJECTED: sections 10 replay both takes at full length (2x10 min), the 40 s
  checkpoint sleep at :262 and the replay sequence expectations would need
  conditional rewrites, and "default run unchanged" becomes a claim to
  re-prove. The additive section makes it structural.
- Loop the existing 120 s WAV instead of generating a longer one --
  REJECTED: no transport looping exists (F4) and a loop seam would break the
  grid assumption `grid_point = round(assetFrame/interval)*interval` (:398).
- Run the long take in the background during the replay sections --
  REJECTED: one recorder, and R5 (replay must never self-record, :608-635)
  is precisely what section 11 is proving at that moment.
- Hard spec-literal 1 ms bound -- REJECTED with the flake math in section 2;
  the strongest counterargument to my recommendation, answered there.
- Replay the long take too (proves replay over 10 minutes) -- DEFERRED:
  doubles runtime; a separate ask if Boris wants a long-replay proof.
- Write the alignment line to `$OUT/t2-long.txt` -- DROPPED: stdout is
  captured by the runner already; one less file.

## 4. DECISION / SPEC -- exact edits to `.harmony/probe-step3.sh`

All edits are bash-3.2-safe and `set -u`-safe. Section numbers of existing
blocks are NOT renumbered (the new block is "11L").

### E1. Header comment -- insert after line 94 (before `set -u` at :95)
```
#   * The LONG take for the T2 drift proof (spec D10.3: offset at minute 10
#     minus offset at minute 0 over a 10-minute file-mode click take) is
#     section 11L, gated behind STEP3_LONG=1 (unset by default;
#     STEP3_LONG_MINUTES=N overrides the length, 2 = cheap dry run of the
#     branch). Adds ~11 min wall time and ~240 MB of disk per 10-minute run
#     (~121 MB click WAV in /tmp, ~117 MB asset in the store -- Ruling 28,
#     never auto-deleted). Nothing in the default run changes.
```

### E2. Config block -- insert after line 107 (`CLICK_INTERVAL=...`)
```
# s-rta-0924b: opt-in LONG take for the T2 drift proof (section 11L). Off by
# default. STEP3_LONG_MINUTES = spec D10.3's "minute 10" (default 10); 2 is
# the cheap dry run. The long click WAV is derived from CLICK_WAV (same
# interval: CLICK_INTERVAL stays the one source of truth for the grid).
LONG="${STEP3_LONG:-0}"
LONG_MINUTES="${STEP3_LONG_MINUTES:-10}"
if [ "$LONG" = "1" ] && ! echo "$LONG_MINUTES" | grep -Eq '^[1-9][0-9]*$'; then
    echo "REFUSE: STEP3_LONG_MINUTES must be a positive integer (got '$LONG_MINUTES')"; exit 64
fi
CLICK_WAV_LONG="${CLICK_WAV%.wav}_long.wav"
```

### E3. Step 0 -- insert after line 165
```
# STEP3_LONG: a second, longer click WAV for section 11L. Length = M*60 + 30 s
# (M*60 + 10 s recorded, plus slack for arm->play and stop latency) so the
# transport never reaches end-of-file mid-take: nothing in the app reacts to
# end-of-file (AudioEngine's onTransportStateChanged has no consumer), the
# take would just go silent and the minute-M window would be empty.
if [ "$LONG" = "1" ]; then
    python3 "$ROOT/.harmony/gen-click-wav.py" "$CLICK_WAV_LONG" --interval "$CLICK_INTERVAL" \
        --duration-s $((LONG_MINUTES * 60 + 30)) >/dev/null \
      && ok "LONG: click-track WAV generated at $CLICK_WAV_LONG ($((LONG_MINUTES * 60 + 30)) s)" \
      || no "LONG: click-track WAV generation FAILED"
fi
```
(Generation runs BEFORE launch so a failure refuses cheaply, same as the
short WAV; ~25 s for 630 s, F12.)

### E4. Step 1 preconditions -- insert after line 170
```
[ "$LONG" = "1" ] && { [ -f "$CLICK_WAV_LONG" ] || { echo "REFUSE: long click WAV missing after generation step"; exit 64; }; }
```

### E5. Section 9 -- hoist the oracle into `t2_align` (lines 349-355)
Replace lines 349-350 with:
```
# t2_align WAV TAKE INTERVAL [M [W]] -- the ONE alignment oracle, shared by
# section 9 (short take) and section 11L (STEP3_LONG). Prints a single
# "key=value ..." line. With M given it appends the D10.3 window keys
# (win_first_ms win_last_ms drift_win_ms drift_win_stderr_ms n_win_first
# n_win_last minutes_for_0p5ms); without M the line is byte-identical to the
# pre-11L output, so section 9's parsing (below) is untouched.
t2_align(){
  "$ROOT/.venv/bin/python" - "$@" <<'PYEOF'
```
Keep lines 351-452 (the python) inside the function with these THREE deltas:

P1 -- after line 354 (`interval = int(interval_str)`) insert:
```
# STEP3_LONG (D10.3, minute M vs minute 1): optional argv[4] = M minutes -> also
# report the mean offset in asset-time window [0, W) vs [(M-1)*W, M*W), with
# W = argv[5] seconds (default 60). Absent: output unchanged.
long_minutes = int(sys.argv[4]) if len(sys.argv)>4 else 0
win_s = float(sys.argv[5]) if len(sys.argv)>5 else 60.0
```
P2 -- after line 444 (`pct_matched = ...`) insert (same indentation as that line):
```
            extra = ""
            if long_minutes>0:
                w0 = [o for t, o in matched if 0.0 <= t < win_s]
                w1 = [o for t, o in matched if (long_minutes - 1) * win_s <= t < long_minutes * win_s]
                if len(w0)>=2 and len(w1)>=2:
                    m0, m1 = float(np.mean(w0)), float(np.mean(w1))
                    se = float(np.sqrt(np.var(w0, ddof=1) / len(w0) + np.var(w1, ddof=1) / len(w1)))
                    extra = (f" win_first_ms={m0:.2f} win_last_ms={m1:.2f} drift_win_ms={m1 - m0:.2f}"
                             f" drift_win_stderr_ms={se:.2f} n_win_first={len(w0)} n_win_last={len(w1)}")
                else:
                    extra = f" drift_win_ms=NA drift_win_stderr_ms=NA n_win_first={len(w0)} n_win_last={len(w1)}"
                # Minutes of take needed for a 0.5 ms slope-drift stderr at THIS
                # run's residual sigma and marker rate (stderr ~ sigma*sqrt(12/n)):
                # n = 12*(sigma/0.5)^2, minutes = n / (markers per second) / 60.
                if len(matched)>2 and drift_stderr_ms>0.0:
                    rate_m = len(matched) / take_duration_s
                    extra += f" minutes_for_0p5ms={12.0 * (float(s_err) / 0.5) ** 2 / rate_m / 60.0:.1f}"
```
(`s_err` is the residual sigma already computed at line 434; it is only
defined when `n_pts>2` and `sxx>0`, which the `len(matched)>2 and
drift_stderr_ms>0.0` guard implies -- builder: keep that guard.)

P3 -- line 450: change the print's last fragment from
`f"pct_matched={pct_matched:.1f} n_peaks={len(peaks)}")` to
`f"pct_matched={pct_matched:.1f} n_peaks={len(peaks)}" + extra)`.

Then replace lines 453-455 (`PYEOF`, `)"`, `echo "T2 alignment: $ALIGN"`) with:
```
PYEOF
}
if [ -x "$ROOT/.venv/bin/python" ]; then
    ALIGN="$(t2_align "$ASSET_DIR/audio.wav" "$TAKE_FOLDER/take.json" "$CLICK_INTERVAL")"
    echo "T2 alignment: $ALIGN"
```
Lines 456-501 (parsing, rows, the .venv FAIL branch) are unchanged. The sed
parsers there stay correct: none of the new keys contains an existing key
as a suffix (`drift_win_ms` vs `drift_ms`, `drift_win_stderr_ms` vs
`drift_stderr_ms` -- VERIFIED by running the existing sed patterns on the
long-mode line, section 6). Builder rule: any FUTURE key must keep that
property, or anchor the parsers with a leading space as 11L does.

### E6. New section 11L -- insert after line 635 (the `fi` closing section 11), before line 637 (`# --- 12.`)
```
# --- 11L. LONG take for the T2 drift proof (OPT-IN: STEP3_LONG=1) ----------
# Spec D10.3 T2 (s167 spec :606-612) measures drift as offset(minute 10) -
# offset(minute 0) over a 10-minute file-mode click take; the ~65 s take of
# section 5-9 cannot support the 1 ms claim (slope-drift stderr ~2.3 ms).
# This block records ONE extra take (step3long) of STEP3_LONG_MINUTES minutes
# (+10 s margin) against the longer click WAV from step 0, stops it, and runs
# the SAME t2_align oracle with the window argument. No replay of this take
# (2x its length for no drift information). Placed AFTER section 11 so its
# "Audio/ gained exactly one asset" delta is untouched, and BEFORE the crash
# row. Endpoints used: /api/perf/record, /api/perf/status, /api/perf/stop
# only -- the A6-verified safe set; no output-window path.
# Thresholds: the spec's 1 ms drift and 15 ms p95, applied with the SAME
# "<= 1 ms + 2*stderr" rule as section 9 (a hard 1 ms bound is at noise level
# even at 10 minutes: window-difference stderr ~1 ms, slope stderr ~0.75 ms at
# the observed ~7.6 ms per-marker jitter -- a ~31% / ~19% false-FAIL rate).
# The WARN prints the take length that WOULD retire it at this run's jitter.
if [ "$LONG" = "1" ]; then
    LONG_TAKE_NAME="step3long"
    LONG_TAKE_FOLDER="$TAKES_DIR/$LONG_TAKE_NAME.adna-take"
    LONG_SECS=$((LONG_MINUTES * 60))
    curl -s --max-time 6 -X POST "$A/api/perf/record" -H 'Content-Type: application/json' \
      -d "{\"name\":\"$LONG_TAKE_NAME\",\"audio\":true,\"audioFile\":\"$CLICK_WAV_LONG\",\"onsetMarkers\":true}" \
      | grep -q '"ok":[[:space:]]*true' \
      && ok "LONG: perf/record accepted (file-mode ${LONG_MINUTES}-minute click, onset markers)" \
      || no "LONG: perf/record refused"
    sleep 2
    LREC="$(perf_field "d.get('recording','NA')")"
    [ "$LREC" = "True" -o "$LREC" = "true" ] && ok "LONG: perf/status recording=true" || no "LONG: perf/status recording is not true ($LREC)"
    LASSET="$(perf_field "d.get('assetId','NA')")"
    # Record LONG_SECS + 10 s so the minute-M window [(M-1)*60, M*60) sits
    # fully inside the asset. Poll every <=30 s and FAIL out the moment the
    # recorder reports recording=false (a self-stop only sets lastError and
    # keeps recording=true -- RecorderHost.cpp:403-415 -- so lastError is
    # printed on every poll and re-checked after stop). A transient
    # perf/status read failure is tolerated, not treated as a stop.
    LONG_EARLY_STOP=0; LNOW=0
    LSTART=$(date +%s)
    while :; do
        LNOW=$(( $(date +%s) - LSTART ))
        LR="$(perf_status | python3 -c 'import json,sys
d=json.load(sys.stdin); print(d.get("recording","NA"), d.get("t","NA"), d.get("markers","NA"), d.get("framesWritten","NA"), repr(d.get("lastError","")))' 2>/dev/null || echo "NA")"
        echo "  long take: wall ${LNOW}s  (recording t markers framesWritten lastError) = $LR"
        case "$LR" in
            True*|true*) : ;;
            False*|false*) LONG_EARLY_STOP=1; break ;;
            *) echo "  (transient perf/status read failure, continuing)" ;;
        esac
        LLEFT=$((LONG_SECS + 10 - LNOW))
        [ "$LLEFT" -le 0 ] && break
        sleep $(( LLEFT<30 ? LLEFT : 30 ))
    done
    [ "$LONG_EARLY_STOP" = "0" ] \
      && ok "LONG: recorder stayed armed for the whole ${LONG_MINUTES}-minute take" \
      || no "LONG: recorder stopped early at wall ~${LNOW}s (see the status line above)"
    curl -s --max-time 6 -X POST "$A/api/perf/stop" >/dev/null
    sleep 3
    LERR="$(perf_field "d.get('lastError','NA')")"
    [ -z "$LERR" ] && ok "LONG: lastError empty after stop (no tap self-stop, no rate change, no save failure)" || no "LONG: lastError=$LERR"
    LRATE="$(perf_field "d.get('rateChangedSinceArm','NA')")"
    [ "$LRATE" = "False" -o "$LRATE" = "false" ] \
      && ok "LONG: rateChangedSinceArm == false across ${LONG_MINUTES} minutes" \
      || no "LONG: rateChangedSinceArm == $LRATE (expected false)"
    LASSET_DIR="$AUDIO_DIR/$LASSET.adna-audio"
    LSEG_FRAMES="$(take_field "$LONG_TAKE_FOLDER" "d['audio']['segments'][0]['frames']")"
    LSEG_RATE="$(take_field "$LONG_TAKE_FOLDER" "d['audio']['segments'][0]['rate']")"
    awk -v f="$LSEG_FRAMES" -v r="$LSEG_RATE" -v s="$LONG_SECS" 'BEGIN{exit !(r+0>0 && f+0>=s*r)}' 2>/dev/null \
      && ok "LONG: take audio covers the full ${LONG_MINUTES} min ($LSEG_FRAMES frames @ $LSEG_RATE Hz)" \
      || no "LONG: take audio shorter than ${LONG_MINUTES} min ($LSEG_FRAMES frames @ $LSEG_RATE Hz) -- the D10.3 window rows below cannot be trusted"
    if [ -x "$ROOT/.venv/bin/python" ]; then
        LALIGN="$(t2_align "$LASSET_DIR/audio.wav" "$LONG_TAKE_FOLDER/take.json" "$CLICK_INTERVAL" "$LONG_MINUTES")"
        echo "T2 alignment (LONG, ${LONG_MINUTES} min): $LALIGN"
        if echo "$LALIGN" | grep -q '^mean_offset_ms='; then
            LMEAN="$(echo "$LALIGN" | sed -n 's/.*mean_offset_ms=\([0-9.-]*\).*/\1/p')"
            LDRIFT="$(echo "$LALIGN" | sed -n 's/.* drift_ms=\([0-9.-]*\).*/\1/p')"
            LDRIFT_SE="$(echo "$LALIGN" | sed -n 's/.* drift_stderr_ms=\([0-9.-]*\).*/\1/p')"
            LP95="$(echo "$LALIGN" | sed -n 's/.*p95_jitter_ms=\([0-9.-]*\).*/\1/p')"
            LPCT="$(echo "$LALIGN" | sed -n 's/.*pct_matched=\([0-9.-]*\).*/\1/p')"
            LWIN="$(echo "$LALIGN" | sed -n 's/.* drift_win_ms=\([0-9.-]*\).*/\1/p')"
            LWIN_SE="$(echo "$LALIGN" | sed -n 's/.* drift_win_stderr_ms=\([0-9.-]*\).*/\1/p')"
            LN0="$(echo "$LALIGN" | sed -n 's/.* n_win_first=\([0-9]*\).*/\1/p')"
            LN1="$(echo "$LALIGN" | sed -n 's/.* n_win_last=\([0-9]*\).*/\1/p')"
            LMIN_NEEDED="$(echo "$LALIGN" | sed -n 's/.* minutes_for_0p5ms=\([0-9.]*\).*/\1/p')"
            # (a) D10.3 literal statistic: offset(minute M) - offset(minute 1).
            if [ -n "$LWIN" ]; then
                LWIN_BOUND="$(awk -v se="$LWIN_SE" 'BEGIN{printf "%.4f", 1.0 + 2*se}' 2>/dev/null)"
                awk -v x="$LWIN" -v b="$LWIN_BOUND" 'BEGIN{exit !(x<=b && x>=-b)}' 2>/dev/null \
                  && ok "LONG D10.3: |offset(min $LONG_MINUTES) - offset(min 1)| within +-${LWIN_BOUND}ms (<=1ms+2*stderr; drift=${LWIN}ms stderr=${LWIN_SE}ms n=${LN0}/${LN1})" \
                  || no "LONG D10.3: |offset(min $LONG_MINUTES) - offset(min 1)| exceeds +-${LWIN_BOUND}ms (<=1ms+2*stderr; drift=${LWIN}ms stderr=${LWIN_SE}ms n=${LN0}/${LN1})"
                awk -v se="$LWIN_SE" 'BEGIN{exit !(se>0.5)}' 2>/dev/null \
                  && warn "LONG D10.3: window stderr ${LWIN_SE}ms exceeds 0.5ms -- two 60 s windows cannot support a tight 1ms claim at this per-marker jitter (informational; the slope row below uses every marker)"
            else
                no "LONG D10.3: window statistic unavailable (n_win_first=$LN0 n_win_last=$LN1 -- fewer than 2 markers in a window: take too short, onsets stopped, or the WAV ran out)"
            fi
            # (b) slope over the whole take (every marker), same rule as section 9.
            LDRIFT_BOUND="$(awk -v se="$LDRIFT_SE" 'BEGIN{printf "%.4f", 1.0 + 2*se}' 2>/dev/null)"
            awk -v x="$LDRIFT" -v b="$LDRIFT_BOUND" 'BEGIN{exit !(x<=b && x>=-b)}' 2>/dev/null \
              && ok "LONG slope: drift over ${LONG_MINUTES} min within +-${LDRIFT_BOUND}ms (<=1ms+2*stderr; drift=${LDRIFT}ms stderr=${LDRIFT_SE}ms)" \
              || no "LONG slope: drift over ${LONG_MINUTES} min exceeds +-${LDRIFT_BOUND}ms (<=1ms+2*stderr; drift=${LDRIFT}ms stderr=${LDRIFT_SE}ms)"
            awk -v se="$LDRIFT_SE" 'BEGIN{exit !(se>0.5)}' 2>/dev/null \
              && warn "LONG slope: stderr ${LDRIFT_SE}ms exceeds 0.5ms -- at this run's jitter a 0.5ms stderr needs ~${LMIN_NEEDED:-?} minutes (STEP3_LONG_MINUTES=N); informational, does not fail the gate"
            echo "(informational) LONG drift bound in ppm: $(awk -v b="$LDRIFT_BOUND" -v s="$LONG_SECS" 'BEGIN{printf "%.1f", b*1000.0/s}' 2>/dev/null) ppm over ${LONG_SECS}s (spec D10.2 warns wall clock drifts tens of ppm)"
            awk -v x="$LP95" 'BEGIN{exit !(x<=15.0)}' 2>/dev/null \
              && ok "LONG: p95 jitter <=15ms ($LP95 ms, D10.3)" || no "LONG: p95 jitter exceeds 15ms ($LP95 ms)"
            awk -v x="$LMEAN" 'BEGIN{exit !(x>=0.0 && x<=60.0)}' 2>/dev/null \
              && ok "LONG: mean offset within [0,60]ms ($LMEAN ms)" || no "LONG: mean offset outside [0,60]ms ($LMEAN ms)"
            awk -v x="$LPCT" 'BEGIN{exit !(x>=90.0)}' 2>/dev/null \
              && ok "LONG: matched markers cover >=90% of grid clicks ($LPCT%)" || no "LONG: matched markers cover only $LPCT% of grid clicks (expected >=90%)"
        else
            no "LONG: T2 alignment could not be computed ($LALIGN)"
        fi
    else
        no "LONG: .venv/bin/python with numpy unavailable -- run from the main checkout"
    fi
else
    skip "LONG take for the T2 drift proof (set STEP3_LONG=1; STEP3_LONG_MINUTES=N overrides, default 10, 2 = dry run)"
fi
```
Sections 10, 12 and 13 are untouched. Rows added when STEP3_LONG=1: 12 ok/no
rows (WAV generated; record accepted; recording=true; stayed armed; lastError
empty; rateChangedSinceArm false; covers M min; D10.3 window; slope; p95;
mean offset; coverage) plus up to 2 WARN lines and 2 informational lines.
Default run: exactly one extra `SKIP` line, PASS/FAIL counts unchanged.

### What "done" looks like
- `bash -n .harmony/probe-step3.sh` clean; `shellcheck` no new findings.
- Offline oracle check (section 6, no app) prints the expected strings.
- Default run (env unset): 63 PASS / 0 FAIL, one more SKIP line, T2 line
  byte-identical in shape to today's.
- Dry run (`STEP3_LONG=1 STEP3_LONG_MINUTES=2`): 75 PASS / 0 FAIL expected
  (63 + 12), both LONG WARN lines present (stderr ~1.7 / ~1.0 ms), runtime
  +~2.5 min.
- Real run (`STEP3_LONG=1`): 75 PASS / 0 FAIL, drift numbers in the handoff
  with their stderr and the ppm bound; expect both WARN lines still (F16
  sizing) unless STEP3_LONG_MINUTES is raised to ~23.
- Commit with `git add -f .harmony/probe-step3.sh` and check
  `git show --stat HEAD` (rig rule); HANDOFF gets the numbers at close.

## 5. Runtime and footprint

| item | default run | dry run (M=2) | spec run (M=10) | bin |
|------|-------------|---------------|-----------------|-----|
| extra WAV generation | 0 | ~6 s (150 s WAV, 29 MB) | ~25 s (630 s WAV, ~121 MB in /tmp, overwritten per run) | INFERRED from F12 |
| extra recording | 0 | 130 s + 5 s | 610 s + 5 s | by construction |
| extra alignment | 0 | ~2 s | ~5 s (numpy reads the whole ~117 MB WAV; ~360 MB peak RSS) | INFERRED |
| total added | 0 | ~2.5 min | ~11 min | INFERRED |
| store growth (never auto-deleted, Ruling 28) | ~13 MB | ~27 MB | ~117 MB | VERIFIED rate (192 kB/s) |
| whole gate | ~5 min (INFERRED from the sleeps) | ~7.5 min | ~16 min | INFERRED |

Disk today: 18 GiB free (F13); AudioTap's 2 GB floor (F11) is far away, but
each 10-minute run costs ~240 MB, so old `step3long` assets under
`~/Documents/Audio-DNA/Audio/` should be pruned BY HAND when space matters
(the probe must not delete -- Ruling 28, `:628-629`).

## 6. Cheap self-tests (in order of cost)

S1 (seconds, no app): syntax -- `bash -n .harmony/probe-step3.sh`; `shellcheck .harmony/probe-step3.sh` if installed.

S2 (seconds, no app): oracle regression on the run-8 take already on disk
(F14). Extract the function body and run it with and without the window arg:
```
WAV=$HOME/Documents/Audio-DNA/Audio/87e3305265d5408bb7fcb956ff890cd0.adna-audio/audio.wav
TAKE=$HOME/Documents/Audio-DNA/Takes/step3gate1.adna-take/take.json
# (a) no window arg -- must print EXACTLY (F15):
#   mean_offset_ms=33.22 drift_ms=-0.14 drift_stderr_ms=2.26 p95_jitter_ms=12.11 n_markers_raw=135 n_dupes=0 n_matched=135 n_spurious=0 n_grid_in_range=134 pct_matched=100.7 n_peaks=130
# (b) M=2, W=30 (windows [0,30) vs [30,60) on the 66 s take) -- must append:
#   win_first_ms=33.11 win_last_ms=33.20 drift_win_ms=0.09 drift_win_stderr_ms=1.41 n_win_first=60 n_win_last=61 minutes_for_0p5ms=<~23>
# (c) M=10 -- must append: drift_win_ms=NA drift_win_stderr_ms=NA n_win_first=121 n_win_last=0 ...
#     and the 11L parser must yield an EMPTY LWIN (the "window statistic unavailable" FAIL branch).
```
(a)-(c) were run today against the prototype of P1-P3 (identical code):
(a) byte-identical to the baseline; (b) and (c) printed the values above
(without `minutes_for_0p5ms`, added afterwards -- builder: verify its value
is ~23 on this take: 12*(7.60/0.5)^2/(135/66.55)/60 = 22.8).
The simplest way to run S2 is to source the function: `sed -n
'/^t2_align(){/,/^}/p' .harmony/probe-step3.sh` into a temp file, prepend
`ROOT=$PWD`, and call `t2_align "$WAV" "$TAKE" 24000 2 30`.

S3 (~5 min, app): default run with the env unset -- must be 63 PASS / 0 FAIL
with one added SKIP line; the T2 line must carry no `win_` keys.

S4 (~7.5 min, app): `STEP3_LONG=1 STEP3_LONG_MINUTES=2 bash .harmony/probe-step3.sh`
from the MAIN checkout (the .venv rule, :493-500). Expect 75 PASS / 0 FAIL,
the poll lines every <=30 s showing `markers` growing by ~60 per poll, both
LONG WARN lines, the ppm line, and a ~27 MB new asset.

S5 (~16 min, app): `STEP3_LONG=1 bash .harmony/probe-step3.sh` -- the D10.3
deliverable. Record in the handoff: drift_win, drift (slope), both stderrs,
p95, mean offset, coverage, ppm bound, and whether the WARN fired.

Rig rules honoured (F18): launch via `open` unchanged (:174); no new pgrep
(the bracket trick lines are untouched); no output-window path; graceful
teardown unchanged; no debugger, no GUI input.

## 7. RISKS

R1 (strongest counterargument): "The spec says drift <= 1 ms; the plan passes
up to ~3 ms." True, and deliberate: the spec's 1 ms is a bound on the TRUE
drift; the measurement carries ~1 ms of noise from tick/hop quantisation
(F6/F7, sigma 7.6 ms per marker, F16). A hard 1 ms bound fails a perfect
recorder ~31% of the time; the +2*stderr rule fails it ~0.25% and still
catches 5 ms per 10 min (8 ppm) with ~98% power. If Boris wants the literal
bound to be SUPPORTED rather than merely not violated, the lever is length:
`STEP3_LONG_MINUTES=23` at today's jitter (the WARN prints the exact number
from each run's own sigma). Reducing per-marker jitter at the source
(stamping markers with the analysis-side sample instead of tick time) is a
product change outside this lane.

R2: Both WARN lines are EXPECTED to fire on the 10-minute run (0.76 / 0.98 ms
stderr, section 2). Readers must not file that as a defect; the WARN text
now says what retires it.

R3: Disk -- 98% full volume (F13). ~240 MB per 10-minute run, assets never
auto-deleted. If free space nears 2 GB the arm is refused (F11) and the
gate FAILs at the "perf/record accepted"/"recording=true" rows with
lastError "AudioTap failed to start (disk / free-space?)" -- correct
behaviour, but prune by hand first.

R4: Too-short WAV or transport stall -- the take goes silent, not stopped
(F4/F5). Caught by the "covers M min" row (frames) plus the window row
(`n_win_last=0`) -- two independent checks, both FAIL loudly. The 30 s of
WAV slack (E3) is the margin against arm-to-play latency.

R5: Periodic saves every 60 s (F8) rewrite a growing take.json (~300 KB at
10 min) on the message thread; a slow save could delay one tick and stamp
one marker late -- a jitter contribution near minute boundaries, not a
drift. If p95 on the long run rises above the short run's ~12 ms, look here
first (compare per-window jitter, the offsets are all in the oracle).

R6: Mid-run device change (headset connect) sets rateChangedSinceArm and
lastError (RecorderHost.cpp:365-383) -- surfaced by two rows; the drift
numbers of such a run are mixed-domain and must be discarded, as the R13
design says.

R7: A transient /api/perf/status failure during the poll is tolerated by
design; a genuine `recording=false` breaks out immediately with a FAIL so a
dead recorder never costs the full ten minutes.

R8 (ASSUMED, verify in S4): `sleep $(( LLEFT<30 ? LLEFT : 30 ))` -- the
arithmetic ternary is bash 3.2 syntax (it is), and `perf_status` piped into
python with `2>/dev/null` inside `$(...)` returns "NA" on any failure; S4's
poll lines confirm both.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0924b/plan-long-drift.md
STATUS: DONE
