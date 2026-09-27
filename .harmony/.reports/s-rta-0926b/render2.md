# lane render2 — s-rta-0926b wave 2 (ruling R1 = B', R4-opaque (1), R4-types, empty active deck, docs)

## BUILDER REPORT

STATUS: DONE_WITH_CONCERNS
RESULT: All six build steps from the ruling are in, one commit each, on `lane/render-0926b` (base 858acd1). Every new probe row was RED on the pre-change app (`build/`) and is GREEN on `build-lane`. The final gate is fully GREEN: probe-render-state `PY 31 PASS / 0 FAIL`, probe-crossfade `PY 35 PASS / 0 FAIL`, probe-effects-parity `PY 46 PASS / 0 FAIL`, ctest `100% tests passed, 0 tests failed out of 596`. The concerns are below: two small edits outside the fence (the /api/state handlers), one small UI addition past the ruling (dimming the disabled toggle), and a rig hazard from another lane.
FACTS: per-step RED/GREEN logs `.harmony/.reports/s-rta-0926b/render2-evidence/probe-render-state-new-rows-MAIN.txt` (RED), `.harmony/.reports/s-rta-0926b/render2-evidence/probe-render-state-r1-step2.txt`, `.harmony/.reports/s-rta-0926b/render2-evidence/probe-render-state-r4opaque-step3.txt`, `.harmony/.reports/s-rta-0926b/render2-evidence/probe-render-state-r4types-step4.txt`, `.harmony/.reports/s-rta-0926b/render2-evidence/probe-render-state-empty-deck-step5.txt`; final gate `.harmony/.reports/s-rta-0926b/render2-evidence/probe-render-state-FINAL.txt`, `.harmony/.reports/s-rta-0926b/render2-evidence/probe-crossfade-FINAL.txt`, `.harmony/.reports/s-rta-0926b/render2-evidence/probe-effects-parity-FINAL.txt`, `.harmony/.reports/s-rta-0926b/render2-evidence/ctest-FINAL.txt`; unit-test proof that the tests fail on the old code (RED + teeth) `.harmony/.reports/s-rta-0926b/render2-evidence/teeth-r1-pure.out`, `.harmony/.reports/s-rta-0926b/render2-evidence/teeth-r4-types.out`; screenshots in `.harmony/.reports/s-rta-0926b/render2-shots/`.
METHOD: For each ruling step I wrote the RED-first rows or tests, implemented the step, built it, and saved a copy of that step's app (`scratchpad/render2/apps/stepN`). Each step's own rows ran live on its own app copy. Each step's exact source was staged by re-applying that step's edit script to HEAD, and I proved that steps 3+4+5 applied together give exactly the working tree (see ISSUES). I ran the full gate at the end on `build-lane`. Every render claim comes from PNGs decoded with PIL/numpy, from a fresh run dir per run, with every `render_frame` JSON response checked; I looked at one decoded strip per item.
CONFIDENCE+VERIFY: High for all five items. Every row moves from its calibrated RED value to 0.00 (R1 ratios ≤ 0.01). Two items do not show RED because nothing on the old code can fail: `r1_counts` fails on base only because the fields are missing, and the three guard rows pass on both builds by design. Re-run with `RSTATE_APP=<build-lane app> bash .harmony/probe-render-state.sh` (needs `/tmp/audiodna-live.lock` and a free port 7070).
UNKNOWNS/NOT-DONE: I could not capture the Persistent toggle's tooltip, because hovering needs synthetic input. I did not decide the questions that are Boris's (defaults were applied as the packet says). The non-persistent frozen crossfade on inactive decks is untouched, as the fence requires.
NUANCE: `beginEmptyActiveDeck` clears to OPAQUE black, while compositeDeck clears to transparent black. This makes an empty active deck byte-identical (RGBA, max abs diff 0.0) to an Opaque black clip. The first fade on a layer with Split/Stutter now creates a spare 480-frame ring, which costs one long frame: 18.6–24.1 ms measured. That is over the 8 ms render budget but under the ruling's 50 ms threshold, so I did not add the ring pre-creation.
HANDOFF-NEEDS: Harmony should decide whether the /api/state edits (production `src/api/ApiServer.cpp` + test-mode `src/test/TestServer.cpp`, 5 lines each) are acceptable outside the listed fence, and should tell the probe-hygiene lane that its `stub_server.py` harness drives a live app on port 7070 without taking the lock.

INBOX-RECHECK: none

Labels: VERIFIED = run/measured here. INFERRED = reasoned from code, not run. Metric d(X,Y) = mean |X−Y| over RGB, 0..255, 756×878 captures. R1 ratio = d(frame, the clip alone) / d(refA, refB) in that clip's wipe region.

