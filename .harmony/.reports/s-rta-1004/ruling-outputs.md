# RULING -- lane "outputs": the blind council's 31 attacks on plan-outputs.md (a settings window for every output screen)

Author: architect (s-rta-1004), opus at max effort. Read-only pass; Harmony decides after this document.
Pin checks run by command: main `git rev-parse --short HEAD` = 185147b, `git status --short -- src tests docs CMakeLists.txt`
printed nothing (plain files read); lane/bf2 = 740b6d6 and lane/bf2-keys = 9eab9bd, both worktrees clean. Nothing was built,
run or launched. Paths are relative to /Users/boriskarpman/projects/RealTimeAudio.
PL = .harmony/.reports/s-rta-1004/plan-outputs.md (859 lines, read in full). Papers, verbatim, 4 of 4 seats, 31 attacks, 40240
characters, array closed: .harmony/.reports/s-rta-1004/attack-outputs-papers.md. Sheets: FO = facts-outputs.md, FR =
facts-resolume-screen-delay.md (same folder; their VERIFICATION sections override their bodies).
Labels: VERIFIED (re-read by me at 185147b, file:line, or re-computed), INFERRED, ASSUMED. Boris is quoted only verbatim, only
from .harmony/binding-decisions.md (BD) and .harmony/boris-feedback-backlog.md (BL).

---------------------------------------------------------------------------------------------------
## 0 VERDICT
---------------------------------------------------------------------------------------------------
RULED FIRST

