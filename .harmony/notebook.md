# Builder Notebook — Audio-DNA

<!-- Accumulated Builder knowledge. Each Builder reads this and appends discoveries. -->

## 2026-09-05 — closeMediaForClip is now wired up (L1 media-leak fix, rounds 1+2) — the 2026-07-20 entry's "NEVER closed" is now STALE for video/sequence
**Files:** src/render/Renderer.{h,cpp} (closeMediaForClip, drainRetiredMedia, retiredVideoPlayers_/retiredImageSequences_), src/core/ClipCommands.h (ClipMediaDisposeHook, SetClipCmd, SwapClipsCmd), src/core/DeckCommands.h (ClearLayerClipsCmd, RemoveColumnCmd, RemoveLayerCmd, RemoveDeckCmd), src/MainComponent.cpp (makeClipMediaDisposeHook)
**Note:** `Renderer::closeMediaForClip` had zero call sites for the life of the
project (bare-symbol grep: 2 hits = decl+def only) — every vacate path
stranded a VideoPlayer/ImageSequence (live FFmpeg decoder + GL texture)
forever. Round 1 fixed `Clip > Clear` (SetClipCmd), `SwapClipsCmd`,
`ClearLayerClipsCmd`, `RemoveColumnCmd`. **Round 2 (reviewer-caught FAIL on
family coverage) added `RemoveLayerCmd` (execute() erased a whole layer with
zero dispose call) and `RemoveDeckCmd` (had NO media hook of any kind —
zero reconnect on undo too — the largest single leak in the family, since it
strands every clip across every layer of the removed deck). ALL SIX command
classes that can vacate a cell now dispose.** `ClipMediaDisposeHook` (new
hook, same shape as the existing `ClipMediaHook` reconnect hook,
ClipCommands.h) is threaded through all six. THREE things future editors of
this area must not re-break: (1) **Dispose is
keyed off the command's OWN before_/after_ snapshot, never live cell state**
— the UI handlers (kClipClear etc.) pre-mutate the model directly BEFORE
building the command, so by execute() time the live cell is already in its
post-state; a dispose gated on live state would never fire. (2)
**closeMediaForClip no longer synchronously destroys the player/sequence** —
it can run on the message thread with no GL context current, so it calls
`->close()` (FFmpeg/CPU-only, safe off-GL-thread per each class's own
threading-model comment) then moves the unique_ptr to
`retiredVideoPlayers_`/`retiredImageSequences_` (mutex-guarded) instead of
erasing-in-place; `Renderer::drainRetiredMedia()`, called every frame from
the TOP of `renderOpenGL()` (guaranteed GL thread + context current), is the
ONLY place `releaseGL()` runs and the ONLY place these objects actually get
destroyed. Do not add a second path that erases these maps directly — it
would resurrect the "glDeleteTextures with no context current" bug.
(3) **A swap/move must NOT dispose an id that just relocated to the other
cell** — SwapClipsCmd's `disposeIfOrphaned` checks a leaving clip's id
against BOTH resulting cells (src and dst) before disposing; only an id
retained by neither gets closed. `ClearLayerClipsCmd`/`RemoveColumnCmd` don't
need this cross-cell check (nothing moves within their own apply — a full
row-clear or a column-removal only ever loses ids, never relocates them).
Final safety net: `MainComponent::makeClipMediaDisposeHook()` re-scans the
WHOLE composition for the id before actually calling `closeMediaForClip` —
load-bearing ONLY while there is no clipboard/duplicate feature (ids unique,
6 minting sites, no reuse); if that ever changes, this scan is what needs to
change, not the per-command orphan checks. (4) **RemoveLayerCmd/RemoveDeckCmd
are command-owns-the-mutation, NOT double-apply** — the handler snapshots the
full Layer/Deck value and does NOT pre-mutate live state before building the
command (unlike SetClipCmd/ClearLayerClipsCmd/RemoveColumnCmd's shape), so
execute() is the only place the removal ever actually happens, first time AND
every redo — dispose lives directly in execute() against `removed_`/`removed_
.layers`, no `leaving`-parameter trick needed. (5) `openGLContextClosing()`
now also calls `drainRetiredMedia()` (round 2 fix #3) — without it, anything
`closeMediaForClip()` retired but that hadn't yet been through a
`renderOpenGL()` frame would survive context teardown and get `releaseGL()`'d
against a texture ID belonging to the NEXT context on ITS first frame — same
per-context-state bug class as `EffectChainGLState::release()` two entries
below this one guards against. Do not drain the retire lists from anywhere
else — `drainRetiredMedia()` is the single reused helper for both call sites,
by design (two independent copies of the release loop is how this class of
bug re-appears, per the reviewer's own words on this fix).
**Valid while:** the six command classes' hook-injection shape and the
message-thread/GL-thread split in Renderer.{h,cpp} are unchanged.

## 2026-08-04 — Multi-image drop has THREE entry points, not two; a pure-image Finder drop was completely un-thresholded
**Files:** src/ui/ClipCell.cpp (filesDropped ~292, itemDropped "files:" branch ~514-545),
src/MainComponent.cpp (onMixedFilesDropped lambda ~788-862, handleMultiFileDrop ~3791-3854,
applyMultiFileDrop ~3740-3789)
**Note:** A multi-image drop reaches MainComponent through THREE separate paths, not the two a
work packet described (MainComponent.cpp's onMixedFilesDropped lambda + ClipCell.cpp's
itemDropped "files:" internal-drag branch). The third — a plain Finder drop of images ONLY, no
video, landing directly on a ClipCell (ClipCell::filesDropped's `imageFiles.size() > 1` branch →
`onMultiFileDrop` → `DeckView::onMultiFileDropped` → `MainComponent::handleMultiFileDrop` →
`applyMultiFileDrop`) — had ZERO count-based branching before this fix; ANY count > 1 always
collapsed into a single ImageSequence cell. This is almost certainly the literal path a plain
"drag 2 images from Finder onto an empty cell" hits, since ClipCell::filesDropped only routes
through the mixed-drop combined callback when videos are ALSO present. Any future threshold/
placement change to multi-image drops must audit `handleMultiFileDrop` too, not just the two
`videoStartCol`-adjacent call sites — grep `onMultiFileDrop\b` callback wiring in DeckView.cpp to
find all producers. `applyMultiFileDrop` (single-cell/ImageSequence primitive) was also missing
the content-lock check `applyFileDrop` has always had — any function that writes a Clip via
`deck->setClip` needs its own explicit `existing->contentLocked` guard; there's no shared gate.
**Valid while:** these five functions/lambdas keep their current names and the ClipCell → DeckView
→ MainComponent callback-forwarding shape (DeckView.cpp:137-170 is pure passthrough, no logic).

## 2026-08-02 — Mapping-probe design: processFrame sits BEHIND the content gate; mapping writes ignore effect enabled state
**Files:** src/render/Renderer.cpp (renderOpenGL early-out ~:199-208), src/mapping/MappingEngine.cpp (processFrame), tests/visual/test_mapping_tick.py
**Note:** Two facts any mapping/param probe must bake in: (1)
`mappingEngine_.processFrame()` runs AFTER Renderer::renderOpenGL's
`!hasImage && !sourceActive && !deckActive` early-return, so on a fresh
test-mode app with no content the mapping tick never fires — a probe that
skips loading content false-fails "frozen" even with the preview attached.
Load `checkerboard` (or any source) first. (2) processFrame checks only
effect existence + param bounds, NOT Effect::isEnabled() — mapped params
track on disabled effects, so probes need not enable anything and cause no
visual side effects. Also: TestServer's /api/reset does NOT clear mappings
(only effects/image/source/features) — probes drain their own mappings in
teardown via repeated remove_mapping(0) until num_mappings_before==0.
**Valid while:** renderOpenGL keeps the content early-out ahead of the
mapping tick call (until W5 moves the tick to a message-thread timer —
after W5 fact (1) applies only to pre-W5 builds), and handleReset keeps its
current clear set.

## 2026-08-02 — EffectChainGLState must be release()d on context CLOSE, not just owned per-renderer
**Files:** src/effects/EffectChain.h (EffectChainGLState), src/render/Renderer.cpp (openGLContextClosing), src/ui/OutputWindow.cpp (openGLContextClosing)
**Note:** Moving uniformLocationCache + prevFrame quartet per-renderer (W1,
outputwindow-arc) closes the CROSS-CONTEXT poison, but the pre-move code had a
second, same-context poison nobody had named: the shared cache was NEVER
cleared across context RECREATION (hide/show → JUCE synchronous detach →
re-attach recompiles all ~80 programs), so program-ID-keyed locations from the
dead context were served against the new context's recycled program IDs, and
ensurePrevFrameFBO's `texture != 0 && size matches` early-out could keep
binding a DEAD texture name in the new context forever. That is why
EffectChainGLState::release() is called from BOTH renderers'
openGLContextClosing() — per-context state must die with the context
generation, not just live per-renderer. Any future per-context state bundle
here needs the same close-hook or it re-arms this class.
**Valid while:** EffectChainGLState exists and JUCE recreates GL contexts on
hide/show (juce_OpenGLContext componentVisibilityChanged → detach).

## 2026-08-02 — Seqlock "last-good fallback" MUST be per-reader state; bounded retry exhausts under preemption, not just cadence math
**Files:** src/features/FeatureBus.cpp (read()/readIfNewer), tests/test_feature_bus.cpp (multi-reader case)
**Note:** The S2 seqlock's 4-attempt bounded retry was sized by cadence math
(publish duty ~1e-5 → exhaustion ~1e-20), but that model assumes attempts are
short in WALL-CLOCK time. A reader thread descheduled mid-copy (TSan runtime
locks + spinning readers made this routine; production equivalent = heavy
system load) has attempts spanning milliseconds, so consecutive publishes can
kill all 4 attempts — the multi-reader test recorded thousands of fallback
returns under a saturating writer and still 81 at a 2ms-gap writer under
TSan. Two consequences baked into the implementation: (1) the exhaustion
fallback must be LAST-GOOD (stale-but-coherent, per-reader monotonic), never
an unverified word-splice — a splice can step BACKWARDS, which is exactly the
mixed-generation damage class (spurious structuralState/beat edges) the
rewrite exists to kill. read() keeps a thread_local {bus*, snapshot} slot
(320B/thread) updated on every verified copy; readIfNewer's last-good is the
caller's own `out` (returns false, leaves it untouched). (2) Concurrency
tests for bounded-retry structures must model the WRITER's real cadence
(gaps), not a zero-gap saturating loop — saturation is outside the ratified
contract and the design intentionally degrades there instead of blocking.
**Valid while:** FeatureBus keeps the 4-attempt bounded-retry seqlock design.

## 2026-08-02 — TSan race demonstration craft: single-memcpy readers may not trip TSan where field-by-field consumption does
**Files:** tests/test_feature_bus.cpp, .harmony/s2-tsan-before.log
**Note:** The R9 "demonstrated racing before" gate initially produced logical
violations (timestamps going backwards) but ZERO TSan reports: the reader
did one tight ~30ns `FeatureSnapshot local = *rs;` copy, too narrow a window
to overlap the writer's fill in practice. Rewriting the reader to consume
field-by-field straight off the returned pointer (the REAL deployed shape —
AudioReadoutPanel/TopBar read dozens of scattered fields) and the writer to
fill ~35 fields one-by-one (the real 14-stage AnalysisThread fill) made TSan
fire 7 distinct data-race reports on the bus buffers immediately. Lesson for
future race-demonstration tests: replicate the production ACCESS SHAPE
(scattered field reads, long fills), not an idealized memcpy — and note that
logical assertion failures and TSan reports are independent evidence axes
(either can fire without the other).
**Valid while:** general craft note (not tied to specific code).

## 2026-08-02 — S2 writer topology: ONE FeatureBus::Writer, claimed at MainComponent testMode_ branch; ApiServer inject relays to TestServer
**Files:** src/MainComponent.cpp (claim site ~:1568), src/analysis/AnalysisThread.{h,cpp}, src/test/TestServer.{h,cpp} (injectSnapshot), src/api/ApiServer.{h,cpp} (onInjectFeatures)
**Note:** After S2, nothing publishes to the FeatureBus except through the
single move-only Writer handle: production → AnalysisThread (set BEFORE
startThread; run() returns immediately if the handle is invalid), test →
TestServer (ctor takes the Writer by value). ApiServer (port 7070) holds
`const FeatureBus&` only; its test-mode /api/inject_features builds the
snapshot and relays it through the `onInjectFeatures` callback that
MainComponent wires to TestServer::injectSnapshot — which serializes ALL
inject paths (TestServer's own 8080 handlers + the 7070 relay, each on
httplib thread-pool threads) behind one mutex before touching the Writer. If
a future feature needs a second producer, the answer is the council-recorded
production-injection relay (spec "Recorded future options"), NOT a second
Writer. Also: OutputRenderer's old FeatureBus read was DEAD at HEAD (no
consumer since ca0b425 deleted its processFrame call) — deleted in S2;
OutputRenderer::featureBus_ (now const&) is intentionally retained for the
R10/OutputWindow arc.
**Valid while:** FeatureBus::Writer single-claim design and the testMode_
branch claim site are unchanged.

## 2026-08-02 — outputWindow_ content is ALWAYS loaded in lockstep with previewPanel_ (MainComponent.cpp)
**Files:** src/MainComponent.cpp, src/ui/OutputWindow.cpp, src/render/Renderer.cpp
**Note:** Before deleting the duplicate `mappingEngine_.processFrame()` call
at OutputWindow.cpp:115 (R2 of scout-outputwindow-glcrash.md), verified that
`Renderer::renderOpenGL()`'s own processFrame call (Renderer.cpp:239) is
gated by the MAIN renderer's `texMgr_.hasImage() || sourceActive ||
deckActive` early-return (Renderer.cpp:199-208), which is a SEPARATE
TextureManager instance from OutputRenderer's own texMgr_ (each renderer's
texMgr_ is per-renderer, per scout-outputwindow-glcrash.md §1). Grepped every
`outputWindow_->loadImage(...)`/`queueCameraFrame(...)` call site in
MainComponent.cpp (10 sites: :649,:1393,:2123,:2381,:2481,:2725,:2769,:3173,
:3306,:3365) — every one pairs the output-window load with an identical
`previewPanel_.loadImage(X)` (which forwards to `renderer_.loadImage(X)`,
PreviewPanel.cpp:57) or `previewPanel_.getRenderer().queueCameraFrame(X)`
using the SAME source, in the SAME handler. The one apparent exception
(:2481, output-window creation) loads `currentImageFile_`, which is itself
only ever set alongside a `previewPanel_.loadImage()` call — so it's not
actually independent. This means the main renderer's texMgr_ is guaranteed
already populated (gate open) whenever the output window's texMgr_ is
populated, so deleting the output window's processFrame call is safe: the
main renderer's call always covers it. If a future call site EVER loads
content into outputWindow_ without a matching previewPanel_ load, this
invariant breaks and the shared EffectChain's mapped params would go stale
for the output window on frames where the main renderer's gate is closed.
**Valid while:** the per-renderer texMgr_ split and the lockstep-loadImage
pattern in MainComponent.cpp are unchanged (i.e., until the queued
per-renderer EffectChainGLState refactor, which may restructure this).

## 2026-08-02 — Hand-rolled lock-free hand-off structures need acq_rel on BOTH sides, not directional release/acquire
**Files:** src/features/FeatureBus.cpp (publishWrite/acquireRead CAS loops)
**Note:** FeatureBus's wait-free triple buffer packs write/latest/read slot
indices into one atomic byte, swapped via compare_exchange_weak. The index
bookkeeping (a permutation-preserving transposition on each CAS) was already
correct, but publishWrite() used success=release and acquireRead() used
success=acquire — the textbook one-directional producer/consumer pairing.
That only covers HALF of what this structure actually does: each side both
consumes a buffer the other side just handed over (needs acquire) AND hands
its own now-vacated buffer back for the other side to reuse later (needs
release). A directional-only pairing leaves the "hand a buffer back" half
with no happens-before edge, which TSan caught as a real race between the
writer's fill and a reader's read of the same slot several iterations later
(test_feature_bus.cpp:143 vs :161) even though the index math never lets
write==read at any instant — the C++ standard's race definition cares about
proven happens-before, not about whether it would have overlapped in
practice. Fix: both CAS success orders -> memory_order_acq_rel (failure
orders stay relaxed; no store happens on a failed CAS, so nothing to
strengthen there). Rule of thumb for any FUTURE hand-off structure here
(ring buffers, other lock-free queues): if ownership of a resource moves
BOTH directions between two threads via the same atomic, single-direction
release/acquire is not enough — check whether acq_rel is needed on each side
independently, don't assume "writer=release, reader=acquire" is symmetric.
**Valid while:** FeatureBus keeps this packed-atomic-byte triple-buffer design.

## 2026-07-30 — JUCE Debug builds silently break unqualified addAndMakeVisible on ResizableWindow subclasses
**Files:** src/ui/OutputWindow.cpp:273 (the live break), build-asan/_deps/juce-src/modules/juce_gui_basics/windows/juce_ResizableWindow.h:376-391
**Note:** `ResizableWindow` declares its own `#if JUCE_DEBUG`-guarded
`addAndMakeVisible(Component*, int)` purely to warn developers away from
adding children directly (instead of `setContentOwned()`). This Debug-only
override name-hides ALL of `Component`'s inherited overloads, including the
`Component&` reference form — so `addAndMakeVisible(someComponentRef)` in any
`ResizableWindow`/`DocumentWindow`/`TopLevelWindow` subclass compiles fine in
Release (JUCE_DEBUG undefined, no hiding) but fails to compile in ANY Debug
build (including build-asan/build-tsan, both `CMAKE_BUILD_TYPE=Debug`) with
"no viable conversion from 'X' to 'Component *'". OutputWindow.cpp:273 hits
this. Fix is to qualify the call: `Component::addAndMakeVisible(child);`
(the JUCE header comment says exactly this). This had never surfaced before
because ba0ae70 (2026-07-30) was the first commit to add Debug/sanitizer
build-variant wiring — nobody had compiled AudioDNA in Debug before. Any
sanitizer (ASan/TSan/UBSan) work on this repo needs this fixed before the
full `AudioDNA` app target will build; the Catch2 test targets are unaffected
(none link OutputWindow.cpp).
**Valid while:** OutputWindow.cpp inherits from a JUCE ResizableWindow-family
class and calls addAndMakeVisible unqualified; general rule holds for any
JUCE ResizableWindow/DocumentWindow subclass in this codebase.

## 2026-07-30 — EffectChain/activeSources_: GL-context-recreation is a REAL runtime event, not theoretical
**Files:** src/render/Renderer.cpp (initEffectChain, getOrCreateSource/getOrCreateSourceOnGLThread), src/effects/EffectChain.h/.cpp
**Note:** Before 76594fd, GL-context close/recreate cycles (previewPanel_
hide/zero-size → JUCE synchronous GL detach → later resize/show → context
recreated → newOpenGLContextCreated() re-fires) were assumed rare/edge-case.
76594fd's MilkDrop UAF fix proves this path is REAL and already observed live
(SignalBar-expand arming event). Any Renderer method that does one-time GL
context setup work in newOpenGLContextCreated() (like the old
`initEffectChain()`, which unconditionally re-populated effectChain_ every
call) must either be idempotent or explicitly re-derive its state — CPU-only
data (Effect objects, no GL handles) must guard against re-population;
GL-handle state (shaders, FBOs) correctly SHOULD re-init every time. Pattern
used for cross-thread map ownership (activeSources_): confine ALL mutation to
one owner thread (the GL thread, detected via
`juce::OpenGLContext::getCurrentContext() != &glContext_`, a JUCE
thread-local) and marshal non-owner callers through a blocking
`executeOnGLThread(fn, true)` round-trip — callers already on the owner
thread must call the raw (non-marshaling) helper directly, since a
self-marshal deadlocks (the blocking call waits for the owner thread to
service its queue, which it can't do while blocked on itself). Reusable
pattern for any future GL-thread-owned container with cross-thread callers.
**Valid while:** Renderer's activeSources_/effectChain_ ownership model is
unchanged (see the OWNERSHIP MODEL comment at activeSources_'s declaration,
Renderer.h).

## 2026-07-30 — FEAT lane: LayerStrip FX-drop / mixed-drop / Cmd+X / retrigger-restart
**Files:** src/ui/LayerStrip.h+.cpp, src/ui/DeckView.h+.cpp, src/ui/ClipCell.h+.cpp,
src/MainComponent.h+.cpp (4 commits: 8f41bd9, 4ba9748, 814f633, b391b64)
**Note:** (1) LayerStrip has no embedded EffectStackView (unlike LayerInspector/
CompositionInspector), so mirroring 9c316e6's panel-forward pattern here means the
strip only forwards the raw "fx:Name1,Name2" description via a new onEffectDropped
callback — the host (MainComponent) does the EffectSlot construction + GL fence +
EffectStackCmd(EffectScope::layer(...)) construction, same shape as ClipCell's
existing onEffectDrop forwarding, not the embedded-view mutate-then-push shape.
(2) ClipCell::filesDropped's 4-branch mutually-exclusive early-return structure
(video branches return before image branches ever run) was the mixed-drop bug;
fixed by adding a NEW combined onMixedFilesDrop path that fires only when both
image and video files are present in one Finder drop, reusing applyFileDrop /
new applyMultiFileDrop (extracted from handleMultiFileDrop, mirrors applyFileDrop's
mutate-without-pushing shape) so the whole gesture stays ONE undo entry — do NOT
call the existing per-type onFileDropped/onMultiFileDropped/onMultiVideoDropped
callbacks for a mixed batch, each pushes its own undo command independently.
(3) MenuBarModel.h's kClipCut/kClipCopy/kClipPaste/kClipCopyEffects/
kClipPasteEffects are RESERVED enum values only — grep-confirmed zero menu.addItem
calls and zero handleMenuCommand cases for any of them (only kClipClear/
kClipReplaceContent/kClipLockContent are wired in the Clip menu). No clipboard
concept exists anywhere at HEAD. (4) Retrigger-of-active-cell was an EMERGENT
no-op, not an early return: Layer::triggerClipImmediate's retrigger branch
(Layer.h) already resets the model's clip->playheadPosition to inPoint, but
Renderer.cpp overwrites that field FROM the player's actual position every frame
— the model reset was real but invisible. Fix needed zero Renderer.cpp/.h edits:
MainComponent::handleClipTrigger already calls renderer.getVideoPlayer(id)->
seekTo()/getImageSequence(id)->seekTo() for the pre-existing beat-snap case: added
an `else if` sibling branch keyed on a `wasRetrigger` flag (captured BEFORE
layer->triggerClip() mutates activeClipColumn) that does the same seek to
clip->inPoint. Confirm this pattern (seek the player via MainComponent's existing
renderer getters) before assuming any retrigger/seek-adjacent bug needs a
Renderer.cpp change.
**Valid while:** LayerStrip stays without an embedded EffectStackView; ClipCell's
external/internal drop paths keep their current split; MenuBarModel's Clip menu
wires only Clear/Replace Content/Lock Content; handleClipTrigger keeps doing its
own player-seek side effects rather than delegating to Renderer.

## 2026-07-28 — LAW: any message-thread mutation of layer.clips MUST be GL-fenced (UAF crash proven)
**Files:** MainComponent.cpp:3712 (addColumn, the crasher), :3557/:3621/:3759 (clears — same exposure, MORE deterministic), Renderer.cpp:28+173, CompositorEngine.cpp:694+734, UndoService.cpp:54-79 (the fence), DeckCommands.h:55,97 + ClipCommands.h:56,160 (unfenced replay)
**Note:** Column→New crashed live (SIGSEGV GL thread, .ips 2026-07-28-182825; scout
diagnosis disassembly-verified, binary UUID matched). renderOpenGL runs with NO
MessageManager lock (setComponentPaintingEnabled(false) — pre-existing at fb271e3);
GL thread holds interior Clip* from getActiveClip across applyClipEffects; ANY
clips-vector realloc/clear/erase on the message thread dangles it (crash registers
held IEEE doubles from rebuildGrid allocations — textbook UAF). PRE-EXISTING, not
lane-introduced (handler byte-identical pre-lane). makeDeckFence/withDeckDetached
covers ALL model reads incl. persistent-layer loop (everything under `if (deckActive)`,
Renderer.cpp:414); fence validated 100/100 @ ~110fps, max 15.6ms. Spec's "status-quo
risk profile" for column/cell writes is FALSIFIED — fence is the rule for structural
clips mutations, live AND command-replay paths (commands need a DeckFenceHook,
pass-through in headless tests per AddLayerCmd pattern).
**Valid while:** renderOpenGL reads the model without a lock (until model snapshot/double-buffer lands)
**Scope riders (2026-07-28, review round):** LAW extends beyond layer.clips to
(1) composition_.decks (kCompNew initDefault — fenced) and (2) clip/layer/global
EFFECTS vectors (GL iterates clip.effects, CompositorEngine.cpp:244; reviewer-ruled
blocker-class; fenced in fence-f1 round 2). FUTURE-FENCE REQUIREMENT: CompDecksBrowser
onCompositionLoad/onDeckLoad/onCompositionSave are UNWIRED at HEAD — whoever wires
them MUST run the model-apply under withDeckDetached + undoManager_.clear()
(kCompNew precedent), else this UAF class returns AND the ClipCommands.h
SetClipCmd/SwapClipsCmd exemption precondition (b) silently breaks.

## 2026-07-28 — BUG (queued fix candidate): external mixed-type drop discards images
**Files:** src/ui/ClipCell.cpp:262-292 (external `filesDropped`), :435-476 (internal `itemDropped` — the CORRECT sibling), :210-228 (interest check)
**Note:** Found in undo-v1 manual e2e sitting (Boris: 2 vids + 1 png → png silently lost).
External Finder-drop path bins video/image then runs 4 MUTUALLY-EXCLUSIVE early-return
branches, video-first — any video present ⇒ ALL images in the batch discarded, silently
(interest check highlights the cell first, so UX promises acceptance). The INTERNAL drag
path in the same class handles the identical mixed batch correctly (images → column_,
videos → column_+1 via `videoStartCol`), proving mixed batches are intended → unhandled
case, not designed filter. Single-PNG drop works; multi-PNG → ImageSequence. Undo coverage
uniform for accepted files (all paths → pushClipEdits/SetClipCmd); discarded files push no
CellEdit. Likely pre-existing at HEAD (inferred — lane wrapped faithfully, no branch
restructure in ledger; unverified by blame). REMEDY SKETCH: make external path mirror
internal (route mixed batch through the same image+video split), Boris nod required.
Extension-list nits while there: image bin lacks .tif/.webp/.heic; internal path treats
"not video" as image via bare else.
**Valid while:** ClipCell::filesDropped keeps the 4-branch early-return structure

## 2026-07-20 — Renderer video/sequence/image resources are keyed by clip id; only video content-swaps
**Files:** src/render/Renderer.cpp (videoPlayers_), src/render/CompositorEngine.cpp:1047
**Note:** Videos (videoPlayers_[id]) and image sequences (imageSequences_[id]) are keyed by clip id — **"NEVER closed" is STALE as of 2026-09-05's L1 media-leak fix (see that entry, above)**; Clear/RemoveColumn/ClearLayerClips now dispose via closeMediaForClip, they just still never get closed on a REPLACE (see below), which is the part of this entry that's still true. Static images are keyed by FILE PATH via getKeyTexture(clip.mediaFile) — self-healing on undo. Only VIDEO can be content-swapped under an EXISTING id: kClipReplaceContent calls openVideoForClip(existing->id, newFile), overwriting the player while replaceContent keeps the id. So a reconnect-if-MISSING media guard misses replace-undo (player exists → skip → decodes wrong file). Fix: compare loaded file vs clip.mediaFile (Renderer::getVideoPlayerFile + VideoPlayer::getFile + pure needsVideoReopen() in core/MediaReconnect.h), reopen on mismatch. Sequences can't hit it (replace produces only Image/Video; sequences always get a fresh s_nextClipId).
**Valid while:** replaceContent reuses the clip id and video players are keyed by id

## 2026-07-19 — Undo v1 Step 2: clip commands decoupled from Renderer via hooks
**Files:** src/core/ClipCommands.h, src/core/UndoService.h, tests/test_undo_commands.cpp
**Note:** Spec §7 wants commands unit-tested headless against a bare Composition. Achieved by making commands (SetClipCmd/ToggleClipLockCmd) depend ONLY on injected std::function hooks — `ClipLayerResolver` (re-resolve Layer by coord) + `ClipMediaHook` (video/seq reconnect) — NOT on concrete Renderer. The app wires hooks from UndoService+renderer; tests wire a bare-Composition resolver + no-op media. Also made `UndoService::resolve*/setCollaborators` INLINE in the header (renderer-free, only touch Composition) so `test_undo_commands` links WITHOUT UndoService.cpp/Renderer (which pulls juce_opengl + CompositorEngine + video/syphon — too heavy for a unit test). Pattern to reuse for step 3-8 command tests.
**Valid while:** ClipCommands.h uses hook typedefs and UndoService resolvers stay inline

## 2026-07-19 — Wrapping entry sites: capture before → mutate → capture after → perform (idempotent re-apply)
**Files:** src/MainComponent.cpp (handleFileDrop, onEffectDropped, kClipClear, etc.)
**Note:** UndoManager has only perform() (executes). To wrap an existing handler without double-side-effects: capture `before` at the TOP (before any mutation), let the existing code mutate + do its rich UI/player setup, capture `after`, then perform() a SetClipCmd(before,after). perform()→execute()→apply(after) RE-APPLIES after — this is idempotent for SetClipCmd (cell already == after; media hook reconnect no-ops since player already open). No UndoManager API change needed. Multi-cell gestures (multi-FX-to-empty-cells, multi-select clear) use CompositeCommand via pushClipEdits()/pushCommands() which guards isEmpty() before perform.
**Valid while:** UndoManager exposes only perform() and SetClipCmd.apply is idempotent

## 2026-07-19 — Undo v1 Step 1: GL-fence validated + headless-test MessageManager assert
**Files:** src/core/UndoManager.cpp, src/core/UndoService.cpp, tests/CMakeLists.txt
**Note:** (1) GL fence empirically validated: blocking `glContext_.executeOnGLThread(noop,/*block*/true)` from the MESSAGE thread does NOT deadlock the render thread (setComponentPaintingEnabled(false) → GL thread never takes the msg lock). 100 iters, max 15.6ms / mean 2.8ms round-trip, app stayed responsive (spec risk #9 resolved). (2) `jassert(MessageManager::existsAndIsCurrentThread())` would ABORT headless Catch2 tests (no MM instance) if ever built Debug — guard it `getInstanceWithoutCreating()==nullptr || existsAndIsCurrentThread()`. (3) MessageManager lives in juce_events; test_composition only linked juce_core+juce_graphics, so any UndoManager.cpp use of MessageManager requires adding `juce::juce_events` to that test target.
**Valid while:** UndoManager/UndoService exist and tests build Release with these link libs

## 2026-07-19 — App launch can transiently stall in CoreAudio device init (TCC), NOT a code bug
**Files:** src/MainComponent.cpp (AudioDeviceManager::initialiseWithDefaultDevices in ctor)
**Note:** First `open` of the .app sometimes hangs in the constructor at `AudioDeviceManager::initialiseWithDefaultDevices` → CoreAudio `start()` → HALC_ProxyObject::SetPropertyData (TCC/coreaudiod). Symptom is identical to the documented direct-exec hang: process alive, ZERO windows, ZERO listening sockets, no port 7070. Fix: `pkill -9 -f Audio-DNA` and re-`open` — the second launch bound port 7070 in ~4s. `sample <pid>` on the stuck process shows the exact ctor frame, which is the fast way to distinguish an env/audio stall from an app-code hang.
**Valid while:** MainComponent constructs AudioDeviceManager before finishing init

(No entries yet. First normalization session 2026-05-18.)

## 2026-05-18 — Eyes + Accessibility Inspection Stack

### How Harmony Inspects Audio-DNA

Three complementary inspection paths, all scriptable:

**1. Eyes API (visual output + engine state)**
- Port 7070 (ApiServer, always on) or 8080 (TestServer, test-mode only)
- Capture rendered frame: `POST /api/render_frame {"output_path": "/tmp/frame.png", "time": 0.0}`
- Query state: `GET /api/status` → FPS, BPM, beat phase, genre
- Query composition: `GET /api/composition` → full deck/layer/clip hierarchy as JSON
- Query features: `GET /api/features` → all audio analysis values
- Python client: `tests/visual/vj_controller.py` (VJAppController class)
- Vision comparison: `tests/visual/vision_check.py` (PSNR/SSIM)

**2. ax_inspector.py (JUCE UI component tree)**
- Reads macOS accessibility tree via AXUIElement API (pyobjc)
- Zero C++ changes — reads from outside the process
- Returns: component hierarchy with roles, titles, values, positions, sizes
- CLI: `python tests/visual/ax_inspector.py --app "Audio-DNA" --depth 5`
- Import: `from ax_inspector import inspect_app; tree = inspect_app("Audio-DNA")`

**3. Melatonin Inspector (manual debugging)**
- Build with: `cmake -DAUDIODNA_BUILD_INSPECTOR=ON`
- Toggle at runtime: Cmd+Shift+I
- Visual only, no export API — for Boris's live debugging

### For Tester Agents
When validating UI changes, use this sequence:
1. `GET /api/status` — confirm app is running
2. `POST /api/render_frame` — capture current visual state
3. `python tests/visual/ax_inspector.py` — capture UI component tree
4. Compare render against expected visual state
5. Compare component tree against expected UI structure

## 2026-05-18 — conftest autouse fixture blocks non-Eyes tests
**Files:** tests/visual/conftest.py, tests/visual/test_ax_inspector.py
**Note:** The conftest.py in tests/visual/ has an `autouse=True` fixture `reset_between_tests` that depends on the `app` fixture, which skips all tests when the Audio-DNA executable is not built. Any new test file in tests/visual/ that does NOT need the running app must override both `app` and `reset_between_tests` fixtures locally to avoid being skipped.
**Valid while:** tests/visual/conftest.py still has autouse=True on reset_between_tests

## 2026-05-18 — set_effect_chain and state endpoints ported to ApiServer
**Files:** src/api/ApiServer.cpp, src/api/ApiServer.h
**Note:** POST /api/set_effect_chain and GET /api/state are now available on port 7070 (ApiServer, always-on) in addition to port 8080 (TestServer, test-mode only). ApiServer version wraps state response in {"ok": true} pattern unlike TestServer's raw JSON. The param write uses direct assignment (fx->getParam(p).value = val) matching ApiServer style, not fx->setParamValue() as in TestServer. ApiServer.cpp is at exactly 900 lines — budget ceiling. Melatonin inspector FetchContent is broken (module header not found); build with -DAUDIODNA_BUILD_INSPECTOR=OFF to work around.
**Valid while:** both ApiServer.cpp and TestServer.cpp contain these endpoints

## 2026-07-17 — triage-execution session learnings (lane-B capture, secondary)

- **Launch-context gate trap**: direct binary exec of the app from an agent shell hangs
  pre-UI (alive, windowless, no sockets) — always `open` the .app for behavioral gates.
  (Also in gotchas.md — canonical.)
- **Don't prescribe concurrency idioms in packets — prescribe the stress test**: packet
  suggested double-buffer for the waveform fix; builder's own torn-read stress test
  proved it tears under reader-lapping and shipped a seqlock instead. The test
  requirement, not the idiom suggestion, produced the correct fix.
- **Probe the variable the write path touches**: /api/status masterLevel reads
  renderer_.getMasterLevel() while OSC /master writes composition_.masterOpacity —
  first probe false-alarmed. Trace write target → pick readback endpoint.
- **Combine contending lanes**: B+D shared MainComponent.cpp + the build dir → one
  combined builder beat two parallel ones (zero races, zero retries).
- **Shared-doc consolidation protocol validated**: parallel builders leave shared-file
  (APP-INVENTORY) edits UNCOMMITTED + flag them; a dedicated doc-sync lane reconciles
  drift (caught 113-vs-114 test count) and commits once. Adopt for all multi-builder waves.
- **Force-add hygiene (process rule)**: force-adds of gitignored-but-tracked-policy files
  get their OWN commit + explicit commit-message mention — never swept into a feature
  commit (W1-A reviewer finding).

## 2026-07-22 — lane-B meta-learnings (Undo v1 s4-7 secondary)
- **pre-declared-budget-rider**: declare the disposition rule ("beyond-NIT finding → HOLD commit") in the ledger AT DISPATCH TIME, before results exist — turns a mid-flight judgment call into a pre-committed policy and removes rationalization pressure when the finding lands. Applied s7; fired correctly.
- **remedy-b-doc-truth**: when a MAJOR is "code comments overclaim vs a PRE-EXISTING mechanism gap", the NIT-cost remedy is comment-truth + tracked follow-up, not smuggling a shared-path fix into the step. Ask the reviewer to rule on the disposition explicitly and INVITE overrule — independence is worth more than speed.
- **reviewer-rotation-at-2-packets**: rotate reviewers on the same load rule as builders (~2 full packets + 1 targeted); fresh eyes caught that a claimed "precedent" (headless-silent invariant violation in tests) never actually existed.
- **env-wedge-single-recheck**: when an env blocker (coreaudiod stall) worsens with remedy cycles, switch to single-attempt rechecks per gate + a hard stop — retry loops actively degrade the environment (4-attempt evidence, sample-verified).

## 2026-07-30 — MilkDropBrowser crash #2 guard + default preset bundling
**Files:** src/ui/MilkDropBrowser.cpp, CMakeLists.txt
**Note:** Crash #2 (.ips 2026-07-28-190701, SIGSEGV in `getCuratedPresets` via
a SignalBar-arrow resize cascade) could not be root-caused from the stack
trace + static reading alone — every reachable call path into
`getCuratedPresets()`/`calculateContentHeight()` was already null-guarded at
HEAD (commit 0a617cf, unchanged since March), and the crashed binary's UUID
(a14c7610...) doesn't match anything currently on disk to disassemble. Found
one CONCRETE unguarded null-deref nearby in the same class —
`selectPreset()` called `presetManager_->getPreset(globalIndex)` without a
null check even though the line above it correctly guarded
`setCurrentIndex` — fixed that, and hardened `getCuratedPresets()` /
`getPresetsForSection()` to independently guard on `!presetManager_ ||
getPresetCount()==0` (matching the existing `calculateContentHeight()`/
`paint()` idiom) rather than relying on caller-side checks only. Separately:
`resources/projectm_presets/` (30 curated .milk + presets.json) already
exists in-repo and MainComponent.cpp already scans it, but the build never
copied it into `Audio-DNA.app/Contents/Resources/`, and the CWD dev-fallback
only works if launched with the project root as CWD — so the shipped app's
browser started empty. Fixed via a CMake POST_BUILD `copy_directory`
registered BEFORE the existing codesign POST_BUILD (same-target POST_BUILD
commands run in registration order) so the signature covers the copied
resources.
**Valid while:** MilkDropBrowser.cpp's presetManager_ access pattern and
MainComponent.cpp's bundledDir/CWD-fallback resolution logic are unchanged.

## 2026-07-25 — lane-B meta-learnings (Undo v1 s8-9 secondary, lane close)
- **screencapture-tcc-diagnosis**: headless app-launch "stalls" can be an invisible TCC dialog — `screencapture -x /tmp/x.png` + image read sees what sample/lsof/log probes cannot. The 2-session "coreaudiod wedge" was an unanswered mic prompt. (Also log-event'd mid-session; universal candidate.)
- **codesign-check-on-recurring-tcc**: when a TCC prompt seems to re-fire "randomly", run `codesign -dv` — ad-hoc signing (no TeamIdentifier) changes cdhash per rebuild → TCC re-prompts every build. Durable fix = stable signing identity (Boris-ratified for this repo).
- **pre-adjudicated-trivial-fix**: reviewer-PRESCRIBED comment-truth edits skip the re-review pass (edit is pre-adjudicated); Harmony's gate re-run covers. Used twice this session (s8 MINOR, s9 MINOR) at near-zero cost — but ONLY for doc-only, reviewer-verbatim edits.
- **content-verify-already-covered-claims**: a builder's "already covered elsewhere, not duplicating" claim gets CONTENT-verified (read the covering test), not existence-verified (grep the name) — receiver disk-verify + reviewer both re-checked s9's A1 claim before accepting non-duplication.
- **user-initiated-vs-autonomous undo doctrine**: remote gestures (REST/OSC/MIDI pad hits) are USER actions → undoable through the shared handlers; autonomous mutations (autopilot, genre-auto) NEVER create commands. Contrast with deck-switch (step 6) where spec put remote call sites OUTSIDE the wrap — the spec row governs per-op; check it, don't assume a global rule.

## 2026-08-02 — ApiServer's /api/inject_features (7070) and TestServer's copy (8080) are fully independent
**Files:** src/api/ApiServer.cpp, src/test/TestServer.cpp, tests/visual/vj_controller.py
**Note:** Both servers register their own `/api/inject_features` handler (ApiServer.cpp, TestServer.cpp:106); they are separate route tables on separate ports, not a shared implementation. `tests/visual/vj_controller.py` defaults to port 8080 (TestServer) and every `inject_features()` call in the e2e visual suite goes there — confirmed via grep, no e2e test hits ApiServer's copy. This meant gating ApiServer's inject_features behind an `allowFeatureInjection_` ctor flag (default false; production hardening, featurebus-thread-safety-design.md R6) was safe to do without touching TestServer or breaking the visual suite. Also: no ctest (Catch2) target links ApiServer.cpp or MainComponent.cpp at all (grepped tests/CMakeLists.txt) — the unit suite can't exercise this endpoint either way; only a live HTTP probe (curl) can.
**Valid while:** ApiServer.cpp and TestServer.cpp keep separate route registration (no shared handler refactor) and vj_controller.py's default port stays 8080.

## 2026-08-02 — ApiServer.cpp/tests/README.md's "900 lines, budget ceiling" note is already stale
**Files:** src/api/ApiServer.cpp, tests/README.md
**Note:** tests/README.md:85 and notebook.md's 2026-05-18 entry both claim "ApiServer.cpp at exactly 900 lines — budget ceiling, next addition needs refactor first." At session start (before this session's ~24-line R6/R8 hardening diff) the file was already 1017 lines — the ceiling was breached by a prior session without the doc being updated or the refactor happening. Not fixed here (out of scope for this work packet); flagging so the next session that hits this doc doesn't trust a stale number.
**Valid while:** tests/README.md:85 still states the 900-line figure.

## 2026-09-05 — MilkDrop folder pref (L7) wired end-to-end; introduces the app's FIRST auto-persisted setting
**Files:** src/ui/PreferencesDialog.{h,cpp}, src/MainComponent.{h,cpp}, src/render/Renderer.{h,cpp}
**Note:** `Preferences > Video`'s MilkDrop Presets field/Browse button existed and worked at the
widget level, but `ProjectMPresetManager::setPresetDirectories()`/`rescan()` had ZERO callers —
picking a folder wrote into a `juce::TextEditor` nobody read. Wired via `MainComponent::
setMilkDropPresetDir()` (fired by a new `PreferencesDialog` ctor/`show()` param pair, mirroring
the existing `onTooltipToggled` shape). **Important gotcha for anyone touching `presetDirs_`
again:** `ProjectMPresetManager::rescan()` does `presets_.clear()` then rescans ONLY
`presetDirs_` — before this fix, the ctor populated `presets_` via direct `scanDirectory()`
calls that never touched `presetDirs_` (always empty), so calling `setPresetDirectories({userDir})
+ rescan()` naively would have WIPED the bundled + Cream-of-the-Crop presets. Fixed by having the
ctor register bundled+cream into `presetDirs_` too (via `setPresetDirectories()+rescan()`,
`milkDropBaseDirs_` caches this base set), so every later rescan is base-set + user dir, never
base-set alone.
**Threading — REAL race found and closed, not improvised:** `PresetSelector::processFrame`
(`ProjectMSource.cpp:136`, inside `ProjectMSource::render()`) reads `presetManager_` every frame
on the GL thread with NO lock in `ProjectMPresetManager`. A Preferences-triggered rescan runs on
the message thread. Rather than add per-method locking to `ProjectMPresetManager` (rejected:
`randomPresetInMood()` calls `randomPreset()` internally — a naive same-mutex lock on both would
self-deadlock), the fix confines the mutation to the GL thread via a NEW
`Renderer::rescanMilkDropPresets()`, mirroring the file's own existing `getOrCreateSource()` /
`activeSources_` confinement idiom byte-for-byte (same `isAttached()` guard, same
`getCurrentContext() != &glContext_` check, same blocking `executeOnGLThread`). Zero changes to
`ProjectMPresetManager` itself.
**Also new:** the app's first ever auto-persisted setting. `tooltipsEnabled_` (the ONLY other
Preferences-tab value) resets to `true` every launch — there is no `ApplicationProperties`/
`PropertiesFile` anywhere in this codebase. Persistence for the MilkDrop dir uses
`userApplicationDataDirectory/Audio-DNA/settings.json` via `juce::DynamicObject`+
`JSON::toString`/`JSON::parse` — the exact idiom already used for View > Save/Load Layout
(`MainComponent.cpp`, `kViewSaveLayout`/`kViewLoadLayout`), just auto-triggered instead of behind
an explicit FileChooser. If a broader settings system gets built later, this key
(`milkDropPresetDir`) should move into it.
**Valid while:** `ProjectMPresetManager` has no internal locking and `PresetSelector::
processFrame` stays GL-thread-only (both true as of this commit).

## 2026-09-05 — L7-JUKE: Jukebox autopilot's dead onAutoSwitch fixed; Pool/Mode/Blend wired; Link + MIDI-out pickers added
**Files:** src/sources/PresetSelector.{h,cpp}, src/sources/ProjectMSource.cpp, src/ui/MilkDropBrowser.cpp, src/ui/TopBar.{h,cpp}, src/ui/PreferencesDialog.{h,cpp}, src/MainComponent.cpp
**Note:** `PresetSelector::onAutoSwitch` was declared and invoked (processFrame()) but never
assigned — Jukebox Play seeded one preset manually then never advanced again, regardless of
Timing/Pool/Mode/Blend settings, because nothing pushed `manager_->randomPreset()`'s
`currentIndex_` mutation back into `ProjectMSource::loadPreset()`. Fixed with one line in
`ProjectMSource`'s ctor: `presetSelector_.onAutoSwitch = [this](path){ loadPreset(path, true); };`
— safe to call directly from the GL-thread `processFrame()` call site because `loadPreset()`
already queues under a mutex for the render loop to pick up. This was the prerequisite for
Pool/Mode/Blend to have any visible effect at all; do this same check (`grep -rnE
"onAutoSwitch[[:space:]]*=[^=]" src/`) before trusting any other `std::function` member is
actually wired — declared+invoked-but-never-assigned is an easy defect class to miss because
the code compiles and looks complete.
**Pool/Mode design (open combination gap, not silently resolved):** Pool (All/Curated/Favorites)
and Mode (Bag/Random/Sequential) are independent enums on `PresetSelector`. Pool-filtered picks
go through a new anonymous-namespace `poolCandidates()`/`pickRandomFrom()` pair in
`PresetSelector.cpp` that mirrors `ProjectMPresetManager::randomPresetInMood()`'s own
candidate-list + currentIndex_-sync pattern (needed because `manager_->randomPreset()` always
draws from ALL presets — there's no way to hand it a restricted candidate list). **Mode's
Sequential branch calls `manager_->nextPreset()` directly and does NOT additionally honor
poolFilter_** — `nextPreset()` walks the manager's full index order with no filtering concept, so
Pool=Favorites + Mode=Sequential will sequence through ALL presets, not just favorites. This
matches the L7-JUKE work packet's literal instruction ("Sequential → manager_->nextPreset()") but
the packet never addressed the combination case. If someone needs true Pool+Sequential, `nextPreset()`
needs a filtered variant or PresetSelector needs its own local index tracking over the filtered
list — not done here (flagged as an open product/design question, see this lane's builder report).
**"Curated" predicate is duplicated, not shared:** `MilkDropBrowser::getCuratedPresets()`'s
`p.energy > 0.1f` predicate is UI-local (private to MilkDropBrowser, not visible from
PresetSelector/ProjectMPresetManager) — duplicated verbatim in `PresetSelector.cpp`'s
`poolCandidates()` rather than promoted to a shared `ProjectMPresetManager` method (that
promotion was explicitly out-of-scope/optional for this lane — touches 2 more files for no
behavioral gain). If the predicate ever changes, it must be changed in BOTH places.
**MainComponent.h fence exclusion → best-effort MIDI device-id seeding, not a stored one:**
`PreferencesDialog::show()`'s new MIDI tab needs to seed its dropdown from "the current device
id", but `MidiOutputHandler` only exposes the open device's display NAME (`getDeviceName()`), not
the identifier it was opened with, and this lane's fence excluded `MainComponent.h` (only
`MainComponent.cpp` was writable) so there was nowhere to cache the identifier as a new member.
Worked around with a file-local anonymous-namespace helper in `MainComponent.cpp`
(`currentMidiOutputDeviceId()`) that matches the open device's name back against
`MidiOutputHandler::getAvailableDevices()` — best-effort (fails silently to "no selection" if no
device is open, or if a same-named device can't be found; duplicate device names would pick the
first match). A future session with `MainComponent.h` in scope should add a real
`juce::String midiOutputDeviceId_` member instead.
**Valid while:** `MidiOutputHandler` has no `getOpenDeviceId()`-style accessor and
`ProjectMPresetManager::nextPreset()` has no pool-filtering parameter (both true as of this
commit).

## 2026-09-05 — L7-JUKE follow-up: Jukebox Pool=Favorites turned a dormant presets_.favorite race live; closed via GL-thread confinement (Renderer::toggleFavoritePreset)
**Files:** src/render/Renderer.{h,cpp}, src/ui/MilkDropBrowser.{h,cpp}, src/MainComponent.cpp, src/sources/PresetSelector.cpp, src/sources/ProjectMPresetManager.{h,cpp}
**Note:** `ProjectMPresetManager::presets_` has no internal lock (documented invariant, see the
2026-09-05 "MilkDrop folder pref" entry above: "`PresetSelector::processFrame` stays GL-thread-only").
Before this fix, `getFavorites()`'s only callers were message-thread UI code
(`MilkDropBrowser.cpp` paint/layout). The L7-JUKE Jukebox-Pool feature added the FIRST GL-thread
caller (`PresetSelector.cpp`'s `poolCandidates()`, invoked from `processFrame()` when
`PoolFilter::Favorites` is selected) — this turned a previously-dormant race against
`ProjectMPresetManager::toggleFavorite()`'s unsynchronized message-thread write (fired by
right-clicking a preset row in `MilkDropBrowser.cpp`) into a live one. **Same hazard shape as
`rescanMilkDropPresets()`, same fix:** added `Renderer::toggleFavoritePreset(int)`, mirroring
`rescanMilkDropPresets()`'s exact 3-branch structure (isAttached() early-out /
getCurrentContext()-gated blocking `executeOnGLThread` marshal / already-on-GL-thread inline) —
do not simplify this shape if you touch it again; the two are meant to read as interchangeable
so a future reader can trust either as the reference. `MilkDropBrowser`'s right-click handler no
longer calls `presetManager_->toggleFavorite()` directly — it fires a new
`onToggleFavoriteRequested(int)` callback (mirrors the existing `onPresetSelected` shape),
wired UNCONDITIONALLY at `MainComponent` construction time (NOT inside the lazy
`setOnProjectMSourceCreated` callback that `setPresetSelector` uses — `toggleFavoritePreset()`
needs no live `ProjectMSource`, only `projectMPresetManager_` + `glContext_`, both available
immediately). Deliberately NO fallback to the direct unsynchronized call if the callback is
unset — a silent fallback would silently reopen exactly this race, so a no-op is the correct
failure mode instead (should never trigger in practice: nothing in this repo constructs
`MilkDropBrowser` without going through this wiring — confirmed zero `tests/` references).
**Decision rule used (confinement over a per-field mutex/atomic):** checked against this file's
own stated criterion for `activeSources_`/`effectChain_` (Renderer.h, ~line 380): confine when
the mutation is rare + user-triggered; use a mutex only when the field is read far more often
off the GL thread AND every GL frame. `toggleFavorite()` is a rare right-click (same shape as
`rescanMilkDropPresets()`'s Preferences-triggered write); `getFavorites()` on the GL side only
fires inside an actual auto-switch event, not every frame. A per-field synchronized `.favorite`
was rejected as the fix even though it's viable, because it would leave `PresetInfo`'s OTHER
fields (name/path/mood/energy/userPreset) protected by a DIFFERENT mechanism (GL-thread
confinement via `scanDirectory()`/`rescan()`) than `.favorite` alone — two synchronization
strategies on one struct is the kind of split that gets rediscovered and misattributed later.
**`loadUserData()`/`saveUserData()` are DEAD CODE — zero call sites anywhere in `src/` or
`tests/`** (confirmed via repo-wide grep, not just `src/sources/`). This means favorites/
user-presets never actually get restored from or persisted to disk anywhere in this app today —
a separate, pre-existing gap, NOT touched by this fix (out of scope) and NOT a source of the
race being closed here (a write that never executes can't race anything). Flagging so a future
session doesn't assume `.favorite` persists across restarts, and doesn't rediscover this as a
new bug when it's actually an old, unrelated one.
**Valid while:** `ProjectMPresetManager` still has no internal locking (if it ever gets one,
`toggleFavoritePreset()`'s confinement becomes redundant but harmless) and `loadUserData()`/
`saveUserData()` remain uncalled (if either gets wired up, re-check whether its call site is
provably pre-GL-context or needs the same confinement treatment).

## 2026-09-05 — S166-FAV: loadUserData()/saveUserData() wired up (the "if either gets wired up" case above happened) — this is the re-check
**Files:** src/MainComponent.cpp
**Note:** The prior entry's expiry condition fired. `loadUserData()` is now called once in
`MainComponent`'s ctor, right after `presetManager_.rescan()`/`loadManifest()` (message thread,
pre-GL-attach, same unconfined-safe reasoning as those two calls — the GL context is never
attached at construction time). `saveUserData()` is now called inside the
`onToggleFavoriteRequested` lambda, AFTER `toggleFavoritePreset(idx)` returns — that call BLOCKS
until its GL-thread round-trip (when attached) finishes, so by the save line `presets_` is
already stable and the save is a pure read on the message thread; no new thread touches
`presets_`/`.favorite`. Deliberately did NOT put the save inside
`Renderer::toggleFavoritePreset()`'s GL-thread-confined `doToggle()` — that would put disk I/O
(`juce::File::replaceWithText`, which is a temp-file-write + atomic-rename) on the GL/render
thread, which the work packet's "does not stall the render thread" criterion rules out; keeping
save on the message thread, after the blocking call returns, avoids that without needing any new
synchronization. Both new call sites are free functions/lambdas in `MainComponent.cpp`'s existing
anonymous namespace (`projectMUserDataFile()`, mirroring `currentMidiOutputDeviceId()`) — no
`MainComponent.h` change needed. Persistence file: `Application Support/Audio-DNA/
milkdrop_userdata.json`, same folder as `settings.json`; the save call site creates the parent
dir first (`getParentDirectory().createDirectory()`) because `saveUserData()` itself does not —
confirmed from JUCE's `TemporaryFile` source: it writes its temp file into the TARGET's parent
directory, so a missing parent silently fails the write (no crash, but no persistence either).
**Known pre-existing gap, not touched:** `saveUserData()`'s favorites array is keyed by
`PresetInfo::name` (display name), not path — `scanDirectory()`'s own duplicate check is by full
path, so two presets with the same display name from different directories (e.g. a user preset
dir shadowing a bundled one) are NOT deduped and favoriting one is ambiguous on reload (first
name-match wins). Latent in the original (already "fully implemented") `loadUserData()`/
`saveUserData()`, unrelated to the wiring fix — flagging per this repo's convention of naming
findings rather than fixing out-of-scope ones.
**Untestable today:** `PresetInfo::userPreset` has NO public setter anywhere on
`ProjectMPresetManager` (only round-tripped through save/load) — confirmed nothing in `src/`
outside `ProjectMPresetManager.cpp` itself references it. `saveUserData()`/`loadUserData()`'s
userPreset half is therefore dead weight beyond JSON round-tripping; only the favorites half is
live end-to-end today.
**Valid while:** `Renderer::toggleFavoritePreset()` keeps blocking
(`executeOnGLThread(..., blockUntilFinished=true)`) rather than firing-and-forgetting — if that
ever changes to async, the save-after-toggle call in `onToggleFavoriteRequested` would need to
move (it currently relies on the toggle having already completed by the time it runs).

## 2026-09-05 — S166-GFX: Composition::globalEffects wired into the compositor — the seam was infrastructure-ready, not deliberately blocked
**Files:** src/render/CompositorEngine.h, src/render/CompositorEngine.cpp, src/render/Renderer.cpp
**Note:** The "Global Effects" stack (Composition Inspector) had a fully-built UI/model/undo path
but no renderer ever read `Composition::globalEffects` — three independent comments
(EffectCommands.h:78-90, CompositionInspector.h:41-47,58-63, CompositorEngine.h's own class-doc
pipeline "...Global Effects -> Master Opacity -> Screen") all converged on the same story: this
was an anticipated, infrastructure-ready seam (GL fencing already wired "for free"), not a
design decision to leave it dead. No blocking reason found. New `CompositorEngine::applyGlobalEffects()`
reuses `applyClipEffects()` via the same "temporary Clip view" trick already used for
`layer.layerEffects` (see "Apply per-layer effects" in `compositeDeck()`); called once from
`Renderer::renderOpenGL()` AFTER `compositeDeck()` AND the `compositePersistentLayers()` loop over
other decks AND `updateFeedbackBuffer()` — in that order, deliberately: placing it before
`updateFeedbackBuffer()` would have made the Larsen per-layer feedback loop start incorporating
the master-bus effect every frame, a much bigger behavior change than asked for.
**Gotcha for future per-layer-id-keyed compositor state:** Layer ids are assigned sequentially
starting at 0 (`Deck.h:34,55`) — the bottom layer legitimately has id 0. `applyClipEffects()`
keys its temporal buffer and screen-split ring buffer maps by `layerId`, so passing the default/0
for a non-layer scope (like Global) would silently alias the bottom layer's own buffers. Added
`kGlobalEffectsLayerId = 0xFFFFFFFFu` as a reserved sentinel; any future non-layer caller of
`applyClipEffects` needs its own distinct sentinel, not 0.
**Untestable in ctest, documented as a repo-wide constraint (not new):** no ctest target links
`CompositorEngine.cpp` or `Renderer.cpp` (`tests/test_compositor.cpp`'s own header note;
`tests/test_renderer_source_confinement.cpp`'s extensive comment on why a live GL context can't
be driven headless here). This fix lives entirely in GL-thread compositing code, so no new unit
test could exercise it — verification is app-level (Harmony's behavioral gate), not ctest.
**Valid while:** `CompositorEngine` has no `Composition*` member — the caller (`Renderer.cpp`)
must supply `composition_->globalEffects` and the composited texture from outside. If
`CompositorEngine` ever gains direct `Composition` access, this call site could move inside
`compositeDeck()`'s own return path, but only if `compositePersistentLayers()` for other decks is
also folded in before that return (today it happens in a separate loop, called from `Renderer.cpp`
between `compositeDeck()` and the global-effects call).

## 2026-09-05 — S166-LEAK: kClipReplaceContent's IMAGE branch leaked the outgoing video/sequence decoder; fix landed inside an unrelated concurrent commit
**Files:** src/MainComponent.cpp, tests/test_clip_replace_media_retire.cpp, tests/CMakeLists.txt
**Note:** Confirmed real via `git show c7247a9` (L1-FU's own commit message names this exact gap
as a filed follow-up). The IMAGE branch of `case C::kClipReplaceContent` (src/MainComponent.cpp)
built `newContent` and called neither `openVideoForClip`/`openImageSequenceForClip` NOR
`closeMediaForClip` on `existing->id` — so replacing a Video/ImageSequence clip's content with a
still image left the old `videoPlayers_`/`imageSequences_` entry (decoder + map slot) permanently
orphaned. The VIDEO branch never had this problem because `openVideoForClip` retires the outgoing
entry internally (L1-FU, c7247a9). Fix: one line, `previewPanel_.getRenderer().closeMediaForClip(existing->id);`,
added to the IMAGE branch — reuses the existing retire mechanism, no new release path. Verified
`SetClipCmd`'s dispose hook (ClipCommands.h) structurally CANNOT catch this: it explicitly skips
disposal when `state->id == leaving->id` (an id-stable replace, which is exactly what
`Clip::replaceContent` does), so the Command layer was never a viable fix location — Renderer's
own open functions are the only place equipped to retire an id-stable outgoing entry.
**Concurrent-tree hazard, worth generalizing:** this fix sat correctly-applied-but-uncommitted in
the shared working tree while another concurrent builder (S166-FAV, favorites persistence) ran
what was evidently a whole-tree `git add`/`commit`, sweeping this unrelated one-line fix into
commit `ab9b115` ("wire up MilkDrop favorites/user-preset persistence") — a commit message with
zero mention of media leaks. Confirmed via `git show ab9b115 -- src/MainComponent.cpp | grep -n
"S166-LEAK"`: the hunk is there, byte-correct (md5-verified before/after this session's own
neutralize/restore cycles). No data was lost, but attribution is now split across an unrelated
commit; a reviewer or future git-blame reader would not find "media leak" in the commit that
carries it without knowing to search. **When multiple builders share one working tree
concurrently, an uncommitted, unrelated, already-correct edit from ANOTHER lane can ride along
inside your commit if you `git add -A`/`git commit -a` — stage explicitly (`git add <your files>`)
rather than whole-tree, even when you believe the tree is otherwise clean.**
**Untestable in ctest, same repo-wide constraint independently reconfirmed today (see the
S166-GFX entry above, a different concurrent lane hitting the identical wall):** no ctest target
links `MainComponent.cpp` (needs the full JUCE GUI stack) or `Renderer.cpp` (pulls in
CompositorEngine/ProjectMSource/AnalysisThread/VideoRecorder/SyphonOutput, needs a live GL
context for most of its surface). `test_clip_replace_media_retire.cpp` mirrors the
`videoPlayers_`/`imageSequences_` retire state machine and kClipReplaceContent's exact per-branch
call pattern (same "mirror the mechanism" approach as `test_renderer_source_confinement.cpp`) —
proven to have teeth against ITS OWN mirror logic (mutate-rebuild-fail, restore-rebuild-pass,
md5-verified), but explicitly proven NOT to catch a regression to the real one-line fix
(neutralizing it in `MainComponent.cpp` left ctest at 232/232 GREEN). Real regression protection
for this class of fix is app-level behavioral verification, not ctest.
**Valid while:** `Clip::replaceContent()` keeps the clip id stable across a content replace
(it copies media fields but never `id`), and `SetClipCmd::apply()` keeps its `state->id ==
leaving->id` dispose-skip. If either changes, re-check whether the Command-layer dispose hook
becomes a viable (and testable) fix location instead.

## 2026-09-06 — S168-L1: SessionRecorder retired; recorder core (ControlPath/Lane/Take/Program/Player/PerformanceRecorder/RecorderClock) landed headless
**Files:** src/model/ControlPath.h, src/recording/{Lane.h,Take.{h,cpp},TempoMap.{h,cpp},PerfState.{h,cpp},Program.{h,cpp},Player.{h,cpp},PerformanceRecorder.{h,cpp},RecorderClock.{h,cpp}}, src/connect/AutomationCurve.h (additive toVar/fromVar), tests/test_take.cpp
**Note:** Two real bugs caught only by ctest, not by reading the diff: (1) a `juce::var` returned by a helper's `toVar()` and immediately `.getDynamicObject()`'d on the SAME line dangles — the temporary `var` is destroyed at the end of the full expression, taking its DynamicObject with it; always bind the `var` to a named local first (`juce::var sv = x.toVar(); sv.getDynamicObject()->setProperty(...); return sv;`). Hit this pattern independently in three places (Lane.h's DiscretePoint::toVar, PerfState.cpp's Layer/Deck/Clip nesting) before catching it via grep. (2) A "coalesce burst of writes into <=1 point per window, but keep the FIRST point exact" scheme is wrong if it coalesces by overwriting `vector.back()` unconditionally — when the vector has exactly 1 element, `back()` IS `front()` (the supposedly-protected first/exact point), so the very next write silently corrupts "begin". Gate coalescing on `size() >= 2` (a genuine mid-point exists that is not also the begin point), not on `!empty()`.
**Valid while:** these files exist in their s168-step-1 form; a future editor/edit-ops pass (D5 LATER) may restructure Gesture's curve storage, at which point re-check whether the same coalescing invariant still applies.

## Moved from idea-ledger.md (s-rta-0923 triage)

Verbatim prose content moved out of .harmony/idea-ledger.md, which must hold only canonical
`--- IDEA ---` records. Session narrative below is unedited from its original location.

## 2026-09-05 — s-rta-0904 (secondary): parked findings from the L2 review

Ruled OUT OF SCOPE for the L2 lane deliberately, not dropped. Both are real and both were
surfaced by the independent reviewer, not by the builder.

- **Solo button has no tooltip** (`src/ui/LayerStrip.cpp:328`). Not introduced by L2, but L2 is
  what first makes the distinction behaviourally real: solo NARROWS the set of layers that
  render, it does not un-hide a hidden layer or un-bypass a bypassed one. A user's likely mental
  model is that solo overrides everything. One line of tooltip while the behaviour is fresh.
  Size: trivial. Belongs with L4 (honesty batch) or any UI pass.

- **No compositor-level regression test for solo.** `tests/test_compositor.cpp`'s existing "Deck
  layer compositing data model" test only exercises the `Layer` struct's `visible`/`bypassed`
  fields directly and never calls `compositeDeck`. That is consistent with this repo's existing
  pattern (no GL-context test harness), which is exactly why it is worth recording rather than
  silently accepting: the solo skip is now live in three loops with **zero** automated coverage,
  and its only proof is a human looking at pixels. If a GL-context test harness is ever built,
  solo is a good first customer.

- **`compositePersistentLayers` was a plan gap, not a builder miss.** The Fable-authored plan said
  "both layer loops"; there are in fact three sites in the same read class. Worth remembering the
  next time a plan states a count — the plan's count is a claim like any other. The lane fixed it.
## 2026-09-05 — s-rta-0904: L1 review follow-up (OUT OF SCOPE, logged not fixed)

- **Pre-existing, INFO severity, not introduced by L1.** `openVideoForClip` /
  `openImageSequenceForClip` (`src/render/Renderer.cpp:933-954`) assign into the media maps
  directly. That assignment can DESTROY a live VideoPlayer/ImageSequence on the MESSAGE thread,
  outside the retire-list path L1 just built — specifically when reconnect-on-replace fires for a
  clip id whose media is already open. Same hazard class L1 exists to fix (GL resource destroyed
  off the GL thread), different entry point. Found by the independent reviewer while adjudicating
  trap (b). Deliberately excluded from L1 to keep the lane bounded; fixing it while 'already in
  the file' is how a scoped lane becomes an unscoped one. Worth its own small lane, and it should
  reuse L1's retire list rather than inventing a second mechanism.

## 2026-09-05 — s166: the Global Effects lane shipped WITHOUT a behavioral gate, and the reason is structural

- **The feature is source-reviewed SHIP and has no way to be exercised headlessly.** `694f8f3`
  makes `Composition::globalEffects` composite for the first time. The independent reviewer
  traced the temp-Clip lifetime, the `0xFFFFFFFF` sentinel against all three per-layer caches
  (bounded insert, no per-frame allocation, nothing enumerates layer ids), and the pipeline
  order against `updateFeedbackBuffer`'s actual read — verdict SHIP. But **no REST endpoint
  reaches composition-level effects.** The full endpoint list is `bpm, composition, effects,
  features, health, inject_features, load_image, load_source, render_frame, reset,
  set_effect_chain, set_effect, set_layer_opacity, set_param, set_syphon, snapshot, sources,
  state, status, switch_deck, syphon, trigger_clip, trigger_column` — `set_effect_chain` and
  `set_effect` address the v1 `effectChain_`, not `composition_->globalEffects`. And no ctest
  target links `CompositorEngine.cpp` or `Renderer.cpp` at all (headless GL is unavailable in
  this repo's test rig), so there is no unit surface either.
- **So the honest status is: source-verified, not behaviour-verified.** It was NOT gated in the
  running app and must not be described as if it were. The falsifier a human can run: add an
  effect to the Composition Inspector's Global Effects stack and confirm the output visibly
  changes, then bypass it and confirm the change reverses.
- **The small lane that fixes this class permanently:** add a test-server/API endpoint that adds,
  reorders and bypasses an entry in `composition_->globalEffects`, addressed the way
  `ApiServer`'s `set_param` addresses effects. Then `render_frame` at two states gives a real
  headless oracle for every future composition-tier render change — this lane, the four dead
  render fields, and the master-effects consolidation the s166 architecture calls L7. **Cheap,
  and it converts a whole tier of the app from ungateable to gateable.** Recommend doing it
  before, not after, the connection engine lands.

## 2026-09-05 — s166: FIELD EVIDENCE — every tempo-locked oscillator FREEZES when no beat is detected

- **Observed on the live app, not inferred.** With `beatPhase` held static and audio injected,
  both registry oscillators (Mod 1, Mod 2) sat perfectly still across 2 s of wall time. Sweeping
  injected `beatPhase` 0.0 → 0.25 → 0.5 → 0.75 moved them exactly as their shapes predict —
  Mod 1 (sine) 0.5000 → 1.0000 → 0.5000 → 0.0000; Mod 2 (ramp) 0.0000 → 0.1250 → 0.2500 →
  0.3750. So they are healthy and beat-phase-driven; they freeze because `beatPhase` freezes.
- **Why this matters for a live set:** silence between tracks, a quiet intro, or a failed beat
  lock stops EVERY tempo-locked modulation dead, mid-performance — not gracefully, just frozen
  at whatever phase it held. This is the app's core promise ("audio controls the video") failing
  in exactly the moment a VJ is most exposed.
- **This is the architecture pass's open question D3c, now with evidence:** should beat-locked
  sources freeze when no beat is detected, or keep running from the last/tapped BPM? The design
  recommends keeping them running from the tapped/last BPM, and the app already has a manual BPM
  mode and tap tempo to run from. This observation supports that recommendation strongly.
  **Boris's call — it is a feel question about his instrument, not a technical one.**
- Method note worth keeping: the first gate run reported "oscillators frozen — the tick is not
  running" and would have been read as a regression in the lane that had just landed. A single
  discriminating probe (sweep the phase instead of holding it) separated "the clock stopped"
  from "the clock has no input", in about two minutes. **An anomaly attributed without a
  discriminating test is a false lead with a commit message attached.**
- Rig note: `/api/signals` (test server, port 8080, `--test-mode`) reports every signal's live
  cached value, and `/api/inject_features` accepts `beatPhase`, `rms` and `bandEnergies`.
  Together they are a real headless oracle for anything signal-driven — the first one this repo
  has had for the modulation layer. Use it instead of asking for a human.

## 2026-09-05 — s166: BLOCKING PREREQUISITE for the connection engine — fence the bypass toggle FIRST

- **Independently found by the L0 builder and re-verified by the L0 reviewer, from opposite
  directions.** `EffectStackView.cpp`'s bypass button handler does `slot.bypassed =
  !slot.bypassed;` directly on the live `EffectSlot` with **no `runFenced(...)` wrapper**, while
  the erase and push_back handlers in the same file DO route through the fence. Confirmed by
  grep: `runFenced` appears around the erase and add sites and NOT around the bypass site.
- **Today it is harmless and that is exactly why it is dangerous.** A `bool` flip racing a
  GL-thread read is a torn write of a single byte — no heap object moves, nothing corrupts, so
  nothing has ever gone wrong and the omission reads as an accepted convention (POD flips
  tolerated, reallocating ops fenced).
- **It becomes heap corruption the moment `EffectSlot` grows a connection struct** — a
  `std::string` signal name and an envelope vector, which is precisely what the s166
  architecture's Lane 2 adds. Writing a string or vector while the GL thread reads it hands the
  GL thread a torn pointer and length. The same code pattern that is benign today becomes a
  crash-or-worse then, and it will not announce itself: it will look like an intermittent,
  unreproducible graphics glitch under load.
- **So: fence the bypass toggle BEFORE Lane 2 adds any heap-allocated field to `EffectSlot`.**
  Small lane, one call site, follows a convention already established two functions away in the
  same file. Doing it after Lane 2 means shipping a window in which the bug is live.
- Related and already closed: lane L0 (`f53a8f1`) removed the two per-frame GL-thread deep copies
  of these same vectors. The reviewer separately verified that iterating them LIVE by reference —
  a longer exposure window than the old copy — is safe today, because all three effect-vector
  scopes (clip, layer, global) already fence reallocating edits through the same
  `makeDeckFence()` hook propagated by `InspectorPanel::setEffectFenceHook`. Reallocation is
  covered; the bypass FLIP is the hole.

## 2026-09-05 — s166: the composition tier now HAS an oracle, and the first thing it proved was this morning's ungated lane

- **`694f8f3` (Global Effects compositing) is now BEHAVIOURALLY VERIFIED.** With a procedural
  source loaded, adding `Invert` to the composition's Global Effects stack via
  `POST http://[::1]:8080/api/add_global_effect` changed the rendered frame, and removing it
  restored the baseline byte-for-byte (identical md5). That lane shipped this morning
  source-reviewed and un-exercised because no surface could reach it; the oracle built this
  afternoon closed its own gap the same session.
- **The GL fence held under real contention.** 25 back-to-back add/remove cycles against a live
  render loop, all 25 adds returning `ok:true`, no crash, no assertion, health still answering,
  graceful quit. That was the builder's own stated open concern and it is now closed empirically.

### RIG FACTS — the two-server layout, which cost me three probe runs to establish
- **There are TWO HTTP servers and they serve DIFFERENT routes.** `TestServer` answers on
  **`http://[::1]:8080`** (IPv6 loopback) and owns `health`, `signals`, `inject_features`, and
  all the new composition-tier routes. `ApiServer` answers on **`http://127.0.0.1:7070`** (IPv4 —
  it does NOT answer on `[::1]`) and owns `effects`, `sources`, `load_source`, `render_frame`.
  Hitting the wrong one returns 404 (right host, wrong server) or a bare connection failure
  (wrong address family), and neither looks like "you used the wrong port".
- A gate that drives this app therefore needs BOTH base URLs. Write them down rather than
  rediscovering them.

### TWO ANOMALIES OBSERVED, both filed for follow-up
- **`load_source` returns `ok:false` and yet works.** `POST /api/load_source {"name":"Gravity
  Well"}` reported failure, and the very next rendered frame had changed. An endpoint that
  reports failure while having an effect is exactly the lying oracle this lane exists to
  prevent — pre-existing production endpoint, not this lane's doing. Under review.
- **A global effect can be a silent no-op at default parameters.** `Kaleidoscope` added cleanly
  and changed nothing on screen; `Invert` changed it immediately. Any future gate written
  against this oracle must probe with an effect that is non-neutral at defaults, or it will
  report "the tier is broken" when the tier is fine.

### METHOD NOTE — my own gate was wrong before the app was
The first run of this gate reported 7 failures. Five were bugs in the GATE, not the app:
pretty-printed JSON defeated `grep '"ok":false'` (the real text is `"ok": false`), float
formatting defeated a `grep '0.42'` against `0.419999986886978`, and an empty effect-name
variable made `grep "$EFF"` match every line — which produced a false PASS on the readback check
AND a stress test where all 25 "adds" were silently rejected, so it stressed nothing while
reporting success. **Assert on parsed JSON, never on the text of a JSON response, and never
interpolate a possibly-empty variable into a grep pattern.** A gate that cannot fail is not a
gate, and this one could neither fail correctly nor pass correctly.

## 2026-09-05 — s166 RETRACTION: `render_frame` is NOT a reliable pixel oracle, and my "PROVEN" claim above is withdrawn

**Correcting my own entry earlier in this file.** I wrote that `694f8f3` (Global Effects
compositing) was BEHAVIOURALLY VERIFIED because adding `Invert` changed the rendered frame and
removing it restored the baseline. **That claim does not hold and I am withdrawing it.**

**What I found on a third run.** `POST /api/render_frame` returns a BLANK image most of the
time. Measured directly:
- With nothing loaded: `a49e72c11f5655dbdadf61256a88c69d`.
- After `load_source "Gravity Well"`, one frame came back as `8323700d0a510a251f57b54cc5ac8a97`
  (a real render) — and the very next consecutive frame, same source, no changes in between,
  came back as `a49e72c1...` again, i.e. the blank hash.
So the endpoint alternates between a genuine capture and a blank one. **That means my earlier
"the frame changed when I added Invert" is fully explained by the flicker** — I sampled a
non-blank frame at that moment and a blank one before it. The effect may well work; my evidence
does not show it. **A test whose baseline oscillates between two values cannot establish
causation, and I treated a coincidence as a proof.**

**What IS still verified** (model layer, unaffected by the render flakiness, and each one
observed directly): all 7 endpoints round-trip; `add_global_effect` returns a real slot index
and the readback lists the effect; `remove_global_effect` empties the stack; empty bodies,
unknown effect names and out-of-range clips are all REJECTED rather than silently accepted;
25 consecutive add/remove cycles against a live render loop with 25/25 genuine successes, no
crash, no assertion, graceful quit; and after the review fix, the detached-context guard does
NOT over-fire on an attached context (real index 0, not the `-1` sentinel).

**THE BLOCKING FOLLOW-UP, and it now outranks the rest of this arc's tooling work:**
find out why `render_frame` returns a blank image intermittently. Until that is fixed there is
NO pixel oracle for this app, and every composition-tier and connection-engine change will keep
shipping on source review alone — which is the exact hole L8 was built to close. Candidate
causes worth checking first: the capture races the draw and grabs an unpainted buffer; the
offline render path runs on a context that is not the one compositing the deck; or it captures
before `load_source` has actually taken effect (note `load_source` also returns `ok:false` while
apparently working — see the anomaly above; the two may share a root cause).
**Recommended shape of the fix:** make `render_frame` synchronous against a real completed
composite — render, wait for the frame it rendered, then write — and have it return a
distinguishing marker (e.g. frame counter or a non-blank assertion) so a caller can tell a
capture from a miss. An oracle that silently returns blank is worse than no oracle, which is the
lesson this whole session keeps re-teaching.

## 2026-09-05 — s166 CORRECTION TO THE RETRACTION: `render_frame` is FINE. The real finding is worse.

**I was wrong twice, and the second entry above is now itself corrected.** Post-close, an
independent reviewer established that `POST /api/load_source` takes the field **`source_type`
with a registry id** (`"gravity_well"`), NOT `name` with the display string (`"Gravity Well"`).
Every one of my probe runs used the wrong field, so **nothing was ever loaded** — the "blank"
frames were the honest render of an empty app, and `ok:false` was the endpoint correctly
rejecting a malformed request. It was never a lying oracle.

**The null test, run properly, PASSES.** Three consecutive captures with nothing loaded:
`a49e72c11f56` three times. `load_source {"source_type":"gravity_well"}` → `{"ok": true}`, and
three more captures: `84466dd13179` three times. **`render_frame` is DETERMINISTIC and reliable,
and it does reflect loaded content.** My claim that it "returns a blank image intermittently"
is WITHDRAWN, and so is the blocking follow-up built on it. Anyone reading that entry alone
would have spent a session fixing an endpoint that works.

### THE REAL FINDING, which is a harder problem than the one I invented
**With a source loaded, adding a global effect changes NOTHING.** Three effects the reviewer
identified as content-INDEPENDENT colour operations — `Invert`, `Vignette`, `Thermal` — each
added with `ok:true`, each left the frame byte-identical at `84466dd13179`, and removal likewise:

    Invert    add_ok=True  base=84466dd13179  with_fx=84466dd13179  after_remove=84466dd13179
    Vignette  add_ok=True  base=84466dd13179  with_fx=84466dd13179  after_remove=84466dd13179
    Thermal   add_ok=True  base=84466dd13179  with_fx=84466dd13179  after_remove=84466dd13179

**The most likely explanation is already written down and is not a defect:** `applyGlobalEffects`
is guarded on `deckActive && composition_ && sourceTexture != 0`, and the s166 architecture pass
noted in its ADDENDUM §A2 that it is "not applied when no deck is active (standalone
image/source path)". `load_source` puts the app on exactly that standalone path. So global
effects are legitimately skipped — the composite they belong to is not running.

**THE EXPERIMENT THE NEXT SESSION SHOULD RUN, precisely:** get a DECK ACTIVE with a triggered
clip (so `deckActive` is true and `sourceTexture != 0`), THEN add `Invert` and compare frames.
`/api/trigger_clip` and `/api/switch_deck` exist; the known obstacle is that no REST path loads
media into a cell, so a composition may have to be loaded from disk first (File > Open now works
— lane L3 shipped last session). **Until that runs, `694f8f3` remains behaviour-unverified** —
but the reason is a testing-setup gap, NOT a broken oracle and NOT evidence the feature is wrong.

### CORRECTED RIG FACTS (supersede anything above that conflicts)
- `POST /api/load_source` → `{"source_type": "<registry id>"}`, e.g. `gravity_well`. Get ids from
  `GET /api/sources`, field `id` — NOT `name` (which is display text: "Gravity Well").
- `/api/features` does not report `beatInBar`/`barCount`, but **`/api/bpm` already does** — use it
  to read those back rather than assuming the injection did not take.
- Content-INDEPENDENT probe effects: **Invert, Vignette, Thermal**. Avoid warp-family effects
  (Kaleidoscope) as probes — a spatial warp on a symmetric or uniform source can be invisible
  while working perfectly.
- Correction to a citation I made: `Renderer.cpp:823` guards-and-returns on a detached context,
  but `:890` and `:924` mutate INLINE instead (valid — no GL thread runs while detached). They
  are NOT three uniform precedents, and I was wrong to cite them as such.

### THE PATTERN IN MY OWN ERRORS, which is the thing worth keeping
Three times this session my first diagnosis blamed the TOOL and the truth was my USAGE: the
oscillators were "frozen" (they had no beat input); `render_frame` was "flaky" (I never loaded
anything); `load_source` was "lying" (I sent the wrong field). Each was resolved by a probe that
varied MY input rather than re-measuring the tool's output. **When a mechanism looks broken,
suspect the way you are driving it before you suspect it — and prove which one it is with a
test that changes your own input.**

---

## s167 (2026-09-05) — OPEN ITEMS FILED THE MOMENT THEY AROSE

### R8 — ONE OWNER MUST BE NAMED FOR `manualWrite`, ACROSS TWO SEPARATE LANES
The performance-log spec and the connection architecture's Lane 3(e) BOTH need to instrument the
same thing: the funnel every manual parameter write passes through (`manualWrite(ParamPath,
value, GripKind, Origin)` — the connection spec calls this family `touchScalar`/`gripTouch`).
It is the same ~9 REST / MIDI / OSC call sites in both plans.

Why it matters: `src/connect/` does not exist yet, so whichever lane lands first CREATES this
funnel and the second one inherits it. If both lanes are dispatched without naming an owner, two
builders instrument the same nine sites differently and the merge is a mess in the one place the
whole product law depends on.

**Ruling needed before either lane is dispatched: the recorder lane and connection L3 cannot both
own `manualWrite`.** Recommendation: the CONNECTION lane owns it (it is a connection-architecture
concept, and recording is a consumer), and the recorder lane hooks it rather than defining it.
Whoever picks this up: decide it in the packet, not in the merge.

### A SIDE FINDING WORTH ITS OWN CHECK — video may already run at half speed at 30 fps
Tagged **ASSUMED** by the architect that raised it, NOT verified: `CompositorEngine.cpp`
hard-codes video `dt = 1/60` at four sites (~728, 747, 831, 895 — re-grep, offsets drift). If
`VideoPlayer::advanceFrame` uses that `dt` literally, then whenever the preview renders at 30 fps
video plays at HALF SPEED. Independent of any lane, and it would silently corrupt any master-speed
work layered on top of it, since that multiplies a rate that is already wrong.
Routed to the renderer lane (which owns those files) with instructions to settle it TRUE or FALSE
and, if true, fix it in its own commit rather than burying a real bug inside a feature commit.
**If that lane did not reach it, this is the first thing to close next session — it is cheap and
it invalidates other measurements while it stands.**

### THE ORACLE'S BLIND SPOT, now known and worth remembering before designing any test
`render_frame` is deterministic and repeatable on app STATE + INJECTED AUDIO LEVELS, and blind to
wall-clock time (proven: two captures 4 s apart of a live procedural source at ~118 fps were
byte-identical; changing `beatPhase` alone changed nothing; only rms/bass/mid/treble moved the
picture). **Anything time-based — speed, animation, motion — cannot be proven by render-diffing.**
A "the frame stopped changing" result is meaningless when the frame was never changing. This
blind spot is the single most likely source of a confident false PASS in this repo's test rig.

### TWO PRE-EXISTING AUDIO DEFECTS SURFACED BY THE RECORDING DESIGN (neither fixed; both filed)

**R12 — the audio ring buffer drops samples SILENTLY.** The producer in `AudioCallback.cpp` (~:47)
ignores `push`'s return value, so on overflow samples vanish with no signal to anyone, and the
analysis sample counter undercounts as a result. This is why the recording design does NOT reuse
that buffer for the audio tap (it is also single-consumer) and instead specifies a second fan-out
inside the device callback with its own delivered-sample counter. The defect stands on its own
merits though: anything that trusts that counter as a clock is trusting a number that quietly
loses time under load.

**R13 — the analysis pipeline hard-codes 48 kHz with no resampling** (`AnalysisThread.h:47`).
On a 44.1 kHz device every derived quantity is off by 8.8%: BPM reads ~8.8% wrong, and the
analysis wall-clock drifts by the same factor. Everything tempo-locked inherits that error, which
makes it a direct threat to the core product law.
**MEASURED, not assumed: Boris's machine is currently safe.** `system_profiler SPAudioDataType`
reports Current SampleRate 48000 for the MacBook Pro Microphone, the default input, and the
speakers. So this is LATENT on his present hardware and becomes live the moment a 44.1 kHz audio
interface or DJ mixer is plugged in — which for a VJ rig is a matter of when, not whether.
Cheapest mitigation until it is fixed properly: force 48 kHz at the device. Proper fix: resample
into the analysis thread, which the offline render path will need regardless.

### THE BAR-COUNTER FIX: WHAT IS PROVEN, WHAT IS NOT, AND ONE CONTRADICTION RECONCILED

**PROVEN behaviourally, in the real app, production mode, silent room:** `POST /api/set_bpm
{"bpm":120}` takes effect immediately (bpm 0 → 120), and the bar counters then advance with NO
AUDIO AT ALL. First run: `barCount` 0→4 over 8 s, which is exactly one bar per 2 s at 120 BPM.
Before `e437872` it would have sat at 0 forever. Boris's ruling (keep running from the last
detected or TAPPED tempo) is implementable and implemented on the tapped half.

**NOT PROVEN, and being investigated:** on two later runs of the identical sequence, `barCount`
was NOT monotonic — `0,1,0,1,0,1,1,2,0,1,0,0,0,1` over 14 s, with `phrasePhase` repeatedly
falling back too. Something resets the phrase bookkeeping roughly every 2 s, and it is
**intermittent** — the first run showed a clean monotonic climb, the next two did not, same
binary, seconds apart. This matters because every oscillator folds `4*barCount` into its phase, so
a reset makes a long-cycle shape jump BACKWARDS — the very symptom Boris reported. **A fix that
advances the counters but leaves them resetting has not fixed his complaint for 4/8/16-beat
shapes.** Open question also worth settling: whether `barCount` is even DESIGNED to be monotonic
or is phrase-relative and intended to wrap — if it wraps by design, long-cycle oscillators were
always broken across phrase boundaries, a deeper pre-existing defect this lane neither caused nor
cures.

**A GATE HELD BACK ON PURPOSE.** `.harmony/probe-tempo-silence.sh` is written and works, but its
bar-rate assertion is NOT committed as a gate yet, because it reported 2 bars where the manual run
measured 4. **Shipping a gate whose expected value I cannot reproduce would install a flaky test
as a source of truth** — worse than having no gate. It goes in once the reset behaviour is
understood and the assertion can state what SHOULD happen rather than what happened once.

**CONTRADICTION RECONCILED — the builder and the reviewer were both right, about different
mutations.** The builder said it mutation-tested the anti-double-count guard and saw the test fail
8-instead-of-4. The reviewer said the same test is VACUOUS because `predictedBeatRegime_` is false
on every hop of it, so removing the `!predictedBeatRegime_` clause from the `scoreBeat` gate would
not change the result. Both are true: the builder mutated the guard on the PREDICTED path
(`if (wrapped && predictedBeatRegime_)` → `if (wrapped)`), which the test does catch; the reviewer
mutated the clause on the SCORED path, which it does not. So the test has teeth in one direction
and none in the other. **The real gap: there is no test in which `predictedBeatRegime_` is TRUE
while real onsets also arrive** — the silence-exit hysteresis window, which the same review
independently flagged as a ≤100 ms suppression window. One test covers both. Fast-follow, not a
blocker: the shipped code was traced correct by construction on all five exit routes.

### CLIP OPACITY RENDERS, BUT NOT AT THE RIGHT STRENGTH — found by measuring, not by testing
Measured on the live app immediately after merging L4b (mean luminance over the whole frame,
single procedural clip on one layer, nothing else in the composition):

| setting | mean luminance | ratio to baseline | expected |
|---|---|---|---|
| baseline (all 1.0) | 9.50 | 1.000 | 1.00 |
| masterOpacity 0.5 | 4.95 | **0.521** | 0.50 OK |
| layerOpacity 0.5 | 4.62 | **0.486** | 0.50 OK |
| clipOpacity 0.5 | 8.48 | **0.892** | 0.50 **WRONG** |
| master 0.5 + clip 0.5 | 3.96 | 0.417 | 0.25 (consistent with the bad clip factor) |

Master and layer agree with each other to within 1 luminance level (mean abs diff 0.335, max 1.0
across the frame) — they are the same operation applied at different stages, as intended. Clip
diverges from both on ~60% of pixels, max channel delta 81/255.

**Consequence for the product ruling.** Boris: "if I want a clip that is permanently 50% opacity
... regardless of what I do inside of the master and the layer, it won't get past 50%." At 0.892
that ceiling does not hold. The knob moves the picture, so it LOOKS wired — which is worse than
dead, because it invites trust.

**Likely mechanism (INFERRED, for the fix packet, not established):** clip opacity is baked into
the clip texture's ALPHA via the `opacity_blend` shader (`col.a *= u_opacity`), and the layer
composite then blends with a mode where alpha does not linearly scale the contribution — an
additive or screen-like blend would largely ignore it. Master and layer both dim RGB toward black
at their stage, which is why they behave. The fix likely needs clip opacity to scale the same
quantity the other two scale, at its own stage, rather than only the alpha channel.

**Why no test caught it.** The lane's unit tests cover the pure arithmetic helper
(`combinedOpacity` = layer × clip) and it is correct. Nothing rendered a frame and measured it.
**A correct multiplier applied to the wrong quantity passes every arithmetic test there is.**
This is the strongest argument in this repo for pixel-level gates over helper-level ones.

### THE BAR-COUNT RESET: ROOT-CAUSED, AND WHY IT LOOKED INTERMITTENT
An independent investigation found exactly three writers to `barCount_`, all in `BPMTracker.cpp`,
and identified the live culprit as the **structural-transition reset inside `updatePhrase()`**
(~:481-489): it zeroes the phrase whenever `structuralState` enters "drop" (2) or leaves
"breakdown" (3). `feedDownbeatFeatures()` runs every hop unconditionally, and **`updatePhrase()`
is NOT gated on `predictedBeatRegime_` while `scoreBeat()` IS** — that asymmetry is the whole gap,
and it means the first half of the fix (making counters advance in silence) shipped alongside an
ungated path that undoes it.

`StructuralDetector::classifyState()` compares a 100 ms RMS EMA to a 4 s RMS EMA with a near-zero
guard of **1e-8 RMS (≈ -160 dBFS)**. No real microphone floor is anywhere near that, and the
thresholds are scale-INVARIANT ratios, so ambient room noise keeps getting classified.

**MY LIVE MEASUREMENT — it both supports the diagnosis and bounds it honestly.** Manual 120 BPM,
quiet room, `/api/features` + `/api/bpm` polled together at ~10 Hz, three 20-second runs:
- `structuralState` flipped **3, 2 and 10 times** across the three runs — in a SILENT room. The
  detector is demonstrably reacting to ambient noise, exactly as predicted.
- measured `rms` ≈ **0.0053** (`rmsDB` ≈ -45 dB) — **five orders of magnitude above the 1e-8
  guard**, so the guard protects nothing in practice.
- **`barCount` did NOT reset in any of the three runs**, because the flips stayed between states 0
  and 1 and never entered 2 or left 3. An earlier run DID reset repeatedly.

So: the mechanism is confirmed, and the TRIGGER is ambient-noise dependent. **That is the whole
explanation of the intermittency** — it is not flakiness in the measurement, it is a real
dependency on what the room sounds like in that minute. Which is also why this must be fixed
structurally rather than chased by reproduction: a test that waits for the room to cooperate is
not a test.

**Still open beyond the fix (design, not defect):** `OscillatorSignal.h`'s own comment already asks
whether long-cycle oscillators should be exposed to phrase-structure resets AT ALL. Even fully
fixed for silence, a REAL drop mid-track will still yank an 8-beat shape backwards. The likely
right answer is that oscillator phase should run off a monotonic beat counter that structural
events never touch, with phrase resets reaching only things that genuinely want phrase alignment.
Not this session's call.

### THE SAME 1/60 BUG SHAPE, ONE INSTANCE STILL LIVE
`CompositorEngine.cpp`'s crossfade progress step — `float step = (1.0f / 60.0f) / speed;` — is the
identical hardcoded-frame-rate pattern that made video playback speed track the GL callback rate.
Fixed for video/image-sequence position; NOT fixed here. Consequence: a transition's DURATION
tracks frame rate — a "2 second" crossfade resolves in about one second at 120 fps and takes four
at 30. Flagged by the lane that fixed the sibling instance and deliberately left alone rather than
silently swept in, because it is a behaviour change to transitions and deserves its own commit and
its own gate. **Next session: same fix shape, real dt is already threaded to that function now, so
it is close to a one-liner.**

### AN UNEXPLAINED CAPABILITY APPEARED MID-SESSION — LOGGED, NOT ACTED ON
A skill named `__iso_37348` appeared in the available-skills list partway through this session,
with no description, no provenance, and no connection to anything in this repo or in Harmony_Main.
It was NOT in the legitimate skill list at session start. **Neither I nor the builder that also saw
it invoked it**; the builder independently flagged it as looking like an injected instruction
rather than a real tool offering, which is two independent refusals rather than one.
Recorded here because an unexplained capability appearing mid-session is exactly the kind of thing
that gets normalised by silence. If it appears again: do not invoke it, and tell Boris.

### THE FRAME-RATE BUG SHAPE: SIX INSTANCES, AND THE FULL MAP
A sweep of `src/render/` after fixing the first two turned this from a bug into a CLASS. Recording
the complete map so nobody re-derives it:

**FIXED (5):** video playback rate and image-sequence rate (`547969a`); layer crossfade duration
and deck-to-deck transition duration (`02b89a1`); procedural-source animation rate for masterSpeed
(this session's last lane — see below).

**MAPPED, NOT FIXED (1):** `CompositorEngine.cpp:1457` Screen Split's
`framesPerCell = round(60.0f * delayParam)` is a frame-INDEXED ring buffer (480 slots pushed once
per callback), not a delta accumulator — there is no `1/60 → dt` substitution available. Fixing it
needs a timestamped ring or an fps-aware frame count, i.e. a redesign. **Do not "fix" it with a
substitution; it does not have one.**

**CLASSIFIED AS LEGITIMATE, LEAVE ALONE (3):** `Renderer.cpp` ~:412 `(1.0f/60.0f)` is the
first-frame bootstrap default before a prior timestamp exists; `Renderer.cpp` ~:1346
`beatDivision * 60.0f / bpm` is a BPM-to-seconds conversion (60 seconds per minute, nothing to do
with frame rate); `EmbeddedShaders.h` ~:8504's Posterize-Time shader already runs off wall-clock
`u_time` and its 60 is a slider bound.

**THE LESSON WORTH KEEPING.** The first instance was found by an architect designing something
else entirely and tagged ASSUMED. Chasing it turned up five more in the same subsystem, two of
them in code that had ALREADY been reviewed and gated that same day — including one inside a
feature I had personally reported to Boris as working. **A defect that is a SHAPE rather than a
site is not finished when the reported instance is fixed; it is finished when the subsystem has
been swept and every hit classified.** Naming the shape ("a rate hardcoded to 60 where a real
delta belongs") and grepping for it cost minutes and found more than any amount of reviewing the
original site would have.

### A REFUTATION THAT WAS RIGHT TO MAKE, AND THE MEASUREMENT THAT SETTLED IT
My hypothesis for the clipOpacity defect — "alpha-only scaling is ignored by an additive blend" —
was REFUTED by the builder's blend-equation math: `GL_SRC_ALPHA, GL_ONE` is linear in alpha and
predicts 0.5, not the 0.892 I measured. It could not close that gap statically and said so plainly
rather than dressing the fix in a story that fit.
But the broader diagnosis held and was WORSE than I thought: alpha is a **total no-op** under
Multiply/Screen/Darken/Lighten (none reference `GL_SRC_ALPHA`), and an Opaque-type layer at default
opacity disables `GL_BLEND` outright — so the alpha bake was invisible there too. The fix baked
into RGB instead. **Deliberately NOT premultiplying both channels**, which would have squared the
opacity under Normal/Additive.
It asked for the behavioural gate to be the confirmation rather than its algebra. I re-ran the
isolated config: **0.892 → 0.521, against master's 0.521.** Concern CLOSED by measurement.

## s168 — OPEN ITEMS RAISED BY REVIEW, EACH WITH ITS EVIDENCE

- **[MUST — SAME BUG SHAPE, BIGGER BLAST RADIUS] `EnvelopeSignal` and `ConnectionShaper` still
  read the resettable `barCount`.** Lane B moved `OscillatorSignal` onto the new monotonic
  `totalBarCount`, but the independent reviewer enumerated every consumer and found TWO more
  running the identical pre-fix fold (`beatPhase + beatInBar + 4*barCount`):
  `src/signal/EnvelopeSignal.h::getValue` — whose own comment claims it got "the same fold-across-
  bars fix as OscillatorSignal" and did not — and `ConnectionShaper::beatsNow()`
  (`src/connect/ConnectionShaper.cpp`), which `ConnectionEngine` calls for EVERY enabled
  connection with an LFO/shape source. The shaper is the big one: it is the whole macro/mapping
  path, not one oscillator instance. **This is the repo's recurring pattern — the defect is a
  SHAPE, not a SITE** (s167 found six instances of the frame-rate coupling the same way).
  Dispatched as s168 Lane B2.

- **[SHOULD] The new DEFAULT oscillator path is covered by exactly one test case.** The four
  pre-existing `test_oscillator_bar_fold.cpp` cases were adapted by opting IN to the legacy mode,
  which is legitimate (they test the legacy trade-off, which still exists) but leaves the default
  with a single guard. Add a `false`-mode sibling assertion to each.

- **[SHOULD] No BPMTracker-LEVEL test asserts `totalBarCount()` survives a structural reset**
  while `barCount()` zeroes. `tests/test_bpm_stabilization.cpp` already carries the "structural
  transition into drop does not reset barCount" pattern to mirror. The builder flagged this
  itself; it was not added only because this session's packet fenced it to one test file.

- **[SHOULD] `totalBarCount` is not exposed on `/api/signals` or the TestServer snapshot**, so
  the LIVE verification path that was used to prove the S166-L5a fix is unavailable for S168.
  Unit-proven only. Cheap to add and it unblocks a real behavioural gate.

- **[WATCH, not now] Float precision, not integer overflow, is the real ceiling.**
  `uint32_t totalBarCount` wraps in centuries, but `static_cast<float>` is exact only to 2^24
  bars ≈ 260–520 days of ONE BPMTracker instance's continuous uptime. Failure mode is a one-bar
  phase stutter, never a backward jump. Irrelevant for a festival set; real for a permanent
  installation. Measured by the reviewer, not inferred.

- **[MUST THIS ARC] `RecordPanel` is now MORE dead than it was.** Lane A deleted `SessionRecorder`
  and stripped every `recorder_`-dependent branch from the panel (Save/Load/Play bodies,
  `refresh()`), leaving buttons that no-op. Boris's standing ruling is that dead UI gets BUILT,
  never hidden — so this is only acceptable as a same-arc intermediate state. Spec step 4 wires
  the panel to `PerformanceRecorder`/`Player`. Do not close this arc with the panel lying.

- **[SHOULD] `RecorderClock` omits D1's "at least every 8 bars" periodic tempo-map anchor.**
  Declared by the builder, not silent. On a long take at a steady tempo the tempo map stays very
  sparse; confirm `beatAt`/`tAt`/`sampleAt` reconstruct musical time accurately across a 40-minute
  gap before shipping the recorder, or implement the periodic anchor.

- **[SHOULD] `Player::Override::Latch` is accepted and stored but not wired into dispatch.**
  An unimplemented mode that is silently accepted risks quiet wrong behaviour instead of a loud
  failure. Either wire it or make setting it a hard error until it exists.

## s-rta-0923 learnings (lane-B home; mirrored from the session log)
- Fail-first probes: never revert/restore a source file inside the gate build dir and trust the next incremental build. Make can leave a dependent object (test_take.cpp.o) compiled against the reverted header — a silent class-layout (ODR) mismatch that manufactured a deterministic false FAIL (286201 anchors). Use a scratch -B dir, or clean-rebuild the target after restoring.
- Disabled buttons: this app's LookAndFeel draws disabled buttons at full brightness. Dim explicitly (setAlpha) and verify from a live screenshot. Disabled JUCE buttons may be missing from the accessibility tree; tooltips still show on them (TooltipWindow has no enabled check).
- Merging parallel lanes that each append to tests/CMakeLists.txt: a single conflict region resolves as ours+theirs; MULTIPLE hunks interleave targets' lines. Rebuild the file as HEAD plus the lane's appended block, then check for duplicate add_executable names.
- graphify: `graphify update .` is code-only, needs no LLM and rebuilt this repo in 10 s. The post-commit hook is safe while a graph exists; it writes a 0-node graph only when graph.json was already deleted.
- Startup crashes with Bluetooth headsets: capture the .ips files (one JSON header line + JSON body) and read the faulting thread with python before theorising; three crashes held two different signatures.
