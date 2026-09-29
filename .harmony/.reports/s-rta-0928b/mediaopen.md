## BUILDER REPORT -- lane mediaopen (s-rta-0928b)

STATUS: DONE_WITH_CONCERNS
RESULT: plan-mediaopen.md commits 1-6 + 8-9 (HARMONY ADOPTION P1 scope; commit 7, the asynchronous loads, NOT built) are on lane/mediaopen, 6ed0a48..bf0d93e + this report, branched from main 328301d (seqvram merged; main did not move during the lane). The fence never shows a deck-less black frame any more: on the counters-only app a 4K video drop showed +6 black frames, a 1080p drop +2, a 300-image drop +8; on the final app every drop and load shows 0 black and holds the canvas 0-2 frames. Drops open their media before the fence; sequences open with no file I/O (two 300-frame sequences load with a 17.9-20.4 ms message-thread peak, was 122.0); a missing sequence frame repeats the previous one; presence is a 1 Hz off-thread flag (0.53 s to "missing"); a file without "speed" plays. probe-media-open GREEN twice (38/0), ctest 880/880 serial, image-load 37/0, crossfade 35/0, seq-vram 66/0, deck-clock 10/0, render-state 35/0, canvas 15/0, capture 9/0.
FACTS: `src/render/FencedPtrSlot.h` (deck + fence in one atomic word, adoption P3), `src/render/Renderer.cpp` (the hold at the top of renderOpenGL, counters), `src/core/UndoService.cpp` (detachActiveDeckFenced), `src/MainComponent.cpp` (prepareFileDrop / prepareMultiFileDrop / commitDrop, debugDropFiles, presence seed + sweeper, mediaStateVar), `src/media/ImageSequence.cpp` (open without I/O, seekTo request), `src/core/MediaPresence.h`, `src/render/CompositorEngine.cpp` (mediaPresent at the 6 stat sites), `src/ui/ClipCell.cpp` (classifyDrop / dispatchDrop, paint flag, sequence thumbnail), `src/ui/LayerStrip.cpp`, `src/model/Clip.cpp` (speed guard), `src/api/MessageHeartbeat.h`, `src/api/ApiServer.cpp`, `src/test/TestServer.cpp`, `.harmony/probe-media-open.py`, `tests/test_fenced_ptr_slot.cpp`, `tests/test_image_sequence_open.cpp`, `tests/test_media_presence.cpp`, `tests/test_hot_thread_io_lint.cpp`, `tests/test_deck_thumbnails.cpp` (B3), `tests/test_composition.cpp` (speed), `docs/claude/rendering.md`, `docs/claude/pitfalls.md` (NN), `CLAUDE.md` (24,659 B).
METHOD: One commit per plan item, in plan order. Every new ctest case was RED first: against the pre-change tree / HEAD's version of the file under test, or (the lint) against the main checkout's src read-only. Teeth were run on COPIES where possible (a mutated header in a scratch include dir; a mutated src copy for the lint). Where only in-place worked (ImageSequence.cpp, Renderer.cpp, MainComponent.cpp teeth builds), the file was restored and checked by sha256. Every probe row was run RED on main (absent) and on the counters-only app (by value), then GREEN on each commit's app and twice on the final app. All live runs used the lock helper, open -g, production mode 7070, quiet-before-lock, load average printed. No Output window was opened, no synthetic input was used, and no debugger was attached.
CONFIDENCE+VERIFY: High for the hold, the drop split, the sequence open / seek, presence and speed: pure ctests with teeth, and live rows RED by value then GREEN with teeth T1 / T2 on the final tree. Verify: `ctest --test-dir build-lane -j1` = 880/880; with the lock held, `MEDIAOPEN_APP=<lane app> bash .harmony/probe-media-open.sh <out>` = `PY 38 PASS / 0 FAIL` + `PROBE-MEDIA-OPEN GREEN`.
UNKNOWNS/NOT-DONE: Commit 7 (MediaOpener / staged loads / adoptVideoPlayer, rows m2 / m2b) is deferred per adoption P1. VideoPlayer.* is untouched (P2), so a 4K video drop still pays today's full-frame thumbnail on the message thread (m3 stall 64.0-84.2 ms, bound 150). The m2c row (a load's fence frame) is a GUARD, not RED evidence: the counters-only app showed 0 black frames in 18 image-composition loads (6 runs). The pitfall number is "NN". HANDOFF.md / APP-INVENTORY.md are untouched.
NUANCE: Every drop runs TWO fences: the drop's own, and SetClipCmd::execute's runFenced when the edit is pushed. A fence covers 0-2 frames when its mutation is trivial, so the "hold <= 3" bound fits both. JUCE runs a blocking executeOnGLThread work item inside renderFrame immediately before renderOpenGL (`juce_OpenGLContext.cpp:403-414`), so whether a fence renders a frame is a race between the message thread's mutation and the GL thread's path to the deck load. That is why loads (a quick move-assign) rarely showed a black frame, while a video open inside the fence always did. m5's missing frame shows up as seq_late_frames (54-58 at 5.4 s): that is expected, because a Failed current frame repeats the shown one.
HANDOFF-NEEDS: none (Harmony: pitfall number, review, merge; the notebook lines below).

