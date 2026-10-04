#!/usr/bin/env python3
"""probe-sync.py -- the sync dial's live rows (s-rta-1002b bf2; plan-bf2.md section 5 G4 as amended by
ruling-bf2.md "FINAL CONSOLIDATED GATE LIST"). Driven by .harmony/probe-sync.sh, which owns the lock, the launch
(PRODUCTION mode, port 7070, no --test-mode: the analysis thread runs), the settings.json sha256 (R6) and the quit.
Written by the S1a builder; RUN by Harmony (the party that builds never verifies).

Stage S1a rows (LATE, 0..+500 ms):
  R0   GET /api/sync answers 200 with targetMs, appliedMs, venue, hopDelayMs, queuedHops, lineOverruns
       (the base commit: 404 -- the RED).
  R1   mechanism, any input (the mic is enough). Interleaved arms [0, 100, 0, 253, 0, 500, 0, 37, 38, 37, 38, 0]. Per
       arm: set, wait until |appliedMs - targetMs| < 0.1 for 0.5 s, then (see EMA_SETTLE_S) sample hopDelayMs 20 times
       over 2 s -> arm mean. Bars: mean(D) - the mean of its neighbouring 0 arms = D +- 0.8 ms for D in {100, 253,
       500}; mean(38) - mean(37) in [0.6, 1.4] ms on both pairs; lineOverruns == 0 throughout; queuedHops at 500 in
       [46, 49].
  R1a  absolute anchor (iteration witness during R1's 0, 253, 500 arms and R3): (i) every iteration with lineSize <
       64 has ringReadyAfterDrain < inputNeededFor(512); (ii) iteration start-to-start <= 10.7 ms on >= 99.5 % of
       iterations; (iii) max hops drained per iteration <= ceil(buffer_size x 48000 / (sample_rate x 512)) + 1
       (GET /api/debug/audio_devices "opened").
  R2   end to end, LATE arms: the click track (gen-click-wav.py, 120 BPM = 24,000 frames at 48 kHz) played by POST
       /api/perf/record {"name":"probesync","audio":false,"audioFile":<wav>}, 30 s per arm, preview visible (one
       render_frame attaches GL). Arms [0, 0, 100, 0, 253, 0, 500, 0]. Per onset: lag = render pulse frameMs - its
       onset hop's stampMs (witness, paired by onsetCount); per arm: median lag. Bars: control pair (arms 1-2)
       |median difference| <= 6 ms; each D arm: median - the mean of its two neighbouring 0-arm medians = D +- 8 ms;
       every arm: render pulse delta == onsetCount delta and onsetCount delta >= 54 of 60. The arms' takes
       (~/Documents/Audio-DNA/Takes/probesync-<run id>-aK) are this probe's own and are removed at the end.
  R3   slew, manual 120 BPM (POST /api/set_bpm {"bpm":120}): 0 -> 500 then 500 -> 0. Least-squares slope of the
       published beat position (totalBeatCount + beatPhase) against witness processMs over every 200 ms window inside
       the slew (100 ms trimmed at each end) / 2.0 beats/s: every window in [0.70, 0.80] (up) and [1.20, 1.30] (down);
       appliedMs from its first value > 0 to 500 (and from below 500 to 0) in 2.00 +- 0.05 s.
  R2   (stage S2) + the EARLY arms [-100, 0] after the LATE ones: the -100 arm's median - the mean of its neighbouring 0
       arms' medians within +-8 ms (onsets are never early), and its render pulse delta == onsetCount delta >= 54.
  R4   (S2) EARLY, manual 120 BPM: settled at -100, every hop (published - raw) beatPhase mod 1 = 0.200 +- 0.005; settled
       at -500, published totalBeatCount - raw = 1 with phases equal +- 0.005; slews 0 -> -500 and -500 -> 0: R3's
       metric in [1.20, 1.30] and [0.70, 0.80]; scripted run -100 -> Resync -> -500 -> Resync -> 0 -> +100: published
       totalBeatCount / totalBarCount never decrease, every published totalBarCount increment is downbeatDetected
       rising, the level holds for the view's first beat (a Resync may raise it without an increment), published - raw
       bar increments in [0, 2]; every hop nonBeatCrcBefore == nonBeatCrcAfter.
  R4b  (S2) EARLY tempo steps, manual, at -500: POST /api/set_bpm 174, 87, 174, 200, 60, 200 (20 s each, no realign):
       held == false on every hop; per 200 ms window the published slope / the raw slope in [0.70, 1.30]; leadBeats
       reaches 0.5 x bpm / 60 +- 0.001 within ceil(|change| / (0.25 x aN)) + 2 hops of each step.
  R5   (S2) EARLY, AUTO (runs first: no REST route turns manual mode off, and every other tempo row turns it on), the
       60 s click track, arms [0, -100, 0, -500, 0]: R4's monotonicity / level / CRC bars on every hop; onsets: |median
       lag - the neighbouring 0 arms| <= 8 ms at -100 and -500; holds on every hop BeatLead ran: held => deficitBeats >
       0 or a counter would decrease; not held => published fold == target fold +- 1e-4 beat; where the raw fold did
       not move back (it advanced >= its free-run step aN - 1e-4; beatPhase is a float) the target fold advanced >=
       0.75 x aN - 1e-4 (less any float shortfall of the raw step). INFO: held-hop fraction, longest held run, runs.
  R6   (half of it; probe-sync.sh does the sha256) GET /api/sync shows persist false on every read.
  R7   (S3; re-stated by ruling-bf2-delta D3, s-rta-1003b) take alignment: click takes with onset markers (POST
       /api/perf/record {"audio":true, "audioFile":<60 s click WAV>, "onsetMarkers":true}), 21 arms in one launch:
       [0, 100] x 10 + [0], 60 s each; probe-step3's T2 marker-vs-click math (each marker's asset frame paired to the
       nearest click of the 0.5 s grid, in the asset's own rate; off-grid markers > interval / 2 dropped; duplicates
       on 'sample' dropped). Per arm: a marker farther than 21.333 ms x the take's segment rate (1,024 samples at
       48 kHz) from the arm's median is dropped and counted; the arm's value is the MEAN of the rest.
       dbar = mean of the 100-arm values - mean of the 0-arm values; SE = sqrt(sH^2 / nH + sZ^2 / nZ), sH / sZ the
       sample SD of the arm values in each group. VALID when every arm pairs >= 100 markers and drops <= 4.
       BAR: |dbar| <= 1.0 ms. ONE extension: when |dbar| is within 2 x SE of 1.0 (either side) or SE > 0.33 ms the
       row prints "FAIL  R7 UNDECIDED ..." and the same 21 arms run once more with PROBESYNC_R7_EXTEND=1
       PROBESYNC_R7_PRIOR=<the first run's probe-sync.json>: the pooled 42 arms decide by the same formulas; a pooled
       SE still above 0.33 ms = INCONCLUSIVE (blocks; to the architect). A FAIL with |dbar| < 2.67 ms is a finding
       for the architect, never re-run. INFO (no verdict): each arm's median, mean, trimmed mean, drops and the index
       of every dropped marker; each 100 arm minus the mean of its two neighbours; the lattice in samples; the device
       block size and rate. PROBESYNC_R7_ARMS (a RED / development subset, e.g. 0,100,0) prints "SUBSET
       (... development: not a gate line)" on R7's and R7b's verdict lines: never a gate line.
  R7b  (S3; re-stated by D4) gestures land in show time, inside R7's FIRST 100 arm: the probe watches the onset count
       THE RECORDER has seen (GET /api/perf/status "markers") and, right after it reaches k (k = 10, 20, 30), fires a
       REST gesture (POST /api/trigger_clip layer 0, columns 1 / 2 / 3: a Human activeClip point). For each k: g_k =
       (gesture stamp - the take's first sample) - the click-grid position nearest marker k. B = the median paired
       marker error over ALL 0 arms of the run. BAR: 3 gestures fired and 3 found in the take; the MEDIAN of the
       three (g_k - B) lies in [-32 ms, +80 ms] x the take's segment rate (-1,536 .. +3,840 samples at 48 kHz); all
       three are printed. INFO (no verdict): gesture stamp - marker k stamp for each k (the earlier [0, 60 ms]
       window beside it). The arms' takes AND their Audio-store assets are this probe's own and are removed at the end.
  R1a  (s-rta-1003b) + (ii) and (iii) evaluated over R4's settled -500 window as well (needs R4 in the rows).
  G6   (s-rta-1003b, ruling-bf2-delta D2; quiet machine) per-hop analysis cost from the witness's pipelineUs over R1's
       settled 0 arms and its 500 arm, R4's settled -500 window and R4's settled 0 step (each >= 150 hops): mean,
       p99, max. BARS: (1) in the -500 window leadApplied is 1 on >= 99 % of the hops; (2) mean pipelineUs <= 2,000
       in each window. If a 0 window itself is above 2,000, (2) is replaced by mean(-500) <= mean(R4's 0 step) + 300
       and mean(+500) <= mean(its two neighbouring R1 0 arms) + 300 (us), the second valid only when those two 0 arms
       differ by < 100; otherwise INCONCLUSIVE. INFO: the -500 / +500 differences in us and DSP points (us / 106.67)
       beside the earlier 0.3-point figure; GET /api/status frameTimeMs once per window. Needs R1 and R4 in the rows.
       Not a gate for the delay line: R1a is.
  R5   (s-rta-1003b, D6) + the number of free-running hops (the "did not move back" antecedent) is printed and must
       be >= 5,000 and >= half of the hops BeatLead ran. PROBESYNC_R5_ARMS / PROBESYNC_R5_SECONDS: a development
       subset only (the gate runs [0, -100, 0, -500, 0] x 60 s); every R5 verdict line of such a run starts "R5 SUBSET
       (development: not a gate line)".
Self-test of the pure verdict functions (r7_trim, r7_verdict, g6_verdict, subset_tag), no app:
.harmony/probe-sync-selftest.py.
  R9   (BLOCKING, Q6 default) at +100 and +253, manual 120 BPM, POST /api/resync three times per value, >= 3 s apart:
       the first witness hop whose trackerRequestSeq exceeds its pre-POST value publishes the Resync (beatInBar 0,
       beatPhase 0, resyncBarOrigin == totalBarCount) and its processMs is <= 50 ms after the POST returned (clocks
       aligned through the witness reply's nowMs).
  RD   (s-rta-1003b stage D0; ruling-bf2-stops.md A1..A5, section 4's decision table, section 5 row RD) a DIAGNOSIS
       run, never a gate line. PROBESYNC_RD=1 runs row_rd in R7's place (rows must hold R7): R7's take loop at dial 0
       (PROBESYNC_R7_ARMS, default 0,0,0,0,0,0; PROBESYNC_R7_SECONDS, default 12, read in RD mode only), no R7
       verdict, no R7b. Per take, before the asset is deleted: the AUDIO ruler (c0 = the median over the clicks of
       asset onset - source onset; onset = the first sample above half full scale, hits within 100 samples collapsed;
       e = the median marker error as R7 reads it today; e' = e - c0), the WITNESS ruler (C = the median of marker
       stamp - the `timestamp` of the witness hop on which onsetCount rose; compared inside ONE launch only), GET
       /api/debug/audio_devices "opens" / block / rate, the take's gap count, the hops with appliedMs != 0. Lines
       "INFO  RD take <i>: ..." and "INFO  RD launch: ..."; every take's record in probe-sync.json rows.RD.takes.
       Offline (no app): probe-sync.py --rd-verdict FILE ... prints "RD outcome O<n> (...)" or "RD INCOMPLETE (...)"
       by the table, tested O4, O2, then O1 / O5 (exit 0 on an outcome, 3 on INCOMPLETE, 2 on an unreadable file).
       With PROBESYNC_RD unset nothing here runs: row R7 is row_r7, unchanged.
EMA_SETTLE_S (R1, a procedure note for Harmony): hopDelayMs is an EMA with alpha 0.05 per hop (time constant ~213 ms);
after a slew it trails the ramp by ~53 ms, so 0.5 s after the slew ends it still carries ~5 ms and would bias a 2 s
mean by ~0.5 ms per arm (~1 ms on a D - 0 difference, against a +-0.8 ms bar). The probe therefore waits a further
EMA_SETTLE_S = 1.0 s (7 time constants in all) before sampling. Thresholds are the ruling's, unchanged.

Every HTTP request uses a fresh connection with "Connection: close". Prints "PASS  ..." / "FAIL  ..." / "INFO  ..."
lines; exit 1 on any FAIL. Writes the raw numbers to <out>/probe-sync.json.
"""
import argparse
import json
import math
import os
import statistics
import sys
import threading
import time
import urllib.error
import urllib.request

A = "http://127.0.0.1:7070"
EMA_SETTLE_S = 1.0
RUNID = time.strftime("%Y%m%d%H%M%S") + "-%d" % os.getpid()
TAKES_DIR = os.path.expanduser("~/Documents/Audio-DNA/Takes")
RESULTS = {"rows": {}, "fails": 0, "passes": 0}