### SUMMARY
| Step | Commit | What | RED on build/ (858acd1 code) | GREEN on the fix |
|---|---|---|---|---|
| 1 R1 pure | ff110f2 | `LayerStateKey::outgoingChain` (bit 30) + pure `CrossfadeStartDetector` (`src/render/CrossfadeHistory.h`) + unit tests | the new tests do not compile on base (RED by absence); a mutated copy of each header fails its test | 3 + 7 test cases pass |
| probes | 6c015ff | 11 new probe-render-state rows + rig guards | `PY 3 PASS / 8 FAIL` (the 3 PASS are the three guards, as the ruling says) | — |
| 2 R1 compositor | 13a9ec3 | hand-over at fade start (copy the temporal buffer, swap the ring), outgoing chain uses the slot key, counters, `peak_frame_time_ms`, /api/state fields, ring-size comment fix | r1_temporal IN 0.34 / OUT 0.32; r1_ring 1.00 / 1.00; r1_retrigger 0.33 / 0.33; r1_counts: fields missing | `PY 6 PASS / 0 FAIL`: all ratios ≤ 0.01; frame_rings +2 exactly, temporal_buffers +0 |
| 3 R4-opaque | 3d92481 | a persistent Opaque layer below opacity 0.999 goes through the Alpha keying pass (`u_opacity`) then blends; the opacity-1.0 path is unchanged | r4_opaque_opacity d = 14.57 (subject equals the full-opacity frame) | `PY 8 PASS / 0 FAIL` (r4_opaque_opacity 0.00, overlay guard, r2_temporal/r2_ring 0.00) |
| 4 R4-types | 7ac95b8 | `Layer::canBePersistent` used by the compositor and the LayerInspector; FX Only and media-less effect clips work as persistent layers | r4_fxonly_persistent / _medialess d = 228.68 (skipped) | `PY 3 PASS / 0 FAIL` (both 0.00, mask guard 0.00) |
| 4b UI | 7dc15b3 | the disabled toggle is dimmed (it looked enabled); screenshots | disabled Persistent label pixels = enabled label (143 = 143) | disabled label 73 vs enabled 143 |
| 5 empty deck | f0d5050 | `hasPersistentContent` + `beginEmptyActiveDeck`; Renderer presents the accumulator | r4_empty_active_deck: subject is black, d = 19.46 | `PY 4 PASS / 0 FAIL`: d = 0.00, RGBA max diff 0.0 |
| 6 docs | 97713b0 | rendering.md, performance-controls.md (persistent layers + Layer Router line), Pitfall 35 + CLAUDE.md index line (24,366 bytes) | — | — |

Frames I looked at: `render2-evidence/r1_temporal_strip.png` (RED shows ghosts of the other clip in both wipe regions; GREEN is a clean wipe), `render2-evidence/r1_ring_strip.png` (RED shows each region holding the OTHER clip), `render2-evidence/r4_opaque_strip.png` (RED shows B at full strength; GREEN shows the 50/50 blend that equals the reference), `render2-evidence/r4_types_strip.png` (RED shows plain A; GREEN shows inverted A that equals the reference), `render2-evidence/r4_empty_active_deck_strip.png` (RED is black; GREEN is B, identical to the black-deck reference), and both LayerInspector screenshots.

### FILES CHANGED
- `src/render/LayerStateKey.h`: adds `kOutgoingChainBit`, `outgoingChain()`, and a header comment explaining the three keys.
- `src/render/CrossfadeHistory.h` (new, pure): `CrossfadeStartDetector` as specified in the ruling.
- `src/render/CompositorEngine.h/.cpp`:
  - R1 changes:
    - `crossfadeStart_` detectors.
    - `handOverClipHistory` (copy the buffer; clear a stale spare; swap the ring and reset the incoming one).
    - `renderLayerStages` computes `outKey` and passes it to `applyTransition`.
  - Relaxed-atomic `temporalBufferCount_`/`frameRingCount_` counters and their getters.
  - Ring comment corrected to 248.8 MB at 1080p.
  - R4-opaque: `applyLayerKeying(layer, mode, ...)` and the persistent Opaque opacity branch.
  - R4-types: FX Only and media-less branches in `compositePersistentLayers`.
  - Step 5: `clipHasContent` (the content check now shared with compositeDeck), `hasPersistentContent`, `beginEmptyActiveDeck`.
- `src/render/Renderer.h/.cpp`:
  - `peakFrameTimeMs_` + `takePeakFrameTimeMs()` (reading resets it).
  - The empty-active-deck branch before `compositePersistentLayers`.