INBOX-RECHECK: none

### SUMMARY
A withDeckDetached fence now HOLDS the canvas instead of rendering black. The deck pointer and the fence state are one atomic word, read once per frame. Drops open their media before the fence and set the clip inside it. ImageSequence::open does no file I/O. A sequence's thumbnail comes from ClipThumbnails. ImageSequence::seekTo is a GL-thread request. File presence is a 1 Hz off-thread flag. A missing "speed" loads 1.0. The counters, a TEST-ONLY heartbeat, a TEST-ONLY drop endpoint and the probe witness all of it.

### ITEMS (plan section 8; commit per item)
| # | commit | item | RED | GREEN |
|---|---|---|---|---|
| 1 | 6ed0a48 | FencedPtrSlot (P3) + fence_hold/black counters (7070 + 8080), TEST-ONLY /api/debug/heartbeat (MessageHeartbeat.h, P6) + /api/debug/drop_files (ClipCell::classifyDrop / dispatchDrop), /api/composition clip fields, probe-media-open.{sh,py,json}, test_fenced_ptr_slot | main: `PY 5 PASS / 18 FAIL` (404 / absent fields); counters-only app: `PY 28 PASS / 10 FAIL` (by value, below) | ctest 871/871; test_fenced_ptr_slot teeth (two-load reader) 5/5 FAIL; TSan build 5/5 clean |
| 2 | ceeeabc | Clip::fromVar speed guard + test_composition cases | `test_composition.cpp:552: FAILED: clip.speed, WithinAbs(1.0f, 0.001f) for: 0.0f is within 0.00100000004749745 of 1.0`; m8 `playhead 0.0 >= 0.15` | ctest 871/871; m8 PASS (0.337) |
| 3 | 53bce4f | the fence HOLD (4.1) | counters-only app: drops black +6 / +2 / +2 / +8 | c3 app: black +0 on every drop and load; teeth T1 below |
| 4 | 06dee9e | prepare outside the fence / commitDrop inside (4.2; VideoPlayer untouched per P2) | c3 app: m3 hold +6, m3d hold +6 (`(b) ... +6 <= 3` FAIL) | c4 app: hold m3 +1, m3b +1, m3c 0, m3d +2; teeth T2 below |
| 5 | 3d74ae1 | ImageSequence::open without I/O, seekTo request, sequence thumbnails from ClipThumbnails (4.5), test_image_sequence_open, test_deck_thumbnails B3 | HEAD's ImageSequence: `:54 FAILED: seq.open(files) for: false`, `:78 FAILED: seq.getFrameCount() == 12 for: 11 == 12`, `:101 FAILED: seq.getPlayheadPosition(), WithinAbs(0.0, 1e-9) for: 0.5 ...`; B3 on the pre-change tree `test_deck_thumbnails.cpp:338: FAILED: post.waitFor(1) for: false`; c4 app m3d stall 90.1, m4 146.1, m5 code 6 | ctest 874/874; c5 app m3d 23.5, m4 14.6, m5 code 4 |
| 6 | 8158b3d | MediaPresence + mediaMissing at the 6 compositor sites and ClipCell::paint, /api/state media, test_media_presence, test_hot_thread_io_lint | lint vs main src: `loadFrom outside the decoders: MainComponent.cpp:2940, MainComponent.cpp:4957, media/ImageSequence.cpp:56, media/ImageSequence.cpp:501`, `render/CompositorEngine.cpp: 6 stat(s)`, `media/ImageSequence.cpp: 1 stat(s)`, `ui/ClipCell.cpp: 1 stat(s)`; presence vs HEAD Clip.h `:72 FAILED: !(clip.mediaMissing) for: !true`, `:80`; c5 app m7 `a deleted file is missing within 1.5 s (None)` | ctest 880/880; c6 app `PY 38 PASS / 0 FAIL` |
| 7 | -- | asynchronous loads | DEFERRED (P1) | -- |
| 8 | bf0d93e | docs: rendering.md paragraph, pitfall NN, Pitfall 51 sentence, CLAUDE.md index NN (paid: 24,665 -> 24,659 B by shortening index lines 45 / 48 / 53) | -- | -- |
| 9 | this commit | report | -- | -- |

