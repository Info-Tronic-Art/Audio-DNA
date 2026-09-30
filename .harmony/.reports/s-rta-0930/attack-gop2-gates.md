# ATTACK PAPER — lane gop2, seat gates (s-rta-0930)

Recovered by Harmony from the workflow journal (wf_d5f2da66-bc5): the seat returned its paper as structured output and did not write this file.

VERDICT: AMEND

## A1 [MUST] G5 omits the pixel-identity probe (u12_reverse_pixel_identity) and any capped identity check, although both c2 (window) and c3 (store gate) change which frames are stored and when.

EVIDENCE: Plan section 5 G5 row list is u7,u8,u9,u11 only. probe-vupload.py:15 lists u12_reverse_pixel_identity as a row. The plan's own B3 claim ('never which pixels are shown') and T1b identity asserts are offline 64x64 only. T3's identity assertions are admitted 'not yet executed' (plan :253, :335). The 4K/1080p pixel path at a capped share is never gated.

FIX: Add u12 to G5 (uncapped) and a capped run (ENV 256 and 128 MB, where the window changes) as a hard PASS gate. Also re-run u5/u6-style forward rows that touch seekToTimestamp/decodeNextFrame, since c3 edits the forward decode path.

## A2 [MUST] G4 can pass vacuously or by luck: its median-of-5 rule has no control-validity check and no per-launch ceiling, and the tested event is one transient per launch.

EVIDENCE: Decision rule (plan :384-393) judges only B's median late <= 40. If the orphaned burner or quiet machine makes A itself <= 40 in that session, B passes with no evidence (E6: rig late varies 44-101 on HEAD with load, 93-101 at load 6-9; live A 112-143 in E1). The late count is one GOP-0 entry per launch (E6: all late at 1.5-2.5 s; a250 is a single GOP, so the Loop wrap and steady-state lead-ins are never reached inside the 5 s window, probe-vupload.py:594-605). A median of 5 hides 2 bad launches.

FIX: Pre-register: verdict INCONCLUSIVE unless A median late > 40 (same session); B requires >= 4/5 launches <= 40 and max <= ~60; also require B per-launch dec/up <= ~11.5 and frames == 84 so the new window is proven to have engaged. Add a longer/multi-GOP or Loop-wrap window row.

## A3 [MUST] Load/orphaned-burner control is one pre-run ps check, not a per-launch control; the prior lane already had orphaned yes burners (commit fd18198).

EVIDENCE: Plan :371-373 'a ps burner check first'. Nothing records loadavg/top CPU per launch into ab.tsv, nothing discards a launch when a burner starts mid-run, and late is demonstrably load-sensitive (E6, K1 batch5). The offline rig numbers in E7 were taken at load 2-7, unmatched between arms by construction only via interleaving.

FIX: Have the driver log loadavg and the top non-app CPU process before and after every launch into ab.tsv; define a rejection threshold (e.g. non-app CPU > 50% or load1 > 3) that voids and reruns that launch pair; kill/assert no orphan `yes`/ffmpeg processes before each launch, not once.

## A4 [SHOULD] The bytes bar has no teeth and capped over_budget is never checked.

EVIDENCE: probe-vupload-ab.py:134: budget = cap*1MiB + floors where floors = 4*floorFrames*frameBytes1080 -> 350.9 MB in summary.txt vs an observed 249.4 MB: ~100 MB of slack. Line 141: for capmb the over_budget rule is `enough` only (always true with 5 launches), so it cannot fail.

FIX: For capped groups require bytes <= cap + one frame per player and over_budget delta == 0 per B launch, since c2 deliberately lets windows over-plan against a shared cap.

## A5 [SHOULD] T1a's RED is a compile failure on main and (iv) cannot fail on HEAD; the avail0 == 0 case is excluded.

EVIDENCE: Plan :231 'RED: does not compile on 655d232'; (iv) asserts avail0 <= window <= share, which HEAD's planPrefetch already satisfies by construction (GopCache.h:381-395). The patch's new branch is guarded `avail0 > 0` (plan :205) and the grid starts at avail0=1, yet a full share (avail0 == 0 at the steady state of a capped player) is precisely the case that still returns None. T1a (ii) pre-registered numbers (target 231, windowLo 218) were computed on a scratch p6.py, not the shipped code (plan :523-524).

FIX: Make RED behavioural: a T1a variant using the existing signature that asserts the u8-shape returns Prefetch on a stub (fails on HEAD as None). Extend grid to avail0 = 0 with an explicit expected result. Drop (iv) or strengthen to exact value monotonicity in lead.

## A6 [SHOULD] T1b pins decodeMsEma_ before every step, so it never tests the cold-EMA state that occurs at the very moment of the measured failure.

EVIDENCE: decodeMsEma_ is seeded per megapixel (VideoPlayer.cpp:298) and updated at :1378 (0.9/0.1 EMA). u8's late frames are the first reverse run after a trigger, when the EMA may still hold the seed or a forward-decode value; servedDuringLead and prefetchAt both hang on it. The plan's K2 argues over/under-estimate effects only in theory.

FIX: Add a T1b config with an unpinned EMA (real 64x64 timing is ~0.05 ms: should degrade to HEAD behaviour, asserted as such) and a config pinned to the seed value, to record what the first entry actually does.

## A7 [SHOULD] Load-bearing E7/E8 evidence is from scratch code that is not the shipped patch; the GC9 token interaction is unverified live.

EVIDENCE: E7 'patch = scratch gc7/p6.py' with NOINPLACE/NOLANDKEY switches, builder 'writes a clean version' (plan :523-524); E8 verified 'on this exact formula'. The earlier PREFETCH trigger now makes four players compete for the single GC9 token (VideoPlayer.cpp:1314-1320; urgent = ceil(250*1.76/33.3*1.2) = 16 frames vs u = 17), a margin of one frame.

FIX: Builder must re-run u8h/gc7step on the committed code and paste numbers before c2 is accepted. Record token-denied/urgent-bypass counts in G4 as INFO and add a rig case with urgent+1 covered frames.

## A8 [SHOULD] Selftest/probe fix RED is weak and G3 GREEN depends on a file absent from the worktree.

EVIDENCE: ab.tsv is gitignored (.gitignore:63) so the builder cannot reproduce G3 second clause; selftest (i) is the only builder-side proof. G4 itself sets ENV_A=256 (plan :385), so the cap-0 group never appears in the real gate: the artefact fix is unexercised by G4.

FIX: Commit a tiny synthetic fixture TSV for the selftest to read, and add the mixed/A-only cases as assertions on exact rule counts, not just 'contains'.

## A9 [NIT] Slowest >= 20 at 256 MB is saturated and the pooled/slowest figures cannot discriminate.

EVIDENCE: Patch slowest 30.0 every run (E7) equals the 30 fps source cap; HEAD 27.8 already passes by 8/s margin (E1).

FIX: Report it as INFO; gate on late and dec/up instead.

