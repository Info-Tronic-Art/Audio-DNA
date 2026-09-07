# Reviewer Verdict — L2-layer-solo-s-rta-0904
STATUS: DONE
VERDICT: PASS_WITH_CONCERNS
FILES: /Users/boriskarpman/projects/RealTimeAudio/src/render/CompositorEngine.cpp (lines 645-691; diff is exactly 3 hunks, +14/-2)

## 1. BOTH loops, not one — plus a third site the plan didn't mention

Both targeted loops in `compositeDeck` (line 660, the active-content scan, and
line 690, the actual composite loop) carry the identical skip clause
`(anySolo && !layer.solo)`. Confirmed textually identical.

There IS a third site iterating `deck.layers` with the same read class
(`visible`/`bypassed`) that was NOT touched: `compositePersistentLayers()` at
`CompositorEngine.cpp:862`:
```
if (!layer.persistent || !layer.visible || layer.bypassed)
    continue;
```
No solo check. This function is called from `Renderer.cpp:451-461` once per
frame, for every deck OTHER than the currently-active one
(`otherDeck == deck ? skip : compositePersistentLayers(otherDeck, ...)`), to
paint that other deck's `persistent` layers onto the accumulator on top of the
active deck's output.

This is a real gap, not a style nit: `layer.solo` is a per-Layer model field
(`Layer.h:48`), not deck-scoped, and it persists on a deck even while that
deck is inactive. Concretely: solo a persistent layer in deck B, then make
deck A active. `compositeDeck(A, ...)` correctly honors solo state within A.
But `compositePersistentLayers(B, ...)` — called every frame for the
now-inactive B — still composites EVERY visible/non-bypassed persistent layer
in B, including ones that would have been solo-suppressed had B stayed
active. Solo's effect on a deck's persistent output is inconsistent depending
on whether that deck happens to be the active one this frame — same read
class, same file, same author's stated rule ("same read class as
visible/bypassed"), left out.

Scanned exhaustively — `grep -n "\.layers\b" src/render/*.cpp` finds all
`deck.layers` iteration lives only in `CompositorEngine.cpp`, 4 sites total:
lines 652 (new anySolo scan), 658 (active-check, fixed), 688 (composite,
fixed), 862 (`compositePersistentLayers`, NOT fixed — the gap above). The
other `for` loops in the file (lines 44/52/56/65/99/101) iterate
`layerTemporalBuffers_` / `layerOutputFBOs_` / GL-resource maps keyed by layer
id for cleanup/allocation — not rendering-decision loops, correctly out of
scope for solo. `Renderer.cpp` and `MainComponent.cpp` also index into
`deck->layers` (clip lookup, column resolution, clip removal) but those are
data operations, not "does this layer render" decisions — not in scope.

