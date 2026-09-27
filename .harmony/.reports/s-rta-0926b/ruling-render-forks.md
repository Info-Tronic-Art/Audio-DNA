# RULING — render forks R1, R4-opaque, R4-router, R4-types (Audio-DNA, s-rta-0926b)

Chair: Architect (Fable). Source of truth read: the lane worktree at commit `9edb050f` (files cited as
`path:line @9edb050`). Seat papers were treated as advice; every claim below that matters was re-read in
the source or re-derived. Labels: VERIFIED = read in the pinned source / re-derived numerically here;
INFERRED = reasoned from the code, not run; ASSUMED = stated without a source.

## Summary

| Fork | Ruling | One line |
|---|---|---|
| R1 | **B′** — per-layer outgoing slot by handle swap (option B), with one refinement: the temporal buffer is COPIED to the outgoing slot (the incoming clip inherits the layer's picture exactly as a cut does today); the frame ring is SWAPPED (the incoming starts empty). Memory bounded to one spare buffer + one spare ring per (deck, layer); no per-frame cost; one blit per fade start. Rejected A (unbounded VRAM, changes cut semantics), C (the outgoing clip un-freezes / loses its grid the moment a fade starts), D (measured 1/3 ghost, worse at higher amounts). Not Boris's call on merit; one optional taste question below. |
| R4-opaque | **(1)** — blend over the active deck, honour layer opacity (route the persistent Opaque layer through the same alpha-keying pass a Transparent layer gets). (2) would black out every other deck from a default-typed layer; (3) leaves a dead slider. |
| R4-router | **(1)** — do not publish. The router resolves indices in the ACTIVE deck only (Renderer.cpp:1052-1061), so a persistent layer cannot even be addressed; publishing would re-open the R2 collision. |
| R4-types | **(1) for FX Only and media-less effect clips; (2) for Mask and 3D**, with the Persistent toggle disabled for those types through one pure rule (`Layer::canBePersistent`) that the compositor and the UI both consult. |

Seat verdicts vs this ruling: live-safety (B/1/1/1+2) — agreed, its "third clip contention" worry is refuted
below; visual-correctness (A/2) — rejected on both, reasons below; minimal-change (C/3/1/1+2) — C and (3)
rejected, its "fold the UI disable in now" adopted.

---

## R1 — whose history does each clip see during a crossfade?

### QUESTION
During a clip-to-clip crossfade on one layer, which Echo / Freeze / Posterize-Time buffer (`u_prev_frame`)
and which Screen Split / Frame Stutter ring should the outgoing and the incoming clip chain read and write?

### Mechanism (VERIFIED in the pinned source)
- `renderLayerStages` computes `clipKey = LayerStateKey::clipChain(deckId, layer.id)`
  (`src/render/CompositorEngine.cpp:821 @9edb050`), runs the INCOMING chain with it (`:827`), then
  `applyTransition(layer, clipKey, ...)` (`:832`) runs the OUTGOING chain with the same key (`:1398`).
  Both `applyClipEffects` calls read `tempBuf->tex` as `u_prev_frame` (`:381-391`) and save their output into
  the same buffer (`:465-466`); both push into the same ring (`:325-326`, `:1620-1621`).
- Frame order: incoming reads the buffer, saves; outgoing reads (= the incoming's output), saves. Fixed point at
  Freeze amount a: incoming = (B + a·A)/(1 + a), i.e. the other clip's share is a/(1+a) = 1/3 at 0.5 (the
  lane measured 0.34), 0.47 at 0.9, 0.5 at 1.0. Rings interleave pushes IN, OUT, IN, OUT, so "1 frame ago"
  is always the other clip (the lane's r2b-style row: 100%).
- Exactly ONE outgoing clip ever renders: `applyTransition` reads `layer.previousClipColumn` only
  (`:1342`), and a re-trigger during a fade REPLACES it (`Layer::triggerClipImmediate`, `src/model/Layer.h:263-265
  @9edb050`: `previousClipColumn = activeClipColumn; activeClipColumn = column; crossfadeProgress = 0`). So at
  most two chains per layer exist at any time — one spare slot is sufficient (this refutes the live-safety
  seat's "third clip contention" counterargument; there is no third chain).
- Cuts today: `triggerClipImmediate` never clears history; a clip triggered with `transitionSpeed <= 0` gets
  `crossfadeProgress = 1.0` (`Layer.h:265`), `applyTransition` returns immediately (`:1338-1339`), and the new
  clip's chain reads the layer buffer = the previous clip's last output. Trails/freezes CARRY ACROSS A CUT
  today whenever both clips carry a temporal effect. Nothing in the packet asks to change cuts ("Cuts are
  unchanged" is option B's own text), so any ruling must keep this.
- Feedback (`FeedbackProcessor`, `:834-839`) and the layer chain (`:841-847`) run AFTER the transition on the
  blended result; they are per-layer by nature and are NOT part of R1.
- Sizes (re-derived here, VERIFIED): temporal buffer = RGBA8 at render size: 8,294,400 B (7.9 MiB) at 1080p,
  33,177,600 B (31.6 MiB) at 4K, 2,655,072 B at the 756x878 preview (the lane wrote 2,654,976 — a 96-byte slip,
  immaterial). Ring = 480 RGBA8 textures + 480 FBOs at 1/4 x 1/4: 248,832,000 B (237.3 MiB / 248.8 MB) at
  1080p, 995,328,000 B (949.2 MiB) at 4K, 79,470,720 B at the preview. `CompositorEngine.h:223`'s "~120MB" is
  wrong by 2x (fix the comment in this build).
- Lifetime today: created lazily at first use (`:1477-1498`, `:1519-1557`), re-created on a render-size change,
  freed only in `releaseGL` (`:46-52`, `:67-76`). Never freed on clip change / layer delete / composition load.
- Temporal shader semantics that decide what "starting from nothing" looks like (VERIFIED,
  `src/render/EmbeddedShaders.h @9edb050`): Freeze = `mix(current, frozen, amount)` (`:8534`; default amount
  1.0, `EffectLibrary.cpp:682-684`) — with a black history at amount 1.0 the clip shows TRANSPARENT BLACK forever;
  Posterize Time shows the HELD frame between samples (`:8515-8519`) — with a black history that is black for up
  to one hold interval (1 s at rate 1.0); Echo builds trails from the current frame (`:8437-8460`) — harmless.

### APPROACH — B′
At the first frame of every crossfade on a layer, hand the layer's clip-chain history over to a per-layer
OUTGOING slot; the outgoing chain uses the slot's key for the whole fade, the incoming chain keeps the layer's
key:
- Temporal buffer: COPY layer → outgoing slot (one passthrough blit, existing `saveToTemporalBuffer`). The
  outgoing clip continues its freeze/trails unbroken from its own private copy. The incoming clip inherits the
  layer buffer as its first `u_prev_frame` — exactly what a cut gives it today — so Freeze 1.0 on an incoming
  clip holds the layer's last picture (not black), Posterize Time holds the last picture (not black) until its
  first sample, Echo starts from the layer's trails. Every temporal effect converges to its own fixed point at
  its own rate from there (Freeze 0.5: 1% of the inherited picture left after 7 frames).
- Frame ring: SWAP the map entries (O(1), vector moves) and reset the layer's ring (`frameCount = 0,
  writeIndex = 0`, no clears). The outgoing Screen Split / Frame Stutter keeps its 480 frames; the incoming
  starts empty, which already degrades gracefully today (fresh layer: `getFrameFromRing` clamps to the frames
  available, `:1587-1588`; Screen Split cells fall back to the current frame, `:1666`).
- Nothing happens at fade end: the outgoing slot simply idles as the spare for the next fade. It is
  overwritten (copy) or reset (ring) at the next fade start, so stale contents can never be read.

Why copy for the buffer but swap for the ring: a copy costs one full-frame blit (~0.1 ms at 1080p) and makes
fades consistent with cuts and avoids black frames for Freeze 1.0 / Posterize Time incoming clips; copying a
ring would be 480 blits, and an empty ring is already the well-behaved fresh-layer case. Pure swap for the
buffer (option B as written) would make a fade INTO a Freeze-1.0 clip go to transparent black for the whole
fade (a change the performer would read as "the clip is broken"); today's cut path does not do that.

### TRADEOFFS CONSIDERED
- **A. Per-clip history (deck + layer + clip)** — rejected. (i) Memory is unbounded until a lifetime policy
  exists: one 8 MB buffer per clip that ever ran a temporal effect and one 237 MB ring per clip that ever ran
  Split/Stutter (1080p; 949 MB at 4K). A 12-column layer with Split on every clip = 2.8 GB. (ii) It changes CUT
  semantics: with per-clip keys the previous clip's trails vanish at a cut and the new clip resumes its own
  stale trails from minutes ago (or black) — nobody asked for that, and "resume old trails" is the only thing A
  buys over B′. (iii) Every first trigger of a Split/Stutter clip pays the 480-object creation on the render
  thread (today: once per layer). The visual-correctness seat's premise ("a clip's chain output must depend only
  on its own history") is not the shipped model even for cuts (VERIFIED above), and its claim that B "recurs by
  design" the R2 latent collision conflates the feedback-processor key (a per-layer stage after the transition)
  with the clip chain. Choose A only if Boris explicitly wants "a re-triggered clip resumes the trails it had
  last time" — see the question below; nothing in the handoff asks for it.
- **B as written (swap both, incoming cleared to black)** — accepted in structure, refined to copy-for-buffer
  for the Freeze 1.0 / Posterize Time reasons above. Same memory bound, one extra blit per fade start.
- **C. Bypass the outgoing clip's history effects during the fade** — rejected on performer-visible grounds:
  the outgoing clip CHANGES LOOK at the exact moment the performer starts a fade — a frozen video un-freezes,
  a Screen Split grid collapses to a single full-frame image, Echo trails vanish — for the entire fade. The R3
  fix (lane commit f36bc71) exists precisely so the outgoing clip keeps the look it had while active; C would
  reintroduce a look-change at fade start for the whole Time category. "Reads as a transition characteristic"
  (minimal-change seat) does not survive the Freeze/Split cases. Memory 0 is not worth it.
- **D. Keep** — rejected: the ghost is 1/3 at Freeze 0.5 and grows to 1/2 at 1.0; rings show the other clip
  outright (100%).

### DECISION / SPEC (Builder-executable)

Files: `src/render/LayerStateKey.h`, new `src/render/CrossfadeHistory.h` (pure), `src/render/CompositorEngine.h`,
`src/render/CompositorEngine.cpp`, `src/render/Renderer.h/.cpp` (peak frame time, 6 lines),
`src/test/TestServer.cpp` (3 fields in `/api/state`), `tests/test_layer_state_key.cpp`, new
`tests/test_crossfade_history.cpp`, `tests/CMakeLists.txt`, `.harmony/probe-render-state.{py,json}`.

1. **Key** (`LayerStateKey.h`): add `inline constexpr std::uint32_t kOutgoingChainBit = 0x40000000u;` and
   `constexpr std::uint64_t outgoingChain(deckId, layerId) { return clipChain(deckId, layerId | kOutgoingChainBit); }`.
   Update the header comment: a layer now has THREE state keys (clip chain, outgoing clip chain, layer chain);
   real layer ids never reach bits 30/31 (`Deck::nextLayerId_` starts at 100, `src/model/Deck.h:198`).
2. **Pure detector** (`CrossfadeHistory.h`, no GL, no JUCE):
   ```cpp
   struct CrossfadeStartDetector {
       bool wasRunning = false; int lastPrev = -1, lastActive = -1; float lastProgress = 1.0f;
       // true exactly on the first frame of a crossfade: the layer is crossfading now AND either it was not
       // last frame, or the (previous, active) pair changed (re-trigger mid-fade), or progress went backwards
       // (a PerfState/routine restore). Never true for a cut (progress >= 1) or with no previous clip.
       bool observe(int prev, int active, float progress) noexcept {
           const bool running = progress < 1.0f && prev >= 0;
           const bool start = running && (!wasRunning || prev != lastPrev || active != lastActive
                                          || progress < lastProgress);
           wasRunning = running; lastPrev = prev; lastActive = active; lastProgress = progress;
           return start; } };
   ```
   Thread note (INFERRED, pre-existing class): `Layer` fields are written on the message thread and read here
   without atomics, like every other Layer field the compositor reads. The detector is correct under every
   observable write ordering of `triggerClipImmediate` (`Layer.h:263-265`): the intermediate states
   (prev=X,active=X,p=1) and (prev=X,active=Y,p=1) are "not running"; the first frame that shows p<1 with the
   new pair fires the start. `advanceCrossfade` (`:786-813`) runs BEFORE `renderLayerStages` and only increases
   progress, so a fade never fires twice; dt = 0 keeps progress equal (not less), so no false start.
3. **Compositor state** (`CompositorEngine.h`): `std::unordered_map<uint64_t, CrossfadeStartDetector> crossfadeStart_;`
   (keyed by `clipKey`; cleared in `releaseGL`); `std::atomic<int> temporalBufferCount_{0}, frameRingCount_{0};`
   with public const getters (incremented where `createFBO` runs in `getOrCreateTemporalBuffer` / the ring
   loop in `getOrCreateRingBuffer`, decremented on their release paths and zeroed in `releaseGL`; atomics because
   the test server reads them from the HTTP thread — the same pattern as `Renderer::frameTimeMs_`).
   New private `void handOverClipHistory(uint64_t clipKey, uint64_t outKey, ShaderManager&, FullscreenQuad&, int w, int h);`.
4. **Handover** (`CompositorEngine.cpp`, `handOverClipHistory`), GL-state rules: runs OUTSIDE any pass (it may
   create FBOs — `createFBO` rebinds `GL_TEXTURE_2D` on the active unit and leaves framebuffer 0 bound, the R5
   lesson at `:272-280`); every pass after it binds its own FBO/textures, so nothing needs restoring; it ends
   with framebuffer 0 bound.
   ```
   temporal: auto itL = layerTemporalBuffers_.find(clipKey);
             if (itL != end && itL->second.tex != 0) {
                 auto& out = getOrCreateTemporalBuffer(outKey, w, h);        // creates/resizes, no pass bound
                 saveToTemporalBuffer(out, itL->second.tex, ...);            // one blit; src != dst always
             } else if (auto itO = layerTemporalBuffers_.find(outKey); itO != end && itO->second.fbo != 0) {
                 bind itO->second.fbo; glClearColor(0,0,0,0); glClear; bind 0   // stale spare, nothing to copy
             }
   ring:     std::swap(layerRingBuffers_[clipKey], layerRingBuffers_[outKey]);   // O(1): vector moves
             auto& r = layerRingBuffers_[clipKey]; if (r.initialized) { r.frameCount = 0; r.writeIndex = 0; }
   ```
   `unordered_map::operator[]` default-constructs missing entries; `getOrCreateTemporalBuffer`/`getOrCreateRingBuffer`
   already treat `tex == 0` / `!initialized` as "create" (`:1480`, `:1525`), so an empty entry left on either
   key is created lazily on first use, before any pass binds (`:281`, `:325`, `:1620`). `releaseGL` iterates
   whole maps and skips zero handles — unchanged.
5. **Call sites** (`renderLayerStages`, `:815-851`): right after computing `clipKey`, add
   `const uint64_t outKey = LayerStateKey::outgoingChain(deckId, layer.id);
    if (crossfadeStart_[clipKey].observe(layer.previousClipColumn, layer.activeClipColumn, layer.crossfadeProgress))
        handOverClipHistory(clipKey, outKey, shaderMgr, quad, width, height);`
   BEFORE the incoming `applyClipEffects` (`:827`), then `applyTransition(layer, outKey, ...)` (`:832`) so the
   outgoing chain (`:1398`) keys by `outKey`. Update the `applyTransition` doc comment (`CompositorEngine.h:339-347`)
   and the "open fork R1" note at `:1395-1397`. Both `compositeDeck` and `compositePersistentLayers` get this for
   free (shared function). The order incoming-then-outgoing inside a frame is unchanged.
6. **Instrumentation** (small, needed to prove the memory bound and the hitch): `Renderer`: `std::atomic<float>
   peakFrameTimeMs_`, updated with `max` next to the EMA at `Renderer.cpp:733-735`, `float takePeakFrameTimeMs()`
   = `exchange(0)`. `TestServer::handleState` (`src/test/TestServer.cpp:590-632`): add `"temporal_buffers"`,
   `"frame_rings"` (from `renderer_.getCompositor()`, public at `Renderer.h:92`) and `"peak_frame_time_ms"`.
7. **Comment fix**: `CompositorEngine.h:223` "~120MB at 480x270" → "248.8 MB (237 MiB) at 1080p, 995 MB at 4K;
   two per (deck, layer) clip chain after a fade with Split/Stutter on both sides".

Lifetime rule (creation / hand-over / free), stated plainly: created lazily on first use (unchanged);
handed over at the first frame of each crossfade (copy for the buffer, swap for the ring); the outgoing slot is
never freed per fade — it is the layer's spare, reused by every later fade; all freed in `releaseGL`
(unchanged). Bound: at most 2 temporal buffers + 2 rings per (deck, layer) clip chain, plus the layer chain's
1 + 1 and Global Effects' 1 + 1 — i.e. +1 buffer and +1 ring per layer that ever crossfades with those
effects, never more.

Frame time: zero per-frame cost. Per fade start: one 8 MB blit (~0.1 ms, INFERRED) plus, ONCE per layer,
the creation of the spare buffer (one `glTexImage2D`) and — only if the incoming clip uses Split/Stutter —
the spare ring (480 FBO+texture creations, the same hitch the first use on a layer pays today; not measured
by the lane; the `peak_frame_time_ms` field makes the builder measure it — report the number; if it exceeds
~50 ms, pre-create the spare ring at the moment the first ring is created for that layer, which moves the
hitch to first use where it already exists rather than adding one at fade start).

### Tests (real code path, RED on the merge base = lane HEAD `9edb050`)
Unit (ctest):
- `tests/test_layer_state_key.cpp`: extend case 2 with `outgoingChain`: distinct from `clipChain`,
  `layerChain` and `kGlobalEffects` for every (deck, layer) in the existing grids; case 3 stability. RED by
  absence (does not compile on base); show teeth on a mutated copy of the header (drop the bit) as the lane did.
- new `tests/test_crossfade_history.cpp` (Catch2 only, registered like `test_scratch_pool`): (a) not running
  → never fires; (b) first running frame fires once, later frames with rising progress do not; (c) re-trigger
  mid-fade (pair (X,Y) → (Y,Z)) fires again; (d) same pair, progress lower than last (restore) fires; (e) a cut
  (progress 1.0, prev >= 0) never fires and resets `wasRunning`; (f) the write-ordering sequence
  (X,X,1.0) → (X,Y,1.0) → (X,Y,0.0) fires exactly once, on the last; (g) dt = 0 (equal progress) does not
  fire. RED by absence.
Live (`.harmony/probe-render-state`, decoded pixels, WipeLeft = `transitionMode 26`, T = 8 s, region metric
from the lane's `diag.py r1`: IN region x < (p-0.12)·W vs refB, OUT region x > (p+0.12)·W vs refA):
- `r1_temporal`: OUT = A + [Freeze 0.5], IN = B + [Freeze 0.5]. PASS: every mid frame (t in 1.0..0.8T, >= 4
  frames) has d(f,refB)/d(A,B) <= 0.05 in the IN region AND d(f,refA)/d(A,B) <= 0.05 in the OUT region.
  Calibration: base 0.34 / 0.32-0.33 (lane, VERIFIED); expected fixed ~0.00 (the inherited picture decays to
  <1% within 7 frames, first capture at >= 60 frames).
- `r1_control`: same with OUT without Freeze — 0.00 on both builds (guards the metric).
- `r1_ring`: OUT = A + [Frame Stutter 0.0 0.0], IN = B + [Frame Stutter 0.0 0.0]. PASS: same 0.05 bounds.
  Calibration (INFERRED from the interleaving): base ratio ~1.00 in BOTH regions (each clip shows the other);
  fixed 0.00.
- `r1_retrigger`: OUT = A + [Freeze 0.5], IN = B + [Freeze 0.5]; trigger B, at t = 2 s trigger A again
  (Y→X fade, A is now the IN side). PASS from 1 s after the re-trigger: IN region vs refA and OUT region vs
  refB both <= 0.05. RED on base (shared key), GREEN proves the second handover (spare re-used).
- `r1_counts` (memory bound, `/api/state`): one layer, 6 columns alternating A/B each with [Screen Split
  0.15 0.15 0.25 0], fade through all 6 with T = 1 s. PASS: `frame_rings` == 2 and `temporal_buffers` <= 2 after
  the run (base reads 1 — RED; A would read 6). Also log `peak_frame_time_ms` sampled right after the first
  fade; FAIL only above 50 ms; report the value in the lane report.
Existing rows that must stay GREEN: probe-render-state r2_*, r4_*, r5_hold, r5_burst; probe-crossfade a–l;
probe-effects-parity; ctest 583/583 + new.

### RISKS
- Strongest counterargument to B′: A is the semantically pure model and gives "resume own trails"; B′ keeps a
  hybrid per-layer model. It loses because the hybrid IS the shipped cut model, A's cost is unbounded without a
  policy the lane has not scoped, and no user asked for resume. If Boris wants resume, A can be layered later
  on top of B′'s keys (the outgoing slot machinery stays valid).
- Under B′ a fade INTO a clip with Freeze 1.0 holds the layer's last picture (as a cut does); the visual-
  correctness seat would call that contamination. It is deliberate and consistent; see the taste question.
- VRAM (medium, pre-existing class): rings are 249 MB each at 1080p and now up to 2 per (deck, layer) clip
  chain; 4K rings are ~1 GB each. Not made worse per frame; made worse by one spare per fading layer. A general
  idle-reclaim policy (free a ring untouched for N s; re-creation is a hitch, content loss is invisible because
  a resumed ring is stale anyway) is a separate follow-up, not part of this build.
- One-time hitch at the first fade per layer with Split/Stutter on the incoming side — measured by the new
  field; mitigation named above.
- Transition shorter than one frame (`transitionSpeed` tiny): progress reaches 1.0 in the first
  `advanceCrossfade`, the detector never sees "running", no handover — and the transition is effectively a cut
  (applyTransition returns early), so the cut semantics apply: consistent.
- `render_frame` capture cadence differs between builds (lane note); the rows sample at >= 1 s so they do not
  depend on frame alignment.

### What must NOT change
Cut behaviour (no handover, no clear); the incoming-then-outgoing order inside `applyTransition`; ScratchPool /
`holdTex` rules; the R3 stage order for the outgoing clip; `applyClipEffects` internals; `layerChain` and
`kGlobalEffects` keys; `FeedbackProcessor` keying; `releaseGL`/`resize` semantics; r5 rows' fresh-history
semantics (a fresh buffer is transparent black); no new `Layer` model fields (the detector is compositor state).

### Boris (optional, taste, non-blocking — default applied if unanswered)
"When a new clip comes in on a layer — cut or fade — its time effects (Echo trails, Freeze, Posterize) start
from what the layer was just showing: trails carry over and a Freeze holds the picture that was there. That is
what a cut does today and this fix makes fades do the same. Do you instead want every incoming clip to start
its time effects from a blank slate?" Default: carry over (B′ as specified). A "resume the trails this clip
had last time it played" answer means option A on top of B′ — say so and it becomes a follow-up lane with a
memory policy.

---

## R4-opaque — a persistent Opaque layer over another deck

### QUESTION
Should a persistent Opaque layer, rendered while another deck is active, replace everything (clear the
accumulator and apply layer opacity as the active-deck Opaque path does) or blend over the active deck?

### Facts (VERIFIED)
- Active-deck Opaque: clears the accumulator to opaque black and draws with `opacity_blend`
  (`CompositorEngine.cpp:966-999`), alpha-blending only when `layer.eff(Opacity) < 0.999`.
- Persistent Opaque: `blendLayerOntoAccumulator(layer, processedTex)` (`:1121-1124`), which applies the layer's
  `blendMode` and NEVER reads layer opacity (`:1197-1282`: no `u_opacity`). A persistent Transparent layer DOES
  honour opacity, through the keying pass (`applyLayerKeying`, `u_opacity` at `:1185`; `key_alpha` =
  `vec4(col.rgb, col.a * u_opacity)`, `EmbeddedShaders.h transparencyAlpha`). `opacity_blend` has the identical
  body (`EmbeddedShaders.h opacityBlend`).
- Persistent layers are composited AFTER the active deck, in deck order (`Renderer.cpp:509-519`), i.e. always at
  the TOP of the stack. `Layer.h:40` "replaces everything below" therefore means "hides the whole active deck".
- The first layer of every deck is Opaque by default (`Deck::initDefault`, `src/model/Deck.h:35`); a layer's
  default blend mode is Additive (`Layer.h:98`).

### APPROACH — (1)
A persistent Opaque layer composites over the active deck with its blend mode, honouring layer opacity exactly
the way a persistent Transparent layer does: through the alpha-keying pass. Implementation: give
`applyLayerKeying` an explicit `Layer::KeyingMode mode` parameter (existing callers pass `layer.keyingMode`);
in `compositePersistentLayers` the Opaque branch becomes: if `layer.eff(LayerScalar::Opacity) < 0.999f`,
`applyLayerKeying(layer, Layer::KeyingMode::Alpha, processedTex, scratchFBO_, ...)` then
`blendLayerOntoAccumulator(layer, scratchTex_, ...)`; else today's direct blend (so the GL sequence at
opacity 1.0 — every existing r2/r4 row — is byte-identical). `processedTex` is never `scratchTex_`
(`applyLayerTransform` avoids it, `:635-641`), so the keying pass never samples its own target.

### TRADEOFFS
- **(2) true Opaque (hide the whole active deck)** — rejected. The visual-correctness seat's premise ("the same
  layer must look the same on every deck") fails because a persistent layer's position in the stack differs by
  construction (always top). Under (2), marking the DEFAULT-typed first layer of any deck persistent blacks
  out every other deck with no error and no visible cause — the worst live failure mode available here, and it
  buys nothing the performer cannot get by switching to that deck. The only defensible reading of "persistent"
  (tooltip `LayerInspector.cpp:157`: "Keep this layer rendering when switching to another deck") is an overlay.
- **(3) keep** — rejected: the opacity slider on such a layer is dead; a RED row for it is trivial (below).
- Alpha-scaling vs RGB-scaling for opacity: alpha, to match Transparent persistent layers exactly. Screen /
  Multiply / Darken / Lighten ignore source alpha in `blendLayerOntoAccumulator` — a pre-existing property shared
  with Transparent layers (documented at `applyClipOpacity`, `:570-586`), unchanged here.

### Tests
- `r4_opaque_opacity`: base deck 0 layer 0 Opaque = A; deck 1 layer id 5 Opaque, persistent, blendMode 0
  (Normal), opacity 0.5, clip = B (covers the frame). Reference = the same layer id 5 as a NORMAL Transparent
  layer (Alpha keying, opacity 0.5) on deck 0 above A. PASS: d(subject, reference) <= tol 1.5 and d(reference,
  subject-at-opacity-1.0) >= 5 (the stage is exercised). Calibration (INFERRED): base subject = B, i.e.
  d ≈ 0.5·d(A,B) ≈ 15; fixed 0.00.
- `r4_opaque_overlay` (guard, PASS on both builds — pins the "blend over, not hide" decision): same layer,
  opacity 1.0, clip scale 0.5, Normal blend. PASS: the frame outside the centre quarter equals A (d <= tol);
  under (2) it would be black.
- Existing r2_temporal / r2_ring rows use a persistent Opaque layer at opacity 1.0 — unchanged path, must stay
  GREEN.

### RISKS / must not change
Opacity 1.0 path unchanged. The active-deck Opaque path unchanged. The keying pass adds one full-frame draw per
frame only for persistent Opaque layers below opacity 0.999 (INFERRED ~0.1 ms). Boris check (confirmation
only, not blocking): "When you are on Deck 2 and a Deck 1 layer is marked Persistent, it sits on top of Deck 2
with its blend mode and its opacity. It never hides Deck 2. OK?"

---

## R4-router — Layer Router output from persistent layers

### Ruling — (1) do not publish (no code change; one doc line).
VERIFIED: `saveLayerOutput(layer.id, ...)` runs only in `compositeDeck` (`:956`); `layerOutputTextures_` is
keyed by bare layer id (`CompositorEngine.h:372`); `Renderer::renderSource` maps the router's index to an id IN
THE ACTIVE DECK (`Renderer.cpp:1052-1061`), so a persistent layer of another deck is not addressable at all —
publishing under its id would only overwrite the active deck's same-id output (ids 0,1,2 repeat in every deck:
`Deck.h:34`), the R2 collision class. (2) needs a deck axis in the router's param, key and UI — a feature with
its own lane; if ever wanted, step one is re-keying `layerOutputTextures_` by `LayerStateKey::clipChain`. (3) is
a state-dependent source that flips between decks' content — worse than absent. Doc line for
`performance-controls.md` Layer Router section: "Only the active deck's layers publish output; a persistent
layer of another deck is never a router source."

---

## R4-types — FX Only / Mask / media-less effect clips as persistent layers

### Facts (VERIFIED)
`compositePersistentLayers` skips every type but Opaque/Transparent (`:1078-1080`, "for now") and skips a
media-less clip (`:1101`), which the active-deck path applies as FX Only (`:940-947`, `applyFXOnlyLayer` with
the deck-scoped key); the FXOnly type is applied at `:1003-1008`. `applyFXOnlyLayer` (`:669-733`) is
self-contained (accumulator in, accumulator out, `scratchFBO_` only inside). No runtime writer of `Layer::type`
exists in `src/` (grep: only `Deck::initDefault`/`addLayer`/`fromVar`), so the toggle's enabled state only needs
refreshing in `LayerInspector::syncFromLayer` (`LayerInspector.cpp:880-`, runs from `setLayer` and `refresh`).

### Ruling — (1) for FX Only and media-less effect clips; (2) for Mask and 3D, toggle disabled.
- One pure rule in `src/model/Layer.h`: `static constexpr bool canBePersistent(Type t) { return t == Type::Opaque
  || t == Type::Transparent || t == Type::FXOnly; }` — consulted by BOTH `compositePersistentLayers` (replace the
  type gate at `:1079`) and `LayerInspector::syncFromLayer` (`persistentToggle_.setEnabled(Layer::canBePersistent(
  layer_->type))`, next to `:920`; tooltip when disabled: "Persistent is available for Opaque, Transparent and
  FX Only layers"). A Mask layer already marked persistent in a saved file stays skipped (unchanged) and shows
  checked-but-disabled.
- `compositePersistentLayers`: for `type == FXOnly` call `applyFXOnlyLayer(*clip, layer,
  LayerStateKey::clipChain(deck.id, layer.id), ...)` and `continue` (mirror `:1003-1008`); for Opaque/Transparent
  with `clipTex == 0`, `if (clip->hasEffects()) applyFXOnlyLayer(...)` then `continue` (mirror `:940-947`).
  Semantics: an FX Only persistent layer is a look over whatever is on screen at that point of the persistent
  pass (active deck + persistent layers of lower-index decks) — exactly the accumulator semantics it has on its
  own deck.
- Mask persistent: skipped. Masking "everything below" across decks has no fixture, no user ask, and an
  unresolved order question (two decks with persistent masks). 3D is unimplemented on every path (`:1033-1035`).

### Tests
- Unit (`tests/test_compositor.cpp`, data-model style): `Layer::canBePersistent` truth table over all five
  types. RED by absence.
- `r4_fxonly_persistent`: base deck 0 layer 0 Opaque = A; deck 1 layer id 5 type 2 (FXOnly), persistent, clip =
  {mediaType 0, effects [Invert 1.0]}. Reference = the same FX Only layer as a normal layer on deck 0 above A.
  PASS: d(subject, reference) <= 1.5 and d(reference, A) >= 5. Base: subject = A (RED; the lane's r4b Invert
  calibration is d = 228 for this stage).
- `r4_fxonly_medialess`: same with layer type 1 (Transparent) and the media-less effect clip. PASS/RED as above.
- `r4_mask_skipped` (guard, PASS on both builds): deck 1 layer id 5 type 4 (Mask), persistent, clip = B. PASS:
  frame == A within tol.
- Builder must confirm `Clip::fromVar` accepts `mediaType 0` with an effects array (INFERRED from
  `Clip.cpp:173`; if not, load via the REST clip-edit path the r4 rows already use).

### Must not change
Active-deck FX Only / Mask paths; `applyFXOnlyLayer`'s opacity product rule (`combinedOpacity`, `:685`); the
found_not_fixed #1 behaviour (persistent layers — including FX Only ones — vanish when the active deck has no
active clip, `Renderer.cpp:543-568`) is a separate item, not touched here.

---

## Build order (one commit each; every commit RED-first as above)
1. R1 pure: `LayerStateKey::outgoingChain` + `CrossfadeHistory.h` + unit tests.
2. R1 compositor: handover, `outKey` into `applyTransition`, counters, `Renderer` peak field, `/api/state`
   fields, comment fix; probe rows r1_*.
3. R4-opaque: `applyLayerKeying(mode)` + persistent Opaque branch; rows r4_opaque_*.
4. R4-types: `Layer::canBePersistent`, `compositePersistentLayers` FX Only + media-less, LayerInspector toggle;
   unit test + rows r4_fxonly_*, r4_mask_skipped.
5. Docs (outside `src/render`, Harmony's call): `docs/claude/rendering.md:66` (keyed by `LayerStateKey` = deck +
   layer + chain, with an outgoing slot during a crossfade), `:68` (ring = 249 MB at 1080p, up to two per layer
   clip chain), `docs/claude/performance-controls.md:39` (persistent layers get every per-layer stage; Opaque
   persistent blends over the active deck honouring opacity; FX Only supported; Mask/3D not, toggle disabled;
   never a router source); a Pitfall 35 candidate: "a layer has two live clip chains during a crossfade — never
   key per-chain GL history by (deck, layer) alone".

## Gates
`ctest` serial: 583 + new, 0 failed. `RSTATE_APP=<fix app> .harmony/probe-render-state.sh` all rows PASS
(base: r1_*, r4_opaque_opacity, r4_fxonly_* FAIL; guards PASS on both). `probe-crossfade` 35/35,
`probe-effects-parity` 46/46 unchanged. Look at one decoded frame of r1_temporal mid-fade and of
r4_opaque_opacity.

STATUS: DONE — ruling complete on all four forks; R1 = B′ (per-layer outgoing slot; copy buffer / swap ring), R4-opaque = (1), R4-router = (1), R4-types = (1) FX Only + media-less, (2) Mask/3D with the toggle disabled; specs, RED-first tests, live rows, risks and must-not-change lists above; one optional taste question for Boris on R1, one confirmation on R4-opaque.
