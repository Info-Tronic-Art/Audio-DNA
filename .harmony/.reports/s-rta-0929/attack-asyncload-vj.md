# ATTACK — plan-asyncload, VJ/live-performer seat
VERDICT: Fundamentally sound and re-derived correctly against main HEAD (checked file:line exactly:
loadComposition :3071, openMediaForDeck :3013, appendDeckFromFile :3214, duplicateDeck :3435,
swapCompositionModel :2934, ~MainComponent :2357 — all match). But it ships the ONE new visible
witness (the "Loading X..." label) unprotected from being clobbered mid-load, and reorders the
destructor against its own documented invariant without justifying it.

## MUST

1. **The "Loading..." label — the only thing Boris sees change during a ~1s load — is not protected
   from a live trigger, and no row tests it.** `MainComponent::refreshPreviewFromActiveClip`
   (`src/MainComponent.cpp:4682-4713`) unconditionally `fileLabel_.setText(...)`s (clip name, source
   type, or `""`) on any active-clip change; R2/R7 (plan lines 183-188, 217-229) route a live trigger
   during the staged window straight through this function. So: trigger a clip while a 16x4K load is
   staged -> "Loading V32..." vanishes (replaced by the clip name, or blanked) for the rest of the
   window; then `finishStagedLoad` overwrites it again with the unconditional `doneLabel` (plan 5.4).
   Performer sees the indicator disappear, then a "Loaded: V32" flash out of nowhere. R13 (line 268)
   *acknowledges* this ("a trigger's label write meanwhile is kept") but no row checks `ui_text`
   under a trigger during the window (a4 checks only `activeClipColumn`; a5's `ui_text` check has no
   concurrent trigger). The label lies exactly when it matters most.
   FIX: guard `refreshPreviewFromActiveClip`'s label write while `staged_->loadingText` is showing
   (same compare `labelAfterCancel` already does), or add an explicit ui_text assertion under a
   concurrent trigger and accept the flicker in writing — right now it's neither guarded nor tested.

2. **`cancelStagedOpen(Cancelled)` is the literal first statement of `~MainComponent` (plan 5.4, line
   443), ahead of `apiServer_->stop()`** — but the destructor's own comment (`MainComponent.cpp:
   2358-2369`) states the HTTP-first order is deliberate and load-bearing ("closes a disputed race
   class... just after this non-UI server join"), and the plan's own section 1 (line 55) calls this
   order load-bearing too. No argument is given for why a new call into `Renderer::closeMediaForClip`
   is exempt from that boundary — only that adopted players go to a retire list drained later.
   `closeMediaForClip` is in fact safe off the GL thread (`Renderer.cpp:1531-1541`), so this is
   likely benign — but silently reordering a comment that says "this order matters" needs one
   sentence of justification, not silence.
   FIX: move `cancelStagedOpen(Cancelled)` after `apiServer_->stop()` (still before `detach()`), or
   add a line to the destructor comment explaining the exemption.

## SHOULD

3. Two paths finish a quit-time ticket as Cancelled — `cancelStagedOpen` and `ApiServer::stop()`'s
   own sweep (plan 5.5, line 473). Race-safe (`LoadTicket::finish` is compare-exchange), but the plan
   should state which one is expected to win so a future reader doesn't re-derive it.

4. R4's known gap ("two Duplicate clicks in one window yield one copy", Q3, line 624) is filed
   honestly, but it silently changes visible behavior (today: two copies) on an accidental
   double-click mid-show; worth a one-line debounce mitigation, not just an open question.

## NIT

5. `end_quit_mid_load` (line 559) checks only clean termination, not that the in-flight pool job's
   `stats_` pointer stays valid until drained — true by member-declaration order (`mediaOpener_`
   destructs, blocking, before `previewPanel_`/Renderer — plan 407-409) but never stated as a row;
   safety rests entirely on that order never changing later.

## Strongest counterargument to my own position
MUST-1 is cosmetic — no black frame, no crash, no data loss, audio untouched, and the label
self-heals at completion. A chair could rank it SHOULD. I hold it at MUST because it negates the
plan's own accepted trade (R2: "keep playing, no replay") by making the one compensating signal —
visible load-in-progress feedback — silently unreliable during the exact window it exists to cover,
with zero test coverage; for a live-performance app that is a first-class UX defect.