**Verdict on this axis: half-fix.** Recommend either extending the same
one-line skip to `compositePersistentLayers:864`, or an explicit comment
there stating persistent-cross-deck layers are deliberately solo-exempt (if
that's the actual intended design) — silence reads as an oversight given the
plan's own "same read class" framing.

## 2. Semantics and precedence

(a) Internally consistent: yes, the exact same boolean expression
`!layer.visible || layer.bypassed || (anySolo && !layer.solo)` appears at
both fixed sites, so solo strictly narrows an already-visible/non-bypassed
set — it can't resurrect a hidden or bypassed layer, in either loop.

(b) Matches the UI's promise: no promise found to contradict it. The solo
button (`LayerStrip.cpp:328,339-343`) has label `"S"` (`LayerStrip.h:82`),
sets `layer_->solo` directly, and has **no `setTooltip()` call anywhere** —
grepped `LayerStrip.cpp`/`.h` for "tooltip", zero hits. The OSC doc comment
(`OscHandler.h:17`) just says "toggle layer solo"; the undo-command comment
(`DeckCommands.h:329`) says "#15 solo" with no semantic elaboration; no test
asserts solo-overrides-visibility semantics (`tests/test_undo_commands.cpp`
only round-trips the boolean flag, doesn't touch rendering). So there is no
UI copy or spec text this implementation contradicts — the chosen semantics
(solo narrows, never un-hides) is a defensible reading of the only sentence
that exists ("same read class as visible/bypassed"), but it is also an
UNDOCUMENTED product decision the UI gives the user zero signal about (no
tooltip distinguishes "soloed but still hidden" from "soloed and visible").
Not a defect in this diff; flagging because nothing upstream of this diff
disambiguates it either, so if Boris's mental model is "solo always wins",
this diff will surprise him.

## 3. Cost and placement

Confirmed hoisted correctly: `anySolo` is computed once (single loop,
`break`s on first hit, `CompositorEngine.cpp:651-655`) before either
consuming loop, is a plain local `bool` (not a reference or lazily-evaluated
expression), and is computed over the exact same `deck.layers` (same `Deck&
deck` parameter) that both consuming loops iterate — no copy, no second
`Deck` in play. The only control-flow between its computation and its second
use is the early `return 0` at line 676 (`if (!hasActiveLayers_ ||
!glInitialized_) return 0;`), which exits the function outright rather than
skipping past a stale read — a local `bool` cannot go stale across a
same-thread, single-invocation function body with no reassignment. No
staleness path exists.

## 4. `hasActiveLayers_` — the subtle one, answered from consumers

Traced every consumer. The public getter `hasActiveLayers()`
(`CompositorEngine.h:86`) has **zero call sites in the entire repo** — grepped
`hasActiveLayers()` (with parens) across `src/`, the only match is its own
declaration. Dead externally; not a concern from this diff (pre-existing).

The real consumer is internal: `CompositorEngine.cpp:676`,
`if (!hasActiveLayers_ || !glInitialized_) return 0;` — this is the sole
functional gate. Traced downstream in `Renderer.cpp:443-467`:
`sourceTexture = compositor_.compositeDeck(...)`; if that returns 0,
`Renderer.cpp:469` falls through to `if (sourceTexture == 0 && sourceActive)
{ ... render active procedural source instead ... }` — i.e. a `0` return is
read as "the whole deck produced nothing" and triggers a **different visual
source entirely** (whatever procedural source or loaded image is configured
as fallback), not a blank/transparent frame. This fallback contract is
documented at `MainComponent.cpp:3436-3453` ("the renderer's fallback path
only matters when ... hasActiveLayers_ false").

Is gating this flag by solo correct? For the common case (soloed layer has
real content) it's correct and necessary — without it, hasActiveLayers_ would
stay true from a non-soloed layer while the actual composite loop (already
solo-gated) renders nothing, silently desyncing the two loops.

But there's a real edge case, exactly as the prompt suspected: **solo an
empty layer** (a layer with `solo=true` but `getActiveClip()==nullptr` or no
media) while a DIFFERENT, non-soloed layer has genuine visible content. Before
this diff, solo was inert everywhere, so this scenario didn't exist. After
this diff: the first loop (line 660) also skips the non-soloed content-having
layer, finds nothing among soloed layers, `hasActiveLayers_` stays false,
`compositeDeck` returns `0` — and per the Renderer fallback contract above,
the app now paints an UNRELATED fallback source/image instead of the blank
composite a "solo an empty channel = silence" mental model would predict.
Whether this is "harmless" or "latent bug" depends on whether that fallback
path can currently trigger at all when a deck is genuinely active with other
content — that's a product-intent question I can't resolve from source alone
(see below), but it IS a behavior that did not exist before this diff and
is a direct, traceable consequence of gating the first loop, not the second.
Also note: `MainComponent.cpp:3444`'s comment describing `hasActiveLayers_`
semantics ("no visible, non-bypassed layer has an active clip") is now
incomplete — it doesn't mention solo — a documentation-staleness nit, low
severity, easy fix (append "and not solo-excluded").

## 5. The inherited claim ("compositor never reads solo today")

Verified independently against HEAD with two patterns across every file in
`src/render/`, not just `CompositorEngine.cpp`:
```
git show HEAD:src/render/CompositorEngine.cpp | grep -ni "solo"   → no output, exit 1
git show HEAD:src/render/CompositorEngine.cpp | grep -n "\.solo\b" → no output, exit 1
git show HEAD:src/render/Renderer.cpp | grep -ni "solo"           → no output, exit 1
```
Then swept all 14 files under `src/render/*.cpp`/`*.h` at HEAD with both
patterns (case-insensitive "solo" substring AND `.solo` member-access) —
zero hits anywhere. Claim confirmed: the entire render path was fully blind
to `layer.solo` before this diff. (Contrast with the cited
`lockResolution`/`setLockedResolution` failure mode — I did not rely on one
grep pattern, and checked the whole directory, not just the one file.)

## DEFECTS

1. **[MEDIUM] `src/render/CompositorEngine.cpp:862-865`** —
   `compositePersistentLayers()` checks `!layer.persistent || !layer.visible
   || layer.bypassed` but omits the `(anySolo && !layer.solo)` clause added
   to the other two loops in this same file. A soloed-but-inactive-deck's
   persistent layers keep contributing to every OTHER deck's composite,
   inconsistent with the solo semantics just established two functions
   above. Fix: compute an `anySolo` scan local to this function (same
   3-line pattern) and add the clause, OR add a one-line comment explaining
   why persistent cross-deck layers are intentionally solo-exempt if that's
   the real intent. This is the "third site" the review was asked to hunt
   for.

2. **[LOW] `src/MainComponent.cpp:3436-3453`** — the comment documenting
   `hasActiveLayers_`'s semantics is now stale (omits solo). Doc-only, no
   functional impact; update for the next reader who trusts the comment
   over the code.

3. **[LOW / informational] No tooltip on the solo button**
   (`src/ui/LayerStrip.cpp:328`) to disambiguate "solo doesn't un-hide a
   hidden layer" from a user's likely mental model. Not introduced by this
   diff, but this diff is what first makes that distinction behaviorally
   real; worth a one-line tooltip while the behavior is fresh.

4. **[LOW / informational] No compositor-level regression test added.**
   `tests/test_compositor.cpp`'s existing "Deck layer compositing data
   model" test (lines 46-58) only exercises the `Layer` struct's
   `visible`/`bypassed` fields directly, never calls `compositeDeck` itself
   (consistent with this repo's existing pattern — no GL-context test
   harness for the actual skip logic, so this is not a new gap this diff
   introduces, just one it inherits and doesn't close). If a data-model-only
   test is the achievable ceiling here, mirror lines 46-58 for `solo` at
   minimum so a future regression on the flag itself is caught even without
   testing the render-time skip.

## WHAT I COULD NOT DETERMINE FROM SOURCE ALONE

- Whether the empty-soloed-layer → fallback-source-paints-through scenario in
  §4 is reachable in practice today (i.e., whether `sourceActive` can be true
  at the same moment a deck with genuine layer content is also active) — this
  needs either a runtime check or someone who knows the fallback's intended
  triggering conditions.
- Whether item 1 (persistent-layers solo gap) is a deliberate design choice
  (persistent layers as "always-on background regardless of the active
  deck's solo state") or an oversight — I found no comment or spec text
  either way.

## RUNTIME CHECK (REST-only, deterministic, no output window)

No REST endpoint sets `layer.solo` directly — grepped every
`server_.Get`/`server_.Post` registration in `ApiServer.cpp`; there's
`/api/set_layer_opacity` but no `/api/set_layer_solo` and no generic
`/api/set_param` path that reaches the solo bool (solo is only settable via
UI click, keybind, or OSC `/audiodna/layer/{n}/solo`, none of which are the
REST API). Flagging this as the actual constraint: solo has to be flipped
out-of-band once; everything else below is pure REST and deterministic.

1. `GET /api/composition` → confirm baseline `solo` flags per layer
   (response includes `layer.solo`, `ApiServer.cpp:288`).
2. `POST /api/render_frame {"output_path": "/path/A.png", "time": 0.0}` —
   `time` is an explicit override (`Renderer::captureFrame(path,
   timeOverride, ...)`, `Renderer.cpp:1589`), so pinning it makes the capture
   reproducible/non-wall-clock-dependent.
3. Flip solo on ONE layer (out-of-band — UI click or keybind), on a layer
   known to have visible content.
4. `POST /api/render_frame {"output_path": "/path/B.png", "time": 0.0}` with
   the identical `time`.
5. `GET /api/composition` again → confirm the `solo` field actually flipped
   (fully REST-observable proof the model changed).
6. `cmp -s A.png B.png` (or hash) — expect a diff if other non-soloed layers
   had visible content (proves solo actually changed the pixel output, not
   just the JSON flag) — this is the check that closes "not yet verified:
   runtime pixel behaviour."
7. To specifically probe the §4 edge case: repeat with solo toggled onto a
   layer that has NO active clip while a different non-soloed layer has real
   content — expect (correct) a blank/transparent B.png; if B.png instead
   shows an unrelated procedural-source/loaded-image frame, that confirms
   the fallback-leak concern in §4 is real and reachable.

## SUMMARY
1 file reviewed (CompositorEngine.cpp, +14/-2). 4 issues: 1 blocking-grade
(MEDIUM, defect #1 — third solo-skip site missed), 3 suggestions (LOW). The
two targeted loops are correctly and identically fixed; the plan's inherited
claim about the compositor's prior blindness to solo is verified true with
two independent patterns across the whole render directory. Confidence:
VERIFIED for defects #1, #2, #5 and the loop-consistency finding (all
read directly off disk with line citations above); INFERRED for the
runtime-visible consequence of defect #1 and the §4 fallback-leak scenario
(traced through source/call graph, not observed on screen — no build/run
performed per instructions).
