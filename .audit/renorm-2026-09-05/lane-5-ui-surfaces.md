# Lane 5 — UI Surfaces — drift report (FEATURES.md re-normalize, 2026-09-05)

Scope: `src/ui`, `src/MainComponent.cpp`, `src/MainComponent.h`.
Baseline: `9139dd4` (2026-07-16 normalize). Current HEAD as run.
Read first (per task instructions): `.harmony/APP-INVENTORY.md`, `.harmony/surface-audit-2026-08-04d.md` (both current — this report only states what has *changed* or was *wrong* relative to them, not a re-derivation from zero).

## Scope size

```
git log --oneline 9139dd4..HEAD -- src/ui src/MainComponent.cpp src/MainComponent.h | wc -l
```
→ **51 commits** touch lane-5 paths since baseline (not the 13 I eyeballed on a first pass of the raw log before piping through `wc -l` — stating the correct, counted number).

```
git diff --stat 9139dd4..HEAD -- src/ui src/MainComponent.cpp src/MainComponent.h
```
→ 46 files changed, 3173 insertions(+), 835 deletions(-). `MainComponent.cpp` alone: +1935/-lines (largest single file). `TimingWindow.cpp/.h` and `EffectsRackPanel.h` do **not** appear in the diff stat at all → zero changes since baseline (confirms both are as stale/frozen as APP-INVENTORY already says).

---

## FINDING 1 (confirms task premise) — `resolutionSelector_` carve-out is REAL, and I traced the full mechanism

**Verdict: CONFIRMED. Do not let it get swept into the v1 row-1 deletion.**

Evidence:
- `src/MainComponent.h:278` — `juce::ComboBox resolutionSelector_;`
- `src/MainComponent.cpp:307-326` — `resolutionSelector_.onChange` parses the combo text and calls `previewPanel_.getRenderer().setLockedResolution(w, h)` (or `(0,0)` for Auto).
- `src/render/Renderer.h:240-246` — `setLockedResolution(w,h)` stores `lockedWidth_`/`lockedHeight_` (atomics), consumed at `src/render/Renderer.cpp:396-397` inside the render-frame sizing logic of the **main** `Renderer` — the same Renderer instance whose composited output `publishSyphonFrame` blits to Syphon (per `surface-audit-2026-08-04d.md`'s verified finding that Syphon is the only path to a projector). So this control genuinely gates the pixel dimensions of the only frame that reaches an external audience today. The task's framing is correct.
- This is **not new** since baseline — `git show 9139dd4:src/MainComponent.cpp` already has the identical onChange wiring and the identical `setVisible(false)` at old line 1342. This carve-out predates the 51 lane-5 commits; it is a standing fact, not drift.
- Also corroborated in `.harmony/essentials-plan-2026-08-04d.md:133-134`, which already states this exact carve-out ("sits visually in row-1 but is NOT v1 debris — it drives the live renderer lock L-OUT depends on"). FEATURES.md itself does **not** carry this caveat anywhere (see Finding 2).

**Nuance beyond a yes/no confirm — the control is currently unreachable for *setting*, only for *restoring*:**
- `resolutionSelector_` is `setVisible(false)` unconditionally on every `MainComponent::resized()` pass (`MainComponent.cpp:2004`), with **no** `setBounds()` call and **no** conditional path that ever shows it (checked: no `showV1`/`legacyMode` flag exists — grepped `src/MainComponent.cpp` and `.h` for that pattern, zero hits). It is never on screen.
- No REST endpoint, OSC pattern, or menu item sets resolution either (`grep -in "resolution" src/api/ApiServer.cpp src/ui/MenuBarModel.cpp src/osc/OscHandler.cpp` → zero hits, two independent greps, both zero).
- The only two writers of the combo's *value* are: (a) the user directly clicking it — impossible, it's invisible; (b) `MainComponent.cpp:2793`, `resolutionSelector_.setSelectedId(deck.viewportResolution, juce::sendNotificationSync)`, fired when the **v1 "Deck Load" button** (`deckLoadButton_`, still visible and live — see `MainComponent.cpp:185-188, 2031-2033`) restores a deck. `sendNotificationSync` means this *does* fire `onChange` and *does* re-arm the resolution lock — so the mechanism is genuinely live end-to-end. But `deckSaveButton_`/`viewportResolution` is only ever populated by reading `resolutionSelector_.getSelectedId()` (`MainComponent.cpp:2707`) — and since the widget can never be interacted with, any deck saved by the current build will only ever persist "Auto" (id 1). A non-Auto lock can only survive as long as it exists in an **old preset file predating the widget's hiding**, reloaded via Deck Load.
- **Practical consequence:** today, a user cannot set a new resolution lock through any UI, OSC, or REST surface. They can only inherit one from a legacy `.json` deck preset that already has `viewportResolution` baked in. If Row1/preset-slot deletion also removes `deckLoadButton_`/`saveDeck`/`loadDeck`, that removes the *only* remaining path — even the restore path — not just "v1 debris." This is a second-order risk beyond what the task called out and worth flagging explicitly to whoever executes the deletion plan.

## FINDING 2 (MAJOR, doc gap) — FEATURES.md §21 mischaracterizes `resolutionSelector_` as equivalent v1 debris

`.harmony/FEATURES.md` §21 ("v1/v2 Layout Component Split") lists `resolutionSelector_` in one undifferentiated list of "~15 hidden v1 controls" (line: "Hidden v1 controls (~15): `audioSourceLabel_`, ... `resolutionSelector_`, ... `syncButton_`") with no note that it (alone, among that list) drives a live renderer consumer. Every *other* name in that list is a genuine dead duplicate of a v2 TopBar control — I spot-checked `audioSourceSelector_`, `inputGainSlider_`, `masterLevelSlider_`, `displaySelector_` and confirmed **TopBar owns its own separate, identically-named private members** (`src/ui/TopBar.h:61-98`) that are the live v2 objects — the MainComponent-level ones really are inert duplicates. `resolutionSelector_` has no TopBar equivalent (grepped `TopBar.h` for "resolution" — zero hits) and is the odd one out. FEATURES.md should carve it out the way `essentials-plan-2026-08-04d.md` already does. **Severity: MAJOR** — a reader trusting FEATURES.md §21 alone would conclude `resolutionSelector_` is safe to delete with the rest of row-1, which per Finding 1 it is not.

## FINDING 3 (new discovery, same defect class) — `beatRandomToggle_` / `beatCountSelector_` are a second, undocumented instance of the exact same carve-out shape

Neither FEATURES.md nor APP-INVENTORY nor the surface-audit flags this; I found it while tracing consumers of the other "hidden v1" widgets named in FEATURES.md §21's list (which does **not** even name these two — they aren't mentioned in that ~15-item enumeration at all, though they are hidden by the identical `setVisible(false)` block at `MainComponent.cpp:2004`-adjacent lines).

