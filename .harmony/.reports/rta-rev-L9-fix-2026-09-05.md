# Reviewer Verdict — rta-rev-L9-fix-2026-09-05
STATUS: DONE
VERDICT: PASS

## Scope
Repo: /Users/boriskarpman/projects/RealTimeAudio (read-only, focused re-review of
the fix to the ONE prior blocking finding in
memory/.reports/rta-rev-L9-2026-09-05.md §3/Finding 1).
Files touched by the fix (git diff -- src/ui src/MainComponent.cpp src/MainComponent.h,
642 lines, whole-lane diff): MainComponent.cpp, src/ui/{ClipInspector,EffectStackView,
LayerInspector,CompositionInspector,InspectorPanel}.{h,cpp}.
Verified compiles clean: `make -j4` in build/ rebuilt ClipInspector.cpp.o (confirmed
newer mtime than source) and linked `AudioDNA` to 100% with no errors.

## Q1 — Is the stuck-display defect gone? (slow / fast / sub-epsilon-jitter)
Executed the corrected algorithm as a standalone simulation
(scratchpad/repro_fix.py) against a sine "modulator" at periods 2s-600s, plus a
sub-epsilon jitter case and a fast near-epsilon-amplitude oscillation:

```
period_s  ticks   pushes  max_render_cache_staleness
       2     960     944  0.000171
      10    4800    4416  0.000497
      30   14400   10832  0.000500
      60   28800   11216  0.000500
     120   57600   13584  0.000500
     300  144000   15136  0.000500
     600  288000   15488  0.000500
sub-epsilon jitter (amp=0.0002): pushes=1, max_stale=0.000200   -> pushes once, then holds, no thrash
fast osc (period=0.05s, amp just above epsilon): 800/1200 pushes -> tracks correctly, no under-suppression
```
Even at a 600s period, staleness never exceeds the epsilon bound (0.0005) — it is
NOT "stuck forever" like the original defect (which the first review reproduced as
literally 0 updates over an entire 60s+ cycle). The new `changed` compares against
`lastPushedValue`/`lastPushedSourceParam_` (the last value actually pushed), not
against the render field that gets overwritten every tick — this is the correct
fix for the reported root cause. Oscillation exactly at/near the epsilon band does
not thrash: each push resets the reference to the current value, so a sub-epsilon
dither pushes once and then correctly goes silent forever (verified above), while
any oscillation whose amplitude clears epsilon is correctly tracked at whatever
rate it actually crosses the band.

## Q2 — Is the render path actually ungated? (the critical half)
**The writes are unconditional, confirmed for both instances:**
- `EffectStackView::tickModulation()`: `fx.paramValues[p] = signalValue;` — every
  tick, unconditional (EffectStackView.cpp:197). `CompositorEngine` reads this
  vector directly every GL frame — no cache in between, so the unconditional
  write alone is sufficient here.
- `ClipInspector::tickModulation()`: `clip_->sourceParams[i].value = val;` — every
  tick, unconditional (ClipInspector.cpp:877). Feeds `CompositorEngine`'s
  `sourceRenderFn_` directly for the deck-compositing path — also sufficient
  alone for that path.

**The two render-notify callbacks differ, and this needs to be stated precisely
rather than as a blanket "yes, ungated":**
- `EffectStackView::onParamChanged` was moved outside the `changed` gate
  (EffectStackView.cpp:229-231) — literally unconditional now, called every tick
  when `found`. Checked: `grep -rn "onParamChanged" src/` outside
  EffectStackView.cpp/.h returns nothing — **this callback has zero listeners
  anywhere in the codebase.** Its unconditionality is functionally inert (neither
  a fix nor a risk) since nothing consumes it; the actual render correctness for
  this instance rests entirely on the unconditional field write above, which was
  already correct before this fix-round.
- `ClipInspector::onSourceParamsChanged` (ClipInspector.cpp:908-909) is the ONE
  callback that is actually load-bearing for render: it feeds
  `Renderer::updateActiveSourceParams()` → the mutex-protected `activeSourceParams_`
  member (Renderer.cpp:97-101), which is a genuine cache read by the GL thread at
  Renderer.cpp:205 for the standalone-source (no-deck-compositing) path. **This
  callback remains inside the `if (changed)` block — it is not literally
  unconditional.** However, per Q1's executed simulation, `changed` is now the
  CORRECTED delta-since-last-push check, which bounds `activeSourceParams_`'s
  staleness to at most epsilon (0.0005) at every tested speed from 2s to 600s
  period — categorically different from the original defect (unbounded/permanent
  staleness for anything slower than ~0.06 units/s). This is a legitimate
  bounded-staleness design (a dead-band filter applied to the cache-sync
  trigger, not just the UI paint), not a re-creation of the blocking defect.
  Flagging this distinction explicitly since the packet asked pointedly whether
  this path is "ungated": the accurate answer is "gated by a threshold that is
  now provably bounded and self-correcting," not "unconditional" — functionally
  equivalent to unconditional for any real modulator, given the epsilon size.