def ok(msg):
    RESULTS["passes"] += 1
    print("PASS  " + msg, flush=True)


def no(msg):
    RESULTS["fails"] += 1
    print("FAIL  " + msg, flush=True)


def info(msg):
    print("INFO  " + msg, flush=True)


def http(method, path, body=None, timeout=5.0):
    """(status, parsed-json-or-text). Fresh connection per request, Connection: close."""
    data = None if body is None else json.dumps(body).encode()
    req = urllib.request.Request(A + path, data=data, method=method,
                                 headers={"Connection": "close", "Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            raw = r.read().decode("utf-8", "replace")
            status = r.status
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode("utf-8", "replace")
    except Exception as e:  # noqa: BLE001 -- a probe reports, never crashes
        return -1, str(e)
    try:
        return status, json.loads(raw)
    except ValueError:
        return status, raw


def get_sync():
    st, d = http("GET", "/api/sync")
    if st != 200 or not isinstance(d, dict):
        return None
    if d.get("persist") is not False:
        RESULTS.setdefault("persistTrue", 0)
        RESULTS["persistTrue"] = RESULTS.get("persistTrue", 0) + 1
    return d


def frame_time_ms():
    """GET /api/status frameTimeMs (G6 INFO: read once per window), or None."""
    st, d = http("GET", "/api/status")
    return d.get("frameTimeMs") if st == 200 and isinstance(d, dict) else None


def set_sync(ms):
    st, d = http("POST", "/api/sync/set", {"ms": ms})
    return st == 200 and isinstance(d, dict) and d.get("ok") is True


# ------------------------------------------------------------------------------------------------ witness collector
class Witness:
    """Polls GET /api/debug/sync_witness every `period` s in a thread; keeps every hop / iteration / pulse entry and
    the clock offset (app clock - local monotonic ms) from each reply's nowMs."""

    def __init__(self, period=0.25):
        self.period = period
        self.hops, self.iters, self.pulses = {}, {}, {}
        self.since = {"sinceHop": 0, "sinceIter": 0, "sincePulse": 0}
        self.offsets = []
        self.lock = threading.Lock()
        self.stop = threading.Event()
        self.ok = True
        self.lostIters = 0
        self.thread = None

    def poll_once(self):
        q = "&".join("%s=%d" % (k, v) for k, v in self.since.items())
        t0 = time.monotonic() * 1000.0
        st, d = http("GET", "/api/debug/sync_witness?" + q)
        t1 = time.monotonic() * 1000.0
        if st != 200 or not isinstance(d, dict):
            self.ok = False
            return False
        with self.lock:
            self.offsets.append((d["nowMs"] - (t0 + t1) / 2.0, t1 - t0))
            # entries older than the ring keeps were overwritten before we read them: count iteration gaps
            if d.get("iters") and self.since["sinceIter"] and d["iters"][0]["i"] > self.since["sinceIter"]:
                self.lostIters += d["iters"][0]["i"] - self.since["sinceIter"]
            for e in d.get("hops", []):
                self.hops[e["i"]] = e
            for e in d.get("iters", []):
                self.iters[e["i"]] = e
            for e in d.get("pulses", []):
                self.pulses[e["i"]] = e
            self.since = {"sinceHop": d["hopsNext"], "sinceIter": d["itersNext"], "sincePulse": d["pulsesNext"]}
        return True

    def _run(self):
        while not self.stop.is_set():
            self.poll_once()
            self.stop.wait(self.period)

    def start(self):
        self.poll_once()
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()

    def finish(self):
        self.stop.set()
        if self.thread:
            self.thread.join()
        self.poll_once()

    def app_now_offset(self):
        """app clock - local monotonic ms, from the lowest-RTT reply (+- RTT/2)."""
        with self.lock:
            if not self.offsets:
                return None, None
            off, rtt = min(self.offsets, key=lambda x: x[1])
            return off, rtt

    def hops_between(self, a_ms, b_ms):
        with self.lock:
            return [self.hops[k] for k in sorted(self.hops) if a_ms <= self.hops[k]["processMs"] <= b_ms]

    def iters_between(self, a_ms, b_ms):
        with self.lock:
            return [self.iters[k] for k in sorted(self.iters) if a_ms <= self.iters[k]["startMs"] <= b_ms]


def app_ms(w):
    off, _ = w.app_now_offset()
    return time.monotonic() * 1000.0 + (off or 0.0)


def wait_settled(target, timeout=6.0):
    """|appliedMs - targetMs| < 0.1 continuously for 0.5 s. Returns the last /api/sync or None."""
    t_end = time.monotonic() + timeout
    since = None
    d = None
    while time.monotonic() < t_end:
        d = get_sync()
        if d is not None and d.get("targetMs") == target and abs(d["appliedMs"] - d["targetMs"]) < 0.1:
            since = since or time.monotonic()
            if time.monotonic() - since >= 0.5:
                return d
        else:
            since = None
        time.sleep(0.05)
    return None


# ------------------------------------------------------------------------------------------------ rows
def row_r0():
    st, d = http("GET", "/api/sync")
    RESULTS["rows"]["R0"] = {"status": st, "body": d}
    keys = ["targetMs", "appliedMs", "venue", "hopDelayMs", "queuedHops", "lineOverruns"]
    if st == 200 and isinstance(d, dict) and all(k in d for k in keys):
        ok("R0 GET /api/sync 200 with %s: %s" % (", ".join(keys), json.dumps(d)))
        return True
    no("R0 GET /api/sync answered %s (%s) -- missing route or keys" % (st, str(d)[:200]))
    return False


def row_r1(w):
    arms = [0, 100, 0, 253, 0, 500, 0, 37, 38, 37, 38, 0]
    witness_arms = {0, 253, 500}
    res = []
    overruns_seen = []
    for d_ms in arms:
        if not set_sync(d_ms):
            no("R1 POST /api/sync/set %d refused" % d_ms)
            return None
        s = wait_settled(d_ms)
        if s is None:
            no("R1 arm %d: applied never settled within 6 s" % d_ms)
            return None
        time.sleep(EMA_SETTLE_S)
        a0 = app_ms(w)
        vals, queued = [], []
        for _ in range(20):
            s = get_sync()
            vals.append(s["hopDelayMs"])
            queued.append(s["queuedHops"])
            overruns_seen.append(s["lineOverruns"])
            time.sleep(0.1)
        a1 = app_ms(w)
        res.append({"D": d_ms, "mean": statistics.fmean(vals), "min": min(vals), "max": max(vals),
                    "queuedMin": min(queued), "queuedMax": max(queued), "appWindow": [a0, a1],
                    "witness": d_ms in witness_arms, "frameTimeMs": frame_time_ms()})
        info("R1 arm %3d: hopDelayMs mean %.3f (min %.3f max %.3f), queuedHops %d..%d"
             % (d_ms, res[-1]["mean"], min(vals), max(vals), min(queued), max(queued)))
    RESULTS["rows"]["R1"] = res
    for i, r in enumerate(res):
        if r["D"] in (100, 253, 500):
            neigh = [res[j]["mean"] for j in (i - 1, i + 1) if 0 <= j < len(res) and res[j]["D"] == 0]
            shift = r["mean"] - statistics.fmean(neigh)
            (ok if abs(shift - r["D"]) <= 0.8 else no)(
                "R1 D=%d: mean - neighbouring 0 arms = %.3f ms (bar %d +- 0.8)" % (r["D"], shift, r["D"]))
        if r["D"] == 500:
            (ok if 46 <= r["queuedMin"] and r["queuedMax"] <= 49 else no)(
                "R1 queuedHops at 500 in [%d, %d] (bar [46, 49])" % (r["queuedMin"], r["queuedMax"]))
    pairs = [(7, 8), (9, 10)]
    for a, b in pairs:
        d = res[b]["mean"] - res[a]["mean"]
        (ok if 0.6 <= d <= 1.4 else no)("R1 mean(38) - mean(37) = %.3f ms (arms %d/%d; bar [0.6, 1.4])" % (d, a, b))
    (ok if max(overruns_seen) == 0 else no)("R1 lineOverruns == 0 throughout (max seen %d)" % max(overruns_seen))
    return res


def row_r1a(w, windows, label="", parts=("i", "ii", "iii"), key="R1a"):
    """label / parts / key: s-rta-1003b -- (ii) and (iii) are evaluated a second time over R4's settled -500 window."""
    st, dev = http("GET", "/api/debug/audio_devices")
    opened = dev.get("opened", {}) if isinstance(dev, dict) else {}
    sr, bs = float(opened.get("sample_rate", 0) or 0), int(opened.get("buffer_size", 0) or 0)
    iters = []
    for a, b in windows:
        iters.extend(w.iters_between(a, b))
    hops = sorted(w.hops.values(), key=lambda e: e["i"])
    rate = hops[-1]["sourceRate"] if hops else 48000.0
    ratio = 1.0 if (rate <= 0 or abs(rate - 48000.0) < 0.5) else rate / 48000.0
    needed = math.ceil(ratio * 512) + 2
    RESULTS["rows"][key] = {"iters": len(iters), "sample_rate": sr, "buffer_size": bs, "needed": needed,
                            "lostIters": w.lostIters}
    if not iters:
        no("R1a%s no iteration witness entries collected (witness route missing?)" % label)
        return
    if w.lostIters:
        info("R1a %d iteration entries were overwritten before the probe read them (ring 1024)" % w.lostIters)
    bad_i = [e for e in iters if e["lineSize"] < 64 and e["ringReadyAfterDrain"] >= needed]
    if "i" in parts:
        (ok if not bad_i else no)("R1a(i) %d iterations, %d with lineSize < 64 left >= %d samples in the ring%s"
                                  % (len(iters), len(bad_i), needed, (": e.g. %s" % bad_i[0]) if bad_i else ""))
    gaps = []
    by_i = {e["i"]: e for e in iters}
    for e in iters:
        prev = by_i.get(e["i"] - 1)
        if prev is not None:
            gaps.append(e["startMs"] - prev["startMs"])
    frac = sum(1 for g in gaps if g <= 10.7) / len(gaps) if gaps else 0.0
    (ok if gaps and frac >= 0.995 else no)(
        "R1a(ii)%s iteration start-to-start <= 10.7 ms on %.3f %% of %d gaps (max %.2f ms; bar >= 99.5 %%)"
        % (label, 100.0 * frac, len(gaps), max(gaps) if gaps else -1))
    if sr > 0 and bs > 0:
        bound = math.ceil(bs * 48000.0 / (sr * 512.0)) + 1
        mx = max(e["hopsDrained"] for e in iters)
        (ok if mx <= bound else no)("R1a(iii)%s max hops drained per iteration %d <= %d (buffer %d @ %.0f Hz)"
                                    % (label, mx, bound, bs, sr))
    else:
        no("R1a(iii)%s GET /api/debug/audio_devices gave no opened sample_rate / buffer_size (%s)"
           % (label, str(dev)[:160]))


def row_r2(w, wav, fixture, outdir):
    arms = [0, 0, 100, 0, 253, 0, 500, 0, -100, 0]   # stage S2 appends the EARLY arm [-100, 0]
    if os.environ.get("PROBESYNC_R2_ARMS"):          # a RED / diagnostic subset only; the gate runs the full list
        arms = [int(x) for x in os.environ["PROBESYNC_R2_ARMS"].split(",")]
    st, d = http("POST", "/api/load_composition", {"path": fixture}, timeout=10)
    info("R2 load_composition %s: %s" % (fixture, str(d)[:120]))
    time.sleep(1.0)
    st, d = http("POST", "/api/render_frame", {"output_path": os.path.join(outdir, "attach.png")}, timeout=15)
    (ok if st == 200 else no)("R2 render_frame %s (GL attached -- preview visible)" % st)
    res = []
    for k, d_ms in enumerate(arms):
        set_sync(d_ms)
        if wait_settled(d_ms) is None:
            no("R2 arm %d (%d ms): applied never settled" % (k, d_ms))
            return None
        time.sleep(0.5)
        f0 = http("GET", "/api/features")[1]
        s0 = http("GET", "/api/status")[1]
        a0 = app_ms(w)
        st, r = http("POST", "/api/perf/record", {"name": "probesync-%s-a%d" % (RUNID, k), "audio": False,
                                                  "audioFile": wav})
        if st != 200 or not (isinstance(r, dict) and r.get("ok")):
            no("R2 arm %d: perf/record refused (%s %s)" % (k, st, str(r)[:160]))
            return None
        time.sleep(31.0)
        http("POST", "/api/perf/stop")
        time.sleep(1.5 + d_ms / 1000.0)   # the last D ms of audio is still in the analysis delay line
        a1 = app_ms(w)
        f1 = http("GET", "/api/features")[1]
        s1 = http("GET", "/api/status")[1]
        donset = f1["onsetCount"] - f0["onsetCount"]
        dpulse = s1["renderOnsetPulses"] - s0["renderOnsetPulses"]
        hops = sorted(w.hops_between(a0, a1), key=lambda e: e["i"])
        first_hop_for = {}
        prev = None
        for h in hops:
            if prev is not None and h["onsetCount"] > prev["onsetCount"]:
                first_hop_for.setdefault(h["onsetCount"], h)
            prev = h
        with w.lock:
            pulses = [p for p in w.pulses.values() if a0 <= p["frameMs"] <= a1]
        lags = []
        for p in pulses:
            h = first_hop_for.get(p["onsetCount"])
            if h is not None:
                lags.append(p["frameMs"] - h["stampMs"])
        med = statistics.median(lags) if lags else float("nan")
        res.append({"D": d_ms, "median": med, "n": len(lags), "onsetDelta": donset, "pulseDelta": dpulse,
                    "fps": s1.get("fps")})
        info("R2 arm %d (%d ms): median lag %.2f ms over %d paired onsets; onsetCount delta %d, render pulse delta %d"
             % (k, d_ms, med, len(lags), donset, dpulse))
    RESULTS["rows"]["R2"] = res
    # The arms' takes are this probe's own (unique RUNID names): remove them, as probe-tempo-start.sh does.
    import glob
    import shutil
    mine = glob.glob(os.path.join(TAKES_DIR, "probesync-%s-a*.adna-take" % RUNID))
    for t in mine:
        shutil.rmtree(t, ignore_errors=True)
    left = glob.glob(os.path.join(TAKES_DIR, "probesync-%s-a*.adna-take" % RUNID))
    (ok if not left else no)("R2 the probe's %d takes were removed (probesync-%s-a*)" % (len(mine), RUNID))
    if len(res) >= 2 and res[0]["D"] == 0 and res[1]["D"] == 0:
        c = abs(res[0]["median"] - res[1]["median"])
        (ok if c <= 6.0 else no)("R2 control pair |median difference| = %.2f ms (bar <= 6)" % c)
    else:
        info("R2 control pair not in this arm list (PROBESYNC_R2_ARMS subset)")
    for i, r in enumerate(res):
        if r["D"] > 0:
            neigh = [res[j]["median"] for j in (i - 1, i + 1) if 0 <= j < len(res) and res[j]["D"] == 0]
            shift = r["median"] - statistics.fmean(neigh)
            (ok if abs(shift - r["D"]) <= 8.0 else no)(
                "R2 D=%d: median lag - neighbouring 0 arms = %.2f ms (bar %d +- 8)" % (r["D"], shift, r["D"]))
        elif r["D"] < 0:
            # EARLY moves no onset: the lag stays where the neighbouring 0 arms have it
            neigh = [res[j]["median"] for j in (i - 1, i + 1) if 0 <= j < len(res) and res[j]["D"] == 0]
            shift = r["median"] - statistics.fmean(neigh)
            (ok if abs(shift) <= 8.0 else no)(
                "R2 D=%d (EARLY): median lag - neighbouring 0 arms = %.2f ms (bar |.| <= 8: onsets are never early)"
                % (r["D"], shift))
        (ok if r["pulseDelta"] == r["onsetDelta"] and r["onsetDelta"] >= 54 else no)(
            "R2 arm %d (%d ms): render pulse delta %d == onsetCount delta %d, >= 54 of 60"
            % (i, r["D"], r["pulseDelta"], r["onsetDelta"]))
    return res


def lsq_slope(xs, ys):
    n = len(xs)
    mx, my = sum(xs) / n, sum(ys) / n
    sxx = sum((x - mx) ** 2 for x in xs)
    return sum((x - mx) * (y - my) for x, y in zip(xs, ys)) / sxx if sxx > 0 else float("nan")


def slew_windows(hops, up):
    """The slew interval from the witness: first hop with appliedMs > 0 (up) / < 500 (down) until applied reaches
    500 / 0. Returns (t_start, t_end, slopes per 200 ms window / 2.0 beats/s)."""
    if up:
        start = next((h for h in hops if h["appliedMs"] > 0.0), None)
        end = next((h for h in hops if start and h["i"] > start["i"] and h["appliedMs"] >= 500.0), None)
    else:
        start = next((h for h in hops if h["appliedMs"] < 500.0), None)
        end = next((h for h in hops if start and h["i"] > start["i"] and h["appliedMs"] <= 0.0), None)
    if start is None or end is None:
        return None
    t0, t1 = start["processMs"], end["processMs"]
    ratios = []
    a = t0 + 100.0
    while a + 200.0 <= t1 - 100.0 + 1e-9:
        win = [h for h in hops if a <= h["processMs"] < a + 200.0]
        if len(win) >= 3:
            xs = [h["processMs"] / 1000.0 for h in win]
            ys = [h["pubTotalBeatCount"] + h["pubBeatPhase"] for h in win]
            ratios.append(lsq_slope(xs, ys) / 2.0)
        a += 200.0
    return t0, t1, ratios


def row_r3(w):
    http("POST", "/api/set_bpm", {"bpm": 120})
    set_sync(0)
    wait_settled(0)
    time.sleep(2.0)
    out = {}
    for up, target in ((True, 500), (False, 0)):
        a0 = app_ms(w)
        set_sync(target)
        time.sleep(2.6)
        wait_settled(target)
        a1 = app_ms(w)
        hops = sorted(w.hops_between(a0, a1), key=lambda e: e["i"])
        r = slew_windows(hops, up)
        name = "0 -> 500" if up else "500 -> 0"
        if r is None:
            no("R3 %s: the slew was not found in the witness" % name)
            continue
        t0, t1, ratios = r
        lo, hi = (0.70, 0.80) if up else (1.20, 1.30)
        out[name] = {"durationS": (t1 - t0) / 1000.0, "ratios": ratios}
        (ok if ratios and all(lo <= x <= hi for x in ratios) else no)(
            "R3 %s: %d windows, published beat rate / 2.0 in [%s] (bar [%.2f, %.2f])"
            % (name, len(ratios), ", ".join("%.3f" % x for x in ratios), lo, hi))
        (ok if abs((t1 - t0) / 1000.0 - 2.0) <= 0.05 else no)(
            "R3 %s: appliedMs moved in %.3f s (bar 2.00 +- 0.05)" % (name, (t1 - t0) / 1000.0))
    RESULTS["rows"]["R3"] = out
    return out


def row_r9(w):
    http("POST", "/api/set_bpm", {"bpm": 120})
    res = []
    for d_ms in (100, 253):
        set_sync(d_ms)
        wait_settled(d_ms)
        time.sleep(1.0)
        for rep in range(3):
            w.poll_once()
            with w.lock:
                last = w.hops[max(w.hops)] if w.hops else None
            pre_seq = last["trackerRequestSeq"] if last else 0
            st, d = http("POST", "/api/resync")
            t_ret = app_ms(w)
            time.sleep(1.0)
            w.poll_once()
            with w.lock:
                cand = [w.hops[k] for k in sorted(w.hops) if w.hops[k]["trackerRequestSeq"] > pre_seq
                        and w.hops[k]["processMs"] >= t_ret - 200.0]
            if st != 200 or not cand:
                no("R9 D=%d rep %d: no hop carried the Resync's request sequence (POST %s)" % (d_ms, rep, st))
                continue
            h = cand[0]
            lag = h["processMs"] - t_ret
            resync = (h["pubBeatInBar"] == 0 and h["pubBeatPhase"] == 0.0
                      and h["pubResyncBarOrigin"] == h["pubTotalBarCount"])
            res.append({"D": d_ms, "lagMs": lag, "hop": h})
            (ok if resync and lag <= 50.0 else no)(
                "R9 D=%d rep %d: the Resync published %.1f ms after the POST returned (bar <= 50; beatInBar %d, "
                "beatPhase %.4f, origin %d == bar %d)" % (d_ms, rep, lag, h["pubBeatInBar"], h["pubBeatPhase"],
                                                          h["pubResyncBarOrigin"], h["pubTotalBarCount"]))
            time.sleep(2.2)
    RESULTS["rows"]["R9"] = res
    off, rtt = w.app_now_offset()
    info("R9 clock alignment through the witness nowMs: best RTT %.2f ms (+- %.2f ms)" % (rtt or -1, (rtt or 0) / 2))


# ------------------------------------------------------------------------------------------------ stage S2 (EARLY)
HOP_S = 512.0 / 48000.0


def a_n(h):
    """The tracker's per-hop phase step at this hop's bpm (beats)."""
    return HOP_S * max(h.get("bpm", 0.0), 0.0) / 60.0


def pub_fold(h):
    if h.get("barsAdvance"):
        return 4.0 * h["pubTotalBarCount"] + h["pubBeatInBar"] + h["pubBeatPhase"]
    return h["pubTotalBeatCount"] + h["pubBeatPhase"]


def raw_fold(h):
    if h.get("barsAdvance"):
        return 4.0 * h["rawTotalBarCount"] + h["rawBeatInBar"] + h["rawBeatPhase"]
    return h["rawTotalBeatCount"] + h["rawBeatPhase"]


def missing_hops(hops):
    """Published hops the witness never delivered (timestamps advance 512 per published hop)."""
    return sum(max(0, (b["timestamp"] - a["timestamp"]) // 512 - 1) for a, b in zip(hops, hops[1:]))


def early_hop_bars(hops, name):
    """R4's every-hop bars: counters never decrease, the level contract, the non-beat CRC; returns the fail count."""
    fails = 0
    if not hops:
        no("%s: no witness hops" % name)
        return 1
    if "nonBeatCrcBefore" not in hops[0]:
        no("%s: the witness carries no BeatLead fields (stage S2 not in this build)" % name)
        return 1
    miss = missing_hops(hops)
    (ok if miss == 0 else no)("%s: %d consecutive published hops witnessed, %d missing" % (name, len(hops), miss))
    fails += miss != 0
    dec = [(a["i"], b["i"]) for a, b in zip(hops, hops[1:])
           if b["pubTotalBeatCount"] < a["pubTotalBeatCount"] or b["pubTotalBarCount"] < a["pubTotalBarCount"]]
    (ok if not dec else no)("%s: published totalBeatCount / totalBarCount never decrease (%d decreases%s)"
                            % (name, len(dec), (", first at hop %s" % (dec[0],)) if dec else ""))
    fails += bool(dec)
    lvl = []
    for a, b in zip(hops, hops[1:]):
        if not b.get("barsAdvance"):
            continue
        if b["pubTotalBarCount"] > a["pubTotalBarCount"] and not (b["pubDownbeat"] and not a["pubDownbeat"]):
            lvl.append("bar +1 without the level rising at hop %d" % b["i"])
        if b["pubDownbeat"] and b["pubBeatInBar"] != 0:
            lvl.append("level true on beat %d at hop %d" % (b["pubBeatInBar"], b["i"]))
        if a["pubDownbeat"] and b["pubBeatInBar"] == 0 and not b["pubDownbeat"]:
            lvl.append("level dropped inside the first beat at hop %d" % b["i"])
    (ok if not lvl else no)("%s: level contract on every hop (%d violations%s)"
                            % (name, len(lvl), (": " + lvl[0]) if lvl else ""))
    fails += bool(lvl)
    crc = [h["i"] for h in hops if h["nonBeatCrcBefore"] != h["nonBeatCrcAfter"]]
    (ok if not crc else no)("%s: nonBeatCrcBefore == nonBeatCrcAfter on all %d hops (%d differ)" % (name, len(hops), len(crc)))
    fails += bool(crc)
    return fails


def slope_ratio_windows(hops, fold_fn, t0, t1, rate):
    """Least-squares slope of fold_fn(hop) vs processMs over every 200 ms window in [t0 + 100, t1 - 100], / rate."""
    out = []
    a = t0 + 100.0
    while a + 200.0 <= t1 - 100.0 + 1e-9:
        win = [h for h in hops if a <= h["processMs"] < a + 200.0]
        if len(win) >= 3:
            out.append(lsq_slope([h["processMs"] / 1000.0 for h in win], [fold_fn(h) for h in win]) / rate)
        a += 200.0
    return out


def early_slew(w, target, start_pred, end_pred, lo, hi, name):
    a0 = app_ms(w)
    set_sync(target)
    time.sleep(2.6)
    if wait_settled(target) is None:
        no("R4 %s: applied never settled at %d" % (name, target))
        return
    time.sleep(0.5)
    hops = sorted(w.hops_between(a0, app_ms(w)), key=lambda e: e["i"])
    start = next((h for h in hops if start_pred(h["appliedMs"])), None)
    end = next((h for h in hops if start and h["i"] > start["i"] and end_pred(h["appliedMs"])), None)
    if start is None or end is None:
        no("R4 %s: the slew was not found in the witness" % name)
        return
    ratios = slope_ratio_windows(hops, lambda h: h["pubTotalBeatCount"] + h["pubBeatPhase"],
                                 start["processMs"], end["processMs"], 2.0)
    (ok if ratios and all(lo <= x <= hi for x in ratios) else no)(
        "R4 %s: %d windows, published beat rate / 2.0 in [%s] (bar [%.2f, %.2f]); appliedMs moved in %.3f s (INFO)"
        % (name, len(ratios), ", ".join("%.3f" % x for x in ratios), lo, hi,
           (end["processMs"] - start["processMs"]) / 1000.0))


def row_r4(w):
    """Returns {"zero": (a0, a1, frameTimeMs), "m500": (a0, a1, frameTimeMs)}: the app-clock windows of the settled 0
    step and the settled -500 window (R1a over -500, G6), or None when the row stopped early."""
    http("POST", "/api/set_bpm", {"bpm": 120})
    set_sync(0)
    wait_settled(0)
    z0 = app_ms(w)
    time.sleep(2.0)
    wins = {"zero": (z0, app_ms(w), frame_time_ms())}
    out = {}
    # settled at -100: the beat fields lead by 0.2 beat on every hop
    set_sync(-100)
    if wait_settled(-100) is None:
        no("R4 POST /api/sync/set -100: applied never settled at -100 (GET /api/sync %s)" % str(get_sync())[:160])
        return None
    time.sleep(1.0)
    a0 = app_ms(w)
    time.sleep(3.0)
    hops = sorted(w.hops_between(a0, app_ms(w)), key=lambda e: e["i"])
    d = [((h["pubBeatPhase"] - h["rawBeatPhase"]) % 1.0) for h in hops]
    bad = [x for x in d if abs(x - 0.2) > 0.005]
    out["at-100"] = {"hops": len(hops), "min": min(d) if d else None, "max": max(d) if d else None}
    (ok if hops and not bad else no)(
        "R4 settled at -100: (published - raw) beatPhase mod 1 = 0.200 +- 0.005 on every hop (%d hops, range %s, %d out)"
        % (len(hops), ("[%.4f, %.4f]" % (min(d), max(d))) if d else "-", len(bad)))
    # 0 -> -500 (the beat runs 1.25x while the lead grows), settled at -500, -500 -> 0 (0.75x)
    set_sync(0)
    wait_settled(0)
    time.sleep(1.5)
    early_slew(w, -500, lambda v: v < 0.0, lambda v: v <= -500.0, 1.20, 1.30, "0 -> -500")
    time.sleep(1.0)
    a0 = app_ms(w)
    time.sleep(3.0)
    wins["m500"] = (a0, app_ms(w), frame_time_ms())
    hops = sorted(w.hops_between(wins["m500"][0], wins["m500"][1]), key=lambda e: e["i"])
    badc = [h["i"] for h in hops if h["pubTotalBeatCount"] - h["rawTotalBeatCount"] != 1
            or abs(h["pubBeatPhase"] - h["rawBeatPhase"]) > 0.005]
    (ok if hops and not badc else no)(
        "R4 settled at -500: published totalBeatCount - raw = 1 and phases equal +- 0.005 on every hop (%d hops, %d out)"
        % (len(hops), len(badc)))
    early_slew(w, 0, lambda v: v > -500.0, lambda v: v >= 0.0, 0.70, 0.80, "-500 -> 0")
    # the scripted run: -100 -> Resync -> -500 -> Resync -> 0 -> +100
    time.sleep(1.0)
    a0 = app_ms(w)
    for step in (-100, "resync", -500, "resync", 0, 100):
        if step == "resync":
            http("POST", "/api/resync")
            time.sleep(3.0)
        else:
            set_sync(step)
            time.sleep(2.6)
            if wait_settled(step) is None:
                no("R4 scripted run: applied never settled at %d" % step)
            time.sleep(2.0)
    time.sleep(0.5)
    hops = sorted(w.hops_between(a0, app_ms(w)), key=lambda e: e["i"])
    early_hop_bars(hops, "R4 scripted run")
    if hops:
        pub = hops[-1]["pubTotalBarCount"] - hops[0]["pubTotalBarCount"]
        raw = hops[-1]["rawTotalBarCount"] - hops[0]["rawTotalBarCount"]
        (ok if 0 <= pub - raw <= 2 else no)(
            "R4 scripted run: published - raw bar increments = %d - %d = %d (bar [0, 2])" % (pub, raw, pub - raw))
    out["windows"] = wins
    RESULTS["rows"]["R4"] = out
    return wins


def row_r4b(w):
    http("POST", "/api/set_bpm", {"bpm": 120})
    set_sync(-500)
    if wait_settled(-500) is None:
        no("R4b POST /api/sync/set -500: applied never settled at -500 (GET /api/sync %s)" % str(get_sync())[:160])
        return
    time.sleep(1.5)
    prev = 120.0
    out = []
    for bpm in (174.0, 87.0, 174.0, 200.0, 60.0, 200.0):
        a0 = app_ms(w)
        http("POST", "/api/set_bpm", {"bpm": bpm})
        time.sleep(20.0)
        hops = sorted(w.hops_between(a0, app_ms(w)), key=lambda e: e["i"])
        name = "R4b %g -> %g" % (prev, bpm)
        if not hops or "held" not in hops[0]:
            no("%s: no witness hops with BeatLead fields" % name)
            prev = bpm
            continue
        held = sum(1 for h in hops if h["held"])
        (ok if held == 0 else no)("%s: held == false on every hop (%d of %d held)" % (name, held, len(hops)))
        step_i = next((k for k, h in enumerate(hops) if abs(h["bpm"] - bpm) < 0.01), None)
        if step_i is None:
            no("%s: the tempo never reached the witness" % name)
            prev = bpm
            continue
        want = 0.5 * bpm / 60.0
        conv = next((k - step_i for k in range(step_i, len(hops)) if abs(hops[k]["leadBeats"] - want) <= 0.001), None)
        aN = HOP_S * bpm / 60.0
        bound = math.ceil(abs(want - 0.5 * prev / 60.0) / (0.25 * aN)) + 2
        (ok if conv is not None and conv <= bound else no)(
            "%s: leadBeats reached %.4f +- 0.001 after %s hops (bar <= %d)" % (name, want, conv, bound))
        t0, t1 = hops[0]["processMs"], hops[-1]["processMs"]
        pubs = slope_ratio_windows(hops, lambda h: h["pubTotalBeatCount"] + h["pubBeatPhase"], t0 - 100.0, t1 + 100.0, 1.0)
        raws = slope_ratio_windows(hops, lambda h: h["rawTotalBeatCount"] + h["rawBeatPhase"], t0 - 100.0, t1 + 100.0, 1.0)
        ratios = [p / r for p, r in zip(pubs, raws) if r > 0]
        lo, hi = (min(ratios), max(ratios)) if ratios else (float("nan"), float("nan"))
        (ok if ratios and all(0.70 <= x <= 1.30 for x in ratios) else no)(
            "%s: published / raw slope per 200 ms window in [%.3f, %.3f] over %d windows (bar [0.70, 1.30])"
            % (name, lo, hi, len(ratios)))
        out.append({"from": prev, "to": bpm, "held": held, "convergeHops": conv, "bound": bound, "ratioMin": lo,
                    "ratioMax": hi})
        prev = bpm
    RESULTS["rows"]["R4b"] = out


def row_r5(w, wav60, fixture, outdir):
    """EARLY in AUTO mode with the click track. Must run before any set_bpm of the session (manual mode has no REST off
    switch)."""
    st, d = http("POST", "/api/load_composition", {"path": fixture}, timeout=10)
    info("R5 load_composition %s: %s" % (fixture, str(d)[:120]))
    time.sleep(1.0)
    st, d = http("POST", "/api/render_frame", {"output_path": os.path.join(outdir, "attach-r5.png")}, timeout=15)
    (ok if st == 200 else no)("R5 render_frame %s (GL attached -- preview visible)" % st)
    arms = [0, -100, 0, -500, 0]
    seconds = 61.0
    if os.environ.get("PROBESYNC_R5_ARMS"):       # a development subset only; the gate runs the full list x 60 s
        arms = [int(x) for x in os.environ["PROBESYNC_R5_ARMS"].split(",")]
    if os.environ.get("PROBESYNC_R5_SECONDS"):
        seconds = float(os.environ["PROBESYNC_R5_SECONDS"])
    tag = subset_tag("R5", arms != [0, -100, 0, -500, 0] or seconds != 61.0)
    if tag != "R5":
        info("R5 DEVELOPMENT SUBSET (arms %s, %.0f s each): not a gate line" % (arms, seconds))
    res, all_hops = [], []
    for k, d_ms in enumerate(arms):
        set_sync(d_ms)
        if wait_settled(d_ms) is None:
            no("R5 arm %d (%d ms): applied never settled (GET /api/sync %s)" % (k, d_ms, str(get_sync())[:160]))
            return None
        time.sleep(0.5)
        f0 = http("GET", "/api/features")[1]
        s0 = http("GET", "/api/status")[1]
        a0 = app_ms(w)
        st, r = http("POST", "/api/perf/record", {"name": "probesync-%s-e%d" % (RUNID, k), "audio": False,
                                                  "audioFile": wav60})
        if st != 200 or not (isinstance(r, dict) and r.get("ok")):
            no("R5 arm %d: perf/record refused (%s %s)" % (k, st, str(r)[:160]))
            return None
        time.sleep(seconds)
        http("POST", "/api/perf/stop")
        time.sleep(1.5)
        a1 = app_ms(w)
        f1 = http("GET", "/api/features")[1]
        s1 = http("GET", "/api/status")[1]
        hops = sorted(w.hops_between(a0, a1), key=lambda e: e["i"])
        all_hops.extend(hops)
        first_hop_for, prev = {}, None
        for h in hops:
            if prev is not None and h["onsetCount"] > prev["onsetCount"]:
                first_hop_for.setdefault(h["onsetCount"], h)
            prev = h
        with w.lock:
            pulses = [p for p in w.pulses.values() if a0 <= p["frameMs"] <= a1]
        lags = [p["frameMs"] - first_hop_for[p["onsetCount"]]["stampMs"] for p in pulses if p["onsetCount"] in first_hop_for]
        med = statistics.median(lags) if lags else float("nan")
        bpm = statistics.median([h["bpm"] for h in hops]) if hops else 0.0
        res.append({"D": d_ms, "median": med, "n": len(lags), "onsetDelta": f1["onsetCount"] - f0["onsetCount"],
                    "pulseDelta": s1["renderOnsetPulses"] - s0["renderOnsetPulses"], "bpm": bpm, "hops": len(hops)})
        info("R5 arm %d (%d ms): median lag %.2f ms over %d paired onsets; onsetCount delta %d, render pulse delta %d; "
             "median bpm %.2f; %d hops" % (k, d_ms, med, len(lags), res[-1]["onsetDelta"], res[-1]["pulseDelta"], bpm,
                                          len(hops)))
        early_hop_bars(hops, "%s arm %d (%d ms)" % (tag, k, d_ms))
    import glob
    import shutil
    mine = glob.glob(os.path.join(TAKES_DIR, "probesync-%s-e*.adna-take" % RUNID))
    for t in mine:
        shutil.rmtree(t, ignore_errors=True)
    left = glob.glob(os.path.join(TAKES_DIR, "probesync-%s-e*.adna-take" % RUNID))
    (ok if not left else no)("R5 the probe's %d takes were removed (probesync-%s-e*)" % (len(mine), RUNID))
    for i, r in enumerate(res):
        if r["D"] < 0:
            neigh = [res[j]["median"] for j in (i - 1, i + 1) if 0 <= j < len(res) and res[j]["D"] == 0]
            if not neigh:
                no("R5 D=%d: no neighbouring 0 arm (PROBESYNC_R5_ARMS)" % r["D"])
                continue
            shift = r["median"] - statistics.fmean(neigh)
            (ok if abs(shift) <= 8.0 else no)(
                "%s D=%d (AUTO): median onset lag - neighbouring 0 arms = %.2f ms (bar |.| <= 8)" % (tag, r["D"], shift))
    # the holds, on every hop BeatLead ran (all arms: a 0 arm may still be catching up)
    hops = sorted({h["i"]: h for h in all_hops}.values(), key=lambda e: e["i"])
    spurious, stale, slow, applied, held, runs, longest, run = [], [], [], 0, 0, 0, 0, 0
    qualifying = 0   # D6: the hops that met the "did not move back" antecedent (free-running hops)
    for a, b in zip([None] + hops, hops):
        if not b.get("leadApplied"):
            if run:
                runs += 1
                longest = max(longest, run)
                run = 0
            continue
        applied += 1
        if b["held"]:
            held += 1
            run += 1
            if not (b["deficitBeats"] > 0.0 or b["targetBeatCount"] < b["pubTotalBeatCount"]
                    or b["targetBarCount"] < b["pubTotalBarCount"]):
                spurious.append(b["i"])
        else:
            if run:
                runs += 1
                longest = max(longest, run)
                run = 0
            if abs(pub_fold(b) - b["targetFold"]) > 1e-4:
                stale.append(b["i"])
        if (a is not None and a.get("leadApplied") and b["i"] == a["i"] + 1 and not b["rebase"]
                and b.get("barsAdvance") == a.get("barsAdvance") and b["bpm"] > 0):
            aN = a_n(b)
            raw_adv = raw_fold(b) - raw_fold(a)
            if raw_adv >= aN - 1e-4:
                qualifying += 1
                t_adv = b["targetFold"] - a["targetFold"]
                if t_adv < 0.75 * aN - 1e-4 - max(0.0, aN - raw_adv):
                    slow.append(b["i"])
    if run:
        runs += 1
        longest = max(longest, run)
    (ok if applied > 0 and not spurious else no)(
        "%s held => deficitBeats > 0 or a counter would decrease (%d held of %d hops BeatLead ran; %d spurious%s)"
        % (tag, held, applied, len(spurious), (", first hop %d" % spurious[0]) if spurious else ""))
    (ok if applied > 0 and not stale else no)(
        "%s not held => published fold == target fold +- 1e-4 beat (%d stale%s)"
        % (tag, len(stale), (", first hop %d" % stale[0]) if stale else ""))
    (ok if applied > 0 and not slow else no)(
        "%s where the raw fold did not move back, the target fold advanced >= 0.75 x aN - 1e-4 (%d short%s)"
        % (tag, len(slow), (", first hop %d" % slow[0]) if slow else ""))
    # D6: the clause above cannot pass empty
    (ok if qualifying >= 5000 and 2 * qualifying >= applied else no)(
        "%s free-running hops (raw fold advanced >= aN - 1e-4 on a non-re-base hop): %d of the %d hops BeatLead ran "
        "(bar >= 5,000 and >= half = %d)" % (tag, qualifying, applied, (applied + 1) // 2))
    info("R5 holds: %.2f %% of the %d hops BeatLead ran held; %d runs; longest %d hops (%.0f ms)"
         % (100.0 * held / applied if applied else 0.0, applied, runs, longest, longest * HOP_S * 1000.0))
    RESULTS["rows"]["R5"] = {"arms": res, "applied": applied, "held": held, "runs": runs, "longestHops": longest,
                             "qualifying": qualifying}
    return res


AUDIO_DIR = os.path.expanduser("~/Documents/Audio-DNA/Audio")


def t2_errors(take, rate_hint=48000.0, wav_interval=24000, wav_rate=48000):
    """probe-step3's T2 math on a saved take: (offsets ms of the paired markers, raw markers, n_dupes, n_spurious,
    the segment, the index in the raw marker list of each paired marker). The click grid is wav_interval frames of a
    wav_rate WAV = 0.5 s; in the asset (device-rate) frames it is 0.5 s x the segment's rate."""
    seg = take["audio"]["segments"][0]
    rate = float(seg.get("rate", rate_hint)) or rate_hint
    first = int(seg.get("firstSample", 0))
    interval = wav_interval * rate / wav_rate
    raw = [m for m in take.get("markers", []) if m.get("action") == "onset"]
    seen, markers, dupes = set(), [], 0
    for idx, m in enumerate(raw):
        if m["sample"] in seen:
            dupes += 1
            continue
        seen.add(m["sample"])
        markers.append((idx, m))
    offs, idxs, spurious = [], [], 0
    for idx, m in markers:
        f = m["sample"] - first
        d = f - round(f / interval) * interval
        if abs(d) > interval / 2:
            spurious += 1
            continue
        offs.append(d * 1000.0 / rate)
        idxs.append(idx)
    return offs, raw, dupes, spurious, seg, idxs


R7_TRIM_MS = 1024.0 * 1000.0 / 48000.0   # 21.333 ms (x the take's segment rate = 1,024 samples at 48 kHz)
R7_BAR_MS = 1.0
R7_SE_LIMIT_MS = 0.33


def subset_tag(row, subset, detail=""):
    """The name a row's verdict lines start with: a development subset (fewer arms or seconds than the gate's) says so
    on EVERY verdict line, so a short run's PASS cannot be copied into a gate record."""
    return "%s SUBSET (%sdevelopment: not a gate line)" % (row, detail) if subset else row


def r7_trim(offs, idxs=None):
    """D3 per-arm statistic: (trimmed mean ms, median ms, mean ms, [raw index of each dropped marker])."""
    if not offs:
        return float("nan"), float("nan"), float("nan"), []
    med = statistics.median(offs)
    keep = [o for o in offs if abs(o - med) <= R7_TRIM_MS]
    dropped = [(idxs[j] if idxs else j) for j, o in enumerate(offs) if abs(o - med) > R7_TRIM_MS]
    return (statistics.fmean(keep) if keep else float("nan")), med, statistics.fmean(offs), dropped


def r7_verdict(arms, pooled=False, subset=False):
    """D3. arms: [{"D", "value" (trimmed mean ms), "n", "drops"}] in run order. Prints the verdict line(s); returns
    {"dbar", "se", "valid", "verdict"} with verdict in PASS / FAIL / UNDECIDED / INCONCLUSIVE / INVALID."""
    H = [a["value"] for a in arms if a["D"] > 0]
    Z = [a["value"] for a in arms if a["D"] == 0]
    tag = "R7 (pooled %d arms)" % len(arms) if pooled else subset_tag("R7", subset, "%d arms, " % len(arms))
    if not H or not Z:
        no("%s: needs at least one 100 arm and one 0 arm (arms %s)" % (tag, [a["D"] for a in arms]))
        return {"verdict": "INVALID"}
    invalid = [k for k, a in enumerate(arms) if a["n"] < 100 or a["drops"] > 4]
    dbar = statistics.fmean(H) - statistics.fmean(Z)
    se = (math.sqrt(statistics.variance(H) / len(H) + statistics.variance(Z) / len(Z))
          if len(H) >= 2 and len(Z) >= 2 else float("nan"))
    out = {"dbar": dbar, "se": se, "valid": not invalid, "nH": len(H), "nZ": len(Z)}
    line = ("%s: dbar = mean of the %d 100-arm values - mean of the %d 0-arm values = %+.3f ms; SE %s ms (bar |dbar| "
            "<= %.1f)" % (tag, len(H), len(Z), dbar, ("%.3f" % se) if se == se else "n/a (< 2 arms in a group)",
                         R7_BAR_MS))
    if invalid:
        no("%s -- INVALID: arm(s) %s pair < 100 markers or drop > 4" % (line, invalid))
        out["verdict"] = "INVALID"
        return out
    if se != se:   # a development subset: no SE, so no extension rule -- the bar alone
        (ok if abs(dbar) <= R7_BAR_MS else no)(line)
        out["verdict"] = "PASS" if abs(dbar) <= R7_BAR_MS else "FAIL"
        return out
    near = abs(abs(dbar) - R7_BAR_MS) <= 2.0 * se
    if not pooled and (near or se > R7_SE_LIMIT_MS):
        no("%s -- R7 UNDECIDED: %s. Run the ONE extension: the same 21 arms once more with PROBESYNC_R7_EXTEND=1 "
           "PROBESYNC_R7_PRIOR=<this run's probe-sync.json>; the pooled 42 arms decide"
           % (line, "|dbar| is within 2 x SE of 1.0" if near else "SE > %.2f ms" % R7_SE_LIMIT_MS))
        out["verdict"] = "UNDECIDED"
        return out
    if pooled and se > R7_SE_LIMIT_MS:
        no("%s -- R7 INCONCLUSIVE: the pooled SE is still above %.2f ms. Blocks; to the architect" % (line, R7_SE_LIMIT_MS))
        out["verdict"] = "INCONCLUSIVE"
        return out
    if abs(dbar) <= R7_BAR_MS:
        ok(line)
        out["verdict"] = "PASS"
    else:
        no(line + (" -- |dbar| < 2.67 ms: a finding for the architect, never re-run" if abs(dbar) < 2.67 else ""))
        out["verdict"] = "FAIL"
    return out


G6_BUDGET_US = 2000.0
G6_POINT_US = 106.67   # 1 DSP-load point = 1 % of the 10,667 us hop period


def g6_stats(hops):
    v = sorted(h["pipelineUs"] for h in hops)
    return {"n": len(v), "mean": statistics.fmean(v), "p99": v[min(len(v) - 1, int(math.ceil(0.99 * len(v))) - 1)],
            "max": v[-1]}


def g6_verdict(windows):
    """D2 / section 5 G6. windows: {"r1_zero": [hop lists, one per settled R1 0 arm, in run order], "r1_500": hops,
    "r1_500_neigh": (index, index) into r1_zero of the 500 arm's two neighbouring 0 arms, "r4_zero": hops,
    "r4_m500": hops, "frameTimeMs": {name: value}}. Pure (no HTTP): also run offline on a saved probe-sync.json."""
    named = [("R1 0 arm %d" % k, h) for k, h in enumerate(windows.get("r1_zero", []))]
    named += [("R1 500 arm", windows.get("r1_500") or []), ("R4 0 step", windows.get("r4_zero") or []),
              ("R4 -500 window", windows.get("r4_m500") or [])]
    if any(h and "pipelineUs" not in h[0] for _, h in named):
        no("G6 the witness hops carry no pipelineUs (a build before ruling-bf2-delta D2)")
        return None
    short = [n for n, h in named if len(h) < 150]
    if short:
        no("G6 window(s) with fewer than 150 hops: %s" % ", ".join("%s (%d)" % (n, len(dict(named)[n])) for n in short))
        return None
    st = {n: g6_stats(h) for n, h in named}
    ft = windows.get("frameTimeMs", {})
    for n, _ in named:
        info("G6 %-15s %4d hops: pipelineUs mean %.1f, p99 %.1f, max %.1f (%.2f DSP points mean); frameTimeMs %s"
             % (n, st[n]["n"], st[n]["mean"], st[n]["p99"], st[n]["max"], st[n]["mean"] / G6_POINT_US, ft.get(n)))
    m500 = windows["r4_m500"]
    led = sum(1 for h in m500 if h.get("leadApplied"))
    (ok if led >= 0.99 * len(m500) else no)(
        "G6(1) -500 window: leadApplied on %d of %d hops = %.2f %% (bar >= 99 %%)" % (led, len(m500), 100.0 * led / len(m500)))
    zero_names = [n for n, _ in named if n.startswith("R1 0 arm") or n == "R4 0 step"]
    over = [n for n in zero_names if st[n]["mean"] > G6_BUDGET_US]
    d_m500 = st["R4 -500 window"]["mean"] - st["R4 0 step"]["mean"]
    i, j = windows.get("r1_500_neigh", (None, None))
    neigh = [st["R1 0 arm %d" % k]["mean"] for k in (i, j) if k is not None]
    d_p500 = st["R1 500 arm"]["mean"] - statistics.fmean(neigh) if neigh else float("nan")
    if not over:
        for n, _ in named:
            (ok if st[n]["mean"] <= G6_BUDGET_US else no)(
                "G6(2) %s: mean pipelineUs %.1f <= %.0f (the analysis budget: under 2 ms per hop)"
                % (n, st[n]["mean"], G6_BUDGET_US))
    else:
        info("G6 OVER-BUDGET BASELINE (to the architect): 0 window(s) above %.0f us: %s -- bar (2) is replaced by the "
             "pre-stated relative bar" % (G6_BUDGET_US, ", ".join("%s %.1f" % (n, st[n]["mean"]) for n in over)))
        (ok if d_m500 <= 300.0 else no)(
            "G6(2-rel) mean(-500) - mean(R4's 0 step) = %+.1f us (bar <= +300)" % d_m500)
        if len(neigh) == 2 and abs(neigh[0] - neigh[1]) < 100.0:
            (ok if d_p500 <= 300.0 else no)(
                "G6(2-rel) mean(+500) - mean(its two neighbouring R1 0 arms) = %+.1f us (bar <= +300)" % d_p500)
        else:
            no("G6 INCONCLUSIVE: the 500 arm's two neighbouring R1 0 arms differ by >= 100 us or are missing (%s) -- "
               "repeat on a quiet machine" % ", ".join("%.1f" % x for x in neigh))
    info("G6 mean(-500) - mean(R4's 0 step) = %+.1f us = %+.3f DSP points; mean(+500) - mean(R1's neighbouring 0 arms) "
         "= %+.1f us = %+.3f DSP points (the earlier ruled figure: 0.3 point = 32.0 us). Not a gate for the delay "
         "line: R1a is" % (d_m500, d_m500 / G6_POINT_US, d_p500, d_p500 / G6_POINT_US))
    return {"stats": st, "ledFraction": led / len(m500), "dMinus500Us": d_m500, "dPlus500Us": d_p500, "over": over}


def row_g6(w, r1, r4wins):
    if not r1 or not r4wins or "m500" not in r4wins:
        no("G6 needs R1 and R4 in the same run (their settled windows are its windows)")
        return
    zero_idx = [k for k, r in enumerate(r1) if r["D"] == 0]
    i500 = next((k for k, r in enumerate(r1) if r["D"] == 500), None)
    if i500 is None:
        no("G6 R1 has no 500 arm")
        return
    hopsof = lambda r: sorted(w.hops_between(r["appWindow"][0], r["appWindow"][1]), key=lambda e: e["i"])
    win = {"r1_zero": [hopsof(r1[k]) for k in zero_idx], "r1_500": hopsof(r1[i500]),
           "r1_500_neigh": tuple(zero_idx.index(k) if k in zero_idx else None for k in (i500 - 1, i500 + 1)),
           "r4_zero": sorted(w.hops_between(r4wins["zero"][0], r4wins["zero"][1]), key=lambda e: e["i"]),
           "r4_m500": sorted(w.hops_between(r4wins["m500"][0], r4wins["m500"][1]), key=lambda e: e["i"]),
           "frameTimeMs": {"R1 500 arm": r1[i500].get("frameTimeMs"), "R4 0 step": r4wins["zero"][2],
                           "R4 -500 window": r4wins["m500"][2]}}
    for n, k in enumerate(zero_idx):
        win["frameTimeMs"]["R1 0 arm %d" % n] = r1[k].get("frameTimeMs")
    res = g6_verdict(win)
    keep = ("i", "processMs", "appliedMs", "leadApplied", "pipelineUs")
    slim = lambda hs: [{k: h.get(k) for k in keep} for h in hs]
    RESULTS["rows"]["G6"] = {"result": res, "windows": {
        "r1_zero": [slim(h) for h in win["r1_zero"]], "r1_500": slim(win["r1_500"]),
        "r1_500_neigh": win["r1_500_neigh"], "r4_zero": slim(win["r4_zero"]), "r4_m500": slim(win["r4_m500"]),
        "frameTimeMs": win["frameTimeMs"]}}


def row_r7(w, wav60, fixture, outdir):
    """R7 (take alignment, D3) and R7b (gestures land in show time, D4; inside the FIRST 100 arm)."""
    import glob
    import shutil
    st, d = http("POST", "/api/load_composition", {"path": fixture}, timeout=10)
    info("R7 load_composition %s: %s" % (fixture, str(d)[:120]))
    time.sleep(1.0)
    st, d = http("POST", "/api/render_frame", {"output_path": os.path.join(outdir, "attach-r7.png")}, timeout=15)
    (ok if st == 200 else no)("R7 render_frame %s (GL attached -- preview visible)" % st)
    full = [0, 100] * 10 + [0]
    arms = list(full)
    if os.environ.get("PROBESYNC_R7_ARMS"):          # a RED / development subset only; the gate runs the full list
        arms = [int(x) for x in os.environ["PROBESYNC_R7_ARMS"].split(",")]
    subset = arms != full
    prior = None
    if os.environ.get("PROBESYNC_R7_EXTEND") == "1":
        try:
            prior = json.load(open(os.environ.get("PROBESYNC_R7_PRIOR", "")))["rows"]["R7"]["arms"]
        except (OSError, ValueError, KeyError) as e:
            no("R7 PROBESYNC_R7_EXTEND=1 needs PROBESYNC_R7_PRIOR=<the first run's probe-sync.json> (%s)" % e)
            return None
        if subset or [a["D"] for a in prior] != full:
            no("R7 the extension pools two FULL 21-arm passes (this run's arms %s; the prior run had %d arms)"
               % ("are a subset" if subset else "are full", len(prior)))
            return None
    st, dev = http("GET", "/api/debug/audio_devices")
    opened = dev.get("opened", {}) if isinstance(dev, dict) else {}
    info("R7 device: block size %s, rate %s (GET /api/debug/audio_devices opened)"
         % (opened.get("buffer_size"), opened.get("sample_rate")))
    res, assets, r7b_raw = [], [], None
    for k, d_ms in enumerate(arms):
        set_sync(d_ms)
        if wait_settled(d_ms) is None:
            no("R7 arm %d (%d ms): applied never settled (GET /api/sync %s)" % (k, d_ms, str(get_sync())[:160]))
            return None
        time.sleep(0.5)
        name = "probesync-%s-t%d" % (RUNID, k)
        st, r = http("POST", "/api/perf/record", {"name": name, "audio": True, "audioFile": wav60,
                                                  "onsetMarkers": True})
        if st != 200 or not (isinstance(r, dict) and r.get("ok")):
            no("R7 arm %d: perf/record refused (%s %s)" % (k, st, str(r)[:160]))
            return None
        t_start = time.monotonic()
        fired = []   # R7b: (k, column, markers seen when fired)
        first100 = d_ms == 100 and r7b_raw is None
        if first100:
            targets = [(10, 1), (20, 2), (30, 3)]
            while targets and time.monotonic() - t_start < 40.0:
                st, ps = http("GET", "/api/perf/status")
                seen = ps.get("markers", 0) if isinstance(ps, dict) else 0
                if seen >= targets[0][0]:
                    kk, col = targets.pop(0)
                    http("POST", "/api/trigger_clip", {"layer": 0, "column": col})
                    fired.append((kk, col, seen))
                else:
                    time.sleep(0.005)
        time.sleep(max(0.0, 61.0 - (time.monotonic() - t_start)))
        http("POST", "/api/perf/stop")
        time.sleep(3.0 + d_ms / 1000.0)
        tj = os.path.join(TAKES_DIR, name + ".adna-take", "take.json")
        try:
            take = json.load(open(tj))
        except (OSError, ValueError) as e:
            no("R7 arm %d: no saved take at %s (%s)" % (k, tj, e))
            return None
        offs, raw, dupes, spurious, seg, idxs = t2_errors(take)
        assets.append(seg.get("id", ""))
        rate = float(seg.get("rate", 48000.0))
        value, med, mean, dropped = r7_trim(offs, idxs)
        res.append({"D": d_ms, "value": value, "median": med, "mean": mean, "n": len(offs), "drops": len(dropped),
                    "dropped": dropped, "raw": len(raw), "dupes": dupes, "spurious": spurious, "rate": rate,
                    "frames": seg.get("frames"), "firstSample": int(seg.get("firstSample", 0)), "offsMs": offs})
        info("R7 arm %d (%d ms): trimmed mean %.3f ms (median %.3f, mean %.3f) over %d paired markers; dropped %d "
             "(raw marker index %s; 0 = the take's first marker); %d raw, %d dupes, %d off-grid; asset rate %.0f, %s "
             "frames" % (k, d_ms, value, med, mean, len(offs), len(dropped), dropped, len(raw), dupes, spurious, rate,
                         seg.get("frames")))
        if first100:
            gestures = []
            for lane in take.get("lanes", []):
                if lane.get("key", {}).get("control") == "activeClip":
                    gestures += [p for p in lane.get("points", []) if p.get("origin") == "human"]
            gestures.sort(key=lambda p: p["seq"])
            r7b_raw = {"fired": fired, "gestures": gestures, "raw": raw, "rate": rate,
                       "first": int(seg.get("firstSample", 0))}
    # ---- R7 (D3)
    for i, r in enumerate(res):
        if r["D"] > 0:
            neigh = [res[j]["value"] for j in (i - 1, i + 1) if 0 <= j < len(res) and res[j]["D"] == 0]
            if neigh:
                info("R7 arm %d (100) - the mean of its %d neighbouring 0 arm(s) = %+.3f ms (d_i, INFO)"
                     % (i, len(neigh), r["value"] - statistics.fmean(neigh)))
    allm = [round(o * x["rate"] / 1000.0) for x in res for o in x["offsMs"]]
    if len(allm) > 1:
        g = 0
        for v in allm:
            g = math.gcd(g, abs(v - allm[0]))
        info("R7 marker-error lattice: the paired errors differ by multiples of %d samples (%.3f ms)"
             % (g, g * 1000.0 / (res[0]["rate"] or 48000.0)))
    verdict = r7_verdict(res, pooled=False, subset=subset) if prior is None else None
    if prior is not None:
        info("R7 this pass alone (INFO): %s" % json.dumps({k: v for k, v in r7_stat_only(res).items()}))
        verdict = r7_verdict(prior + res, pooled=True)
    # ---- R7b (D4)
    r7b = None
    if r7b_raw is None:
        no("R7b no 100 arm in this run (PROBESYNC_R7_ARMS)")
    else:
        fired, gestures, raw, rate = r7b_raw["fired"], r7b_raw["gestures"], r7b_raw["raw"], r7b_raw["rate"]
        interval = 24000 * rate / 48000.0
        zero_errs = [o * x["rate"] / 1000.0 for x in res if x["D"] == 0 for o in x["offsMs"]]
        info("R7b fired %s (k, column, markers seen); %d Human activeClip points in the take" % (fired, len(gestures)))
        if len(fired) != 3 or len(gestures) != 3:
            no("R7b want 3 gestures fired and 3 found in the take (fired %d, in the take %d)" % (len(fired), len(gestures)))
        elif not zero_errs:
            no("R7b no 0 arm in this run: B (the median paired marker error over all 0 arms) is undefined")
        elif any(kk > len(raw) for kk, _, _ in fired):
            no("R7b the take has only %d markers" % len(raw))
        else:
            B = statistics.median(zero_errs)
            lo, hi = -0.032 * rate, 0.080 * rate
            r7b = {"B": B, "g": []}
            for (kk, col, seen), g in zip(fired, gestures):
                mk = raw[kk - 1]["sample"]
                grid = round((mk - r7b_raw["first"]) / interval) * interval
                gk = (g["sample"] - r7b_raw["first"]) - grid
                r7b["g"].append({"k": kk, "gk": gk, "gkMinusB": gk - B, "gesture": g["sample"], "marker": mk})
                info("R7b k=%d (column %d): g_k = %.0f samples, g_k - B = %+.0f samples (%+.2f ms); gesture stamp - "
                     "marker %d stamp = %d samples = %.2f ms (INFO; the earlier window [0, %.0f] samples = 0-60 ms)"
                     % (kk, col, gk, gk - B, (gk - B) * 1000.0 / rate, kk, g["sample"] - mk,
                        (g["sample"] - mk) * 1000.0 / rate, 0.060 * rate))
            vals = [x["gkMinusB"] for x in r7b["g"]]
            m = statistics.median(vals)
            (ok if lo <= m <= hi else no)(
                "%s the MEDIAN of the three (g_k - B) = %+.0f samples (%+.2f ms); all three: %s; B = %.0f samples over "
                "%d markers of the 0 arms (bar [%.0f, %+.0f] samples = [-32, +80] ms x %.0f Hz)"
                % (subset_tag("R7b", subset), m, m * 1000.0 / rate, ", ".join("%+.0f" % v for v in vals), B, len(zero_errs), lo, hi, rate))
    RESULTS["rows"]["R7"] = {"arms": res, "R7b": r7b, "verdict": verdict, "subset": subset, "pooled": prior is not None}
    mine = glob.glob(os.path.join(TAKES_DIR, "probesync-%s-t*.adna-take" % RUNID))
    for t in mine:
        shutil.rmtree(t, ignore_errors=True)
    gone = 0
    for a in assets:
        if a and len(a) == 32 and all(c in "0123456789abcdef" for c in a):
            shutil.rmtree(os.path.join(AUDIO_DIR, a + ".adna-audio"), ignore_errors=True)
            gone += not os.path.exists(os.path.join(AUDIO_DIR, a + ".adna-audio"))
    left = glob.glob(os.path.join(TAKES_DIR, "probesync-%s-t*.adna-take" % RUNID))
    (ok if not left and gone == len(assets) else no)(
        "R7 the probe's %d takes and %d of %d Audio-store assets were removed" % (len(mine), gone, len(assets)))
    return res


def r7_stat_only(arms):
    H = [a["value"] for a in arms if a["D"] > 0]
    Z = [a["value"] for a in arms if a["D"] == 0]
    return {"dbar": statistics.fmean(H) - statistics.fmean(Z) if H and Z else None,
            "se": math.sqrt(statistics.variance(H) / len(H) + statistics.variance(Z) / len(Z))
            if len(H) >= 2 and len(Z) >= 2 else None}


# ------------------------------------------------------------------------------------------------ RD (stage D0)
# The DIAGNOSIS run of ruling-bf2-stops.md (s-rta-1003b; A1..A5, section 4's decision table, section 5 row RD). Never a
# gate line. PROBESYNC_RD=1 replaces row R7 by row_rd (R7's take loop at dial 0, no R7 verdict, no R7b); row_r7 itself
# is untouched. The functions below are pure except row_rd and read_wav_ch0; probe-sync-selftest.py drives them.
RD_SPREAD = 4            # samples: a click farther than this from c0 is an anomaly (O4)
RD_ONE = 128             # samples: "one-valued" = the take medians span <= this
RD_TWO = 384             # samples: "two-valued" = they span >= this
RD_E1_NOMINAL = 1472     # samples: the normal state's e' at the pin (the six saved normal takes, ruling V7)
RD_MIN_VALID = 24        # valid takes on record before RD may read O5 (or O1 without the early-stop rule)
RD_MIN_CLICKS = 20       # clicks found in both files, and paired markers, a VALID take needs
RD_CLICK_INTERVAL = 24000   # frames of the 48 kHz click WAV between two clicks (gen-click-wav.py)


def read_wav_ch0(path, max_frames=None):
    """(channel-0 samples, rate, full scale) of a PCM / float WAV. Walks the RIFF chunks itself (the app's asset WAV
    carries a JUNK chunk before 'fmt '); 16 / 24 / 32-bit integer and 32-bit float data."""
    import array
    import struct
    with open(path, "rb") as f:
        head = f.read(12)
        if head[:4] not in (b"RIFF", b"RF64") or head[8:12] != b"WAVE":
            raise ValueError("%s is not a WAV" % path)
        fmt = None
        while True:
            ch = f.read(8)
            if len(ch) < 8:
                raise ValueError("%s has no data chunk" % path)
            cid, size = ch[:4], struct.unpack("<I", ch[4:])[0]
            if cid == b"fmt ":
                body = f.read(size + (size & 1))
                tag, nch, rate, _, _, bits = struct.unpack("<HHIIHH", body[:16])
                if tag == 0xFFFE and len(body) >= 26:
                    tag = struct.unpack("<H", body[24:26])[0]
                fmt = (tag, nch, rate, bits)
            elif cid == b"data":
                if fmt is None:
                    raise ValueError("%s: data before fmt" % path)
                tag, nch, rate, bits = fmt
                width = bits // 8
                want = size if max_frames is None else min(size, max_frames * nch * width)
                raw = f.read(want)
                raw = raw[:len(raw) - len(raw) % (nch * width)]
                if tag == 1 and bits == 16:
                    a = array.array("h")
                    a.frombytes(raw)
                    if sys.byteorder == "big":
                        a.byteswap()
                    return a[::nch], rate, 32768.0
                if tag == 1 and bits == 32:
                    a = array.array("i")
                    a.frombytes(raw)
                    if sys.byteorder == "big":
                        a.byteswap()
                    return a[::nch], rate, 2147483648.0
                if tag == 1 and bits == 24:
                    step = 3 * nch
                    return ([int.from_bytes(raw[k:k + 3], "little", signed=True) for k in range(0, len(raw), step)],
                            rate, 8388608.0)
                if tag == 3 and bits == 32:
                    a = array.array("f")
                    a.frombytes(raw)
                    if sys.byteorder == "big":
                        a.byteswap()
                    return a[::nch], rate, 1.0
                raise ValueError("%s: WAV format tag %d with %d bits is not read here" % (path, tag, bits))
            else:
                f.seek(size + (size & 1), 1)


def click_onsets(samples, full_scale, collapse=100):
    """The frame of each click's onset: the first sample whose magnitude is above half full scale; hits within
    `collapse` samples of the previous hit belong to the same click (R7's re-stated detector, ruling section 5)."""
    thr = full_scale / 2.0
    out, last = [], None
    for i, s in enumerate(samples):
        if s > thr or -s > thr:
            if last is None or i - last > collapse:
                out.append(i)
            last = i
    return out


def rd_click_origin(asset_onsets, source_onsets, asset_rate=48000.0, source_rate=48000.0):
    """The AUDIO ruler (A2). Each asset click is paired to the nearest source click (within half a click interval);
    o_k = asset onset - source onset (in asset frames). Returns (c0 = the median o_k, the largest |o_k - c0|, the
    number of clicks found in both files, the o_k) -- (None, None, 0, []) when no click pairs."""
    ratio = asset_rate / source_rate
    half = RD_CLICK_INTERVAL * ratio / 2.0
    src = [s * ratio for s in source_onsets]
    used, o = set(), []
    for a in asset_onsets:
        if not src:
            break
        k = min(range(len(src)), key=lambda j: abs(a - src[j]))
        if k in used or abs(a - src[k]) > half:
            continue
        used.add(k)
        d = a - src[k]
        o.append(int(d) if d == int(d) else d)
    if not o:
        return None, None, 0, []
    c0 = statistics.median(o)
    return c0, max(abs(x - c0) for x in o), len(o), o


def rd_witness_c(stamps, hops, min_pairs=RD_MIN_CLICKS, reach=3):
    """The WITNESS ruler (A2). stamps: the take's onset-marker stamps in order; hops: the witness hops of the take's
    window. A hop on which onsetCount rose by n stands for n onsets. Markers and count-rising hops are paired in
    order; the pairing offset d (-reach..reach) is the one that minimises the spread (population SD) of marker stamp -
    hop timestamp, the smaller |d| on a tie. Returns {"C" (the median difference or None), "n", "d", "sd", "cands"}."""
    hs = sorted(hops, key=lambda h: h["i"])
    rises = []
    for a, b in zip(hs, hs[1:]):
        rises += [b["timestamp"]] * max(0, int(b["onsetCount"]) - int(a["onsetCount"]))
    best, cands = None, []
    for d in sorted(range(-reach, reach + 1), key=lambda x: (abs(x), x < 0)):
        diffs = [stamps[j] - rises[j + d] for j in range(len(stamps)) if 0 <= j + d < len(rises)]
        if len(diffs) < min_pairs:
            continue
        sd = statistics.pstdev(diffs)
        cands.append([d, len(diffs), round(sd, 3), statistics.median(diffs)])
        if best is None or sd < best[0] - 1e-9:
            best = (sd, d, diffs)
    if best is None:
        return {"C": None, "n": 0, "d": None, "sd": None, "cands": cands, "rises": len(rises)}
    return {"C": statistics.median(best[2]), "n": len(best[2]), "d": best[1], "sd": round(best[0], 3), "cands": cands,
            "rises": len(rises), "diffs": best[2]}


def rd_take_record(i, take, asset, source, hops, dev, opens_before, settled=True):
    """One RD take's record from raw data only. take: the saved take.json; asset / source: (samples, rate, full
    scale) of the take's own audio asset and of the click WAV it played (None = unreadable); hops: the witness hops
    of the take's window; dev: GET /api/debug/audio_devices read after the take; opens_before: its "opens" before."""
    offs, raw, dupes, spurious, seg, _ = t2_errors(take)
    rate = float(seg.get("rate", 48000.0)) or 48000.0
    errs = [round(o * rate / 1000.0, 3) for o in offs]
    e = statistics.median(errs) if errs else None
    c0, spread, m, o = None, None, 0, []
    if asset is not None and source is not None:
        c0, spread, m, o = rd_click_origin(click_onsets(asset[0], asset[2]), click_onsets(source[0], source[2]),
                                           float(asset[1]), float(source[1]))
    wit = rd_witness_c([mk["sample"] for mk in raw], hops)
    opened = dev.get("opened", {}) if isinstance(dev, dict) else {}
    opens = dev.get("opens") if isinstance(dev, dict) else None
    return {"i": i, "settled": bool(settled), "c0": c0, "spread": spread, "clicks": m, "o": o,
            "e": e, "e1": (e - c0) if e is not None and c0 is not None else None, "n": len(offs), "errs": errs,
            "C": wit["C"], "witness": wit, "block": opened.get("buffer_size"), "rate": opened.get("sample_rate"),
            "opens": opens,
            "opensDelta": (opens - opens_before) if opens is not None and opens_before is not None else None,
            "gaps": len(take.get("audio", {}).get("gaps", []) or []),
            "hopsApplied": sum(1 for h in hops if h.get("appliedMs", 0) != 0), "hops": len(hops),
            "raw": len(raw), "dupes": dupes, "spurious": spurious, "segRate": rate, "frames": seg.get("frames"),
            "firstSample": int(seg.get("firstSample", 0)), "stamps": [mk["sample"] for mk in raw]}


def rd_num(x):
    if x is None:
        return "n/a"
    return "%d" % x if float(x) == int(x) else "%.1f" % x


def rd_take_line(t):
    q = (t["c0"] / t["block"]) if t.get("c0") is not None and t.get("block") else None
    d = t.get("opensDelta")
    return ("RD take %d: c0 %s samples = %s blocks (spread %s, %d clicks); e %s / e' %s samples over %d markers; C %s "
            "samples; block %s, rate %s, opens %s (%s); gaps %d; hops with applied != 0: %d"
            % (t["i"], rd_num(t.get("c0")), "n/a" if q is None else "%g" % q, rd_num(t.get("spread")), t.get("clicks") or 0,
               rd_num(t.get("e")), rd_num(t.get("e1")), t.get("n") or 0, rd_num(t.get("C")), rd_num(t.get("block")),
               rd_num(t.get("rate")), rd_num(t.get("opens")), "n/a" if d is None else "%+d" % d, t.get("gaps") or 0,
               t.get("hopsApplied") or 0))


def rd_span(v):
    v = [x for x in v if x is not None]
    return (max(v) - min(v)) if len(v) >= 2 else 0


def rd_classify(takes):
    """Marks each take of ONE launch (in take order): "deviceEvent" (A5: its "opens" rose, or its block or rate
    differs from the launch's first take) and "valid" (the decision table's VALID take), with "why" when not valid."""
    for t in takes:
        first = takes[0]
        t["deviceEvent"] = bool((t.get("opensDelta") or 0) > 0 or t.get("block") != first.get("block")
                                or t.get("rate") != first.get("rate"))
        why = []
        if not t.get("settled") or t.get("hopsApplied"):
            why.append("the dial was not settled at 0")
        if t.get("gaps"):
            why.append("%d gap(s)" % t["gaps"])
        if t["deviceEvent"]:
            why.append("a device event")
        if t.get("opensDelta") is None or not t.get("block") or not t.get("rate"):
            why.append("no device fields")
        if t.get("c0") is None or (t.get("clicks") or 0) < RD_MIN_CLICKS:
            why.append("%d clicks found in both files (< %d)" % (t.get("clicks") or 0, RD_MIN_CLICKS))
        if t.get("e") is None or (t.get("n") or 0) < RD_MIN_CLICKS:
            why.append("%d paired markers (< %d)" % (t.get("n") or 0, RD_MIN_CLICKS))
        t["why"] = why
        t["valid"] = not why
    return takes


def rd_launch_line(takes):
    rd_classify(takes)
    v = [t for t in takes if t["valid"]]
    return ("RD launch: %d valid takes; c0 = 0 in %d, >= 1 block in %d; e' span %s samples; C span %s samples"
            % (len(v), sum(1 for t in v if t["c0"] == 0), sum(1 for t in v if t["c0"] >= t["block"]),
               rd_num(rd_span([t["e1"] for t in v])), rd_num(rd_span([t["C"] for t in v]))))


def rd_verdict(launches, names=None):
    """Section 4's decision table over the saved takes of several launches ([[take record, ...], ...]). Tested in
    the ruling's order: O4 (anomaly), O2 (product), then -- when the run is long enough -- O1 (artefact, shown) or O5
    (not reproduced); otherwise INCOMPLETE. Prints the per-launch lines and the verdict line; returns {"outcome"}."""
    names = names or ["launch %d" % (k + 1) for k in range(len(launches))]
    for name, L in zip(names, launches):
        info("%s -- %s" % (rd_launch_line(L), name))
        for t in L:
            if not t["valid"]:
                info("RD %s take %d is NOT VALID (left out of the outcome): %s" % (name, t["i"], "; ".join(t["why"])))
    valid = [t for L in launches for t in L if t["valid"]]
    zero = [t for t in valid if t["c0"] == 0]
    shifted = [t for t in valid if t["c0"] >= t["block"]]
    s = rd_span([t["e1"] for t in valid])
    cspans = [rd_span([t["C"] for t in L if t["valid"]]) for L in launches]
    cs = max(cspans) if cspans else 0
    one = s <= RD_ONE
    eref = statistics.median([t["e1"] for t in valid]) if valid else None
    nmax = max([1] + [int(t["c0"] // t["block"]) for t in shifted])
    counts = ("%d valid takes in %d launches; c0 = 0 in %d, >= 1 block in %d; e' span %s; C span %s; NMAX %d; EREF %s"
              % (len(valid), len(launches), len(zero), len(shifted), rd_num(s), rd_num(cs), nmax, rd_num(eref)))
    met = len(shifted) >= 2 and len(zero) >= 2
    info("RD early-stop rule (A3 / HD9: 2 valid takes with c0 >= one block and 2 with c0 = 0): %s"
         % ("MET" if met else "not met"))
    out = {"valid": len(valid), "launches": len(launches), "zero": len(zero), "shifted": len(shifted), "e1Span": s,
           "cSpan": cs, "NMAX": nmax, "EREF": eref, "stopRule": met}

    def done(outcome, ruling):
        print(("RD outcome %s (%s)" % (outcome, counts)) if outcome != "INCOMPLETE" else "RD INCOMPLETE (%s)" % ruling,
              flush=True)
        if outcome != "INCOMPLETE":
            info("RD ruling for %s: %s" % (outcome, ruling))
        out["outcome"] = outcome
        return out

    # ---- O4 ANOMALY
    anomalies = []
    for name, L in zip(names, launches):
        vm = statistics.median([t["e1"] for t in valid]) if valid else None
        for t in L:
            if t["valid"] and t["spread"] > RD_SPREAD:
                anomalies.append("%s take %d: its clicks scatter %s samples about c0 (> %d)"
                                 % (name, t["i"], rd_num(t["spread"]), RD_SPREAD))
            if t["valid"] and (t["c0"] < 0 or t["c0"] % t["block"] != 0):
                anomalies.append("%s take %d: c0 %s is negative or not a whole multiple of the block %s"
                                 % (name, t["i"], rd_num(t["c0"]), rd_num(t["block"])))
            if t["deviceEvent"]:
                info("RD DEVICE-EVENT take (A5; left out of the outcome): %s take %d -- opens %s (%s), block %s, rate %s"
                     % (name, t["i"], rd_num(t.get("opens")),
                        "n/a" if t.get("opensDelta") is None else "%+d" % t["opensDelta"], rd_num(t.get("block")),
                        rd_num(t.get("rate"))))
                if ((t.get("c0") is not None and t.get("block") and t["c0"] >= t["block"])
                        or (vm is not None and t.get("e1") is not None and abs(t["e1"] - vm) >= RD_TWO)):
                    anomalies.append("%s take %d: a shifted take that is also a device-event take (to the architect)"
                                     % (name, t["i"]))
    if RD_ONE < s < RD_TWO:
        anomalies.append("e' spans %s samples: more than %d and less than %d" % (rd_num(s), RD_ONE, RD_TWO))
    if one:
        for name, L, c in zip(names, launches, cspans):
            if c > RD_ONE:
                anomalies.append("the rulers disagree: e' is one-valued but C spans %s samples inside %s (C per valid "
                                 "take: %s)" % (rd_num(c), name, ", ".join(rd_num(t["C"]) for t in L if t["valid"])))
                if abs(c - RD_CLICK_INTERVAL * round(c / RD_CLICK_INTERVAL)) <= 600 and c > 12000:
                    info("RD note: a C difference of about a whole click interval (%d samples) is what a slipped marker-to-"
                         "hop pairing gives (ruling K3), not a block; the take lines' 'witness pairing' candidates show it"
                         % RD_CLICK_INTERVAL)
        if shifted and abs(eref - RD_E1_NOMINAL) > RD_ONE:
            anomalies.append("e' is one-valued with a c0 >= B take, but the median e' %s is more than %d from %d"
                             % (rd_num(eref), RD_ONE, RD_E1_NOMINAL))
    if anomalies:
        for a in anomalies:
            info("RD O4 clause: " + a)
        return done("O4", "ANOMALY. STOP. S6 stays stopped, R7 is not re-stated. To the architect with RD's lines")
    # ---- O2 PRODUCT
    if s >= RD_TWO:
        return done("O2", "PRODUCT. e' is two-valued. S6 stays stopped, R7 is not re-stated, no builder guesses a fix; "
                          "stage D1 (A20), then RD's launches are repeated with it (RD2)")
    # ---- not an outcome yet
    if not valid:
        return done("INCOMPLETE", "0 valid takes on record")
    for name, L in zip(names, launches):
        v = [t for t in L if t["valid"]]
        missing = sum(1 for t in v if t["C"] is None)
        if v and missing * 2 > len(v):
            return done("INCOMPLETE", "C is missing in %d of the %d valid takes of %s: one more launch" % (missing, len(v), name))
    if len(valid) < RD_MIN_VALID and not (shifted and met):
        return done("INCOMPLETE", "%d valid takes on record (< %d)%s: one more launch"
                    % (len(valid), RD_MIN_VALID,
                       " and the early-stop rule is not met (%d valid takes with c0 >= one block, %d with c0 = 0)"
                       % (len(shifted), len(zero)) if shifted else ""))
    # ---- O1 ARTEFACT, SHOWN / O5 NOT REPRODUCED
    if shifted:
        return done("O1", "INSTRUMENT ARTEFACT -- on some takes the file starts c0 samples into the take; markers and "
                          "audio agree. No product change. R7r is built. NMAX = %d, EREF = %s" % (nmax, rd_num(eref)))
    return done("O5", "NOT REPRODUCED on this tree in %d valid takes over %d launches (every valid take has c0 = 0, e is "
                      "one-valued). HD1: six more launches run before R7r is built. NMAX = 1, EREF = %s"
                % (len(valid), len(launches), rd_num(eref)))


def rd_verdict_files(paths):
    """`probe-sync.py --rd-verdict FILE ...` (no app): each FILE is one launch's probe-sync.json. Exit 0 when an
    outcome O1 / O2 / O4 / O5 is printed, 3 on INCOMPLETE, 2 when a file cannot be read."""
    if not paths:
        print("RD INCOMPLETE (no file given: probe-sync.py --rd-verdict FILE ...)")
        return 2
    launches = []
    for pth in paths:
        try:
            launches.append(json.load(open(pth))["rows"]["RD"]["takes"])
        except (OSError, ValueError, KeyError, TypeError) as e:
            print("RD INCOMPLETE (cannot read rows.RD.takes in %s: %s)" % (pth, e))
            return 2
    return 3 if rd_verdict(launches, list(paths))["outcome"] == "INCOMPLETE" else 0


def row_rd(w, wav60, fixture, outdir):
    """RD (PROBESYNC_RD=1, in place of row R7): R7's take loop at dial 0, WITHOUT R7's verdict and WITHOUT R7b. Per
    take the two rulers (A2), the device fields (A5), the gap count and the hops with applied != 0; every take's
    record is saved in probe-sync.json rows.RD.takes for `--rd-verdict`. A diagnosis run, never a gate line."""
    import glob
    import shutil
    st, d = http("POST", "/api/load_composition", {"path": fixture}, timeout=10)
    info("RD load_composition %s: %s" % (fixture, str(d)[:120]))
    time.sleep(1.0)
    st, d = http("POST", "/api/render_frame", {"output_path": os.path.join(outdir, "attach-rd.png")}, timeout=15)
    (ok if st == 200 else no)("RD render_frame %s (GL attached -- preview visible)" % st)
    arms = [int(x) for x in os.environ.get("PROBESYNC_R7_ARMS", "0,0,0,0,0,0").split(",")]
    seconds = float(os.environ.get("PROBESYNC_R7_SECONDS", "12"))
    if any(a != 0 for a in arms):
        no("RD every take runs at dial 0 (PROBESYNC_R7_ARMS %s)" % arms)
        return None
    try:
        source = read_wav_ch0(wav60, max_frames=int((seconds + 5.0) * 48000))
    except (OSError, ValueError) as e:
        no("RD the click WAV %s cannot be read (%s)" % (wav60, e))
        return None
    st, dev = http("GET", "/api/debug/audio_devices")
    opens_prev = dev.get("opens") if isinstance(dev, dict) else None
    info("RD device before the first take: block size %s, rate %s, opens %s; %d take(s) of %.0f s at dial 0"
         % (dev.get("opened", {}).get("buffer_size") if isinstance(dev, dict) else None,
            dev.get("opened", {}).get("sample_rate") if isinstance(dev, dict) else None, opens_prev, len(arms), seconds))
    takes, assets = [], []
    for k, d_ms in enumerate(arms):
        set_sync(d_ms)
        settled = wait_settled(d_ms) is not None
        if not settled:
            no("RD take %d: applied never settled at 0 (GET /api/sync %s)" % (k, str(get_sync())[:160]))
            break
        time.sleep(0.5)
        name = "probesync-%s-t%d" % (RUNID, k)
        a0 = app_ms(w)
        st, r = http("POST", "/api/perf/record", {"name": name, "audio": True, "audioFile": wav60,
                                                  "onsetMarkers": True})
        if st != 200 or not (isinstance(r, dict) and r.get("ok")):
            no("RD take %d: perf/record refused (%s %s)" % (k, st, str(r)[:160]))
            break
        t_start = time.monotonic()
        time.sleep(max(0.0, seconds - (time.monotonic() - t_start)))
        http("POST", "/api/perf/stop")
        a1 = app_ms(w)
        time.sleep(3.0)
        tj = os.path.join(TAKES_DIR, name + ".adna-take", "take.json")
        try:
            take = json.load(open(tj))
        except (OSError, ValueError) as e:
            no("RD take %d: no saved take at %s (%s)" % (k, tj, e))
            break
        seg = take["audio"]["segments"][0]
        assets.append(seg.get("id", ""))
        wav = os.path.join(AUDIO_DIR, seg.get("id", "") + ".adna-audio", "audio.wav")
        try:
            asset = read_wav_ch0(wav)
        except (OSError, ValueError) as e:
            info("RD take %d: the asset %s cannot be read (%s)" % (k, wav, e))
            asset = None
        st, dev = http("GET", "/api/debug/audio_devices")
        hops = sorted(w.hops_between(a0 - 250.0, a1 + 250.0), key=lambda h: h["i"])
        rec = rd_take_record(k, take, asset, source, hops, dev if isinstance(dev, dict) else {}, opens_prev, settled)
        rec["name"] = name
        rec["appWindow"] = [a0, a1]
        if rec["opens"] is not None:
            opens_prev = rec["opens"]
        takes.append(rec)
        info(rd_take_line(rec))
        wit = rec["witness"]
        info("RD take %d witness pairing: offset %s over %s pairs (SD %s samples); candidates [offset, pairs, SD, median]: "
             "%s; %d count rises in %d hops, %d raw markers" % (k, wit["d"], wit["n"], wit["sd"], wit["cands"],
                                                               wit["rises"], rec["hops"], rec["raw"]))
    if takes:
        info(rd_launch_line(takes))
    RESULTS["rows"]["RD"] = {"takes": takes, "seconds": seconds, "arms": arms, "wav": wav60}
    mine = glob.glob(os.path.join(TAKES_DIR, "probesync-%s-t*.adna-take" % RUNID))
    for t in mine:
        shutil.rmtree(t, ignore_errors=True)
    gone = 0
    for a in assets:
        if a and len(a) == 32 and all(c in "0123456789abcdef" for c in a):
            shutil.rmtree(os.path.join(AUDIO_DIR, a + ".adna-audio"), ignore_errors=True)
            gone += not os.path.exists(os.path.join(AUDIO_DIR, a + ".adna-audio"))
    left = glob.glob(os.path.join(TAKES_DIR, "probesync-%s-t*.adna-take" % RUNID))
    (ok if not left and gone == len(assets) else no)(
        "RD the probe's %d takes and %d of %d Audio-store assets were removed" % (len(mine), gone, len(assets)))
    return takes


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--rows", default="R0,R5,R1,R1a,R2,R3,R9,R4,R4b,G6,R7,R6")
    p.add_argument("--out", required=True)
    p.add_argument("--wav", default="/tmp/click_probesync_30s.wav")
    p.add_argument("--wav60", default="/tmp/click_probesync_60s.wav")
    p.add_argument("--fixture", required=True)
    args = p.parse_args()
    rows = set(args.rows.split(","))
    os.makedirs(args.out, exist_ok=True)

    if not row_r0():
        info("R0 failed: every later row needs /api/sync -- stopping")
        json.dump(RESULTS, open(os.path.join(args.out, "probe-sync.json"), "w"), indent=1, default=str)
        return 1
    w = Witness()
    if not w.poll_once():
        no("witness GET /api/debug/sync_witness unavailable (not a TEST-SERVER build?)")
        return 1
    w.start()
    if "R5" in rows:   # first: AUTO mode -- every later tempo row turns manual mode on, and nothing turns it off
        row_r5(w, args.wav60, args.fixture, args.out)
    r1a_windows = []
    r1, r4wins = None, None
    if "R1" in rows:
        r1 = row_r1(w)
        if r1:
            r1a_windows += [tuple(r["appWindow"]) for r in r1 if r["witness"]]
    if "R3" in rows:
        a0 = app_ms(w)
        row_r3(w)
        r1a_windows.append((a0, app_ms(w)))
    if "R1a" in rows:
        row_r1a(w, r1a_windows)
    if "R9" in rows:
        row_r9(w)
    if "R4" in rows:
        r4wins = row_r4(w)
        if "R1a" in rows:   # s-rta-1003b: (ii) and (iii) also over R4's settled -500 window
            if r4wins and "m500" in r4wins:
                row_r1a(w, [(r4wins["m500"][0], r4wins["m500"][1])], label=" [R4's settled -500 window]",
                        parts=("ii", "iii"), key="R1a-500")
            else:
                no("R1a [R4's settled -500 window]: R4 gave no settled -500 window")
    if "G6" in rows:
        row_g6(w, r1, r4wins)
    if "R4b" in rows:
        row_r4b(w)
    if "R2" in rows:
        row_r2(w, args.wav, args.fixture, args.out)
    if "R7" in rows and os.environ.get("PROBESYNC_RD") == "1":   # stage D0: the diagnosis run in R7's place
        row_rd(w, args.wav60, args.fixture, args.out)
    elif "R7" in rows:   # R7b runs inside R7's 100 arm
        row_r7(w, args.wav60, args.fixture, args.out)
    set_sync(0)
    w.finish()
    if "R6" in rows:
        n = RESULTS.get("persistTrue", 0)
        (ok if n == 0 else no)("R6 GET /api/sync showed persist false on every read (%d reads said otherwise)" % n)
    json.dump(RESULTS, open(os.path.join(args.out, "probe-sync.json"), "w"), indent=1, default=str)
    info("probe-sync.py: %d PASS, %d FAIL" % (RESULTS["passes"], RESULTS["fails"]))
    return 1 if RESULTS["fails"] else 0


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--rd-verdict":   # offline, no app: RD's decision table over saved files
        sys.exit(rd_verdict_files(sys.argv[2:]))
    sys.exit(main())
