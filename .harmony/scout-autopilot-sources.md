# Scout dossier — Autopilot vs Source clips (2026-07-30 PM, HEAD 972e8dd)

Trigger: Boris live finding at gate feedback: "Autopilot does not work for
sources." Scout: read-only Explore, delivered same day (after idle-nudge).

VERDICT: **OTHER — "ADVANCE-GATE STALL."** Not EXPLICIT-SKIP, not TRIGGER-NOOP.
Autopilot has ZERO type-based filtering, and layer.triggerClip works correctly
on source clips (they activate and render). The failure: autopilot can never
advance AWAY FROM a source clip — both exit gates are structurally unreachable
for MediaType::Source. Ranked: (1) End-of-Video mode permanently frozen for
sources; (2) On-Beat mode blocked whenever clip->playing is false, which is how
every source clip is born.

## 1. What a "source" is (VERIFIED)
- Discriminant Clip::MediaType src/model/Clip.h:17 — None, Image, Video,
  Camera, Source, ImageSequence; std::string sourceType names the generator
  (Clip.h:21, e.g. "perlin_noise", "projectm_visualizer").
- Sources-tab creation sites (mediaType=Source): multi-source drop
  MainComponent.cpp:911-914; MilkDrop single :1037-1040; MilkDrop playlist
  :1094-1097; onSourceActivated double-click :1255-1258.
- Load-bearing: Clip::isPlayable() (Clip.h:164) true ONLY for
  Video/ImageSequence — sources/images/camera have NO playhead.
- MilkDrop presets are NOT a separate type — Source clips with
  sourceType=="projectm_visualizer".

## 2. Autopilot selection — NO type filter anywhere (VERIFIED, full-file read)
advanceClip picks next column purely on getClipAt(col)!=nullptr for all six
actions (Autopilot.cpp:202/:217/:232/:243/:255/:270); smartAdvanceClip same
predicate :294; call site Renderer.cpp:220-222 unfiltered. EXPLICIT-SKIP ruled
out.

## 3. Where it dies (VERIFIED mechanism)
(a) **End-of-Video mode** (Autopilot.cpp:11-41): advances only when
    playheadPosition >= outPoint-0.01 (:19-22, threshold 0.99). NOTHING
    advances a source's playhead — only writers are Renderer.cpp:931/:1001
    inside getVideoFrameTexture, which early-returns for non-Video/ImageSeq
    (:881-886,:953,:1022); triggerClipImmediate pins playhead to inPoint
    (Layer.h:244); beat-snap sync gated on isPlayable()
    (MainComponent.cpp:2912). Playhead sits at 0.0 vs 0.99 forever; beat loop
    explicitly skips EoV layers (Autopilot.cpp:64). Frozen permanently, 100%
    reproducible.
(b) **On-Beat mode** (Autopilot.cpp:62-104): requires clip->playing at :68.
    Sources are born playing=false (Clip.h:156) — none of the 4 creation sites
    sets it (video/image paths DO: MainComponent.cpp:3365/:3371/:3426).
    Sources only gain playing via Layer.h:247-248, gated on !hasBeenTriggered;
    hasBeenTriggered set at MainComponent.cpp:2910 and NEVER reset (not by
    Clip::clear() Clip.h:243-280, not replaceContent, not serialized). So any
    source clicked once then paused/stopped (TopBar :551/:559) or cleared
    (Layer.h:293) is permanently dead to on-beat autopilot. Videos are rescued
    by Renderer.cpp:934 re-syncing playing from the player each frame; sources
    have no such writer.
Ranking (a) first is INFERRED — unconditional + matches "sources freeze while
videos advance." (b) needs specific click history. Boris's layer mode
NOT-CONFIRMED (LayerInspector.cpp:74-76 dropdown defaults "On Beat" id 2;
Layer::autopilotEndOfVideo defaults false, Layer.h:147).

## 4. Trigger is NOT the problem (VERIFIED)
triggerClip on a source sets activeClipColumn; compositor renders via
sourceRenderFn_ independent of clip->playing (CompositorEngine.cpp:655-656,
:710-713); deck compositor has priority over legacy single-source path
(Renderer.cpp:413-420). No silent no-op.

## Fix surface — 3 files, low risk to hardened paths
- Autopilot.cpp:11 + :64 — EoV branch condition becomes
  (layer.autopilotEndOfVideo && clip->isPlayable()) so non-playable clips fall
  through to beat-based advancement instead of freezing. Keyed on isPlayable()
  → same fix repairs Image and Camera clips (identical bug).
- MainComponent.cpp:911/:1037/:1094/:1255 — add clip.playing = true; at the 4
  source-creation sites (parity with :3365/:3371/:3426).
- RISK contained: Autopilot edit changes only WHEN advanceClip is called (no
  new triggerClip call sites, no clear-path contact); MainComponent edits are
  single field assignments inside existing withDeckDetached fences (fence
  topology unchanged). No contact with a718572/20fe75d clear-path or 8bd09ba
  GL fences.

## Side notes — do NOT bundle (logged separately)
- Source clips never get an id: all share id=0 (vs s_nextClipId++ at
  MainComponent.cpp:3358). Currently INERT (CompositorEngine keys nothing on
  clip id; only Video/ImageSeq use id-keyed player maps). Latent — separate.
- Autopilot bypasses handleClipTrigger (self-documented
  MainComponent.cpp:2890-2892) → inspector/file-label STALE after autopilot
  advance (composited output unaffected). Cosmetic — separate.
- NO unit tests for Autopilot anywhere in tests/ (no test_autopilot.cpp).

CONFIDENCE: items 1, 2, 3(a), 3(b) mechanism, 4, side notes, test absence —
VERIFIED by source read at HEAD 972e8dd. INFERRED: which leg Boris hit.
NOT-CHECKED: app not run, no live state observed (read-only boundary).