(1) THE DELAY'S RANGE AND ITS MEMORY BILL. The Delay runs 0 to 100 ms in whole milliseconds, default 0: Resolume's range (FR Q1,
    CONFIRMED). PL's 0..500 with three tiers, a measured 60/120 publish class and a slider whose range moves with the canvas is
    OVERRULED. Reasons: Boris, BD:792-793: "I think we should copy what resolume does for delay. each output screen can be
    delayed and that is set on output display properties. Makes it simpler."; the 500 rested only on reading R11, which itself
    said "Resolume's own maximum is being looked up" (boris-clarify-38-41.md:26); his purpose, BD:802-803: "so the video matches
    the audio at the soundboard or wherever the vj is stationed in middle of room preferably" -- sound travels about 2.9 ms per
    metre (343 m/s), so 100 ms is 34 m between the speakers and his ears BEFORE the picture's own lateness is subtracted
    (analysis, the +1 frame of the output tap, the projector). His earlier dial's 500 belonged to a different mechanism (a
    signed audio-side offset) that he replaced: BD:795 "replace our sync with this".
    Because R11 told him 500, the range is put to him as question 51 (default A = 0..100; B = 0..250). The design makes either
    answer three constants (A-1).
    THE BILL: ONE ring shared by every output (PL's fork (c) stands), at ONE of two depths: 4 slots while no live output asks
    for a Delay (today, byte for byte), 18 slots while any does (sized for 120 publishes a second, never measured, so the
    applied Delay never changes by itself). Extra memory while a Delay is on, whatever the number of outputs:
    1280x720 49 MiB; 1920x1080 111 MiB; 2560x1440 197 MiB; 3840x2160 443 MiB. Above 4K the extra slots are cut to a 640 MiB
    budget (5120x2880: 11 extra, 619 MiB; 7680x4320: 5 extra, 633 MiB) and the Delay row shows, as a state, how far it can
    reach. Per output: zero beyond the first. Transient at a depth or canvas change: one retired generation for 120 publishes,
    never more than the newest retired one plus the one the front frame still names (A-2, A-3).

(2) HOW "THE SAME SCREEN" IS RECOGNISED. A screen's settings are keyed by the macOS display UUID and applied ONLY on an exact
    match of that key -- never by size, never by position, never through the Reopen ladder (PL's "matched to a record by the
    Reopen ladder's size rule" is OVERRULED). A screen without a usable UUID gets a fallback key that includes its place in
    the arrangement, so two identical projectors never share one record (PL's "geom:WxH@scale" is OVERRULED: it merged them).
    Two screens the Mac itself cannot tell apart (same maker, same model, no serial number) are DETECTED and the window says
    so as a state in the Device row: their settings follow the socket, and no software can see a cable swap between them --
    said in plain words to Boris (section 6, B-4), reported in /api/state, never left silent. A screen with any setting off
    its default shows that in the Outputs list itself, next to its name (A-10, A-12).

(3) HOW AN OUTPUT'S PICTURE IS PROVEN WITHOUT OPENING AN OUTPUT WINDOW. One function, `output::presentOutputFrame`, is the
    whole present step of a window, of Syphon, of the offscreen ctest and of the in-app probe. It is proven (a) offscreen in
    private GL contexts with an injected clock and frames whose pixels encode their number; (b) in the running app through
    the existing probe's private context, where a delayed probe's PNG must be byte-identical to the PNG a Delay-0 probe
    recorded for the very frame the log names; (c) by a path tag and a log-read counter that make "all default = today's
    plain blit of the newest frame" a claim a mutant can break (PL's byte comparison could not fail); (d) by source-law rows
    that pin the only lines no gate may run -- the window's own call site and the manager's hand-over; (e) by a pure function
    for "which stored settings go to which live screen". What is left is named for Boris in section 6, not implied by green
    rows (A-7, A-13).

VERDICT ON THE PLAN: SOUND IN ITS CORE, NEEDS REVISION. Of 31 attacks: 21 ACCEPT, 9 PARTIAL, 1 REJECT. What stands: the shared
ring deepened (fork (c)), the stamp at the moment a frame is offered, the colour arithmetic and its order, one 64-bit settings
word per output, Syphon as one more reader, a NEW settings.json key, the debounced store, one window with a list and a panel,
the source-law proofs, the fence with the beat-nudge lane. What changes: 17 amendments (section 3). The two findings that
would have shipped a defect: the torn-entry protocol accepts a torn pair (GL-6, proven by interleaving), and the window-law
guard fails on the settings window's own title (GA-6; .harmony/probe-outputs.py:134 flags any window whose name contains
"Output"). The two that would have shipped an unprovable claim: the identity gate and the age gate compared a function with
itself (GA-1, GA-2, GL-7).

STATUS: DONE (what only a run can establish is in section 8 "FACTS HARMONY MUST MEASURE", each with the ruling per outcome).

---------------------------------------------------------------------------------------------------
## 1 FACTS RE-DERIVED (every seat citation and every plan fact a ruling rests on, re-read or re-computed)
---------------------------------------------------------------------------------------------------
BORIS'S RECORD
RF-1  VERIFIED BD:792-794, BL:432: "I think we should copy what resolume does for delay. each output screen can be delayed and
      that is set on output display properties. Makes it simpler. These are the resolume screen output adjustment window".
RF-2  VERIFIED BD:795, BL:434: "replace our sync with this". BD:802-803, BL:436: "so the video matches the audio at the
      soundboard or wherever the vj is stationed in middle of room preferably".
RF-3  VERIFIED BD:832, BL:484: "yes add all of those output settings". BD:834, BL:485: "syphon is an output and treated with
      same output settings as a screen".
RF-4  VERIFIED boris-clarify-38-41.md:8-10, :26, :39: question 38's default A is "With the screen, as Resolume does: the app
      remembers it between launches, whatever show is open."; he did not name 38 (BD:839, default stands). Reading R11 as told:
      "The Delay runs from 0 to 500 ms in 1 ms steps; Resolume's own maximum is being looked up." He never spoke about the range.
RF-5  VERIFIED BD:864-897: questions 47, 48, 49 and 50 are ANSWERED in the record (47 default, 48 b, 49 default; 50 changed at
      12:52:00 to the show AND the computer, then 83 / 85: the show's keys take over). The dispatch lists them as open; the
      record is newer. None changes a line of this lane. Numbers 51-79 are reserved for the plans in flight (BL:567); this lane
      has 51..59.
RF-6  VERIFIED by reading the image boris-resolume/resolume-screen-delay.png: a panel "Screen"; Device is a drop-down reading
      "Display 2 (1920x1200)"; Delay "0 ms"; Opacity "100 %"; Brightness, Contrast, Red, Green, Blue "0".
RESOLUME
RF-7  VERIFIED FR:14 and FR VERIFICATION (CONFIRMED): "Each screen can have a delay between 0 and 100 ms, to account for small
      delays introduced by the signal chain after the outputs leave Resolume." 1 ms is a usable setting (FR:183, Arena 7.20
      notes). Arithmetic of the colour rows, the number scale, whether a preview or a recording is delayed: NOT DOCUMENTED.
ARITHMETIC (re-computed)
RF-8  Frame bytes at 4 B/pixel: 1280x720 3.52 MiB; 1920x1080 7.91; 2560x1440 14.06; 3840x2160 31.64; 5120x2880 56.25;
      7680x4320 126.56. GL-3's sums hold: 37 x 31.64 = 1171 MiB; 73 x 7.91 = 577; 124 x 7.91 = 981. SC-1's hold: 8 x 31.64 =
      253; 14 x 31.64 = 443.
RF-9  With K slots written round robin, the frame written at write n is rewritten at write n + K. Today's reader shows the
      front (written at m - 1, m = the latest write): it is rewritten 3 writes later (K = 4). Keeping that same margin for a
      history read: pickable iff m - n <= K - 3, so K - 3 completed frames are pickable and the oldest was offered K - 4
      publishes before the latest one. GUARANTEED reach = (K - 4) x publish interval. K = 18: 233 ms at 60 Hz, 116.7 ms at
      120 Hz, 97.2 ms at 144 Hz. (PL H-4's "K - 3 pickable" agrees.)
THE CODE AT 185147b
RF-10 VERIFIED src/output/SurfacePool.cpp:53-89: `ensure` creates every slot synchronously; on a failed create it releases the
      new generation, keeps the current one and returns false (:64-71); a replaced generation is pushed on `retired_` with no
      cap (:77-82); `tick` frees one after `kRetireFrames = 120` ticks (:91-107; SharedFrameSet.h:68) and runs only inside
      `publish` (SharedFrameSet.cpp:20). `next.gen = (current_.gen % 0xFFFFFF) + 1` (:74): the number is derived from the
      CURRENT generation.
RF-11 VERIFIED src/output/SharedFrameSet.cpp:19-22: `publish` calls `ensure` every time and returns while the pool's size is
      not the canvas's -- so a failed create is retried on every publish, and a failed size change freezes every output.
      :24-44: at a new generation the pending fence is deleted and `releaseGL` runs, so the copy made by the previous publish
      is NEVER offered: every generation change costs every output one repeated frame. :46-62: the stamp point for "offered"
      is the `front_.store` at :58. :64 `write_ = (write_ + 1) % kSlots`. `serial_` is never reset (:58, .h:146).
RF-12 VERIFIED src/output/OutputPresenter.cpp:34-74: `bindGeneration` releases, retains ALL slots and binds ALL slots (one
      `CGLTexImageIOSurface2D` + one FBO check each); :87-90 it runs whenever the front's generation changes, for every reader.
      :80-84 `presentSharedFrame` sets the framebuffer, the viewport, disables GL_SCISSOR_TEST and sets the clear colour and
      restores none of them. It has only ever run in a private context (OutputWindow.cpp:26, TestServer.cpp:1899).
RF-13 VERIFIED src/render/Renderer.cpp:685 and :829-837: `peak_frame_time_ms` spans renderStart..renderEnd and contains
      `publishToOutputs` (:819, comment :816-818). The recorder (:884-888, binds GL_READ_FRAMEBUFFER = canvasFBO_ itself),
      Syphon (:893-894) and the capture (:897) run AFTER renderEnd: they are in `peak_callback_ms` only (Renderer.h:437-440).
      `publishToOutputs` :966-972 returns unless an output is live or the test tap is forced. `publishSyphonFrame` :2576-2599
      restores only the framebuffer binding (:2595).
RF-14 VERIFIED src/test/TestServer.cpp:1850-1965: the probe runs on an HTTP thread under `probeMutex_`, in ONE private CGL
      context created with no share (:1871-1886), with ONE persistent `probeState_` (:1899) -- it cannot read a texture of the
      main context; its reader state already persists between probes.
RF-15 VERIFIED .harmony/probe-outputs.py:126-139, :654: the law guard fails when any on-screen Audio-DNA window's name equals
      the Output window's OR CONTAINS "Output" (:134-135), and when more than one Audio-DNA window sits at layer 0 (:137-139,
      :654). `o_tap_cost` is "REPORT, never FAILs; OPT-IN" (:67-69). So a window titled "Output Screens" trips BOTH checks.
RF-16 VERIFIED src/ui/MenuBarModel.cpp:148-149: "Syphon Output" is added always enabled; src/output/OutputMenuModel.h:43-58:
      the one list holds displays, "All Outputs Off", "Restore Last Outputs" only; `OutputMenuItem::shortcut` is a display-only
      text column (:21). :25-29 `displayLabel` is pinned by tests and by probe row o_state_displays (probe-outputs.py:53-56).
RF-17 VERIFIED src/MainComponent.cpp:2417-2419 (`addKeyListener(this)` on MainComponent), :3993-4046 (`keyPressed`, the panic
      chord at :4032-4036), MainComponent.h:77, :97 (KeyListener, `keyStateChanged`): keys reach the launcher and the panic
      chord only while the MAIN window is the key window. JUCE's own popup-menu windows carry `windowIgnoresKeyPresses`
      (build/_deps/juce-src/modules/juce_gui_basics/menus/juce_PopupMenu.cpp:379) and such a peer answers
      `canBecomeKeyWindow` false (juce_NSViewComponentPeer_mac.mm:1186-1188): a mouse-driven window that never takes the
      keyboard is JUCE's own practice. That a slider drags in such a window: INFERRED (no gate may drag; section 6 B-0).
RF-18 VERIFIED src/output/OutputTargets.h:17-27 (six-field fingerprint, all in `operator==`), OutputTargets.cpp:9-26 (Reopen
      = Exact, SameSize, Main), juce_Windowing_mac.mm:478-479 (JUCE's display list is `[NSScreen screens]` in that order),
      ColorSyncDevice.h:233 (`CGDisplayCreateUUIDFromDisplayID`) and NSScreen.h:58 (`localizedName`, macOS 10.15+) in the
      installed SDK. What a UUID survives, and whether two identical projectors get two: NOT VERIFIED by reading (measure M4).
RF-19 VERIFIED src/model/AppSettings.cpp:19-41: a file that does not parse to an object reads as empty and the next `update`
      rewrites it without its other keys; `replaceWithText` is the only write. facts-saves.md:93 says the same.
RF-20 VERIFIED docs/claude/pitfalls.md:142 (Pitfall 66): `ProjectMSource::render` "saves the caller's GL state as its FIRST
      statement ... and restores it on every return" -- the house precedent for code that leaves GL state behind in the main
      context. Pitfall 40 (:89): an output never touches the main context's objects and never blocks.
RF-21 VERIFIED rulings-bf2.md:128-140 (H-17): both sync-dial branches stay unmerged, kept as parts until this plan is ruled;
      Pitfall 68 is free.

---------------------------------------------------------------------------------------------------
## 2 ATTACK RULINGS (one row per attack; "decided by" = the line that settles it; A-n = the amendment in section 3)
---------------------------------------------------------------------------------------------------
| id | verdict | decided by | what it changes |
|---|---|---|---|
| GL-1 | ACCEPT | RF-10, RF-11: a failed create is retried on every publish and a failed size change freezes every output | A-3: a failed deep generation falls back to 4 slots at once; the failed (w, h, slots) is not retried for 300 publishes; `history.alloc_failed`; U-P4, U-G9 |
| GL-2 | PARTIAL | RF-12 (every reader re-binds ALL slots at a generation change), RF-13, RF-14 (PL's row saw only the writer) | Claim accepted. A-1 cuts K from 36/66 to 18. Remedy differs: eager binding stays (smallest change), the reader's bind is TIMED by the probe and barred (A-6, row oa_depth_spike); binding one slot per present is the pre-named fallback, not the design |
| GL-3 | ACCEPT | RF-10: `retired_` uncapped, freed only by `tick` inside `publish`; RF-8 sums | A-2, A-3: retired generations capped at the newest plus the one the front names; idle trim releases them; bars on peak (alloc + retired) and on retired == 0; U-P5 |
| GL-4 | ACCEPT | PL:145-146, :154-158, :232: a measured two-valued class moves the clamp; RF-9: above 120 Hz the ring holds less | A-1: no class, slots sized by a constant 120 Hz; the reach is REPORTED and shown as a state when it limits a Delay; U-H8 covers 144 Hz |
| GL-5 | ACCEPT | RF-12, RF-13, RF-20: the present step leaves GL state behind and would run in the Renderer's context for the first time | A-9: the Syphon call saves and restores every piece of state the present step touches; U-G8 compares a state snapshot; the canvas row captures 3 frames |
| GL-6 | ACCEPT | The interleaving: reader loads old word; writer stores new stamp; reader loads new stamp; reader re-loads old word; both reads match, a torn pair is accepted | A-4: the writer zeroes the word first, then stamp, then word; all six operations seq_cst; U-H10 is deterministic (three write steps, a read between each) plus a hammer case |
| GL-7 | ACCEPT | PL:703-705: `shown_age_ms` is "now minus the picked stamp" -- d by construction | A-13, row oa_delay_live: a delayed probe's PNG must equal, byte for byte, the PNG a Delay-0 probe recorded for the serial the log names; the age is measured from the Delay-0 series, not from the log |
| GA-1 | ACCEPT | PL:178-181: pick(now, d) is DEFINED by the threshold now - d; both sides of U-H2 call the one function | U-H2 becomes a brute-force oracle with hand tables (a query equal to a stamp, two equal stamps); M-H2 must fail the hand table |
| GA-2 | ACCEPT | PL:639-640, :700-702: at Delay 0 the pick path returns the front, so bytes and serial agree on both paths | A-7: `ShownFrame` carries a path tag and the count of log reads; U-ID asserts tag = blit and 0 log reads; a one-frame-lag mutant must go RED on serial |
| GA-3 | ACCEPT | PL:706-707: a 10 ms step per probe is smaller than the probe spacing, so the target never moves backwards; RF-14: the probe's reader state already persists | The row is folded into oa_delay_live: one step of the full range between two probes whose measured spacing is under a third of it; M-H4 must show a decrease |
| GA-4 | ACCEPT | PL:434-436 against PL:725-727 (`outputs.live == 0 throughout`): no row reaches the manager's hand-over or the window's atom | A-13: the feed for a row is NAMED (the tap route's `deep` flag); the join is a pure function with units (U-D5); source-law rows L-6, L-7; section 6 says in words what only Boris runs |
| GA-5 | ACCEPT | RF-13 (the window ends before Syphon; no reader is inside it), RF-15 (`o_tap_cost` never fails) | oa_depth_spike bars a DELTA of `peak_callback_ms` and of `peak_frame_time_ms` against no-change windows, plus the probe's own `bind_us`; oa_tap_cost is a NEW asserting row with its own RED arm |
| GA-6 | ACCEPT | RF-15 -- and stronger than claimed: the title "Output Screens" contains "Output" and trips the NAME check too | A-13: the app reports the settings window's Quartz window id; the sampler exempts exactly that id, and fails if that window's bounds equal a display's; a pure selftest with fake window lists is the RED arm |
| GA-7 | PARTIAL | RF-13: the recorder binds its own READ framebuffer and reads the canvas the capture rows prove | Accepted: the rows FAIL unless `GET /api/syphon` says available and enabled; a state-leak mutant; the state snapshot in U-G8. Rejected: a live recorder row (it would decode lossy video to re-prove the canvas); B-7 keeps the recording as a look |
| GA-8 | ACCEPT | PL:159-161 against :715-719 (no bar on retired, no peak); PL:651-652 ("REPORTED") | oa_memory bars the peak and the return to zero; U-G7's 2/255 is ASSERTED on a checkerboard-plus-ramp at one smaller and one larger target; a miss triggers the pre-named two-step path (A-8) |
| ST-1 | PARTIAL | RF-4 (38's default: remembered with the screen) stands; RF-16: nothing in the list he switches outputs with shows a setting | A-12: a screen with any setting off its default shows it in the Outputs list (the display-only text column) and in the window's list. NOT built: "Reset all screens", venue presets (section 9 of PL stands) |
| ST-2 | ACCEPT | PL:345-346 with RF-18: two identical screens have identical w, h, scale -- one fallback key, one record | A-10: the fallback key includes the origin; U-D2 gets the two-identical-screens case with the mutant "origin dropped" |
| ST-3 | PARTIAL | RF-6 (his screenshot's Device is a drop-down), RF-4, PL:352-354 | Accepted: the Device text adds the monitor's name and where the screen sits ("right of the main screen"), and the twin state. Not accepted as a menu: settings belong to the screen (38), so there is nothing to route; told to him as a reading |
| ST-4 | ACCEPT | src/output/OutputManager.h:98-103 (a concrete window, no seam); PL OA7 | A-10 (`resolveLooks`, pure, U-D5), A-13 (L-6, L-7, each live screen's applied settings in /api/state) |
| ST-5 | PARTIAL | RF-17: a second key window takes the launcher keys AND the Cmd+Shift+Esc chord away | Hazard accepted. Remedy differs: the window NEVER takes the keyboard (the output window's own flag), so nothing needs forwarding; values are set by dragging; the menu command raises it every time (A-12) |
| ST-6 | PARTIAL | RF-11 (a generation change repeats one frame on every output), A-1 (one edge instead of three) | The edge is crossed once per set-up (the shrink hold stays, A-5); creation stays on the GL thread exactly where a canvas change creates today; the row gets a drag sweep; off-thread creation is the pre-named fallback (A-6) |
| ST-7 | ACCEPT | PL:154-158, :232 | A-1: the applied Delay changes only by his hand or by a canvas size he chose; a limited Delay is a visible state (U-M4) |
| ST-8 | PARTIAL | RF-16; PL OA3 | Accepted: "Syphon Output" is greyed out when Syphon is not available; the window lists Syphon only when available, with its on / off state. Not accepted: moving the switch into the one list or into the window (the window names no opener and no switch, law L-1); told to him as a reading |
| SC-1 | ACCEPT | RF-1, RF-4, RF-7, RF-9 | A-1, A-2; question 51 |
| SC-2 | PARTIAL | RF-11 | Accepted: no publish class; H-5 and H-6 become one rule (the log is cleared at a generation change and after a gap over 1 s; with nothing pickable the reader shows the front, as today). Rejected: deleting the shrink hold -- each depth change repeats a frame on every output and allocates on the GL thread, and a drag through 0 would do both twice |
| SC-3 | ACCEPT | RF-3, PL:357-362, :414-417 | A-11, A-12: a record is {id, seven numbers}; no name, size, day stamp or cap; the window lists connected displays and Syphon only; V9, U-M3, U-S4 go |
| SC-4 | ACCEPT | RF-19, RF-5 (the file's other users are now the saves lane's subject) | A-15: the settings.json mend goes to the saves lane as a named finding; the two stale-doc fixes are Harmony's decision HD-4 (default: a docs chore after this lane); the test count stays in S7 |
| SC-5 | ACCEPT | PL:772-780 | A-14: hold on a raise, Syphon opacity toward black, not bindable -- ruled here; Device as text -- a reading; ONE question (51, the range) |
| SC-6 | PARTIAL | GL-7, GA-3 | Folded, no row or build of their own: the monotonic check (into oa_delay_live) and the colour pixels (into oa_canvas_untouched's positive control). Kept and rebuilt: the live age row. Visual gate: 9 states (A-12) |
| SC-7 | ACCEPT (NIT) | PL:579, :607; RIG-RULES A2 (two builders never share a worktree) | A-16: (S1, S2, S3) and (S4, S6) in two worktrees; the visual gate starts on the S6 build; S5 merges both |
| SC-8 | REJECT | RF-14: the probe's context shares nothing, so it cannot read `syphonTexture_`; a counter alone would leave the Syphon picture unproven by any machine | `/api/syphon_probe` stays (a read-back on the GL thread); PL's route count stands |

Seats that conflict, reconciled: (i) SC-6 (drop the live age row) against GL-7 (make it decode pixels): GL-7 wins -- only the
running app has real fences, the real clock and a reader on another thread; the row is rebuilt so that it can fail. (ii) SC-2
(no hysteresis) against ST-6 (no hitch on a drag): ST-6 wins for the one remaining edge. (iii) GL-2 / ST-6 (lazy binding and
off-thread creation as the design) against "the smallest change that is correct": measured first (M3), the fallbacks named,
the bars never loosened. (iv) ST-5 (forward the keys) against PL W-1 (a normal window): neither -- the window never takes
the keyboard. (v) ST-1 (unannounced carry-over) against SC-3 (store less): both hold -- the record is smaller AND its effect
is visible in the list.

---------------------------------------------------------------------------------------------------
## 3 AMENDMENTS (numbered; each OVERRIDES the plan body where they differ)
---------------------------------------------------------------------------------------------------
Harmony constraint (lane outputs): the delay never blocks, waits or allocates per frame on the shared GL thread; its memory is
bounded, stated per output and per canvas size, and clamped or refused by a stated rule when it would exceed the budget; an
output at Delay 0 with every colour setting at its default shows exactly what it shows today (the same newest frame, no added
copy, no added frame of latency) and a gate proves it; the in-app preview, recordings, snapshots and the Eyes capture are
never delayed or coloured by an output's settings; a screen's settings are per machine, never in the show file.

A-1 RANGE AND DEPTH (overrides PL R-1, R-2, R-3, the tier table, `kMaxSlots = 66`, `PublishClass`, `DelayTier`, `maxDelayMs`).
 Constants, all in src/output/FrameHistory.h: `kMaxDelayMs = 100`; `kSizingHz = 120`;
 `kDeepExtra = ceil(kMaxDelayMs x kSizingHz / 1000) + 2 = 14`; `kSlots = 4` (unchanged: the base depth);
 `kMaxSlots = kSlots + kDeepExtra = 18`; `kHistoryBudgetBytes = 640 MiB` (for the EXTRA slots).
 Two depths only. Shallow = 4 slots. Deep = 4 + extraSlots(w, h), extraSlots = min(kDeepExtra, kHistoryBudgetBytes / (w x h x 4)),
 read as 0 when it comes out under 2.
   canvas       extra  deep slots  deep total  of which extra  guaranteed reach at 60 / 120 / 144 Hz
   1280x720      14       18        63.3 MiB     49.2 MiB       233 / 116 / 97 ms
   1920x1080     14       18       142.4        110.7           233 / 116 / 97
   2560x1440     14       18       253.1        196.9           233 / 116 / 97
   3840x2160     14       18       569.5        443.0           233 / 116 / 97
   5120x2880     11       15       843.8        618.8           183 /  91 / 76
   7680x4320      5        9      1139.1        632.8            83 /  41 / 34
 (reach = extra x publish interval, RF-9.) The same for 1, 2, 3 outputs and Syphon: the ring is shared.
 The Delay slider is ALWAYS 0..100 ms. There is no separate "applied" value and no moving slider range: the reader asks for the
 stored Delay and the ring answers with the newest frame at least that old, or the oldest frame it safely holds (A-4).
 `reachMs` = extra slots x the measured publish interval (a smoothed average) is REPORTED (/api/state) and, only when the
 stored Delay exceeds it, SHOWN as a state beside the value: "limited to N ms" (A-12). With a canvas up to 4K and a preview
 display up to 120 Hz it never shows. It is a pure function of the canvas size and the interval, whatever the ring's depth
 at that moment; before any publish was measured the interval of `kSizingHz` is used.
 Question 51 = B (0..250): `kMaxDelayMs = 250`, hence `kDeepExtra = 32`, `kMaxSlots = 36`; U-H8's table, oa_memory's numbers and
 the slider's range change; nothing else. Bill then: 1080p 36 slots = 284.8 MiB; 4K is cut by the budget to 20 extra = 24
 slots = 759.4 MiB, reach 333 ms at 60 Hz and 167 ms at 120 Hz (the limited state would show at 120 Hz).
 Measure M1 above 125 publishes a second on Boris's Mac: `kSizingHz` is raised to the next of 144, 165, 240 before S1 starts
 and the table is recomputed (one constant).

A-2 THE MEMORY BILL (overrides PL R-4).
 Steady: A-1's table. Per output beyond the first: zero. The colour stage: zero (one pass), or one canvas frame per coloured
 output if A-8's fallback is taken.
 Transient: at a change between shallow and deep, or of the canvas size, the replaced generation is retired for
 `kRetireFrames = 120` publishes (2 s at 60 Hz, 1 s at 120 Hz; unchanged constant), then freed. The pool never owns more than:
 the current generation + the newest retired one + the one the front frame still names (that third one only for the few
 publishes until the new generation's first frame is offered).
 Worst cases, stated: 1080p shallow <-> deep: 22 slots = 174.0 MiB. 4K shallow <-> deep: 22 slots = 696.1 MiB. 1080p deep ->
 4K deep: 142.4 + 569.5 = 711.9 MiB. 4K deep -> another 4K-sized deep canvas: 2 x 569.5 = 1139 MiB for 120 publishes.
 A reader keeps the generation it is bound to alive until its next present (its own CFRetain, as today): one present for a
 live window; the test probe's reader until the next probe call (test mode only).
 Idle: when the tap stops and the pool is deep or holds a retired generation, the pool is emptied at the next render frame
 (A-3). A pool that never went deep is not touched: today's 4 slots stay, as today.

A-3 POOL RULES (overrides PL's SurfacePool change list and R-5's trim).
 - `ensure(int w, int h, int slots = kSlots)` returns Unchanged, Created or Failed. A different slot count is a new generation,
   like a new size. `Generation` holds `slots` and `s[kMaxSlots]`.
 - Generation numbers come from their own counter (`lastGen_`), never from `current_.gen` (RF-10): after the pool is emptied
   the next generation must not reuse a number a still-bound reader holds, or that reader would keep showing surfaces nobody
   writes. U-P6.
 - Failure (GL-1): a failed create of (w, h, slots) is remembered for 300 ticks; `ensure` for the same triple answers Failed
   without creating until then. `publish`: deep Failed -> asks for (w, h, 4) in the same publish and counts `alloc_failed`;
   (w, h, 4) Failed -> today's path (return; outputs keep the last frame) but without a create per publish.
 - Retired cap (GL-3): `trimRetired(uint32_t keepGen)` after every Created releases every retired generation except the
   newest retired one and `keepGen` (the generation `front()` names). U-P5.
 - `allocBytes()`, `retiredBytes()` (sums of `IOSurfaceGetAllocSize`), `slotCount()`; `retainSurface` checks the slot against
   the generation's own count.
 - `releaseAll()`: the current and every retired generation. `SharedFrameSet::trimIdle()` (context current): `releaseGL()`,
   `pool_.releaseAll()`, `front_` back to 0 ("nothing published", the state before the first publish ever), the log cleared.
   It runs from `Renderer::publishToOutputs`' early-return branch only when `needsIdleTrim()` is true (one relaxed load per
   frame otherwise).
 - A create function pointer replaceable by tests (default: today's `createSurface`).
 - No new mutex. The pool's existing mutex is taken where it is taken today (a generation change, `retainSurface`, `tick`
   while a retired generation ages) and by `trimRetired` / `releaseAll` on those same rare paths; never on the steady frame path.

A-4 THE LOG AND THE PICK (overrides PL H-1..H-7).
 `FrameLog` (FrameHistory.h, no GL): 32 entries, each {`std::atomic<uint64_t> word` (the existing `packFront` word),
 `std::atomic<int64_t> stampUs`}; `writeSeq` (count of slot writes) and `floorSeq`, both atomic. Entry index = write index mod 32.
 Writer (GL thread, inside `publish`):
   `noteWrite()` at every blit: `writeSeq` + 1; that entry's word = 0.
   `offer(seq, word, stampUs)` when a copy is offered as front (RF-11's line): that copy's entry: word = 0, then the stamp,
   then the word. The stamp is `frameClockUs()` read at the `front_.store`.
   `clear()`: `floorSeq = writeSeq + 1`. Called at every generation change and when more than 1 s passed since the last offer.
 Every load and store of an entry is seq_cst (the default). Proof, by total order: a reader accepts (w1, s) only if w1 != 0 and
 the word read again after the stamp equals w1; serials never repeat (RF-11); so an accepted pair was written whole (GL-6's
 interleaving ends with the second read seeing 0 or a new word). No weaker ordering without a new proof in the header.
 Reader: `pick(nowUs, delayUs, slots, minSerial)` -> nothing, or {word, stampUs}. Candidates = entries whose write index n lies
 in [max(floorSeq, m - (slots - 3)), m] (m = `writeSeq`), accepted by the pair rule, with serial >= minSerial. Result = the
 newest candidate with stamp <= now - delay; if none, the OLDEST candidate; if there is no candidate, nothing.
 That ONE rule is: H-1 (a delayed output shows what a Delay-0 output showed d ago); H-2 (still filling: the oldest frame held,
 a still picture, never black); H-3 (`minSerial` = the serial this reader showed last: a raised Delay holds the shown frame
 until the ring catches up, a lowered one jumps forward at once, never backwards; no slew -- RULED, was question 53); H-4 (the
 guard counts WRITES, so a copy that was written and never offered still protects its slot).
 Nothing pickable (the log just cleared) -> the reader shows `front()` exactly as today. Stated consequence, replacing H-5:
 at a canvas size change or a depth change a delayed output jumps to the newest frame, then holds the new generation's first
 frame for its Delay -- at most 100 ms each, never black. Replacing H-6: an output opened after the tap was off shows the old
 last frame for the one or two publishes today's output shows it, never for its whole Delay.
 Clock: one function `output::frameClockUs()` (steady clock) for the writer and every reader; a pointer tests replace.
 Cost: a scan of at most 15 entries, no allocation, no lock, no wait.

A-5 WHO ASKS FOR DEPTH, AND THE SHRINK HOLD (overrides `setWantedDelayMs` and `HistoryDepthPolicy`'s tiers).
 `SharedFrameSet::setDeepWanted(DeepUser who, bool on)`: one atomic bit mask, set with fetch_or / fetch_and. Users: Windows
 (OutputManager, message thread: some live window's Delay > 0); Syphon (the GL thread, from two atoms: enabled-and-initialised
 and its settings' Delay > 0; written only when the answer changes); Test (8080 `set_output_tap`, new optional field `deep`).
 `publish` reads the mask once.
 `DepthPolicy` (pure): deep at once when any bit is set; shallow only after 600 consecutive publishes with no bit set, or at
 the idle trim. The hold stays (SC-2 overruled on this point) because every depth change repeats one frame on every output
 and creates surfaces on the GL thread (RF-11): a drag through 0 must not do that twice.

A-6 BINDING AND CREATION (overrides PL:167-175).
 Writer: `tex_` / `fbo_` sized `kMaxSlots`; it binds the generation's own slot count; `write_ = (write_ + 1) % slots`.
 Reader: `PresenterGLState` arrays sized `kMaxSlots`; `bindGeneration` binds the generation's own slot count, eagerly, as today.
 Per frame in steady state: writer + three atomic stores and one clock read; a delayed reader + one scan; a default reader
 nothing. At a depth change, once: `slots` surfaces created and bound by the writer, `slots` bound by each reader at its next
 present. MEASURED, not assumed (M3, row oa_depth_spike). Pre-registered outcome rule: bars met -> this stands. Reader bar
 missed -> stage S2b: a reader (and the writer) binds only the slot it is about to use, at most one new slot per present,
 with the mutant "binds all up front". Creation bar missed -> stage S1b: `SurfacePool::prepare(w, h, slots)` builds the next
 generation on the message thread and `publish` only adopts it. The bars stay as registered.

A-7 THE DEFAULT PATH AND ITS PROOF (overrides PL OA2 "ALL-DEFAULT" layer 1 and U-ID).
 Signature:
 `bool presentOutputFrame(SharedFrameSet&, PresenterGLState&, uint64_t packedLook, int64_t nowUs, unsigned targetFBO, int targetW, int targetH, ShownFrame* shown = nullptr)`.
 `isDefault(look)` -> it calls `presentSharedFrame` and returns. `presentSharedFrame` keeps its signature; nothing in its body
 changes; `bindGeneration`'s loops take the generation's slot count instead of `kSlots`.
 `ShownFrame {gen, serial, slot, stampUs, path, logReads, bindUs}`; `path` is None, Blit (the default path), Pick (a delayed
 frame, presented with the SAME GL_LINEAR blit, no shader) or Quad (through the colour pass); `logReads` = log entries read by
 this present (0 on the default path); `bindUs` = time spent binding during this present (0 when none).

A-8 THE COLOUR STAGE (PL OA2 stands, with these changes).
 - The word: Delay 0..100 (its field keeps 9 bits, so question 51 = B needs no re-pack), Opacity 0..100, five rows -100..100.
 - Opacity fades RGB toward black on a screen AND on Syphon; alpha is written as read (RULED; was question 52).
 - U-G7's scaled-target bar is ASSERTED (GA-8). Pre-registered outcome rule (M5): met -> one pass stands. Missed -> the
   two-step path: the colour pass runs 1:1 (texel fetch) into one canvas-sized target per coloured reader, then the SAME
   GL_LINEAR blit as the default path; bill + one canvas frame per coloured output (7.9 MiB at 1080p, 31.6 MiB at 4K), created
   when the first colour setting leaves default, never per frame. PL R8's "always use the quad" is OVERRULED: it would have
   changed what a default screen shows.
 - Shader source: `EmbeddedShaders::outputPresent` if the test target can include that header alone (PL N7); else a string in
   OutputPresenter.cpp. The builder reports which; the docs say which.

A-9 SYPHON (PL OA3 stands, with these changes).
 - The non-default branch of `publishSyphonFrame` saves and restores everything the present step touches: the draw and read
   framebuffer bindings, the viewport, the scissor, blend and colour-mask state, the clear colour, the current program, the
   vertex-array binding, the active texture unit and the rectangle-texture binding on it (Pitfall 66's precedent, RF-20). The
   all-default branch is today's body, untouched. The present step itself SETS what it needs (blend off, full colour mask,
   scissor off) and assumes nothing.
 - U-G8 runs in the WRITER's context: the Syphon reader lives in the context that writes the slots, so a second rectangle
   texture on the same surface in one context is ASSUMED to work until that case passes.
 - The tap also runs while Syphon is enabled, initialised and its settings are not default (one more relaxed load in the guard,
   Renderer.cpp:968).
 - Stated consequence, unchanged: with any Syphon setting off default, Syphon shows the front frame -- one canvas frame later
   than today's direct blit; /api/state says so (`outputs.syphon.reader`).
 - When Syphon stops reading (switched off, or its settings back at default) the Renderer releases `syphonReader_` at the next
   frame, before the idle trim can run: a reader that is not presenting must hold no surface (A-2).
 - `/api/syphon_probe` (8080) stays: a read-back of `syphonTexture_` on the GL thread with its own pending flag and promise. It
   is NOT a branch of `processPendingCapture` (L-5 keeps that function free of anything output-related).
 - Every Syphon row FAILS with "syphon not running" unless `GET /api/syphon` answers available and enabled -- never a silent pass.
 - "Syphon Output" in the Output menu is greyed out when Syphon is not available (one new callback beside
   `isSyphonOutputEnabled`, MenuBarModel.cpp:148-149). The switch stays that menu item; Syphon is still OFF at every launch.

A-10 IDENTITY OF A SCREEN (overrides PL OA4 "IDENTITY").
 - src/output/DisplayIdentity.h (pure) and .mm: for each entry of `[NSScreen screens]`: `ScreenIdentity {uuid, name, vendor,
   model, serial}` (`CGDisplayCreateUUIDFromDisplayID`, `localizedName`, `CGDisplayVendorNumber`, `CGDisplayModelNumber`,
   `CGDisplaySerialNumber`; CoreGraphics and AppKit, no new dependency). Message thread only.
 - Joined to `DisplayInfo` by index and checked by the logical bounds. A mismatch (the two lists were read across a display
   change) = no identities this tick: the caller keeps its previous result and tries at the next tick. Never a guess.
 - `screenKey(...)` (pure): the UUID when it is not empty and unique in the list; else "geom:WxH@scale@x,y". `DisplayInfo`
   and its `operator==` are not touched; the wanted-set, the match ladder and Restore keep the fingerprint (PL:347-349 stands).
 - `twin` (pure): true for a screen that shares maker and model with another connected screen while both report serial 0,
   and for every screen on the fallback key.
 - `resolveLooks(displays, identities, records, liveFlags)` (pure): for each display its key, its settings word -- the record
   with EXACTLY that key, else the default word -- and its twin flag; plus `anyLiveDelay`. No loose rung of any kind.
 - Applied by `OutputManager::applyLooks()`: after every `openWindow` (the menu, Restore, the hot-plug reopen), at the end of
   `reconcile`, and from the store's change callback: each live window gets `setLook(word)` (one atomic store), then
   `frames_.setDeepWanted(DeepUser::Windows, anyLiveDelay)`.
 - Outcome rule for M4 / B-4: if one of Boris's screens comes back with another UUID after a re-plug or a restart,
   `screenKey` prefers "edid:maker-model-serial" when the serial is not 0 -- one function and its unit case, on Harmony's word.

A-11 THE STORE (overrides PL OA4 "THE SCHEMA"; PL's "WHO WRITES" stands).
 `AppSettings::kOutputSettings = "outputSettings"`: {"version": 1, "screens": [{"id", "delayMs", "opacity", "brightness",
 "contrast", "red", "green", "blue"}], "syphon": {the seven numbers}}. No name, no size, no day stamp, no cap, no eviction. A
 record whose seven numbers are all default is left out on write. Reader as PL:360-362. AppSettings.h gains the one constant;
 AppSettings.cpp is NOT edited by this lane (A-15). If question 38 is ever re-answered, only this store's place changes.

A-12 THE WINDOW AND THE LIST (overrides PL W-1, W-3, W-4's range and text box, W-5, and the V list).
 - It NEVER takes the keyboard: `OutputSettingsWindow`'s peer carries `windowIgnoresKeyPresses`, added and re-checked exactly
   as `OutputWindow` does (OutputWindow.cpp:52-60, :79-85). The clip-launcher keys and Cmd+Shift+Esc keep working while it is
   open and after a slider was touched (RF-17). Values are set by dragging; the value texts are read-only; a right-click
   resets a row (`ResettableSlider`, `setDefaultValue`). Typing a number is not in this lane.
   Fallback, named: if B-0 shows a slider cannot be dragged in such a window, the window becomes a normal one and registers
   MainComponent as its KeyListener (it is one, RF-17), with a law row that the registration exists; Harmony rules that then.
 - Title "Output Screens", native title bar, normal level, closable. The menu command shows it and calls `toFront(false)`
   every time, so it comes above an output that covers the display (PL R11 is thereby solved, not deferred).
 - The list: every connected display in menu order, then "Syphon" when available. No entry for a screen that is not connected.
   Each entry: the menu's own label, a state word "on" / "off", and the settings text when it is not empty.
 - The settings text (pure `settingsStateText(word)`): "" when all default; "N ms" when only the Delay is set; "adjusted" when
   only colour rows are; "N ms, adjusted" when both. The SAME text goes into the display item's display-only text column of
   the ONE item list (`OutputMenuItem::shortcut`, RF-16): he sees a remembered Delay where he switches the screen on. Labels,
   ids, order and the /api/state labels do not change.
 - The panel "Screen", eight rows in Resolume's order. Device is read-only text: the menu label, the monitor's name, and the
   place -- "main screen", or "left of / right of / above / below the main screen" (pure, from the two rectangles). For a twin
   a second line: "Same model as Display N: settings follow the socket." Syphon: "Syphon (Audio-DNA)".
   Delay: slider 0..100 ms, default 0, value "N ms"; beside it, only while limited, "limited to N ms". Opacity 0..100 %,
   default 100. Brightness, Contrast, Red, Green, Blue -1.00..1.00, default 0. Rows are editable while the output is off.
 - Model-driven at 10 Hz, repaint or slider move only when the paint key changes (PL W-6; Pitfalls 41, 57, 59).
 - Not bindable, not mappable, not recorded, no 7070 setter (RULED; was question 54).
 - VISUAL GATE, nine states (test mode; the display list, identities, live flags, reach and Syphon state are FIXTURES handed to
   the model only; nothing in the route can open an output):
   V1 the Outputs list (TopBar door): three displays, the second with "40 ms, adjusted" in the text column; last item
      "Output Screens...".
   V2 the window: three displays, the second "on" and selected, every row at default; the first shows "40 ms" in the list.
   V3 every row off default (Delay 40 ms, Opacity 60 %, Brightness -0.20, Contrast 0.35, Red -0.10, Green 0.05, Blue 0.40).
   V4 Delay 100 ms with a reach of 76 ms: "limited to 76 ms".
   V5 twins: two entries "Display 2 (1920x1080)" and "Display 3 (1920x1080)"; the Device row's second line.
   V6 Syphon selected, "off", all default.
   V7 Syphon selected, "on", values off default.
   V8 the window at its minimum size, a long monitor name, one row just reset beside changed rows.
   V9 the window beside the main window whose inspector shows the composition's own "Output Settings" section.
   V1's list is a fixture copy shown with `showMenuAsync` and the top-level parent, as the TopBar door shows the real one; its
   pick callback does nothing. The native menu bar's Output menu cannot be captured without synthetic input: its two changes
   (the new last item, the greyed "Syphon Output") are proven by U-MENU1 and U-MENU2.

A-13 PROOF WITHOUT AN OUTPUT WINDOW (overrides PL OA7 items 3, 5 and 7).
 - `output_probe` (8080): new optional fields `look` (the seven numbers; absent = default) and `reader` (0 or 1: two persistent
   reader states, so a Delay-0 series and a delayed series never share the "last shown" floor). The answer adds
   `shown_serial`, `shown_stamp_us`, `path`, `log_reads`, `bind_us`, `history_slots`. The look travels in the request.
 - `set_output_tap` (8080) gains optional `deep` (`DeepUser::Test`). This is the NAMED feed of the live rows. It does not pass
   through OutputManager, and no row is read as proof of the manager's hand-over.
 - `POST /api/output_settings` (8080): writes a screen key's or "syphon"'s settings through the real `OutputSettingsStore`.
 - `POST /api/output_settings_window` (8080): open / close / select / model fixtures; answers the window's Quartz window id.
 - `POST /api/syphon_probe` (8080): A-9.
 - `/api/state.outputs`, 7070 and 8080, read-only: `history {slots, alloc_bytes, retired_bytes, peak_bytes (the largest
   alloc + retired since the previous read), reach_ms, alloc_failed}`; `syphon {available, enabled, reader, settings}`; per
   display `key`, `twin`, `settings`, and `applied` -- the word in the LIVE window's atom, absent without a window -- so
   Boris's own running session shows what each screen was actually given.
 - The law guard (.harmony/probe-outputs.py): exempt exactly the window id the app reported for the Output Screens window;
   FAIL if that window's bounds equal a display's; every other rule as today (RF-15). The decision becomes a pure function
   with a selftest fed made-up window lists.
 - Source law, tests/test_output_law.cpp: L-1..L-7 (section 5).

A-14 QUESTIONS (overrides PL section 7). PL's 52, 53, 54 are NOT asked: ruled in A-8, A-4, A-12. PL's 51 (Device as a menu) is
 NOT asked: told as a reading. The one question is the range (section 7).

A-15 MOVED OUT OF THIS LANE (overrides PL OA4 "MEND", OA8 "CARRIED", S7's carries).
 - The settings.json mend: to the saves lane, as finding SF-1 (section 9).
 - docs/claude/testing-eyes.md:72 and docs/claude/architecture.md:107, :231, :334: Harmony's decision HD-4.
 - .harmony/APP-INVENTORY.md's test count stays in S7: this lane changes it.

A-16 STAGES AND ORDER: section 4 replaces PL section 4.

A-17 PITFALL NN (Harmony assigns 68), text replaced: "An output's Delay and colour live only in its own present step
 (`output::presentOutputFrame`): the ring is the shared IOSurface pipeline at one of two depths (4, or 18 while a live
 output asks for a Delay; one copy per frame whatever the number of outputs), read by TIME through `FrameLog` (word zeroed,
 stamp, word; a reader accepts a pair only when the word reads the same twice); all-default is today's plain blit of
 `front()`; a generation number is never reused; the Output Screens window never takes the keyboard; the canvas, the preview,
 the recorder, snapshots and captures read the CANVAS and are never delayed or coloured; a screen's settings are keyed by
 display UUID, applied only on an exact key match, kept in settings.json `outputSettings`, never in a show -- before
 touching `presentOutputFrame`, `SharedFrameSet::publish`, `SurfacePool`, the Syphon publish, the settings window, or adding
 a canvas reader."

---------------------------------------------------------------------------------------------------
## 4 FINAL STAGES + ORDER (one builder context per stage; every live row and every gate verdict is Harmony's)
---------------------------------------------------------------------------------------------------
A builder builds, runs ctest and its named mutants (RED first) and reports. A builder never launches the app for a gate row,
never opens an Output window, never drives the mouse or keyboard, never touches Boris's running Audio-DNA or Resolume.

G0 (Harmony, on main, before any stage, no code).
   M1 row oa_publish_rate: test mode, `set_output_tap` on, the preview on the built-in display, `/api/state.outputs.frame_serial`
      read twice 10 s apart. It fixes `kSizingHz` for the S1 packet (A-1).
   M2 `GET /api/syphon` on the main build: is Syphon available on this rig (HD-3).

S1 HISTORY CORE (pure + pool). Branch lane/outputs-core, worktree A.
   Owns: NEW src/output/FrameHistory.h (the constants, `FrameLog`, `pick`, `extraSlots`, `reachMs`, `DepthPolicy`);
   NEW src/output/OutputLook.h (the word only: `pack`, `unpack`, `isDefault`, clamps, `settingsStateText`);
   src/output/SharedFrameSet.h (the SurfacePool part only); src/output/SurfacePool.cpp; NEW tests/test_frame_history.cpp,
   tests/test_output_look.cpp; tests/test_surface_pool.cpp; tests/CMakeLists.txt.
   Proves: U-H1..U-H10, U-P1..U-P6, U-L1, U-L4, U-L5, U-L6. The app's behaviour is unchanged (nobody asks for depth).

Then two worktrees side by side, both branched from S1's head (RIG-RULES A2: two builders never share a worktree).

S2 WRITER + READER. Worktree A.
   Owns: src/output/SharedFrameSet.h (the SharedFrameSet part) / .cpp; src/output/OutputPresenter.h / .cpp;
   tests/test_shared_frame_gl.cpp.
   Proves: U-ID, U-G1..U-G4, U-G9, U-G10; the eight existing cases green and unedited (`kSlots` is still 4, :366 holds). Every
   caller still passes a default look: identity by construction.
S3 COLOUR. Worktree A, after S2.
   Owns: src/output/OutputLook.h (adds `applyReference`); src/render/EmbeddedShaders.h (one string, or none: A-8);
   src/output/OutputPresenter.cpp (the program, the Quad path); tests/test_output_look.cpp, tests/test_shared_frame_gl.cpp.
   Proves: U-L2, U-L3, U-G5..U-G8. Reports M5's number in its lane report.

S4 IDENTITY + STORE. Branch lane/outputs-ui, worktree B.
   Owns: NEW src/output/DisplayIdentity.h / .mm; NEW src/output/OutputSettingsStore.h / .cpp; src/model/AppSettings.h (ONE key
   constant; AppSettings.cpp untouched); NEW tests/test_output_settings_store.cpp, tests/test_display_identity.cpp; root
   CMakeLists.txt (the new sources); tests/CMakeLists.txt.
   Proves: U-S1, U-S2, U-S3, U-S5..U-S9, U-D1..U-D5.
S6 THE WINDOW AND THE LIST. Worktree B, after S4.
   Owns: NEW src/ui/OutputSettingsWindow.h / .cpp, src/ui/OutputSettingsModel.h; src/output/OutputMenuModel.h (the text column,
   the new last item); src/ui/MenuBarModel.h / .cpp (`kOutputSettings` APPENDED after `kOutputRestoreLast`; the Syphon item's
   enablement); src/MainComponent.h / .cpp (owns the store and the window; one `case`; the menu callbacks; the store's flush in
   the shutdown order before `outputs_.shutdown()`); src/test/TestServer.h / .cpp (ONLY the routes `output_settings` and
   `output_settings_window`; the second also shows / dismisses a FIXTURE copy of the Outputs list for V1, whose pick callback
   does nothing); NEW tests/test_output_settings_model.cpp; tests/test_output_menu_model.cpp; tests/test_output_law.cpp
   (L-1, L-2).
   Proves: U-M1, U-M2, U-M4..U-M9, U-MENU1, U-MENU2, L-1, L-2. In this worktree a slider writes the store only; "on" / "off"
   and the reach come from fixtures.
   Then the VISUAL GATE (Harmony), on worktree B's build, while S2 / S3 run: a capture builder produces V1..V9 (test mode,
   captures by Quartz window id only, no synthetic input, no Output window), five critic seats review, Harmony rules.

S5 WIRING. Worktree A, after S3 and after S6. Step 0: merge lane/outputs-ui into lane/outputs-core (a conflict is the
   builder's, RIG-RULES A2).
   Owns: src/ui/OutputWindow.h / .cpp (the look atom, `setLook`, the call of `presentOutputFrame` with `frameClockUs()`);
   src/output/OutputManager.h / .cpp (`applyLooks`, identities at open and at reconcile, `DeepUser::Windows`, the state fields);
   src/render/Renderer.h / .cpp (the tap guard, the idle trim, the Syphon branch with its state save / restore,
   `syphonReader_`, `setSyphonLook`, the syphon_probe read-back, `DeepUser::Syphon`); src/test/TestServer.h / .cpp (the probe's
   fields, `deep`, `syphon_probe`, state); src/api/ApiServer.cpp (read-only state fields); src/MainComponent.cpp (the store's
   callback to `applyLooks` / `setSyphonLook`; the model's live feeds); tests/test_output_law.cpp (L-3..L-7);
   .harmony/probe-outputs.py / .sh / .json (the new rows, the guard function and its selftest -- written by the builder, RUN
   by Harmony).
   Proves by ctest: L-3..L-7; by script selftest: the guard function.
   This is the first stage that changes what the app can do.

S1b / S2b: exist only if oa_depth_spike misses a bar (A-6). Same owner files as S1 / S2.

S7 DOCS, after the gates. Owns: docs/claude/integration.md ("Output windows": the settings, the key, the window, the routes),
   docs/claude/rendering.md ("The output tap": the two depths, the log, the present step), docs/claude/pitfalls.md (Pitfall
   68, A-17), docs/claude/testing-eyes.md (the new 8080 routes and fields only), CLAUDE.md (the "Outputs" pattern line -- the
   settings window never takes the keyboard, the list's settings text; Key capabilities; the pitfall index line),
   .harmony/APP-INVENTORY.md (the Output menu row; the test count recounted with `ctest -N`). No source file.

ORDER: G0 -> S1 -> [ (S2 -> S3)  beside  (S4 -> S6 -> visual gate) ] -> S5 -> Harmony's rows -> (S1b / S2b if needed) -> S7 ->
merge -> Boris (section 6). With one worktree only (HD-2): S1 -> S2 -> S3 -> S4 -> S6 -> visual gate -> S5 -> the rest.

WHAT HARMONY RUNS HERSELF, AND WHEN
 - before S1: G0 (M1, M2).
 - after S3: U-G7's number re-witnessed (M5); she rules A-8's outcome.
 - after S6: the visual gate.
 - after S5, in this order: oa_depth_spike (M3: it decides whether S1b / S2b exist); oa_identity_default; oa_delay_live;
   oa_canvas_untouched; oa_syphon_look; oa_memory; oa_tap_cost; oa_settings_roundtrip; oa_identity_report (M4, INFO); oa_tsan;
   every existing o_* row; then the live RED arms -- seven mutant builds, one at a time, each followed by the restore check of
   RIG-RULES A ("after a mutant"): M-SLEEP, M-LAG, M-G1, M-STAMP, M-CANVAS, M-P5, M-BLIT2 (section 5).
 - then the merge decision, the post-merge ctest and rows, and only then Boris.

FENCE with the beat-nudge lane: PL:548-557 stands. Added to this lane's side: src/output/OutputMenuModel.h and the Output case
of src/ui/MenuBarModel.cpp. Still untouched by this lane: src/ui/TopBar.h / .cpp, src/ui/TopBarModel.h, src/analysis/**,
FeatureSnapshot, src/recording/**, src/model/AppSettings.cpp.

---------------------------------------------------------------------------------------------------
## 5 TESTS + GATE ROWS (pre-registered; each RED first; a bar is met or reported, never loosened)
---------------------------------------------------------------------------------------------------
This list REPLACES PL section 5. RED arm = the named mutant (a one-line edit that must turn exactly its named case RED), or
TREE-BEFORE (the case does not compile or fails on the tree before the stage). Harmony copies gate strings only from here.

UNIT -- tests/test_frame_history.cpp, tag [frame_history] (S1)
 U-H1  "frame history: delay 0 picks the newest pickable entry"                 RED: M-H1 pick returns the oldest.
 U-H2  "frame history: pick agrees with a brute-force oracle" -- three hand tables (a query equal to a stamp; two entries with
       equal stamps, the later-written one wins; a query older than every stamp) and 1000 seeded schedules with +-3 ms jitter
       in which every tenth query is set equal to a stamp
                                                                                RED: M-H2 compares stamp < instead of <= (the
                                                                                first hand table must fail).
 U-H3  "frame history: while filling, the oldest pickable entry is held"        RED: M-H3 the fallback returns the newest.
 U-H4  "frame history: a raised delay never shows a lower serial"               RED: M-H4 the serial floor dropped.
 U-H5  "frame history: a lowered delay jumps forward at once"                   RED: M-H5 the floor entry returned although a
                                                                                newer one qualifies.
 U-H6  "frame history: an entry within two writes of being rewritten is never picked, counted in writes" (one write is never
       offered)                                                                 RED: M-H6 the guard counts offers; M-H6b
                                                                                `slots - 3` -> `slots - 1`.
 U-H7  "frame history: a clear leaves nothing pickable until the next offer; a gap over one second clears"
                                                                                RED: M-H7 the gap compare removed.
 U-H8  "frame history: depth and reach" -- exact: extraSlots 1280x720 = 14, 1920x1080 = 14, 2560x1440 = 14, 3840x2160 = 14,
       5120x2880 = 11, 7680x4320 = 5; reachMs(14 extra, 60 Hz) = 233, (14, 120) = 116, (14, 144) = 97, (11, 120) = 91,
       (5, 60) = 83, (5, 120) = 41; a Delay of 100 is limited for exactly the last four
                                                                                RED: M-H8 the budget counts total slots
                                                                                (the 5120 and 7680 rows must fail).
 U-H9  "frame history: deep is dropped only after 600 publishes with nobody asking, and returns at once"
                                                                                RED: M-H9 hold 600 -> 0.
 U-H10 "frame history: a half-written entry is never accepted" -- the writer's three steps run one at a time and a read between
       each accepts nothing; then, tag [tsan], a reader thread against 1,000,000 writes whose stamp is a function of the
       serial: every accepted pair satisfies it                                 RED: M-H10 the stamp stored first and the word
                                                                                never zeroed (the stepwise part must fail);
                                                                                M-H10b word and stamp in one plain struct
                                                                                (TSan must report it).

UNIT -- tests/test_surface_pool.cpp (S1)
 U-P1 "surface pool: a new slot count is a new generation of that many surfaces"     RED: TREE-BEFORE.
 U-P2 "surface pool: retainSurface refuses a slot beyond its generation's count"     RED: M-P2 bound = kMaxSlots.
 U-P3 "surface pool: allocBytes is at least slots x w x h x 4 and at most 2 % more"   RED: M-P3 counts kSlots.
 U-P4 "surface pool: a failed create keeps the current generation and is not retried for 300 ticks" (a create function that
      fails on the Nth surface; the create calls over 100 `ensure` calls are counted)  RED: M-P4 no memo.
 U-P5 "surface pool: after a new generation only the newest retired one and the kept one remain"
                                                                                      RED: M-P5 trimRetired does nothing.
 U-P6 "surface pool: after releaseAll the next generation number is new"              RED: M-P6 the number restarts from the
                                                                                      current generation.

UNIT -- tests/test_output_look.cpp (S1: L1, L4, L5, L6; S3: L2, L3)
 U-L1 "output look: pack / unpack round trip over every range edge"              RED: M-L1 opacity 6 bits.
 U-L2 "output look: applyReference(default) is the identity for all 256 values"  RED: M-L2 contrast pivot 0.5 -> 0.498.
 U-L3 "output look: the table" (contrast -1 -> 0.5; brightness +1 -> 1; red -1 -> r = 0; opacity 50 -> c/2)   RED: M-G5.
 U-L4 "output look: out-of-range input is clamped" (Delay 101 -> 100)            RED: M-L4 no clamp.
 U-L5 "output look: isDefault is exact"                                          RED: M-L5 ignores delay.
 U-L6 "output look: the settings text" ("", "40 ms", "adjusted", "40 ms, adjusted")   RED: M-L6 colour rows ignored.

UNIT, offscreen GL -- tests/test_shared_frame_gl.cpp, tag [shared_frame_gl] (S2, S3). Frame n = a solid colour encoding n; the
clock is injected; frame n is offered at n x 16,667 us.
 U-ID  "output frame: a default look is presentSharedFrame" -- the same target bytes; `path == Blit`; `logReads == 0`;
       `serial == front().serial`; the pool stays at 4 slots                     RED: M-ID a default look takes the pick path;
                                                                                 M-LAG the default path presents the frame
                                                                                 before the front.
 U-G1  "output frame: delay d shows the frame offered d ago" (d = 50, 100 ms; decoded n exact; `path == Pick`)
                                                                                 RED: M-G1 delay ignored.
 U-G2  "output frame: raise holds, lower jumps, never backwards" (the Delay changed by 100 ms between two presents 1 ms apart;
       decoded n never decreases over 0 -> 100 -> 0)                             RED: M-H4.
 U-G3  "output frame: at a size change a delayed reader shows the front, then holds the first new frame, never black"
                                                                                 RED: M-G3 nothing presented while the log is
                                                                                 empty.
 U-G4  "output frame: a deep ring keeps what is being shown; rotation holds at 18 slots"   RED: M-G4 write_ modulo kSlots.
 U-G5  "output look: the pass matches applyReference within 1/255" (7 looks x a 256-step ramp, 1:1 target)
                                                                                 RED: M-G5 brightness before contrast.
 U-G6  "output look: opacity 0 is black; -1 on a channel removes it"             RED: M-G6 opacity multiplies alpha only.
 U-G7  "output look: the forced pass with a default look equals the blit" -- byte-identical at a 1:1 target (bar 0); on a
       checkerboard-plus-ramp source at a 0.5x and at a 1.5x target the largest per-channel difference is <= 2/255, ASSERTED
                                                                                 RED: M-G7 half-texel offset in the quad.
 U-G8  "output frame: the Syphon shape, in the writer's context" -- an FBO target at canvas size shows the delayed, coloured
       frame; a snapshot of the GL state named in A-9 is equal before and after
                                                                                 RED: M-G8 the program left bound; M-G8b the
                                                                                 target framebuffer left bound.
 U-G9  "output frame: a failed deep generation leaves every reader presenting at 4 slots" (`history_slots == 4`,
       `alloc_failed == 1`, the create calls do not grow with publishes)          RED: M-P4.
 U-G10 REPORT, no bar: the time to bind an 18-slot generation in one reader context at 1920x1080 and at 3840x2160.

UNIT -- tests/test_output_settings_store.cpp, tests/test_display_identity.cpp (S4)
 U-S1 "output settings: JSON round trip"                                         RED: TREE-BEFORE.
 U-S2 "output settings: a missing number reads as its default; unknown keys are ignored; no id = skipped"
                                                                                 RED: M-S2 missing opacity reads 0.
 U-S3 "output settings: a write never changes the outputs key or any other key"  RED: M-S3 the store writes kOutputs.
 U-S5 "output settings: changes within 500 ms make one write"                    RED: M-S5 debounce 0.
 U-S6 "output settings: a failed write stays dirty and is retried"               RED: M-S6 clears dirty before the write.
 U-S7 "output settings: flush writes at once"                                    RED: TREE-BEFORE.
 U-S8 "output settings: syphon has its own record"                               RED: M-S8 syphon stored under screens[0].
 U-S9 "output settings: an all-default record is left out"                       RED: M-S9 written anyway.
 (U-S4 and U-A1 of PL are withdrawn: A-11, A-15.)
 U-D1 "display identity: joined by index, checked by bounds; a mismatch gives no identities"   RED: M-D1 no bounds check.
 U-D2 "display identity: an empty or duplicated uuid falls back to a key with the origin; two identical screens get two keys"
                                                                                 RED: M-D2 the origin dropped.
 U-D3 "display identity: DisplayInfo equality is unchanged" (a static check on the six fields)   RED: M-D3 a seventh field.
 U-D4 "display identity: same maker and model with serial 0 are twins; a serial tells them apart"   RED: M-D4 serial ignored.
 U-D5 "display identity: resolveLooks gives each display the record with exactly its key" (a swap, a reorder, a screen gone
      and back, an unknown key -> default, a same-size stranger -> default)       RED: M-D5 falls back to a same-size record.

UNIT -- tests/test_output_settings_model.cpp, tests/test_output_menu_model.cpp, tests/test_output_law.cpp (S6, S5)
 U-M1 "output screens: one entry per connected display, in menu order, with the menu's label and on / off"   RED: TREE-BEFORE.
 U-M2 "output screens: Syphon is listed iff available, with its state"           RED: M-M2 listed always.
 U-M4 "output screens: the Delay row is 0..100; limited to N ms shows iff the stored Delay exceeds the reach"
                                                                                 RED: M-M4 never limited.
 U-M5 "output screens: seven sliders, their defaults 0, 100, 0, 0, 0, 0, 0"      RED: M-M5 opacity default 0.
 U-M6 "output screens: the paint key changes iff something painted changes" (Pitfall 59)   RED: M-M6 the key includes a tick.
 U-M7 "output screens: the Device text names the screen, the monitor and its place"        RED: M-M7 left and right swapped.
 U-M8 "output screens: a twin's Device text says its settings follow the socket"           RED: M-M8 twin ignored.
 U-M9 "output screens: no entry for a screen that is not connected"              RED: M-M9 lists every record.
 (U-M3 of PL is withdrawn: A-12.)
 U-MENU1 "output menu: the list ends with Output Screens..., always enabled; every earlier item's label, id and order are
         unchanged"                                                              RED: TREE-BEFORE.
 U-MENU2 "output menu: a display item's text column carries its settings text; Syphon Output is disabled when Syphon is not
         available"                                                              RED: M-MENU2 the text put into the label.
 L-1 "output law: the settings window, its model and the store name no opener and never take the keyboard" (`openDisplay(`,
     `toggleDisplay(`, `restoreLast(`, `openOnDisplay(`, `setAlwaysOnTop(true)`, `grabKeyboardFocus`,
     `setWantsKeyboardFocus(true)`, `toFront(true)` absent from OutputSettingsWindow.*, OutputSettingsModel.h,
     OutputSettingsStore.*; OutputSettingsWindow's `getDesktopWindowStyleFlags` contains `windowIgnoresKeyPresses`)
                                                                                 RED: a mutant line calling openDisplay(0);
                                                                                 a mutant removing the flag.
 L-2 "output law: the new 8080 routes name no opener"                            RED: likewise in a route body.
 L-3 "output law: presentOutputFrame's default branch is presentSharedFrame and nothing else"   RED: M-ID.
 L-4 "output law: Renderer.cpp calls presentOutputFrame exactly once, inside publishSyphonFrame, between the state save and
     the state restore"                                                          RED: a second call; the restore removed.
 L-5 "output law: presentCanvas, processPendingCapture and VideoRecorder.cpp name no output setting, no FrameLog and no
     Syphon object"                                                              RED: a mutant reference.
 L-6 "output law: OutputWindow's renderOpenGL loads the look atom once and calls presentOutputFrame"
                                                                                 RED: the call replaced by presentSharedFrame.
 L-7 "output law: applyLooks( is called in openDisplay, restoreLast, reconcile and the store's callback"
                                                                                 RED: one call removed.

LIVE ROWS (Harmony; .harmony/probe-outputs.sh; test mode; `set_output_tap`; a scratch settings file; never an Output window;
each row prints `<name> PASS|FAIL <numbers>`; the window law is evaluated after every row). "deep on" = `set_output_tap`
with `deep: true` (A-13: the named feed; it does not pass through OutputManager).

 oa_publish_rate       INFO "publish_hz=<x>" (G0, on main; M1).

 oa_depth_spike        (M3; run FIRST after S5) at 1920x1080 and at 3840x2160, a static source, 5 changes shallow -> deep and 5
                       deep -> shallow (each waits for `history.slots` to change):
                       (a) `peak_frame_time_ms` in the 2 s around each change <= 16.6 ms (PL's bar, kept) AND minus the median
                           of five no-change 2 s windows on the same canvas <= 8.0 ms;
                       (b) the same two bars for `peak_callback_ms` (with Syphon on and its settings off default when M2 says
                           Syphon is available);
                       (c) 4 x the probe's `bind_us` on its first present after each change <= 4000 us (three screens and
                           Syphon binding in one pass of the GL thread);
                       (d) drag sweep: deep toggled every 250 ms for 10 s: `history.slots` changes exactly once (to 18) and (a),
                           (b) hold throughout.
                       Not met -> REPORTED, S1b / S2b scheduled (A-6); the bars stay.
                       RED arm: M-SLEEP build (20 ms in `ensure` when it creates, 2 ms in `bindGeneration`): a, b, c must fail.

 oa_identity_default   probe with no look: PNG against the canvas capture within the existing o_probe_matches_canvas bar; in 30
                       of 30 probes `path == "blit"`, `log_reads == 0`, `shown_serial == serial`, `history_slots == 4`. Then
                       deep on (another consumer wants depth): 30 of 30 probes with no look still `path == "blit"`,
                       `log_reads == 0`, `shown_serial == serial`, with `history_slots == 18`.
                       RED arm: M-LAG build (the serial differs).

 oa_delay_live         1920x1080, an animating source, deep on, probe target 480x270.
                       Preconditions (a miss is a FAIL of the row, never a loosened bar): reader 0's poll period has a median
                       <= 8 ms; at least 90 % of consecutive serials it records have different PNG hashes.
                       Reader 0 (no look) is polled for the whole row and records serial -> (PNG sha256, first time seen).
                       For d in 50, 100: after d + 500 ms, 30 probes on reader 1 with Delay d, 100 ms apart:
                       (i)   PIXELS: at least 20 of the 30 name a serial reader 0 recorded, and for every one of those the
                             probe's sha256 == the recorded one. 0 mismatches.
                       (ii)  AGE: age = probe time - reader 0's first-seen time of that serial; median(age) in
                             [d - 3 ms, d + one publish interval + 3 ms].
                       (iii) `path == "pick"` and `log_reads > 0` in 30 of 30.
                       RAISE: reader 1 at Delay 0 for 1 s; then two probes, Delay 0 then Delay 100, whose measured spacing is
                       under 33 ms (retried up to 5 times; never under -> FAIL "probe too slow"): the second `shown_serial` >=
                       the first. LOWER: Delay 100 then 0: the second `shown_serial` == its `serial`.
                       RED arms: M-G1 build (delay ignored: (ii) fails near 0); M-STAMP build (the stamp taken at the start of
                       the render instead of at the offer: (ii) fails low).

 oa_canvas_untouched   Preconditions: `GET /api/syphon` available and enabled, else FAIL "syphon not running"; after the Syphon
                       settings are set, `outputs.syphon.reader == true`. Static source.
                       With the Syphon settings and the probe's look = {Delay 100, Brightness -1.00, Opacity 0}: three
                       `render_frame` captures in a row, each sha256-equal to the all-default baseline's (same time override).
                       Positive controls in the same run: the probe PNG with look {Brightness -0.20, Contrast 0.35, Red -0.10}
                       is within 1/255 of `applyReference` of the capture on every pixel (PL's oa_colour_pixels, folded in);
                       the `syphon_probe` PNG differs from the capture.
                       RED arm: M-CANVAS build (the Syphon branch draws into `canvasFBO_`): the captures must differ.

 oa_syphon_look        Preconditions as above. Syphon settings = V3's values: the `syphon_probe` PNG is within 1/255 of
                       `applyReference` of the canvas capture on every pixel; settings back at default: byte-identical to the
                       canvas capture and `outputs.syphon.reader == false`.
                       RED arm: the M-CANVAS build (the Syphon texture is never drawn by the reader).

 oa_memory             (`peak_bytes` = the largest alloc + retired since `set_output_tap` was last sent with `reset_peak`.)
                       1920x1080: shallow `slots == 4`, `alloc_bytes` in [33,177,600, +2 %]; deep `slots == 18`, `alloc_bytes`
                       in [149,299,200, +2 %], `reach_ms >= 100`; over one shallow -> deep -> shallow cycle `peak_bytes` <=
                       186,126,336; `retired_bytes == 0` once `frame_serial` is 130 past each change.
                       3840x2160: shallow [132,710,400, +2 %]; deep [597,196,800, +2 %]; cycle `peak_bytes` <= 744,505,344.
                       Canvas change while deep, 1920x1080 -> 3840x2160: `peak_bytes` <= 761,425,920.
                       Four sizes while deep, 250 ms apart, 3840x2160 -> 2560x1440 -> 1920x1080 -> 1280x720: `peak_bytes` <=
                       879,869,952 (the retired cap; without it the four generations reach 1,078,272,000).
                       Deep off: `slots == 4` within 15 s. Tap off after a deep run: within 1 s of the next frame
                       `alloc_bytes == 0` and `retired_bytes == 0`; tap on again: `frame_serial` advances and a probe matches
                       the canvas. In a launch that never went deep: after tap off `alloc_bytes` is still the 4-slot figure.
                       `phys_footprint_mb` is printed beside every figure (INFO, M7).
                       RED arms: M-P5 build (the four-size clause fails); TREE-BEFORE (fields absent).

 oa_tap_cost           NEW asserting row (not the report-only o_tap_cost). One launch; at 1920x1080 and at 3840x2160: six
                       windows of 5 s alternating deep off / on (each after `history.slots` settled); 7070 `frame_time_ms` and
                       `gpu_time_ms` sampled every 0.25 s: mean(deep) - mean(shallow) <= 0.20 ms for each. Waits until no
                       compiler runs; prints the load average.
                       RED arm: M-BLIT2 build (a second blit per publish while deep). If that build does not turn the row RED,
                       the row is INFO and Harmony says so (RIG-RULES B: a bar whose teeth equal the drift is not a gate).

 oa_settings_roundtrip `output_settings` sets the first display's key and "syphon"; 1 s later the scratch file's
                       "outputSettings" holds exactly those numbers and its "outputs" value is byte-identical to before; after
                       a relaunch on the same file `/api/state` reports the same numbers (`displays[0].settings`,
                       `outputs.syphon.settings`); `outputs.live == 0` throughout and no `applied` field exists.
                       RED arm: TREE-BEFORE (the route answers 404); the write rule's mutant is unit U-S3.

 oa_identity_report    INFO (M4): for each connected display: index, label, key, name, maker, model, serial, twin. No bar.

 oa_window_law         The guard's decision function, selftest with made-up window lists: the exempt id alone -> PASS; the
                       exempt id plus one more unnamed layer-0 window -> FAIL; a window named "Audio-DNA Output" -> FAIL; the
                       exempt id with bounds equal to a display's -> FAIL; a window named "Output Screens" under another id ->
                       FAIL. Then, live, the guard with the exemption runs across every row and every visual-gate capture.
                       RED arm: the selftest's four FAIL cases (no build).

 oa_tsan               The [tsan] cases and probe-tsan's output scenario (reader 0 polled while deep toggles) report 0 races.
                       RED arm: M-H10b in the TSan test build.

 REGRESSION            every existing o_* row of probe-outputs and the eight existing test_shared_frame_gl cases: unchanged
                       PASS.

VISUAL GATE: V1..V9 (A-12), a capture builder then five critic seats, before Boris. The critics get RF-1..RF-3 verbatim and the
list of texts that are outside the lane (the main window's own texts; the composition's "Output Settings" section in V9).

LIVE MUTANT BUILDS, one at a time, each followed by RIG-RULES A's restore check: M-SLEEP, M-LAG, M-G1, M-STAMP, M-CANVAS, M-P5,
M-BLIT2 (seven). TREE-BEFORE arms use the pre-merge copy of the app.

---------------------------------------------------------------------------------------------------
## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; his real screens, his ear) -- replaces PL section 6
---------------------------------------------------------------------------------------------------
No gate may open an Output window, drive the mouse or the keyboard, or unplug a screen. So three things are exercised for the
first time by Boris, and green rows do not prove them: the live window's own call of the present step (pinned in source by
L-6), the manager handing the stored settings to the right live window (pinned by L-7, its logic unit-proven by U-D5), and
mouse work in the settings window.

B-0 The window lets you work. Do: start a clip from the keyboard; open Output > "Output Screens..."; drag a Delay slider;
    right-click it; press a clip key; with an output on, press Cmd+Shift+Esc. Expect: the slider follows the mouse, the
    right-click puts it back, the clip key and the all-off keys work without clicking the main window first. Wrong: a slider
    that will not move, or a key that does nothing until you click the main window -- tell Harmony which.
B-1 The Delay is real. Do: a projector or second monitor on; a clip that flashes on the beat; set that screen's Delay to
    100 ms; film the laptop's preview and the screen together with a phone in slow motion. Expect: the screen flashes a tenth
    of a second after the preview, steadily. Wrong: no gap, a gap that wanders, a flash that appears twice.
B-2 Delay 0 is today. Do: Delay 0, everything else untouched. Expect: the screen looks and feels exactly as before this change.
    Wrong: any softness, any lag you did not have, any tint.
B-3 Dragging. Do: drag Delay up slowly, then down, while a moving clip plays. Expect: on the way up the picture pauses a little
    and carries on; on the way down it skips ahead; never black, never a replay. The very first time a Delay leaves 0 every
    screen may repeat one frame, once. Wrong: a black flash, a jump backwards, a hitch every time you pass 0.
B-4 The right screen is remembered. Do: set screen A to 30 ms and screen B to 80 ms; look at the Outputs list; quit; start
    again; unplug and re-plug each; restart the Mac; then, if you have two of the same projector, swap their cables. Expect:
    the Outputs list shows "30 ms" and "80 ms" beside the two screens, and each screen comes back with its own number. Two
    projectors of the same model with no serial number: the window says "settings follow the socket", and after a cable swap
    the numbers stay with the sockets. Wrong: a number back at 0, or a number on a screen you did not give it to -- tell
    Harmony which step did it.
B-5 The colours, against Resolume. Do: the same still on one screen from Resolume and then from Audio-DNA; in both set
    Brightness -0.3, then Contrast +0.3, then Red -0.3. Expect: the same kind of change in both. Wrong: one darkens the blacks
    and the other does not, or a row runs the other way -- say which row.
B-6 Opacity. Do: drag Opacity to 0 on one screen with two screens on. Expect: that screen fades to black; the other and the
    preview do not change. Wrong: the preview or the other screen dims too.
B-7 Syphon. Do: turn Syphon on, receive it in Resolume; set its Delay to 100 ms and its Blue to -1. Expect: Resolume's copy is
    a tenth of a second late and has no blue; Audio-DNA's own preview, a recording and a snapshot are normal. Wrong: the
    recording or the snapshot is late or tinted; Resolume's copy is black or the wrong shape.
B-8 In the room. Do: stand where you mix; play a track with a hard kick; raise that screen's Delay until the flash and the kick
    land together for your eye and ear. Expect: one number does it and it holds all night. Wrong: you reach 100 and it is
    still not enough -- say how far the speakers are (that is question 51's B); or it drifts.
B-9 The next room. Do: the next time you plug the same projector in somewhere else, open the Outputs list before switching it
    on. Expect: last time's number is written beside its name. Wrong: the screen comes up late and nothing told you.
B-10 4K. Do: set the composition to 3840x2160 with a Delay on. Expect: nothing slows down. Wrong: the app stutters or the
    machine runs short of memory.

---------------------------------------------------------------------------------------------------
## 7 BORIS QUESTIONS (new numbers 51..59; each has a default A; nothing waits on them) -- replaces PL section 7
---------------------------------------------------------------------------------------------------
51. How far a screen's Delay goes.
    A (default) 0 to 100 ms, as in Resolume. That is enough when you stand up to about 35 metres from the speakers.
    B 0 to 250 ms, for very large rooms and festival fields (up to about 85 metres). It uses more video memory.
(52-59 are not used.)

READINGS Harmony tells him (not questions; he corrects only what is wrong; Harmony gives them their R numbers):
 - The picture moves one frame at a time (about 17 ms); the Delay is set in whole milliseconds.
 - The Device row names the screen: its name and where it sits. It is not a menu; screens are switched on and off in the
   Outputs list, as today.
 - A screen you have set shows it in the Outputs list, beside its name: "40 ms", or "adjusted" for the colour rows.
 - The Output Screens window never takes the keyboard: your keys keep launching clips while it is open. Values are set by
   dragging; a right-click puts a row back.
 - A screen's settings are remembered with the screen on this computer, whatever show is open (question 38's default).
   Cmd+Z does not touch them.
 - Two projectors of the same model that carry no serial number cannot be told apart by the Mac: their settings follow the
   socket they are plugged into, and the window says so.
 - Raising a Delay holds that screen's picture still for the time you added, then it carries on; lowering it skips ahead.
 - Opacity fades to black, on a screen and on Syphon.
 - Syphon is switched on and off where it is today (Output menu), and is off at every launch; its settings are in the window.
   With any Syphon setting changed, Syphon runs one frame later than untouched.
 - Brightness, Contrast, Red, Green, Blue run from -1.00 to 1.00 with 0 in the middle. Resolume does not publish its formulas;
   ours are the usual ones (B-5 compares them).
 - These settings cannot be put on a key, a knob or a MIDI pad.
 - The window is called "Output Screens" because "Output Settings" is already the composition's size.

---------------------------------------------------------------------------------------------------
## 8 HARMONY'S DECISIONS (each with a default), and the FACTS HARMONY MUST MEASURE
---------------------------------------------------------------------------------------------------
HD-1  Build 0..100 while question 51 is open. Default: yes (B is three constants, A-1).
HD-2  Two worktrees (S2, S3 beside S4, S6) or one. Default: two; disk today 301 GiB free (`df -h /System/Volumes/Data`).
HD-3  The Syphon rows when M2 says Syphon is not available on this rig. Default: the lane does not merge until
      oa_canvas_untouched and oa_syphon_look have run on a build with Syphon; if the framework cannot be had here, that is
      reported to Boris as a blocked check, never passed by default.
HD-4  The two stale-doc fixes (docs/claude/testing-eyes.md:72; docs/claude/architecture.md:107, :231, :334). Default: one docs
      chore after this lane, outside S7 (SC-4).
HD-5  The settings.json mend. Default: the saves lane (SF-1).
HD-6  The pitfall number. Default: 68 (free, RF-21).
HD-7  `kSizingHz` after M1. Default 120; raised only by the rule in A-1.
HD-8  After oa_depth_spike: bars met -> eager binding stands; missed -> S2b and / or S1b (A-6).
HD-9  After M5: met -> one colour pass; missed -> the two-step path and its bill (A-8).
HD-10 After B-0: sliders work -> the window stays as ruled; they do not -> the fallback of A-12.
HD-11 Which unit mutants she re-witnesses herself, and whether an M-H4 app build is added to oa_delay_live's RAISE clause.
      Default: U-G2 is the proof; the RAISE clause runs on the good build only.
HD-12 The read-only state fields on 7070 as well as 8080. Default: both (Boris's own session can then show what a screen got).
HD-13 After M4 / B-4: a UUID that changes -> the "edid:" key (A-10). Default: nothing until a screen shows it.
HD-14 Merge order against the beat-nudge lane. Default: whichever merges second rebases; neither reformats a shared file.
HD-15 The stopped worktrees (lane/bf2 740b6d6, lane/bf2-keys 9eab9bd). Nothing of them enters this lane's code (the debounced
      save is re-typed). Default: removed after this ruling is adopted and HD-4's lines are copied (RF-21).
HD-16 Readings of section 7: she numbers them and tells him with question 51.

FACTS HARMONY MUST MEASURE (only a run can establish them; the ruling for each outcome is fixed here)
M1 The canvas publish rate on Boris's Mac, preview on the built-in display (row oa_publish_rate, G0, on main, no code).
   <= 125 a second -> `kSizingHz = 120` stands. Above -> raised before S1 (A-1).
M2 Whether Syphon is available on this rig (`GET /api/syphon`, G0). Yes -> the Syphon rows run. No -> HD-3.
M3 The cost of a depth change: the writer's create and bind (delta of `peak_frame_time_ms` / `peak_callback_ms`) and a reader's
   bind (`bind_us`), at 1920x1080 and 3840x2160 (row oa_depth_spike). Outcomes: A-6.
M4 What Boris's connected screens report: key, name, maker, model, serial, twin (row oa_identity_report, INFO). What survives a
   re-plug, a restart and a cable swap is B-4. Outcomes: A-10.
M5 The largest difference between the blit and the colour pass at a scaled target on this GPU (U-G7; the S3 builder runs it,
   Harmony re-witnesses). Outcomes: A-8.
M6 Reader 0's poll period in oa_delay_live (precondition <= 8 ms at a 480x270 target). Missed -> the target is halved
   (240x135) and both preconditions are checked again; the bars do not move.
M7 Whether IOSurface memory shows in `phys_footprint_mb` (printed by oa_memory, INFO; no ruling depends on it).
M8 Whether a second rectangle texture on the same surface works in the writer's own context (U-G8, a ctest; the S3 builder
   runs it). Fails -> the Syphon reader reads the writer's own slot framebuffers through one accessor of `SharedFrameSet`
   instead of binding its own; same tests, same law rows.

---------------------------------------------------------------------------------------------------
## 9 SIDE FINDINGS
---------------------------------------------------------------------------------------------------
SF-1 (for the saves lane) settings.json: a file that does not parse is read as empty and the next write drops every other key
     (RF-19). This lane adds the most frequent writer the file has had (every settled slider change). The mend -- copy the
     unreadable file aside before rewriting -- is NOT built here (A-15). Until it is: a damaged settings.json at the moment of
     a settings change loses the MilkDrop folder and the saved output set.
SF-2 (existing, fixed here) A failed surface creation at a canvas size change is retried on every publish and freezes every
     output meanwhile (RF-10, RF-11). A-3's memo and fall-back cover the 4-slot case too.
SF-3 (existing, not fixed here) Every generation change drops the copy made by the previous publish: one repeated frame on
     every output at a canvas size change (RF-11), and from now on at the first Delay. Offering that pending copy before the
     switch would remove it; it is a change to today's path and is left out.
SF-4 (existing, fixed in S6) "Syphon Output" is enabled in the menu even when Syphon is not available (RF-16).
SF-5 The dispatch names questions 47-50 as open; the record shows all four answered (RF-5). Nothing here depends on them.
SF-6 The law guard's name rule (`"Output" in name`, RF-15) flags ANY Audio-DNA window whose title contains "Output". The
     decision is now a function with a selftest; a future window title must be checked against it.
SF-7 After an idle trim that followed a deep run, `front()` reads "nothing published": an output opened next is black for its
     first two publishes instead of showing the last frame from before. A launch that never used a Delay is unchanged.
     Stated so that it is not found later as a regression.
SF-8 Seat citations: every file:line I re-read was right. One worry was already met by the code: GA-3's "a fresh reader state
     per probe" -- the probe's state persists (RF-14). GA-6 was stronger than the seat said (the title itself trips the name
     rule).
SF-9 `PresenterGLState` grows from 4 to 18 entries per array (about 290 bytes, no heap).
SF-10 PL B-9 (the Delay row stopping at 250 ms on a 4K canvas) and PL's reading "on a 4K composition it stops at 250 ms" are
     withdrawn with the tiers.

---------------------------------------------------------------------------------------------------
## 10 RISKS (the strongest counterargument first; the cheapest refuting test for each)
---------------------------------------------------------------------------------------------------
R1 THE STRONGEST COUNTERARGUMENT -- the range. "He was told 0 to 500 and did not object. His purpose names the soundboard; a
   festival desk 50 to 60 m out needs 150 to 175 ms of air. Cutting it to 100 on an architect's reading of his sentence
   about copying Resolume may leave him out of range in exactly the room the feature is for."
   Why it loses here: (1) the Delay he needs is the air time MINUS the picture's own lateness -- the analysis budget (FO Q7
   quotes "~15-25 ms", not re-read by me), the +1 frame of the output tap (RF-11) and the projector -- so 100 ms reaches
   further than 34 m in practice; (2) his sentence is "copy what resolume does ... Makes it simpler" (RF-1) and R11 itself
   said its maximum was being looked up (RF-4); (3) it is ASKED (51), B is three constants, and nothing built for A is thrown
   away for B; (4) the price of 500 was the machinery the council took apart: three tiers, a measured class that could move
   the applied Delay by itself, transients over 1 GiB.
   Cheapest refuting test: B-8 in his usual room. Cost of being wrong: three constants, U-H8's table, oa_memory's numbers.
R2 One depth costs more at 4K for a small Delay: 443 MiB extra for a 10 ms Delay, where PL's first tier at 60 Hz was 253. It is
   the price of an applied Delay that never changes by itself and of no rate measurement in the product. Refuting test: M1. A
   60 Hz result does NOT lower the constant: the built-in panel of a current MacBook Pro can run at 120 Hz (ASSUMED of his
   machine; M1 shows it).
R3 Eager binding and creation on the GL thread may hitch at the first Delay. Test: oa_depth_spike (M3). Fallbacks named (A-6);
   bars fixed.
R4 The settings window never taking the keyboard: mouse work in such a window is INFERRED from JUCE's popup menus (RF-17). It
   fails safe -- B-0 finds it at first use, and the fallback is named (A-12). The opposite failure (a normal window with
   forwarded keys that silently did not forward) would have taken the all-off keys away during a show.
R5 Identity. The UUID may not survive what is ASSUMED; twins without serial numbers follow the socket and no software can see
   a cable swap. Made visible, not solved. Test: oa_identity_report + B-4. Cost of being wrong: a screen comes up at defaults,
   or twins exchange trims; the list shows what each got.
R6 oa_delay_live needs a probe faster than 8 ms and an animating source. A missed precondition is a FAIL ("not run" is not
   "passed"); M6 names the one allowed adjustment (a smaller probe target).
R7 The cross-thread probe can pick a slot and be descheduled before its blit. At 18 slots the slot is safe for 16 further
   writes (133 ms at 120 Hz) against 3 today. Real outputs run on the writer's own thread and cannot be interleaved.
R8 Our colour arithmetic is not Resolume's (not documented, RF-7). Test: B-5. Cost: one formula line, one table (U-L3).
R9 The switch from blit to quad when a colour row first leaves default may shift a scaled picture. Test: U-G7, asserted (M5);
   the two-step path is the answer, with its bill.
R10 Syphon: one frame later once any setting is off default; a second texture on the same surface in the writer's context is
   ASSUMED (M8).
R11 settings.json is not mended here (SF-1).
R12 Seven live mutant builds lengthen the tail before Boris. Eased by the two-worktree order, by TREE-BEFORE arms where a unit
   carries the mutant, and by the visual gate running beside S2 / S3.
NOT VERIFIED by reading: what a display UUID survives and what `CGDisplaySerialNumber` returns for his screens (M4, B-4); the
publish rate (M1); creation and bind costs, and whether IOSurface memory is committed lazily (M3, M7); the real frames-behind
of an output and whether a display-sized window at swap interval 0 tears (FO U1, U2: unchanged by this lane, still
unmeasured); blit against quad on this GPU (M5); Resolume's arithmetic (B-5); whether EmbeddedShaders.h can be included alone
(the S3 builder reports); whether a Syphon receiver accepts the texture drawn by the pass (oa_syphon_look reads our texture;
the receiver is B-7); slider drags in a window that never takes the keyboard (B-0).

STATUS: DONE
