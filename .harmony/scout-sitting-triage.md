# Sitting-triage scout report — 2026-07-30 (read-only, source-cited)

Triage of the 4 findings from Boris's 2026-07-30 manual sitting. Verdicts first;
INFERRED items labeled. Source: Explore scout, nothing edited/built/run.

## Q1 — FX drop targets
VERDICT: channel strip (LayerStrip) = NOT-WIRED (no DragAndDropTarget base).
Layer window = WIRED. Clip inspector = WIRED. Clip cell = WIRED.
Composition/global = WIRED-BUT-CONDITIONAL (only the ~20px effect-stack
sub-component accepts, not the whole panel).
- Exactly four isInterestedInDragSource impls in src/: ClipCell (fx:/source:/
  clip:/files:/milkdrop: — ClipCell.cpp:365-369), ClipInspector (fx: +
  clip_!=null — ClipInspector.cpp:1220-1223), LayerInspector (fx: +
  layer_!=null — LayerInspector.cpp:977-980), EffectStackView (fx: —
  EffectStackView.cpp:412-415).
- LayerStrip inherits only Component+Timer (LayerStrip.h:23-24) — no drop
  surface at all.
- CompositionInspector is plain Component (CompositionInspector.h:23) unlike
  Layer/ClipInspector which are panel-level targets forwarding to their child
  stack (LayerInspector.cpp:995-1000, ClipInspector.cpp:1237-1243). Its stack
  view exists (CompositionInspector.h:102, bound to comp->globalEffects at
  CompositionInspector.cpp:381) so only a precise hit on the stack works.
- INFERRED: empty stack collapses to ~20px (EffectStackView.cpp:234) — global
  drop "not accepted" = target-size problem.
- FIX: mirror LayerInspector's panel-level DragAndDropTarget on
  CompositionInspector (LayerInspector.cpp:977-1000 as template). LayerStrip
  as target = product question (Boris tried it → expectation exists).

## Q2 — retrigger of active cell ignored
VERDICT: EMERGENT no-op, not an early return. The retrigger branch runs but its
write is clobbered by the renderer next frame.
- Path: ClipCell.cpp:187 → DeckView.cpp:134-136 → MainComponent.cpp:2871/2889 →
  Layer::triggerClipImmediate (Layer.h:218).
- Retrigger branch Layer.h:225-235: sets clip->playheadPosition = inPoint,
  zeroes beatsPlayed, returns — never calls seekTo() on the media player.
  Renderer.cpp:923 (images :993) overwrites clip->playheadPosition from the
  player EVERY frame; clip->playing clobbered at :925. Net effect: click does
  nothing visible.
- Only seekTo on the trigger path is behind legacy clip->beatSnap
  (MainComponent.cpp:2904) and seeks to beat phase, not in-point (:2919,:2924).
- Queued-trigger clear DOES fire: pendingTriggerColumn=-1 at Layer.h:223;
  beat-snap queue skipped for active column (Layer.h:207 condition). Clicking
  the playing cell silently cancels a pending queued trigger — correct, zero
  feedback. Undo layer documents the no-op as intended (MainComponent.cpp:
  3986-3988).
- FIX (if Boris wants restart-on-retrigger): make the retrigger branch actually
  seek the player to in-point (player-level seekTo), not just the model field.
  DESIGN CALL = Boris.

## Q3 — file-browser slowness
VERDICT: fully synchronous on the message thread; dominant cost = FULL-RES
image decode per image file, no cache; List button re-runs the entire decode
pass for thumbnails list mode never draws.
- FilesBrowser::refreshFileList (FilesBrowser.cpp:408-441): two blocking
  findChildFiles sweeps + sorts (:414,:423), generateThumbnail(f) inline per
  media file (:431). generateThumbnail (:491-502) = ImageFileFormat::loadFrom
  (full native-res decode) then rescaled(64,64).
- No caching: thumbnail lives in FileEntry (FilesBrowser.h:48); entries_
  cleared every refresh (:410); filterBySearch repeats decode loop (:470).
- Both view buttons call refreshFileList() unconditionally (:358-359); list
  painter draws only glyph+filename (:270-288), never touches thumbnails —
  100% wasted decode per toggle.
- Video NOT the cost: video extensions return {} from generateThumbnail (:501).
- FIX: thumbnail cache keyed by path+mtime, async/background decode, skip
  decode entirely in list mode.

## Q4 — clear-path runtime refs
VERDICT: TWO roots, not one. (a) renderer-purge omission on the X-clear path;
(b)+(c) from kClipClear writing a blank Clip{} instead of emptying the cell.
- X path: LayerStrip.h:66 / LayerStrip.cpp:330-332 → DeckView.cpp:109-111 →
  MainComponent.cpp:659-679 → Layer::clearActiveClip (Layer.h:288-298), which
  DOES reset activeClipColumn=-1 and playing=false — but NEVER purges the
  renderer: no clearActiveSource()/clearImage()/player stop.
- Root (a): renderer priority chain compositor→active source→loaded image
  (Renderer.cpp:413-450). With no active clip, compositeDeck returns 0
  (CompositorEngine.cpp:650-667); Renderer.cpp:440-445 falls back to re-render
  activeSourceType_ — still set from trigger time (MainComponent.cpp:2940),
  never cleared. Shader/projectM clips keep rendering after clear. Videos
  predicted to go black (their trigger path calls clearActiveSource+clearImage,
  MainComponent.cpp:2952-2954) — INFERRED, needs the Boris disambiguator below.
- CONTRAST (correct purge): empty-cell trigger branch MainComponent.cpp:
  2972-2979 (clearActiveSource, clearImage, reset currentImageFile_+label);
  deck-switch repeats at :3087. DIRECT FIX: lift those 4 lines into the
  onLayerClearClip handler at :678.
- Root (b)/(c): kClipClear (MainComponent.cpp:3833-3861) does
  deck->setClip(..., Clip{}) at :3852 — DEFAULT-CONSTRUCTED clip, not
  std::nullopt; Deck::setClip assigns into the optional (Deck.h:126-135) so the
  cell still has_value(). Consequences: Layer::getClipAt returns valid ptr
  (Layer.h:180-188) → autopilot occupancy filters accept the blank cell
  (Autopilot.cpp:205,218,232,243; smartAdvance :294) = (b); activeClipColumn
  untouched by kClipClear, getActiveClip returns the blank = (c); blank clip
  fails hasActiveLayers_ (CompositorEngine.cpp:652-664) → reinforces (a).
  NOTE: fixing this touches the undo lane's clear-command model contract
  (Clip{} snapshot/restore vs empty optional) — conform to lane patterns.
- Known documented leak (both paths): players keyed by clip id never closed
  (MainComponent.cpp:3114-3115) — alive+unpaused but not composited.
- Alternative considered/rejected for (a): stale compositor texture — rejected
  because symptom is continued MOTION and compositeDeck returns 0, not stale
  (CompositorEngine.cpp:666-667).
- DISAMBIGUATOR (Boris, single item): was the clip that kept playing after X a
  built-in shader/projectM source, or a video file? Shader = confirms fallback-
  chain root; video = diagnosis incomplete.