- `src/model/Layer.h`: `static constexpr bool canBePersistent(Type)` only.
- `src/ui/LayerInspector.cpp`: toggle `setEnabled` + tooltip + `setAlpha(0.4)` when disabled, in `syncFromLayer` only.
- `src/api/ApiServer.cpp` + `src/test/TestServer.cpp`: three `/api/state` fields each (OUTSIDE the listed fence, see ISSUES).
- `tests/test_crossfade_history.cpp` (new, 7 cases), `tests/test_layer_state_key.cpp` (outgoingChain in cases 2 and 3), `tests/test_compositor.cpp` (canBePersistent truth table), `tests/CMakeLists.txt`.
- `.harmony/probe-render-state.{sh,py,json}`: 11 rows (r1_temporal, r1_control, r1_ring, r1_retrigger, r1_counts, r4_opaque_opacity, r4_opaque_overlay, r4_fxonly_persistent, r4_fxonly_medialess, r4_mask_skipped, r4_empty_active_deck), a `cap(size=)` option, and rig guards (see ISSUES).
- `docs/claude/rendering.md`, `docs/claude/performance-controls.md`, `docs/claude/pitfalls.md`, `CLAUDE.md` (one index line).

### TESTS
- Unit tests. The new tests do not compile on base; mutated copies of the headers fail:
  - `teeth-r1-pure.out`: dropping the outgoing bit fails case 2. Detector mutations each fail at least one case: fire-every-frame fails 6/7, ignore-pair 1/7, ignore-rewind 1/7, no-prev-gate 1/7.
  - `teeth-r4-types.out`: `allow_mask` and `drop_fxonly` each fail the [persistent] test.
  - Deliverable sha256 is unchanged before and after both teeth runs.
  - While writing the teeth, the first version of the re-trigger case did not catch the ignore-pair mutation (the rewind clause masked it). I added a faster-re-trigger step, and that mutation now fails.
- ctest, serial, `ctest --test-dir build-lane`: `100% tests passed, 0 tests failed out of 596` (588 at the merge + 7 + 1).
- Live, final gate on `build-lane` (HEAD code; docs are not compiled):
  - probe-render-state (all 21 rows) `PY 31 PASS / 0 FAIL`.
  - probe-crossfade `PY 35 PASS / 0 FAIL`.
  - probe-effects-parity `PY 46 PASS / 0 FAIL`.
  - No foreign `render_frame` traffic in any of the three runs.
- `r1_counts` in the final run: frame_rings 4→6 (+2), temporal_buffers 9→9 (+0). `peak_frame_time_ms`: first use of the ring 18.14 ms, first fade 18.64 ms. In the step-2 run: 19.85 / 24.12 ms.

### ISSUES
1. **Fence: the /api/state fields.** The packet asks for "the /api/state fields the ruling names". The ruling puts them in `src/test/TestServer.cpp`, which serves /api/state only in test mode. The rig allows production mode only, and there /api/state is `ApiServer::handleState` (`src/api/ApiServer.cpp`). Both files are outside the listed fence (`src/render/**`, …). I added the same 3 fields to both (5 lines each), because without them `r1_counts` cannot run. Please accept this or strip it.
2. **Rig hazard, found live.** My first RED run was contaminated: the global effect chain gained 3–7 enabled effects mid-run, and even the guard rows failed. An idle discriminator run showed another process driving my app over REST: captures landed in `/var/folders/.../T/tmp.uhU31rKZEU/out/{G1,V1,REF1}_*.png`. Its `stub_server.py` had failed to bind 7070 (`OSError: [Errno 48] Address already in use`), so its probe talked to the real app without the live lock. INFERRED from the capture names: it is the probe-hygiene lane's effects-parity harness. I discarded that run (kept as `render2-evidence/INVALID-contaminated-RED-run-1.txt`) and added guards to probe-render-state.sh:
   - REFUSE if something else already listens on 7070.
   - FAIL if the 7070 listener is not Audio-DNA.
   - FAIL if any `render_frame` capture lands outside the run dir.
   - FAIL if the global chain has enabled effects at a row start.

   Every later run printed `PASS  no foreign render_frame traffic`.
