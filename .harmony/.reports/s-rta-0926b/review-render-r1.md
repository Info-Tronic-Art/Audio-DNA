# Reviewer Verdict — render-r1 (lane/render-0926b)
STATUS: DONE
VERDICT: APPROVE

PINNED: worktree /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_41d6317f-a47-1,
branch lane/render-0926b, base bc69fd0d4da0b6ca78ecb3c0e959215fc4f3ca7f,
head 9edb050f8401b97dc1af71c8c92d5db2bb8932da (HEAD matches pin, verified).

COMMITS (dac0eb7 diagnosis precedes every fix commit, verified via git log):
dac0eb7 diagnosis (R1-R5, before any source change)
e657c26 fix R5 (temporal buffer creation hoisted before pass loop)
f36bc71 fix R3 (outgoing clip transform+opacity restored via applyClipTransform)
6c72dcc fix R2 (LayerStateKey.h: deck+layer keying)
a832d96 fix R4 (renderLayerStages shared between compositeDeck/compositePersistentLayers)
0908476 probes (probe-render-state + probe-crossfade k/l)
9edb050 report (render.md)

FILE: src/render/CompositorEngine.cpp / .h
  [OK] R5 fix: getOrCreateTemporalBuffer hoisted to top of applyClipEffects, before any pass
       binds an FBO/texture unit — matches diagnosed mechanism exactly (createFBO leaves
       framebuffer 0 bound + texture unit 0 rebound; verified by reading createFBO, CompositorEngine.cpp).
  [OK] R3 fix: applyTransition now routes the outgoing clip through applyClipTransform (transform
       + opacity) before applyClipEffects, same order as the active-clip path; holdTex plumbed
       through so neither pass clobbers the held incoming texture (ScratchPool discipline preserved).
  [OK] R2 fix: new src/render/LayerStateKey.h (pure, no GL) replaces layer-id-only keys for
       temporal buffers / frame rings / feedback processors with deck+layer(+chain-bit) keys.
       All 6 call sites updated consistently (verified by grep); Deck::id is uint32_t, matches key type.
  [OK] R4 fix: renderLayerStages extracted as a single shared helper used by both compositeDeck
       and compositePersistentLayers (good DRY — avoids the two-copies drift that caused R4 in
       the first place); advanceCrossfade also shared. saveLayerOutput correctly stays outside
       the helper (active-deck-only, matches report's stated open fork on Layer Router).
  [OK] Scope discipline: R1 correctly left unfixed per packet instruction (needs a design ruling);
       R4-opaque/R4-router/R4-types correctly deferred to open_forks rather than fixed on an
       ambiguous doc reading — each with facts, doc citations, options, and a recommendation.
  [ISSUE-NONE] No dangling trace/debug code: grepped compiled src/render/*.{cpp,h} for TRACE/
       AUDIODNA_FBO_TRACE — none present. render-trace.diff lives only in the docs-only diagnosis
       commit (dac0eb7), never in a fix commit (git apply -R confirmed by report; verified no
       trace remnants in HEAD source).

FILE: tests/test_layer_state_key.cpp, tests/CMakeLists.txt
  [OK] Drives the real LayerStateKey.h (not a mirror) against real Deck::initDefault decks.
  [OK] Mutation ("teeth") check independently reproduced: dropping the deck-id shift or the
       chain bit each flips exactly the assertion that pins that mechanism (2/3 and 1/3 tests
       fail respectively), with the deliverable's sha256 unchanged before/after — this is the
       negative-fixture correctness check the reviewer manual asks for on new invariant-asserting
       code, done live rather than only via synthetic fixture.

FILE: .harmony/probe-render-state.sh / .py / .json, .harmony/probe-crossfade.{py,json,sh}
  [OK] Screen-safe pattern confirmed (open -g, lock-dir discipline, no screencapture/lldb/
       synthetic input, graceful quit with pkill fallback) — read the actual scripts, not just
       the claim.
  [OK] Existing probe-crossfade rows/thresholds untouched; only new rows k/l added (diff confirmed).
  [OK] Independently reproduced the raw RED/GREEN evidence files (probe-render-state-MAIN.txt:
       "PY 1 PASS / 18 FAIL"; probe-render-state-FIX.txt: "PY 19 PASS / 0 FAIL"; probe-crossfade-
       kl-MAIN.txt: "PY 4 PASS / 4 FAIL"; probe-crossfade-FIX.txt: "PY 35 PASS / 0 FAIL"; ctest-
       FIX.txt: "100% tests passed, 0 tests failed out of 583") — all match the builder's claimed
       numbers exactly.

FILE: .harmony/.reports/s-rta-0926b/render.md
  [OK] open_forks R1 file:line citations verified against HEAD source: CompositorEngine.cpp:821,
       827, 832, 846, 1398 all match as claimed. Buffer/ring byte-size math independently re-derived
       (480x270 RGBA8 x480 frames = 248,832,000 B / 237 MiB at 1080p) — matches the report and
       confirms the "off by 2x" flag on the CompositorEngine.h:223 comment.
  [OK] Lifetime claim ("freed only in releaseGL, never on clip/layer/composition change") verified
       by reading releaseGL() directly — the only clear()/erase() sites for the three maps.
  [OK] Visual spot-check: red_r3_strip.png vs green_r3_strip.png shows the RED build's mismatched/
       oversized outgoing-clip ghost resolved to a clean matching grid in the FIX build.

FENCE: git diff --stat outside .harmony/, tests/, src/render/ is empty — no out-of-fence changes.
No build dirs, .venv symlinks, or stray files in the diff; build-lane/ is untracked (not committed).

SUMMARY: 6 files (2 source, 1 new header, 1 new test, 2 probe suites) + evidence/report docs,
0 blocking issues. Diagnosis-before-fix ordering, RED/GREEN evidence, mutation-tested new pure
logic, DRY consolidation for R4, and honest non-fixing of R1/R4 sub-questions as open forks all
independently verified against disk, not taken on the builder's word.
METADATA: reviewer=reviewer-render-r1, builder_packet=lane/render-0926b, date=2026-09-26