- `src/MainComponent.cpp:3103` (inside the beat-detection timer callback): `if (beatRandomToggle_.getToggleState() && beatCounter_ >= beatRandomCount_) { beatCounter_ = 0; juce::MessageManager::callAsync([this] { randomizeAllEffects(); }); }` — a live, currently-running consumer that fires every N beats and randomizes all effects.
- Same restore mechanism as Finding 1: `deck.beatRandomEnabled`/`deck.beatRandomCount` are v1 `PresetManager::DeckState` fields (`src/ui/PresetManager.h:75-82`, serialized at `PresetManager.cpp:501,552`), restored via the same visible `deckLoadButton_` → `MainComponent.cpp:2760-2761` → `beatRandomToggle_.setToggleState(deck.beatRandomEnabled, juce::dontSendNotification)`. Note this restore uses `dontSendNotification` (unlike the resolution restore's `sendNotificationSync`) — it doesn't need to, because the toggle state is *read directly* at line 3103 rather than acted on via a callback, so this is fine, not a bug.
- **Consequence:** loading an old deck preset saved with beat-randomize enabled will silently start randomizing all global effects on a beat cadence, with **zero on-screen indication** anywhere in the current v2 UI that this is happening or how to turn it off (the toggle that would show/control it is permanently invisible). A user would perceive this as the app "spontaneously" changing effects with no visible cause. This is a real, reachable, currently-undocumented behavior gap — I'd call it MAJOR since it's user-reachable (via a legacy preset) and produces surprising, un-attributable behavior with no in-UI kill switch.
- Recommend whoever executes the row-1/deletion pass treat `beatRandomToggle_`/`beatCountSelector_`/`beatRandomCount_` with the same "carve out, don't delete outright" caution as `resolutionSelector_` — either resurface both as real v2 controls, or explicitly sever the restore path (and document that old presets carrying this flag will no longer resume it) as a deliberate decision rather than an accidental side effect of deleting "dead" row-1 widgets.

## FINDING 4 — Record tab / Comp-Decks tab "greyed out" — confirmed in effect, not in the literal sense of a disabled tab

There's no `setEnabled(false)` on the tab buttons themselves (`BrowserPanel.cpp` just does `setVisible(activeTab_ == Tab::X)` switching, `BrowserPanel.cpp:145-146`) — clicking either tab does switch to it, so "greyed out" isn't a disabled-tab-button bug. But the *content* of both tabs is genuinely sparse/dim by default, which is almost certainly what reads as "greyed out" in practice:
- `CompDecksBrowser.cpp:21-27` — on a fresh install (no saved comps/decks), the panel paints only `"No saved compositions or decks"` at `kTextSecondary` alpha 0.5 (i.e., dim gray text on a near-black `0xff1a1a1a` fill) — the whole tab is visually empty/gray until the user has saved something.
- `RecordPanel.cpp:20-21` — `stopBtn_.setEnabled(false)` at idle (correct, nothing to stop), so on open, only Record/Save/Load/Format are active-looking; the layout reads sparse.
- Functionally, `surface-audit-2026-08-04d.md`'s findings that `onCompositionLoad`/`onDeckLoad`/`onCompositionSave` are never assigned (Comp/Decks) and that only `recordClipTrigger` is wired with playback dead (Record) are unchanged — I re-grepped both call sites and they are still true at HEAD (no commits in the 51-commit lane-5 window touch either wiring path; `git log --oneline 9139dd4..HEAD -- src/ui/CompDecksBrowser.h` shows only a header change adding 14 lines, no `.cpp` in this repo layout for it to check further, and it's not in the wiring list above). **Verdict: the "greyed out" perception is real and consistent with a real functional gap (mostly-unwired tabs, empty by default), not a separate rendering bug.**