3. **pgrep false positive.** `pgrep -f 'MacOS/Audio-DN[A]'` matches the LINKER command line of an app build (`-o .../MacOS/Audio-DNA`). One wrapper run refused because my own build was linking; no app was launched. Worse, a probe's 30-second quit fallback would `pkill` a linker. Never build while a live run holds the lock.
4. **Scratch hook.** The UI-shot hook lived only in scratch app copies and is in no commit. `make` missed the same-second revert of MainComponent.cpp, so `build-lane` still held the hook until I touched the file and rebuilt. Verified afterwards: `strings` finds 0 hook strings in the build-lane binary, and MainComponent.cpp has no diff.
5. **Exact per-step staging.** Steps 3–5 share files. Each step's content was staged as HEAD plus that step's edit script. For step 3 I proved that HEAD + step3 + step4 + test + step5 equals the working tree byte-for-byte; for step 4, that HEAD + step4 + step5 does. So each commit is the source its app copy was built from.
6. **Test grid.** In `test_layer_state_key` case 2 the top layer id changed from 0x7FFFFFFE to 0x3FFFFFFF. The ruling's "every (deck, layer) in the existing grids" cannot hold for ids with bit 30 set; real ids start at 100.

### RISKS
- VRAM (medium; the ruling already calls it pre-existing). A layer that crossfades with Split/Stutter on both sides holds 2 rings: 249 MB each at 1080p, ~1 GB at 4K. Nothing frees them until the GL context closes.
- One-time frame hitch (medium-low). The spare ring is created at the first such fade per layer: 18–24 ms measured. The ruling's mitigation (pre-create) triggers only above 50 ms.
- Carry-over (a Boris taste default). An incoming clip's Freeze 1.0 or Posterize Time holds the layer's last picture, exactly like a cut.
- The LookAndFeel ignores the disabled state (found_not_fixed). Every other disabled ToggleButton in the app looks enabled unless its owner dims it.

### SKILL_PROPOSALS
- "live-rig-contamination-check". Use it when a pixel probe goes wrong in ways the code cannot explain (guards failing, effects nobody set). Steps:
  1. Run the app idle and poll /api/state.
  2. grep `err.log` for `Captured frame:` paths outside your run dir.
  3. `lsof -iTCP:<port>` for foreign listeners.

  Existing skills do not cover cross-lane interference.

### METRICS
- Builds: build-lane incremental about 10 times, all rc 0; the only warning in src/render is the pre-existing `totalCells` unused variable.
- Live runs: 8 lock holds (RED ×3 — 1 contaminated, 1 refused with no app launched, 1 valid —, idle ×1, GREEN step 2/3/4/5, UI shots ×2, final ×3).
- About 160 tool calls.

### KNOWLEDGE CONTEXT
- Tools used: grep (no KNOWLEDGE_TOOLS block). Impact authority: grep, not authoritative, so I took a conservative stance: every caller of `applyLayerKeying` / `applyTransition` / `compositeDeck` / `hasActiveLayers` was grepped (CompositorEngine.cpp, Renderer.cpp; `hasActiveLayers()` has no caller). Risk level: NORMAL. The scope gap I found is the /api/state location (ISSUES 1).

### PACKET QUALITY
- Clarity: HAD_TO_INFER.
  - /api/state is served by ApiServer in production mode, not by TestServer.
  - The fence did not list either server.
  - "toggle enabled state" read to include the codebase's disabled-alpha convention.
- Missing context:
  - Another lane's stub harness uses port 7070 without the lock.
  - pgrep matches linker command lines.
  - render_frame's viewport depends on the fallback image. `r4_empty_active_deck` captures at a locked 756×878 so the subject and reference frames match.
- Unused context: none.
- Self-assembly: LEGACY (no DEPARTMENT).
- Self-brief files: ruling-render-forks.md (the spec, current, cites re-located), my wave-1 render.md + diag.py (r1 metric reused), review-render-r1.md, the work log, the notebook render entry — all useful.

### STATUS
DONE_WITH_CONCERNS. Every success criterion is met and every gate is GREEN. The concerns are the out-of-fence /api/state edits (ISSUES 1), the extra toggle dimming (commit 7dc15b3), and the cross-lane rig hazard (ISSUES 2).

### NEXT ACTION
- Harmony:
  - Accept or strip the ApiServer/TestServer fields.
  - Tell the probe-hygiene lane its stub harness must hold `/tmp/audiodna-live.lock`, or bind a port other than 7070.
  - Relay the two Boris confirmations (R1 carry-over, R4-opaque blend-over). The defaults are applied.
- Notebook candidates for Harmony to fold in:
  1. pgrep 'MacOS/Audio-DN[A]' matches linker command lines.
  2. `make` misses a same-second revert; verify scratch hooks are gone with `strings`.
  3. A disabled JUCE toggle looks enabled in this LookAndFeel; dim it with setAlpha 0.4 (the RecordPanel convention).
  4. A foreign REST client can drive the probe app. Use the render_frame capture-path guard.
