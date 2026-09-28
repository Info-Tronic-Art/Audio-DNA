# s-rta-0928 — running work log (secondary, MINIMAL, ultracode/workflows)

Boot 2026-09-28 08:19 (from `date`). HEAD 6db8d67 (code = 233eae7), unpushed 0. build/ binary 00:18:32 (= 233eae7,
current, TEST_SERVER ON). Disk 363 GB free. 10 cores. No app running, no live lock, no worktrees at boot. Load avg ~3 (WindowServer, Codex).
Dirty at boot (not ours, leave): .harmony/.harmony-version, AGENTS.md (untracked).
Prior scratchpad (s-rta-0927) still on disk; copied battery.sh, tempo-witness.sh, tempo-x3.sh, tempo0 run1.sh to scratchpad/prior/.

## START HERE items (birth prompt s-rta-0927)
1. Take start-tempo race (tempo0-diag.md): Fable plan A vs B -> build -> probe row (witness x20 = 0 bpm-0, 0 unknown bar grid).
2. Tier-1 residual (followups.md F1.C 30 lines + F1.E test_fractals smoke 113F/161P on lane head, untriaged).
3. Render leftovers (renderperf.md found_not_fixed 1,2,3,5,6).
4. Routine start/loop restore holds the message thread 38-86 ms (routines-timing.md found_not_fixed 2; + #3 loaded-run stack anomaly) — diagnose first.
5. Boris answers pending (boris-checks.html; defaults in force).

## Plan (lanes; <=3 concurrent build lanes; live app serialized by /tmp/audiodna-live.lock)
- Lane T tempo  (W1 rta0928-w1): Fable plan -> 2 blind attack seats -> Harmony adopts -> opus builder -> reviewers -> <=1 fix.
- Lane P render leftovers (W2 rta0928-w2): same shape.
- Lane V tier1 residual (W3, later): diag (main build, no compile) -> Fable plan -> attack -> adopt -> build.
- Lane R routine restore (W4 rta0928-w4): instrumented diag (build) -> Fable plan -> attack -> adopt -> build.
- Harmony gates each merge: RED-first on the pre-merge app for every new probe row, affected probes, full battery at the end.
- 08:24 launched: wr81e0ris (wf_d7680481-78f) Fable plans tempo + renderleft -> 2 blind attack seats each;
  w9v26h3nf (wf_6b057fc9-7dd) tier1 diag (opus, W3, main app read-only) -> Fable plan -> 2 seats;
  w72wmg1yy (wf_2e48cd9c-745) restore diag (opus, W4 instrumented build) -> Fable plan -> 2 seats.
  Scripts syntax-checked BEFORE launch (scratchpad/wf/check.sh). Shared lock lib scratchpad/lib/lock.sh (LANE=<name>).

## My own errors this session (no gate would surface them)
1. 08:26 My lock helper (scratchpad/lib/lock.sh v1) took the lane name from a `LANE=x . lock.sh` prefix; bash drops a
   prefix assignment on "." after sourcing, so owner lines were written as " <pid> <epoch>" (blank name) and release_lock
   compared "" = "" — any lane could have released another lane's lock, and probes would REFUSE (owner "" != AUDIODNA_LOCK_OWNER).
   Seen on the owner file ~5 min into the tier1diag run (its own script releases correctly: same blank on both sides).
   Fixed: LOCK_LANE pinned by a real assignment at source time + release refuses a blank name; verified with bash -c. Habit:
   exercise a shared helper end-to-end (source, then call a function in the SAME shell and print the owner line) before
   handing it to lanes.
- 08:29-08:31 Harmony RED baseline, tempo witness x20 on main app (233eae7), load 5-7: WITNESS-TALLY base: runs=20 start_bpm0=4
  unknown_bar_grid=5 no_take=0 (bpm-0 runs 02 04 12 18, each followed by a "lock" anchor at 120 after 7.6-13.5 ms; grid-unknown
  = those 4 + run 07). Script scratchpad/gate/witness-batch.sh (one lock hold, takes deleted after each run), evidence gate/witness-base/.
- 08:45 TIER DEVIATION (Law #11 row 2): wr81e0ris both Fable architects died "You've reached your Fable limit" after 881 s /
  563k subagent tokens, no plan written. Per the s66 precedent (system-upgrade-candidates.md "fable audit misattributes quota
  exhaustion"): planners re-pinned to OPUS max, recorded here, surfaced to Boris (he can top up via /usage-credits). Blind
  council attack + Harmony adoption unchanged. tier1/restore workflows' plan stages will hit the wall after their diags ->
  resume them from their run ids with the opus-pinned scripts (diag results replay from cache).
- 09:14 wy228bhti (wf_6f66e39e-7c3) = opus re-run of the tempo + renderleft plans. tier1 diag DONE (31 KB report: 30 lines
  same; 4 legit vanishing / 16 range-math (10 one-liners byte-identical); 3 really dead; 7 harness entries; test_fractals 113F/161P
  x5 identical, 81 = its mean<=5 metric) -> plan stage died on Fable -> resumed opus (wrdqul2wi, same run id).
  restore diag DONE (26 KB): hold = ClipCell/LayerStrip thumbnail decode from disk on EVERY DeckView::refresh, once per discrete
  restore entry (99.6-99.9% attributed, counterfactual fixtures linear in decodes: no-image = background, 4K 178 ms, 5 cells 154 ms);
  stack anomaly = Player::advanceTo jumps a whole short gesture in one tick under a >= ~0.3 s stall: touched+released, value never
  written (450 ms stall 5/5 vs 150 ms 0/5; CPU load alone 0/20). Plan prompt amended (both causes; coordinate with renderleft's
  off-GL-thread decode) -> resumed opus (wnzwhg861, same run id).
- 09:33 tempo plan ADOPTED (A1 fallback 0.25 s, A2 stall-assisted RED mandatory, A3 L1/L2 = lint, A4 POD, A5 no onset lost, A6-A8) -> build lane wraiope56 (wf_bdd0154f-e2c, W1, lane/tempo-0928, opus high; reviews concurrency + gates).
- 09:38 renderleft plan ADOPTED (B1-B5 gl seat, C1 pause crossfade on pending incoming, C2 gate render_frame only, C3 sequences counter, C4, C5 Boris list, C6 fence vs restore lane).
- 09:38 renderleft build lane wx9exnenb (wf_ac7207d2-156, W2, lane/renderleft-0928, opus high; reviews gl + vj). Build lanes running: tempo, renderleft.
2. 09:48 I used cd into the reports dir in a read command (rig rule: never cd); the shell cwd moved. Restored at once; no write happened there. Habit: absolute paths, even for reads.
- 09:58 restore plan ADOPTED (D1-D8; generalized end-value write accepted, Boris list). tier1 plan + seats in (loosening SOUND_WITH_FIXES; vj SOUND_WITH_FIXES with 1 MUST = zoom black 60-75% of travel on 3 fractals -> Boris B1).
- 09:59 tier1 plan ADOPTED (E1 zoom -> strict ledger + Boris #1 with rec option A; E2 kifs try fold; E3 thickness floor try; E4 value-keyed; E5 tetra own entries; E6-E8). Launch when a build slot frees (tempo lane done).
- 10:29 tempo build DONE @ 4a251ab: ctest 797/797; lane RED on main copy W1 FAIL, W2 15/20 FAIL, W3 20/20 FAIL; GREEN x6 (stall x3 + natural + relinked) W1-W4 all pass, W6 x20 0+0; sec-7 re-runs green, 0 'take start:'. Reviews running. Harmony RED of probe-tempo-start on main app started (gate/tempo/red-harmony).
- 10:31 Harmony RED (main app 233eae7, stall-assisted, load 9): W1 FAIL (start bpm 0, routine from beat 0 refused 'recorded before the tempo was known'), W2 3/20, W3 0/20, W4 0; Output-window row FAIL = my copy ran from scratchpad (no .venv beside it) — wrapper outwins 0.
- 10:37 MERGED tempo 734f011 (reviews concurrency + gates PASS_WITH_NITS). main build rebuilt 10:36. ctest main 797/797 (18 s). W1 worktree removed. tier1 build lane launched w0m954i6y (wf_074f429b-1cb, W3). Harmony green gate running (gate/tempo/green.sh).
- 11:16 GATE tempo GREEN on main 734f011 (Harmony, 11:05-11:15, load 3.6-6.7): probe-tempo-start PASS 10/0 (W1 PASS routine from beat 0 saved; W2 20/20; W3 20/20; W4 0; W6 x20 0/20 bpm0 + 0/20 unknown grid); routines 105/0, step3 94/0, beatclock 6/0, downbeat 14/0, resync 16/0, manual-bpm 22/0; 0 'take start:' in app stderr (2 hits = the probe's own row titles). RED on pre-merge app earlier (10:30).
- 11:36 Boris asked: builders stuck? Checked: none stuck. tier1 holds lock (baseline test_sources on base app, since 11:26); renderleft (9 commits, last 11:16) + restore (4 commits, last 10:40) poll for the lock every 20 s. Bottleneck = one live-app lock across 3 lanes.
- 12:58 tier1 build DONE @ 221d087: ctest 803/803 (lane); [source-extremes] 6 cases RED 6/6 -> GREEN; test_sources residual 0 lines; test_fractals 93 passed 8 xfailed (0 XPASS); kifs fold repair tried + failed -> removed; Pitfalls 49/50. Disclosed: one cd; cellular_automata 1080p differs base vs lane (stateful sim, 10 same-app captures = 10 hashes, shader untouched). Reviews + critic running.
- 13:13 tier1 reviews loosening + shaders PASS_WITH_NITS; critic PASS (SHOULD: kifs Iterations 0 / Thickness 0 extremes faint on a projector -> Boris list). Harmony RED: lane test_fractals harness vs pre-merge main app (gate/tier1/red.log).
- 13:14 Harmony RED tier1 (pre-merge main app, lane harness test_fractals): 4 failed, 89 passed, 8 xfailed (4 = mandelbrot Power lines). MERGED tier1 78b2c2a; main rebuilt 13:14; ctest 803/803. Looked at sheet_g6_mandelbulb (B black at 0/1, A draws a slice at every position). GREEN run started.
- 13:24 restore build DONE @ a232c34 (rebased on 734f011): lane ctest 814/814; holdMs 5h/8h/11h 56.7/59.5/64.9 ms (step-B) -> 0.14-0.21 ms x3 quiet; 7m RED on main (opacity stays 1.0) -> GREEN 3/3; T1 0 restoring ticks >= 1 ms (was 53-179); stall 450 ms 0/5 (was 5/5). Reviews threads + gates PASS_WITH_NITS. Merge after tier1 green run (it is using main's binary).
- 13:29 GATE tier1 GREEN on main 78b2c2a (13:16-13:29, load 4.8): test_fractals 93 passed 8 xfailed; test_sources 4 passed (722 s, 0 residual lines). RED earlier 4 failed.
3. 13:31 Rebuilt main after the restore merge BEFORE running my own RED of the new probe-routines rows on the pre-merge app (the lane's RED exists; mine did not). Recovered with the lane's saved pre-merge app copy. Habit: RED on the pre-merge binary BEFORE the rebuild — put the RED step in the merge script ahead of cmake.
- 13:36 MERGED restore 80e3e53 (conflicts CLAUDE.md + pitfalls.md = two lanes appending; kept both, restore NN -> Pitfall 51, recording.md ref fixed). main rebuilt 13:31; ctest 820/820. GATE restore: RED on pre-restore app copy (734f011) probe-routines 105/4 (5h/8h/11h absent, 7m L0 opacity stays 1.0); GREEN main 109/0 (5h 0.195, 8h 0.376, 11h 0.209 ms; 7m lands 0.5); routine-display 16/0.
- 13:36 Looked at routine-display 02-waiting on merged main: image cells + L1/L3 strips show the test-card thumbnail. W4 removed.
- 13:54 renderleft build DONE @ 5f1536f (rebased on 15c5f8d; lane ctest 846/846): i1 1080 38.8 -> 1.3-4.2 ms, i5 34 -> 4-6, i6 46 -> 4.3; render_frame png 81 -> 9.9 ms (4K 304 -> 39), RT 97 -> 26; cr1 3/4 timeouts -> 4/4 in 0.14 s; quit +0.13 s max; R4 filed. Reviews running. Harmony RED of new probes on pre-merge app started.
- 13:56 Harmony RED renderleft probes on pre-merge main app (15c5f8d code, load 2.2): image-load 11/15 FAIL (i1 35.95/135.69 ms, i2 35.92/135.72, i6 48.68, i2m first frame p 0.500, new state fields absent); capture 3/6 FAIL (cr1 3 of 4 at 5.01 s; cr2 640x360 5.01 s; cr3 png 81.0 ms, RT 93.1).
- 14:02 renderleft r1 reviews FAIL: MUST C1 pause excludes ImageSequence; MUST C2 snapshot-while-pending row missing; MUST pitfalls.md lost 48-50 bodies in the lane's rebase; SHOULD O(n^2) imagePaths dedup. Fix round running (workflow).
- 14:13 Boris page written + opened: .harmony/.reports/s-rta-0928/boris-checks.html (zoom B1 strip, B3/B4, fixed/removed knobs, restore feel, tempo, renderleft hold (pending update), Fable note).
- 14:24 renderleft fix round DONE @ ea47823: pitfalls 48-50 restored (verified 46-53 on branch), C1 pause covers ImageSequence (firstFramePending peek), i3n_snapshot_while_pending row added (snapshotMaxMs 500; gated calibration 887-1442 ms vs ungated 72-84), imagePaths O(n). Lane ctest 846/846. r2 reviews running.
- 14:38 renderleft r2: PASS + PASS_WITH_NITS (SHOULD: direct ctest for ImageSequence::firstFramePending -> loose end). Harmony RED of fix-round rows on pre-merge main app: i2ms first frame p 0.258 FAIL; i3n snapshot 1423 ms FAIL (+ not the held picture).
- 14:40 MERGED renderleft 0ab3996; main rebuilt 14:39; ctest 846/846; pitfalls 48-53 present; CLAUDE.md 24,482 B. W2 removed (no worktrees left). Fixed probe-finalize-loop bare open -> open -g (committed). FINAL battery started (gate/final.sh).
- 14:41 Untracked 3 tests/visual/__pycache__ .pyc (gitignored already; pytest rewrote them). Boris page sec 6 updated (renderleft landed).

## Loose ends collected from the four lanes (for the handoff)
tempo: TempoMap::sampleAt single-anchor rate 0 (Program.cpp:451 stampless Sample-clock fallback; recipe in tempo.md FNF);
  probe-step3.sh:1143 bare open (opt-in crash test only); R9 checkpoint0.bpm = arm-time bpm (informational).
restore: unattributed idle message-thread blocks 17-28 ms at ~15 Hz (no routine needed; sets T2 floor); sequence first-frame
  thumbnails decode on the message thread at load/drop/append; ClipCell::paint stats every image/video file every paint; a
  missing/undecodable image is stat'ed once per refresh; ClipThumbnails::get double stat on a cache miss (nit).
tier1: Boris B1 zoom / B3 slices / B4 spirograph (strict ledger); reaction_diffusion Diffusion A PSNR 55.1 once on base (no flake
  verdict); mandelbrot Power 0-0.24 one picture; test_no_discontinuities only warns (190+); lissajous t=0 1080 half brightness (B4 family).
renderleft: video still decodes on the GL thread (F16); ImageSequence keeps every frame's texture (300-frame 1080p ~2.5 GB);
  existsAsFile() per image clip per frame on the GL thread; deck-mode image trigger posts a pointless legacy load; ImageSequence::open
  / openMediaForDeck decode frame 0 on the message thread; R4 ~2 ms 4K warm excess (E3 removes 54-78 %, bar 70 % x3 not met);
  i5 over bar twice (16.85/20.73 ms) in the OLD row order on the pre-restore base, not reproduced since — cause unknown;
  no direct ctest for ImageSequence::firstFramePending (r2 SHOULD).
