# Builder Notebook — Audio-DNA

<!-- Accumulated Builder knowledge. Each Builder reads this and appends discoveries. -->

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
**Note:** Videos (videoPlayers_[id]) and image sequences (imageSequences_[id]) are keyed by clip id and NEVER closed. Static images are keyed by FILE PATH via getKeyTexture(clip.mediaFile) — self-healing on undo. Only VIDEO can be content-swapped under an EXISTING id: kClipReplaceContent calls openVideoForClip(existing->id, newFile), overwriting the player while replaceContent keeps the id. So a reconnect-if-MISSING media guard misses replace-undo (player exists → skip → decodes wrong file). Fix: compare loaded file vs clip.mediaFile (Renderer::getVideoPlayerFile + VideoPlayer::getFile + pure needsVideoReopen() in core/MediaReconnect.h), reopen on mismatch. Sequences can't hit it (replace produces only Image/Video; sequences always get a fresh s_nextClipId).
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