## FINDING 5 — TimingWindow: confirmed unchanged placeholder, "permanent quarter" is approximately right, more precisely ~28% and resizable

- Zero commits touch `TimingWindow.cpp`/`.h` in the 9139dd4..HEAD window (confirmed via `git diff --stat`, file absent from the changed-file list entirely). FEATURES.md §23's detailed placeholder description is still accurate as written.
- Layout: `MainComponent.cpp:2101-2140` splits the bottom row into 4 zones via `vDividerFrac_[0..2]`, default `{0.22, 0.50, 0.75}` (`MainComponent.cpp:4677-4679`). Timing occupies `[d0x, d1x)` = `[0.22, 0.50]` of `bottomAreaWidth_` → **28% by default**, not an exact quarter, and it's a user-draggable divider (`draggingVDivider_` logic at `MainComponent.cpp:4754-4762`), so "permanent" overstates it slightly — it can be dragged narrower (there's a `minFrac` clamp, so it can't be collapsed to zero, but it isn't pinned at a fixed width either). **Characterization: essentially correct, off by "permanent"/"quarter" as precise adjectives — it's a persistent, non-removable, resizable-but-not-collapsible placeholder panel occupying roughly a quarter of the row by default.**

## FINDING 6 — EffectsRackPanel: still present, still hidden, NOT yet deleted — and still received a live bug fix while dead

- `src/MainComponent.h:245` / `.cpp:412,416` — still constructed (`std::make_unique<EffectsRackPanel>`) and `addAndMakeVisible`'d, then immediately/always hidden: `MainComponent.cpp:1984` (expanded-mode hide) and `:2063` (normal-mode hide) — both unconditional `if (effectsRackPanel_) effectsRackPanel_->setVisible(false)`. No path sets it visible again (grepped every `effectsRackPanel_->setVisible(true)` — zero hits). Matches APP-INVENTORY §8's "HIDDEN/UNREACHABLE" entry exactly; unchanged.
- One commit since baseline touches it: `1f5442e "Fix EffectsRackPanel: bounds-check compacted sections_ before indexing"` — a real bug fix (`git diff --stat` shows `EffectsRackPanel.cpp | 4 +-`) landed on a panel that is unreachable in the shipping UI. This doesn't change the panel's live/dead status, but it's worth noting for whoever weighs the deletion proposal: the code still gets touched/maintained, i.e., there's ongoing (small) cost to keeping it around dead. **This is evidence in favor of the deletion proposal, not evidence it has already happened — it has not been deleted.**
- `MappingEditor` (only reachable from `EffectsRackPanel`) is consequently still unreachable too — unchanged from the surface audit.