### PROBE ROWS (.harmony/probe-media-open.sh; plan 4.10 minus the deferred m2 / m2b; m2c = plan m2 (c))
Counters-only app (`red_c1`) -> final app (`mo-green-1` / `mo-green-2`), verbatim values:
| row | counters-only (c1) | final (2 runs) | verdict |
|---|---|---|---|
| m1_heartbeat | PASS (the endpoint lands in c1) | PASS | main: `got 404 None` |
| m2c_load_fence | black +0 in 3 loads (+0 x 5 more runs = 18 loads) | black +0 / +0 | GUARD (not reliably RED) |
| m3_drop_4k_video | black +6, hold +0, stall 54.0 ms | black 0, hold +1 / +1, stall 69.5 / 64.0 ms | RED -> GREEN |
| m3b_drop_1080_video | black +2, stall 27.2 | black 0, hold +2 / +0, stall 28.9 / 20.4 | RED -> GREEN |
| m3c_drop_4k_image | black +2; P4 distribution +1, +0, +1, +1, +1 (fails 4/5) | black 0, hold +1 / +1 | RED per P4 (>= 4 of 5) -> GREEN |
| m3d_drop_seq300 | black +8, stall 70.7 | black 0, hold +1 / +1, stall 16.9 / 15.1 | RED -> GREEN |
| m4_load_two_seq | stall 122.0 ms | stall 17.9 / 20.4 ms | RED -> GREEN |
| m5_missing_frame_repeats | code 6 | code 4 / 4 | RED -> GREEN |
| m6_retrigger_seek (guard) | playhead 0.5216, code 6 | 0.5205 / 0.5226, code 6 | guard, PASS everywhere |
| m7_presence | never missing (`(None)`) | missing after 0.53 / 0.53 s, present after 0.76 / 0.76 s | RED -> GREEN |
| m8_speed_default | playhead 0.0 | 0.336 / 0.338 | RED -> GREEN |
| end_hold_witness | hold +0, black +20 | hold +7 / +6, black +0 | RED -> GREEN |

Per-commit apps (full runs): c3 `PY 32 PASS / 6 FAIL`, c4 `PY 34 PASS / 4 FAIL`, c5 `PY 37 PASS / 1 FAIL` (m7 only), c6 `PY 38 PASS / 0 FAIL`.
Teeth on the final tree (in-place, sha256 restore byte-identical: Renderer.cpp 2abeaf25..., MainComponent.cpp eeb00395...):
- T1 (hold branch disabled): `PY 12 PASS / 4 FAIL` -- `m3_drop_4k_video: (a) ... (fence_black_frames +1 == 0)`, m3b +1, m3d +2, `end_hold_witness: ... (0 frames)`.
- T2 (handleFileDrop prepares inside the fence): `FAIL  m3_drop_4k_video: (b) the fence holds only the setClip (fence_hold_frames +8 <= 3)` (m3b +2, the 1080p open is short).
Looked at the final captures: m3 is the dropped video's flat 0x3060c0, m7 capB is the base layer's blue (the missing layer skipped). All values are decoded from pixels by the probe.

### TESTS
- ctest serial `ctest --test-dir build-lane -j1`: `100% tests passed, 0 tests failed out of 880` (main 869 + 2 fenced-slot + 2 seq-open + B3 + 4 presence + 2 lint; test_composition gained a SECTION, not a case).
- ctest teeth: fenced slot two-load reader (header COPY): `test_fenced_ptr_slot.cpp:86: FAILED: fencedWithDeck.load() == 0 for: 3234`, `:87 ... unfencedWithoutDeck ... 1562` (5/5 runs); TSan (`-DADNA_SANITIZE=thread`, the ctest binary) 5/5 `All tests passed`, no report. Seq-open (in place, sha256 3fe829ce... pre == post): consume moved after the playing check -> `:114 FAILED: ... 0.0 is within 0.000000001 of 1.0`; the stat re-added -> `:54` / `:78` / `:79`. Lint (src COPY): a loadFrom in MainComponent + the stat in ImageSequence -> both cases FAIL.
- Existing probes on the final app (re-run, no threshold touched): image-load `PY 37 PASS / 0 FAIL`, crossfade `PY 35 PASS / 0 FAIL`, seq-vram `PY 66 PASS / 0 FAIL` (R7: the one-frame-later out-point wrap trips no row), deck-clock `PY 10 PASS / 0 FAIL`, render-state `PY 35 PASS / 0 FAIL`, canvas `PY 15 PASS / 0 FAIL`, capture `PY 9 PASS / 0 FAIL`.

