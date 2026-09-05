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
