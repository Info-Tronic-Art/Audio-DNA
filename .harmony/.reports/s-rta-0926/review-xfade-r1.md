# Reviewer Verdict — xfade-r1
STATUS: DONE
VERDICT: PASS_WITH_NITS

## Scope
Lane s-rta-0926 "xfade" (commits 138af1e, 5dc1272, 2876032), worktree
wf_23099956-236-1. Diff vs main: `src/render/CompositorEngine.{h,cpp}`,
new `src/render/ScratchPool.h`, `tests/test_scratch_pool.cpp` +
`tests/CMakeLists.txt`, `.harmony/probe-crossfade.{sh,py,json}` (new),
`.harmony/probe-effects-parity.{sh,py}` (hardened). No build/launch/debugger
run — read-only review of source, tests, probes, and the lane's own
`xfade-report.md`.

## FILE: src/render/ScratchPool.h (new)
[OK] Readability/Complexity: `ScratchPool::pick` is a small, pure,
templated function (no GL) with a clear contract ("never readTex, never
holdTex"); comment names the three prior per-site patches it replaces
(ScratchPool.h:5-12). KISS satisfied — this is the load-bearing rule the
whole lane rests on.
[OK] Error handling: returns -1 only if N<3 (unreachable with the real
3-member pool; exercised only by the negative test with a 2-member array).

## FILE: src/render/CompositorEngine.h
[OK] Patterns: `effectFBO_C_/effectTex_C_` follow the existing A/B
lifecycle exactly (init/release/resize all touch all three — verified
CompositorEngine.cpp:14-16, 27-29, 89-96).
[OK] `kLayerChainStateBit` (0x80000000) documented as never colliding with
a real layer id or `kGlobalEffectsLayerId` (0xFFFFFFFF) — correct, and the
reasoning is stated in the header comment (CompositorEngine.h:196-201).
[ISSUE-NIT] `applyTransition`'s header comment (CompositorEngine.h:343-345)
says "newClipTex is HELD across the outgoing clip's render + effect chain"
but doesn't mention the temporal-state gap the lane itself flags as
BROKEN-UNFIXED (#12 below) — a reader of the header alone would not learn
that the outgoing chain's `u_prev_frame` still aliases the incoming clip's
during a crossfade. Non-blocking (the gap is disclosed in the .md report,
just not co-located with the code it affects) → suggest a one-line pointer
to xfade-report.md §5 item #12 at CompositorEngine.cpp:1343.

## FILE: src/render/CompositorEngine.cpp — rule-walk (packet item 1)

I read every call site in the sweep table and independently re-derived its
verdict from the code, not from the table's own claim:

| # | Site | Table verdict | My check |
|---|---|---|---|
| 1 | `applyTransition` outgoing-chain fx, :1343 (`holdTex=newClipTex`) | FIXED | VERIFIED — :1339-1343, `applyClipEffects(...,/*holdTex=*/newClipTex)` |
| 2 | same-source-type crossfade copy, :1310-1332 | FIXED | VERIFIED — gated on `mediaType==Source && sourceType equal && !isEffectPoolTexture(newClipTex)`, and the whole function early-returns above (:1294) when `crossfadeProgress>=1` or no previous clip, so the extra copy is confined to an active crossfade only |
| 4 | clip fx → layer fx, compositeDeck :901→:918 | FIXED (via picker) | VERIFIED — layer chain now keyed `layer.id \| kLayerChainStateBit` (:918-920), separate temporal/ring state from the clip chain at :901 |
| 5 | clip transform → opacity → clip fx, :496,:549 | SAFE | VERIFIED — `pickEffectTarget(transformedTex,0)` at :541, forceCopy kept as belt-and-braces |
| 6 | dry/wet <1 on non-first effect, :417-443 | FIXED | VERIFIED — in-place fixed-function blend (`GL_ONE_MINUS_CONSTANT_ALPHA,GL_CONSTANT_ALPHA`), no extra sampler read of the write target |
| 7 | layer transform → Transparent keying, :633→:932 | FIXED (was UB) | VERIFIED — `applyLayerTransform` now returns a pool target (:653) instead of `scratchTex_`; keying at :932 draws into `scratchFBO_` sampling the (now-distinct) pool tex |
| 8 | self-routed Layer Router → saveLayerOutput, :171 | FIXED (was UB) | VERIFIED — early-return guard at :169-172, keyed off `layerOutputTexStorage_[layerId]` which is unconditionally populated by `ensureLayerOutputFBO` one line earlier (:164), so the map lookup can't silently read a fresh-inserted 0 and false-negative |
| 9-10 | temporal key split + `renderedAny` save gate | FIXED | VERIFIED — :283 `renderedAny` tracked explicitly, replaces the `currentInput != inputTex` check that misses an A→B→A chain |
| 11 | Screen Split target | FIXED | VERIFIED — `applyScreenSplit` now takes `dstFBO/dstTex` params (:1489-1491) picked by the caller |
| 21 | `applyGlobalEffects`, :1097-1100 | SAFE | VERIFIED — keyed `kGlobalEffectsLayerId`, separate from any real layer |
| 20 | `compositePersistentLayers`, :1012 | table: SUSPECT for state (disclosed unfixed) | AGREE — :1054 uses `layer.id` directly (no deck-scoping), same collision risk as any two decks sharing a layer id; correctly left as a disclosed gap, not silently dropped |
| 27 | `EffectChain.cpp` mid-chain dry/wet | table: BROKEN-UNFIXED, out of fence | VERIFIED out of fence — `src/effects/EffectChain.cpp:111` (`int writeFBO = 0`) is untouched by this diff; correctly declared out of scope, not silently ignored |

No site in the table is marked SAFE that I found to be unsafe. `applyLayerKeying`
/ `blendLayerOntoAccumulator` (CompositorEngine.cpp:1101, 1155) are untouched by
the diff and were already SAFE (they read the layer-transform / effect-chain
output, never `scratchTex_` as a sampler after this fix); consistent with
sweep row 17.

## GL state hygiene (packet item 2)
[OK] Every new/changed pass that does `glEnable(GL_BLEND)` also does
`glDisable(GL_BLEND)` before returning (dry/wet blend :424-434; same-source
crossfade copy explicitly `glDisable(GL_BLEND)` before drawing, :1327).
[OK] No per-frame heap allocation or new mutex introduced — grepped the
diff for `new`/`malloc`/`push_back`/`std::mutex`, none found; the only new
allocation is the one-time `effectFBO_C_/effectTex_C_` pair created in
`initGL`/`resize` (init-time, not steady-state).
[OK] The one new full-frame copy (same-source-type crossfade,
CompositorEngine.cpp:1310-1332) is gated tightly enough that it cannot run
outside an active crossfade between two same-type Source clips — not an
"unjustified" copy.
[N/A] Renderer.cpp's capture/Syphon/comp_transform paths (lines ~586-2170)
are untouched by this diff; I did not find a binding-state change in
CompositorEngine that would leak into those paths (compositeDeck's return
contract — leaving `accumulatorFBO_`/whatever last bound — is unchanged by
this fix; the caller in Renderer.cpp already explicitly rebinds before
using the result, per the existing, unmodified code at Renderer.cpp:593+).

## Trace instrumentation removal (packet item 3)
[OK] VERIFIED — `grep -rn "FBO_TRACE\|AUDIODNA_FBO_TRACE\|FEEDBACK!" src/`
returns nothing. The trace is preserved only as
`.harmony/.reports/s-rta-0926/xfade-trace.diff` (a doc artifact, not
applied to src) and `build-lane`/`build-pre` directories do not exist in
the worktree (confirmed with `ls`).

## Probe honesty (packet item 4)
[OK] `probe-crossfade.sh`: refuses without app/python/media
(`.harmony/probe-crossfade.sh:24-27`), refuses if Audio-DNA already
running, fresh `mktemp -d` per run (:28), checks `/api/health` polling
before proceeding. `probe-crossfade.py`: every `render_frame` response is
checked (`cap()`, :78-88, returns None + FAILs on a bad response or a
missing file), references-distinct assertion (`d(A,B) >= 20`, crossfade():
"references not distinct" FAIL path), the mid-transition "between A and B"
assertion is a real numeric bound (`da>floor and db>floor and
abs(da+db-dab)<=0.10*dab+floor`), plus a monotonic-progress check and a
"final matches IN" check — none of these can pass on an empty capture list
(`len(mids)<4` is an explicit FAIL).
[OK] `probe-effects-parity.py` (hardened): non-blank check per frame,
6 frames/variant, and a genuine PARITY-TO-REFERENCE numeric bound (V1/V5/V6
vs single-chain REF, tol 1.0) that would catch a stale/aliased pass
(report claims it moves the mean by tens on the unfixed build — plausible
given case-a's measured d=155.93/108.13 in the crossfade probe).
[OK] App path (`XFADE_APP`/`PARITY_APP`, default
`build/AudioDNA_artefacts/Release/Audio-DNA.app`) and fixture/media checks
(`REFUSE: media/P16_01_baseline.png not found`) are present in both
scripts — cannot silently run against a stale or wrong build.

## Test honesty (packet item 5) — tests/test_scratch_pool.cpp
[OK] Explicitly scoped in its own header comment: tests the REAL,
pure `ScratchPool::pick`, not a mirror, and explicitly disclaims GL/call-
site coverage (delegated to the live probe). 4 cases: exhaustive
never-sampled/never-held check, historic A/B ping-pong preserved when
nothing held, C used only while a pool texture is held, and a 2-member
pool provably cannot honor a hold (`-1`) — this is the "why 3, not 2"
proof and is a genuine negative case, not synthetic padding. The lane's
own report additionally ran the deliverable against a *mutated copy* of
ScratchPool.h (hold check removed) and got 3/4 failures — a real teeth
check per the negative-fixture requirement, done on a copy, not the tree
under review.

## Non-blocking notes
- Item #12 (BROKEN-UNFIXED, disclosed in xfade-report.md §5): the
  outgoing clip's effect chain in `applyTransition` (CompositorEngine.cpp:1339)
  is keyed by plain `layer.id`, the SAME key the active clip's own chain
  uses in `compositeDeck` (:901). During a crossfade, a temporal effect
  (Echo/Freeze) on the incoming clip's chain and one on the outgoing
  clip's chain read/write the same `u_prev_frame` buffer — cross-
  contamination that pre-dates this lane and is not introduced by it (both
  call sites already used `layer.id` before this diff), but it is a real,
  live gap in the crossfade path this lane is fixing. Correctly disclosed,
  not silently dropped, and the report itself calls out that it needs a
  design ruling (per-clip vs per-layer temporal history) rather than a
  quick patch. Flagging for follow-up, not blocking this round.
- Item #20 (persistent-layer id collision) and #27 (EffectChain.cpp
  dry/wet, out of fence) are likewise disclosed, unfixed, and correctly
  scoped out of this lane.
- `pickEffectTarget`'s `i<0` fallback (CompositorEngine.cpp:143,
  `ScratchTarget{fbos[0], texs[0]}`) is unreachable in production (the
  real pool always has 3 members) and is exercised only by the negative
  unit test against a 2-member array — acceptable as a defensive
  fallback, not a silent fail-open in the shipped path.

## SUMMARY
7 files reviewed (2 source + 1 new header + 1 new test + 3 probe/build
files), 0 blocking issues, 1 non-blocking design gap already disclosed by
the lane (#12) plus a minor documentation-locality nit. Independently
re-derived the rule-walk table against source (not recall) for every row
touched by this diff; confirmed trace removal by grep; confirmed both
probes assert real numeric invariants and cannot pass vacuously; confirmed
the new unit test is honestly scoped and has a real negative-fixture teeth
check on a mutated copy. Confidence: VERIFIED for all claims above (each
backed by a specific file:line read in this session); the lane's own
"GREEN" ctest/probe run counts (543/543, probe PASS tallies) are
self-reported by the lane and were not independently re-executed per the
read-only/no-build constraint of this review — INFERRED plausible from
internal consistency (539+4=543) and from the numeric deltas quoted in the
report (e.g. case-a d(OUT,IN)=155.93 matching mid-frame math).
