# Session log — s-rta-0926b (2026-09-26 18:37 → 2026-09-27 ~06:00, SECONDARY, RealTimeAudio)

Profile: MINIMAL foreign-repo secondary. Harmony_Main SYSTEM files touched: NONE. Harmony_Main writes: NONE (up-channel
reply via idea-capture.sh -> this repo's .harmony/idea-ledger.md). Ultracode on (workflows). Running log:
.harmony/s-rta-0926b-work.md. Reports + plans + evidence: .harmony/.reports/s-rta-0926b/. Every merge: Harmony rebuilt
build/, ran ctest serially and the full live probe suite herself, every new probe RED on the pre-change build first.

| type | ref | msg |
|---|---|---|
| shipped | boot | first-call token number after the CLAUDE.md split = 54,790 (<= 75,000 PASS; was 101,568); replied up-channel |
| shipped | 7713aa4 | render R2 layer GL state keyed by deck+layer, R3 outgoing clip keeps transform in a crossfade, R4 persistent layers get every per-layer stage, R5 temporal buffer created before any pass; probe-render-state + crossfade k/l |
| shipped | efa40ab | BPM: tempo/Tap/set_bpm/Link writes are analysis-thread requests (TSan-proven race removed); Link tick no longer resets phase 30x/s |
| shipped | c9823ef | recorder: tempo map saved as soon as the take is metered |
| shipped | b022a28 | R1 (Fable ruling B'): per-layer outgoing crossfade history slot; persistent Opaque honours opacity; FX Only persistent; Mask/3D toggle disabled; persistent layers visible over an empty active deck; Pitfall 35 |
| shipped | cef89f5 | Link: a Link tempo never realigns; builds without Link: toggle disabled, no fake 120 BPM |
| shipped | 50b4bce 2d02681 | probe rig: fresh capture dirs + stale-PNG guard; every probe refuses without the lock; exact-binary pgrep |
| shipped | a030319 | disabled toggles drawn disabled app-wide; stale Persistent flag clearable |
| shipped | b332e0d | typed/REST/OSC tempo never moves the beat (Tap + Resync are the beat gestures); routine restore GLIDES over the last beat and lands on the bar (start/loop/restart), in-flight glides released |
| shipped | 5285662 | TopBar Bar 1-2-3-4-1, Phr decimal removed |
| shipped | 04e31bb | TopBar Stop = routines only; per-routine restoreStyle Ease/Jump (engine + REST) |
| shipped | b766720 | decks: unique deck ids everywhere; tab row + (New/Load) + right-click (Save/Save As/Rename/Duplicate/Remove + 10 s Undo); Save/Open Composition in menu + Compositions tab (confirm); library Delete -> confirm + Trash (was a SILENT delete); legacy Deck Save/Load retired; square app dialogs/menus |
| shipped | 17a3ebf | canvas: composition resolution drives the picture everywhere (preview letter/pillar-boxed; portrait/square/4:3/Custom); decks keep time off screen (fades, media clocks, autopilot); deck switch fades from the outgoing deck (was a cut); per-clip Fit Stretch/Bars/Crop |
| design | plans | plan3 (glide/topbar/tempo), plan4 (canvas), plan5 (multi-display outputs — NOT built), plan6 (decks), plan-fitmode, ruling-render-forks, routine-ux design-final + mockup (NOT built) |
| gate | ctest | 580 at boot -> 588 -> 599 -> 600 -> 613 -> 620 -> 636 -> 661 (see close) |
| gate | live | every probe GREEN on the final main build (see handoff session section); RED-first shown for render-state, crossfade k/l, tempo witness, manual-bpm S1-S3, routines glide + Jump rows, deck-tabs, canvas, deck-clock, fitmode |
| finding | recorder-census | the recorder lane's take census read the wrong key ("157/157 empty"); real: 139/158 empty, all before ebbff22, 0 after — corrected |
| finding | output-window | the second-display OutputWindow shows only the legacy single image, never the composition (plan5 fixes it) |
| finding | library-delete | Compositions browser right-click deleted saved deck files with no confirm and no Trash (fixed) |
| finding | timing | step3 T2 misses = Stremio (A/B/C + quiet re-run 94/0 x2); resync V2 single miss = load (6/6 re-runs green); routines 8g landing window edge (widened 7.85 -> 7.80 after 3/3 re-runs) |
| slip | cwd | `cd .harmony && ...` moved the main loop cwd (repeat of s-rta-0926 #5) |
| slip | question | my Q9 to Boris misdescribed the same-BPM mechanism (checked afterwards, corrected) |
| slip | workflow | `${ACK = ''}` runtime bug in a workflow script (caught before the step ran); a fix-round prompt embedded STEP 0 "checkout -B main" (builder refused correctly) |
| slip | timestamps | work-log stamps estimated instead of read from `date` for ~1 h |
| rig | stub-port | a stub server that failed to bind 7070 drove another lane's app; a lane launched `open -g` after a failed lock (same bundle id reaches the running app) -> two gotchas filed |