## Q3 — Initialization (first tick after clip/layer swap always pushes)
Confirmed correct. `ClipInspector::buildSourceParamControls()` (called from every
non-null `setClip()`, ClipInspector.cpp:746-752) clears and reassigns
`lastPushedSourceParam_` to all-`nullopt` sized to the new clip's `sourceParams`
(ClipInspector.cpp:768,775) — guarantees the first `tickModulation()` after a
clip swap always pushes for every connected param, regardless of computed value.
Same pattern for `EffectStackView::rebuildRows()` (EffectStackView.cpp:390),
called from `setEffects()` (clip/layer/composition selection) and from the two
other `rebuildRows()` call sites (effect add/delete, EffectStackView.cpp:346,532)
— every model change (new clip, new layer, new composition, effect
added/removed/reordered) resizes+resets `lastPushedValue` fresh. Traced L3's
`swapCompositionModel()` model-swap path per the original review's lifetime
analysis (unchanged by this fix) — `setClip(nullptr)`/`setLayer(nullptr)` and
the subsequent new `setClip()`/`setLayer()` on reload both go through these same
reset paths.
Minor, non-blocking: `ClipInspector::setClip(nullptr)`'s else-branch clears
`sourceParamControls_` but not `lastPushedSourceParam_`, leaving a stale-sized
vector briefly. Harmless — `tickModulation()` early-returns on `!clip_`, and the
next non-null `setClip()` unconditionally clears+reassigns it again before it is
ever read. Not worth a fix-round of its own, but a `.clear()` alongside
`sourceParamControls_.clear()` in that branch would be tidier.

## Q4 — Sizing/indexing
Both new parallel containers are read/written defensively:
`p >= row.lastPushedValue.size() || !row.lastPushedValue[p].has_value() || ...`
and `i >= lastPushedSourceParam_.size() || !lastPushedSourceParam_[i].has_value() || ...`
short-circuit before any `[]`/`*opt` dereference, and the writes are guarded
symmetrically (`if (p < row.lastPushedValue.size()) row.lastPushedValue[p] = ...`).
No out-of-bounds read or write is reachable even if the container were
undersized; in that case the code just always treats the param as `changed`
(fails safe toward "always push", not toward silently dropping updates or
corrupting memory). In practice this branch is unreachable in the current call
graph — both containers are resized in lockstep with `paramControls`/
`sourceParamControls_` by the same rebuild function, on the same (message)
thread, before `tickModulation()` can observe a mismatched size.

## Q5 — Visibility gate
For `EffectStackView`, `pc.isVisible()` reflects only the row's own local
collapse flag (`setVisible(false)` in `resized()` for collapsed rows) — a
background/inactive Inspector *tab* does not touch this flag (confirmed by the
prior review and unchanged here), so switching tabs does not suppress the
push. For a row that IS collapsed for 30s then re-expanded: `lastPushedValue[p]`
is frozen at whatever it was when last visible (the `if (changed && pc.isVisible())`
block, which is the only place that updates it, no-ops the whole time the row is
hidden) — so on the very next tick after re-expanding, `changed` is
overwhelmingly likely true (the real value has almost certainly drifted more
than 0.0005 over 30s) and the display self-corrects immediately. No
stale-after-unhide bug.
For `ClipInspector`'s `sourceParamControls_`, the code comment claims
`pc.isVisible()` is always true there; verified by `grep -n "setVisible"
src/ui/ClipInspector.cpp` — no call touches `sourceParamControls_[i]` (only
unrelated video/beat-sync widgets) — the comment is accurate, and the visibility
check on that path is a genuine no-op, not a source of staleness.

## Q6 — Epsilon
Unchanged value, `kModulationChangeEpsilon = 0.0005f`, now correctly measuring
"since last pushed" instead of "since last tick" (see Q1). Simulation confirms
this bounds worst-case display/render-cache staleness to exactly 0.0005 (0.05%
of a typical 0-1 param range) regardless of modulator speed, and correctly
avoids spurious pushes for sub-epsilon jitter. Defensible as-is; no change
needed.

## Q7 — Did the fix disturb anything the prior review already passed?
No. Re-checked and confirmed unchanged/intact on current disk state:
- Coverage of all 5 instances (ClipInspector/EffectStackView, LayerInspector,
  CompositionInspector, InspectorPanel dispatch, MainComponent's MacroBank
  ordering) — all still present and wired the same way.
- Paint/compute split — `EffectStackView::refresh()` still calls
  `tickModulation()` first then a plain unconditional display-sync loop;
  `ClipInspector::refresh()`'s tail (ClipInspector.cpp:918-934) is still
  display-sync-only via `setParamValue`, unchanged from what the prior review
  passed.
- Fence — same file set as the original lane (MainComponent.cpp + the 5 named
  src/ui/*.{h,cpp} pairs); MacroPanel.*/MacroBank.h still untouched
  (`git status` confirms zero diff there).
- Full project build (`make -j4`) succeeds end-to-end, `AudioDNA` links at 100%,
  confirming no compile regression from the fix-round changes.

## FINDINGS
None blocking. One clarification worth carrying forward (not a defect, see Q2):
`ClipInspector::onSourceParamsChanged` — the one render-notify callback that
actually matters (feeds the GL-thread-read `Renderer::activeSourceParams_`
cache) — is gated by the corrected epsilon-since-last-push check rather than
being literally unconditional. This is a legitimate bounded-staleness design
(proven bounded to 0.0005 at every tested modulator speed from 2s-600s period)
and closes the actual reported defect (permanent freeze), but it is technically
"gated," not "unconditional" — worth knowing if a future change touches this
path again, since the two are easy to conflate.
Non-blocking tidiness nit: `ClipInspector::setClip(nullptr)` doesn't clear
`lastPushedSourceParam_` (harmless, re-cleared on next non-null setClip()).

## WHAT ONLY A HUMAN AT THE MACHINE CAN VERIFY
Same as the original review's list — connect a Clip/Layer effect param or a
Source clip's own param to a genuinely slow (30-60s) LFO/envelope, watch the
Inspector's on-screen readout track it continuously (not freeze) while the tab
stays active, and confirm the standalone-source render output (no deck
compositing) also keeps modulating rather than freezing.

METADATA: reviewer=rev-L9, builder_packet=rta-rev-L9-fix-2026-09-05, date=2026-09-05T00:00:00Z