### FILES CHANGED
- src/render/FencedPtrSlot.h (new) -- the deck pointer with bit 0 = fenced; set / detachFenced / get / view (one load).
- src/render/Renderer.h/.cpp -- activeDeck_ is a FencedPtrSlot; detachActiveDeckFenced; the hold path (fenced && deck-less: re-present canvasTex_, publish to outputs, return; black counted only without a canvas); fence counters.
- src/core/UndoService.h/.cpp -- the fence begins with detachActiveDeckFenced (the restore's setActiveDeck ends it).
- src/MainComponent.h/.cpp -- PreparedDrop / prepareFileDrop / prepareMultiFileDrop / commitDrop replace applyFileDrop / applyMultiFileDrop in handleFileDrop, handleMultiFileDrop, the multi-video and mixed drops; debugDropFiles; the sequence frame-0 thumbnail decodes removed (load + drop); presence seed in openMediaForDeck + MediaPresenceSweeper member; mediaStateVar; server wiring.
- src/media/ImageSequence.h/.cpp -- open without stat / decode; width_ / height_ / getThumbnail removed; seekTo request + consume.
- src/ui/ClipCell.h/.cpp -- classifyDrop + dispatchDrop (filesDropped uses them); the "!" reads mediaMissing; a sequence's thumbnail from ClipThumbnails (first file).
- src/ui/LayerStrip.cpp -- the same thumbnail rule.
- src/render/CompositorEngine.cpp -- mediaPresent() at the 6 former stat sites.
- src/model/Clip.h/.cpp -- mediaMissing (runtime; clear / replaceContent); speed hasProperty guard.
- src/core/MediaPresence.h (new) -- presence::mediaPaths / apply / seed + MediaPresenceSweeper.
- src/api/MessageHeartbeat.h (new), src/api/ApiServer.h/.cpp, src/test/TestServer.h/.cpp -- /api/state fence + media fields, TEST-ONLY heartbeat / drop_files, /api/composition clip fields.
- tests: test_fenced_ptr_slot.cpp, test_image_sequence_open.cpp, test_media_presence.cpp, test_hot_thread_io_lint.cpp (new), test_deck_thumbnails.cpp (B3), test_composition.cpp (speed), CMakeLists.txt (EOF blocks).
- .harmony/probe-media-open.{sh,py,json} (new; probe-image-load.sh's scaffolding).
- docs/claude/rendering.md, docs/claude/pitfalls.md, CLAUDE.md.

### SLIM CHECK
Nothing left to cut. Every new symbol has a caller or a test. ImageSequence's width_ / height_ / getThumbnail (dead after open() lost its decode) were removed. openDirectory stays (unused before this lane; out of scope per plan 4.5). The plan's no-op `clip.mediaMissing = false` seed in prepareFileDrop was not added, because a fresh Clip already holds false.

### ISSUES
- m2c is not RED on the counters-only app (0 black in 18 loads; diag-media measured 1 per load with video compositions). Reported as a guard (the P4 logic), not as RED evidence. Plan m2's (c) part is therefore asserted (0 black) but not proven by a RED.
- Commit 4 and commit 5 edits briefly shared MainComponent.cpp in the working tree. Commit 4 was committed with the commit-5 hunks reverted, byte-for-byte the state that was built and tested as the c4 app, and the hunks were then re-applied.
- VideoPlayer is untouched (P2), and the video lane has NOT merged (main 328301d at the end). So the drop's UI hold still includes today's full-frame thumbnail: m3 message stall 64-84 ms on the final app. The OUTPUT holds regardless.

### RISKS
- A deleted file stays on screen for up to about 1 s before it is treated as missing (P5 semantics; measured 0.53 s). Low.
- The sequence's own out-point wrap seek lands one GL frame later (video parity); probe-seq-vram 66/0 shows no row depends on it. Low.
- mediaMissing is a plain bool written on the message thread and read on the GL thread, the house class of clip->playing. /api/composition reads the new clip fields on the HTTP thread, the existing unguarded pattern of that handler. Low.

### NOTEBOOK (for Harmony to append to .harmony/notebook.md)
- 2026-09-29 | Files: src/core/UndoService.cpp, src/render/Renderer.cpp | JUCE's blocking executeOnGLThread runs the work item inside renderFrame immediately BEFORE renderOpenGL (juce_OpenGLContext.cpp:403-414). Whether a withDeckDetached fence renders a fenced frame is a race between the mutation and the GL thread's path to the deck load. A trivial mutation (a composition move-assign) usually renders none; a video open inside the fence always does. A "load shows a black frame" RED needs a slow mutation. | Valid while: withDeckDetached drains via executeOnGLThread.
- 2026-09-29 | Files: src/MainComponent.cpp, src/core/ClipCommands.h | Every drop runs TWO fences: the handler's withDeckDetached, then SetClipCmd::execute's runFenced when pushClipEdits / pushCommands performs the command. Count both when bounding fenced frames. | Valid while: drops push SetClipCmd via UndoManager::perform.
- 2026-09-29 | Files: tests/test_image_sequence_open.cpp | An in-place teeth mutation written in the same second as the last build is NOT rebuilt by make. `sleep 1; touch <file>` before each teeth build (a T-stat run silently reused the T-seek object until then). | Valid while: CMake Makefile generator.
- 2026-09-29 | Files: .harmony/probe-media-open.py, src/api/ApiServer.cpp | Finder-equivalent drops are drivable without synthetic input: POST /api/debug/drop_files {layer, column, files} (TEST_SERVER build, 7070) reaches the same DeckView callbacks via ClipCell::classifyDrop + dispatchDrop. Message-thread freezes are measurable with POST /api/debug/heartbeat {on:true} -> /api/state peak_message_stall_ms (reset on read; poll every 15 ms). | Valid while: the TEST-ONLY routes exist.

### BORIS CHECKS (for Harmony's page)
1. Dropping a video, an image or a folder of images onto a cell no longer flashes the projector black. At most the picture freezes for a frame or two.
2. A file deleted while the show runs disappears from the output within about half a second to a second (it used to be the next frame). Say if he would rather keep the picture.
3. An image sequence with a missing frame now holds the previous frame for that slot instead of skipping it (later frames keep their timing).
4. A dropped image sequence's cell thumbnail appears a moment after the drop (it loads in the background) instead of the drop pausing to make it.
5. An old show file whose clips have no "speed" now plays its videos instead of loading them frozen.
(The load freeze with many videos is unchanged in this lane: that is the deferred asynchronous-load lane.)

### METRICS
- Self-check: build rc 0 after every commit (no new warnings in changed files); ctest serial 871 -> 871 -> 871 -> 871 -> 874 -> 880 -> 880 final; TSan run of test_fenced_ptr_slot 5/5 clean.
- Tool calls: ~150. Files read: ~40.
- Live: 7 lock holds (each <= 8 min, released between; 45 s cooldown honoured by the helper); 0 Output-named windows after every batch (`audio-dna windows 0, Output-named 0`).

### KNOWLEDGE CONTEXT
- Tools used: grep. Impact authority: grep (not authoritative); I read every caller of setActiveDeck / getActiveDeck, seekTo, ImageSequence::getWidth / getThumbnail and Clip::thumbnail before changing them. Risk level: ELEVATED (Renderer / MainComponent are central); mitigated by the full regression probe set above.

### PACKET QUALITY
- Clarity: CLEAR (plan + adoption rulings), HAD_TO_INFER on three points: (a) the plan puts classifyDrop in commit 4, but commit 1's drop endpoint needs it, so it landed in commit 1 (a behaviour-preserving factor-out); (b) the plan's `media` state block names opens_* fields that belong to the deferred commit 7, so only presence_sweeps / presence_changed ship; (c) the presence seed at load lives in openMediaForDeck, because the plan's beginStagedOpen is commit 7.
- Missing context: none blocking.
- Unused context: plan 4.4 (deferred), attack-mediaopen-vj MUST 1-3 (they live in commit 7).
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files: CLAUDE.md, the plan, both attacks, diag-media.md S2 / S3, seqvram.md (format), the lock helper -- all useful.

### STATUS
DONE_WITH_CONCERNS. Every in-scope item is shipped with RED-first gates, GREEN twice, the regression probes are green and ctest is 880/880. The concerns are m2c being a guard rather than RED evidence, and the drop's UI hold keeping today's thumbnail cost until the video lane lands (P2).

### NEXT ACTION
Harmony: assign the pitfall number and review. At merge, if lane/video merges first, rebase: both lanes touch CLAUDE.md index lines (disjoint), rendering.md (adjacent paragraphs), pitfalls.md (append) and tests/CMakeLists.txt (EOF blocks). Then the deferred asynchronous-load lane: it builds on Renderer's FencedPtrSlot + the drop split, with attack acceptance items as filed in the adoption.
