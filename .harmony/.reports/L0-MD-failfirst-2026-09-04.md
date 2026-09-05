# L0-MD FAIL-FIRST — captured 2026-09-04, session s-rta-0904 (secondary)

Captured by HARMONY HERSELF against **unmodified code at HEAD**, before any builder
was dispatched at the lane. Required by the packet, and required by Iron Law #6:
a gate that never saw the failure state cannot prove the flip.

## Tree state at capture
- HEAD: `5ab9010`-line (docs/.harmony only above `3b940a0`); **`src/` unchanged since `3b940a0`**.
- Build: `cmake --build . -j8` → `BUILD_RC=0` (already current, no source drift).
- Baseline ctest: **203/203 passed, 0 failed** — RE-RUN, not inherited (`CTEST_RC=0`).

## Launch
```
open --stdout /tmp/adna-failfirst-out.log --stderr /tmp/adna-failfirst-err.log \
  build/AudioDNA_artefacts/Release/Audio-DNA.app --args --test-mode
```
Health probe on the ONLY valid address (`::1`, never 127.0.0.1):
```
$ curl -s 'http://[::1]:8080/api/health'
{ "status": "ready", "gl_version": "4.1", "fps": 109.09, "effects_count": 135 }
```
8080 bound ⇒ this is a REAL green, not the documented 7070 false-green.

## THE ORACLE — the failure, observed
```
$ grep -ic milkdrop /tmp/adna-failfirst-err.log /tmp/adna-failfirst-out.log
/tmp/adna-failfirst-out.log:0
/tmp/adna-failfirst-err.log:0

$ grep -c 'Eyes' /tmp/adna-failfirst-err.log
1
```
**ZERO `[MilkDrop]` lines. `Eyes` present.** `Eyes` is emitted ~77 lines LATER in the SAME
MainComponent constructor, so the constructor demonstrably ran past the MilkDrop wiring
block and the block was skipped — exactly the `if (pmSource)` root cause. stderr carried
289 lines, so this is not an empty-log artifact.

## Post-fix gate (what must flip)
Same launch, same grep: `grep -ic milkdrop` must become **non-zero**, and the `[MilkDrop]`
line must carry a **non-zero preset count**. Plus ctest re-run 203/203 and a clean Release build.

## THE DISCRIMINATING UI CHECK — CLOSED 2026-09-04 (open since 2026-08-03)

Carried for a month as an ONLY-BORIS-CAN-CHECK item. It did not need Boris; it needed
someone to actually look. Harmony read the pixels herself (`/tmp/milkdrop_favorites.png`,
cropped to `/tmp/mdfav_crop.png`).

**Favorites sub-tab, selected and highlighted, renders:**
> `No presets loaded.  To add MilkDrop presets: 1. Place .milk files in the app's …`

**NOT** `No favorites yet`.

Per the discriminator stated in HANDOFF.md, that string on the FAVORITES tab **CONFIRMS THE
ROOT CAUSE**: the preset manager was never wired at all, so every sub-tab falls through to the
same global empty-state. An empty-but-wired manager would have shown a favorites-specific
string here. All four sub-tabs exist and are reachable: Curated · Favorites · Recent · All.

Evidence class: **OBSERVED** (screenshot read, not inferred, not relayed).

### Side observation, same screenshot — free intel for the L4 honesty batch
The `Record` tab and the `Comp/Deck` tab both render **greyed out / dimmed** relative to Files,
FX, Sources and MilkDrop. L4 proposes hiding the Record tab; it is already visibly de-emphasised
in the live UI, which is worth knowing before deciding hide-vs-remove.

### HAZARD SEEN ON THE OWNER'S SCREEN — see the EOS report
`/tmp/milkdrop_all.png` also caught a macOS **TCC security dialog** ("Ghostty is requesting to
bypass the system private window picker and directly access your screen and audio") sitting
modal on the desktop, raised by the probe's own `screencapture`. Harmony did NOT click either
button: granting or refusing screen-recording permission is Boris's decision, not hers. This is
exactly the class the SCREEN-SAFETY LAW §3 names — state no CLI probe can see.

---

# POST-FIX GATE — RUN BY HARMONY, WHO DID NOT BUILD IT (2026-09-05)

| # | Packet success criterion | Result | Evidence class |
|---|---|---|---|
| 1 | Browser lists presets: mood headers with counts, clickable rows | **PASS** | OBSERVED — `/tmp/gate-md-crop.png`, read. `Energetic (9)`, `Psychedelic (9)`, named rows. The packet predicted "Energetic 9 per today's manifest" and that is exactly what rendered. |
| 2 | `[MilkDrop]` startup line with a NON-ZERO count | **PASS** | OBSERVED — `grep -ic milkdrop` 0 → 1; the line is `[MilkDrop] Loaded 30 presets` |
| 3 | Jukebox **Play** actually starts | **NOT TESTED — out of this lane's reach** | The packet itself scopes it out: "Preset LISTING must work at startup with no selector present — that is the actual bug being fixed; jukebox playback can light up later when a source exists." No projectm clip exists in a fresh session, so the lazy selector is legitimately still unwired. Recorded as untested, NOT as passed. |
| 4 | ctest 203/203 | **PASS** | RE-RUN, not inherited: `100% tests passed, 0 tests failed out of 203` |
| 5 | Release build 0 errors, no NEW warnings | **PASS** | `BUILD_RC=0`, `grep -ci 'error:'` = 0. One warning fires in a file this session touched — `CompositorEngine.cpp unused variable 'totalCells'` — and it is **PRE-EXISTING, proven**: line 1347 in HEAD, line 1357 now, shifted by exactly the 10 lines the L2 lane adds. Same code, same warning, not introduced. |

**The flip is real, not asserted.** Same binary path, same launch flags, same grep, same machine —
the only variable changed is the source. Fail-first was captured against unmodified code BEFORE
any builder was dispatched, which is what makes this a gate rather than a green screenshot.

**Independence:** Harmony delegated the build, then ran the compile, the tests, the runtime log
assertion and the visual check herself, and dispatched a separate reviewer at the source. The
party that built never verified.