## Recount table

| What | APP-INVENTORY/FEATURES.md says | What I found at HEAD | Changed? | How counted |
|---|---|---|---|---|
| Lane-5 commits since 9139dd4 | (not previously stated as a number) | **51** | n/a (new count) | `git log --oneline 9139dd4..HEAD -- src/ui src/MainComponent.cpp src/MainComponent.h \| wc -l` |
| BrowserPanel tabs | 6 (Files/FX/Sources/Comp-Decks/Record/MilkDrop) | 6, unchanged | No | `enum class Tab` in `src/ui/BrowserPanel.h:39` — 6 members |
| InspectorPanel tabs | 4 (Clip/Layer/Composition/Signal) | 4, unchanged | No | `enum class Tab` in `src/ui/InspectorPanel.h:61` — 4 members |
| Prefs tabs | 3 (General/Video/About) | not re-derived beyond confirming a `Tab` enum still exists at `PreferencesDialog.h:30`; did not enumerate members | Could not fully confirm | grep only, did not open the enum body |
| Bottom-row "timing quarter" | "permanent quarter of the bottom row" (task prompt framing) | 28% by default (`vDividerFrac_[1]-vDividerFrac_[0] = 0.50-0.22`), user-resizable, floor-clamped not zero-clamped | Refines, doesn't reverse | `MainComponent.cpp:2101-2140`, `:4677-4679`, `:4754-4762` |
| `resolutionSelector_` live-consumer claim | Task premise: "drives a live renderer resolution lock" | CONFIRMED true, traced full chain to `Renderer::setLockedResolution` → consumed at render time → feeds the Syphon-published frame | No (confirms) | See Finding 1 |
| EffectsRackPanel deletion status | "proposed for deletion" (task prompt) | Still present, still hidden, not deleted; received 1 bug-fix commit since baseline | No (still pending) | `git log`, grep for `setVisible` calls |

## Dead surfaces (control/field with no consumer, traced)

None found in lane-5 scope beyond what APP-INVENTORY/surface-audit already list (Layer Solo, BPM multiplier, Quantize, Comp-Inspector Autopilot block, Comp-Inspector global FX stack, MilkDrop jukebox Pool/Mode/Blend, param-source BPMSync/ClipPosition/Timeline — all previously identified, all in files outside or only partially in my three paths, and I did not re-verify each one from scratch since they're not new since baseline and re-deriving the whole surface-audit was explicitly out of scope ("re-deriving them from scratch is waste"). Where lane-5 scope gave me a *new* angle not covered before, see Findings 1/3 above — those are the inverse shape (a live consumer with an unreachable control), which is the more interesting and previously-unflagged failure mode in this pass.

## Could not determine

- Full enumeration of PreferencesDialog's `Tab` enum members (grepped only, didn't open the body) — low risk, FEATURES.md's "3 tabs (General/Video/About)" is consistent with everything else I saw and nothing in the 51-commit diff touches `PreferencesDialog.h`'s tab enum specifically (only `.cpp` body-content diffs, 201 lines removed net — could be content pruning within existing tabs, not a tab-count change, but I did not open the diff to confirm which).
- Whether the "Record tab / Comp/Decks tab render greyed out" description in the task prompt refers to something more specific I'm missing (e.g., a LookAndFeel override that dims the tab *button* itself under some condition) — I found no `setEnabled(false)`/reduced-alpha styling on the tab buttons themselves, only on tab *contents*. If there's a screenshot this was derived from, it would be worth checking whether it was taken pre- or post- some commit that changed tab-button coloring; I did not have a screenshot to compare against.
- Did not exhaustively verify all "~15 hidden v1 controls" listed in FEATURES.md §21 — spot-checked 4 (audioSourceSelector_, inputGainSlider_, masterLevelSlider_, displaySelector_) against TopBar equivalents and found them genuinely dead-duplicate; did not check `viewportLabel_`, `fpsLabel_`, `cpuLabel_`, `randomLabel_` individually beyond confirming they're simple display labels with no interactive onChange/onClick to trace (labels have no consumer to mis-wire).
