# L5 critic panel (4 seats)

Only one recording-related binding action exists (`ToggleRecording`), and it drives the unrelated, pre-existing `VideoRecorder` (P22 MP4 capture) — not the RecordPanel/PerformanceRecorder path being reviewed. That's fine; separate systems, no naming collision in UI text.

Confirmed: `getRecordPanel()` (BrowserPanel.h:36) has zero callers in MainComponent.cpp — `onStartRecording`/`onStopRecording`/`onPlayRecording` are never assigned, so even the internal onClick lambdas (dead since buttons are `setEnabled(false)`) would no-op if somehow reached. No keyboard/MIDI/menu path reaches RecordPanel at all.

```
VERDICT: PASS-WITH-FIXES

1. MUST-FIX: RecordPanel.cpp:13-33 — recordBtn_/stopBtn_ onClick lambdas still flip internal `recording_` state and status text ("Recording...", red color / "Stopped") even though the buttons are disabled. JUCE disabled buttons never fire onClick in normal use, but this is dead/inconsistent state logic sitting behind a single setEnabled(false) flag — if anything ever re-enables these buttons (a future refactor, a stray setEnabled(true) call), the panel would silently claim "Recording..." with zero backend wired (onStartRecording is never assigned by MainComponent, so nothing happens — pure UI lie). Fix: delete the onClick bodies (or the lambdas entirely) now, matching Save/Load which correctly have no onClick handlers at all.

2. MUST-FIX: RecordPanel.h:24 `isRecording()` returns `recording_`, which the disabled recordBtn_'s dead onClick can still set to true (see #1) — an unreachable-today but real landmine for the exact "performer thinks he recorded something" failure this task is trying to prevent. Fix alongside #1: remove the `recording_ = true/false` assignments or delete `recording_`/`isRecording()` entirely since nothing reads it externally (confirmed: `getRecordPanel()` at BrowserPanel.h:36 has zero external callers).

3. NICE-TO-HAVE: playBtn_.onClick (line 42-44) similarly still calls `onPlayRecording` — harmless since the callback is never bound and the button is disabled, but for consistency should be removed with #1 so all five buttons are uniformly inert, not "disabled but internally wired."

4. Confirmed no other path can start/imply a recording: `Binding::Action::ToggleRecording` (Binding.h:44) is a separate, functional, already-wired feature (VideoRecorder H.264 MP4 capture, MainComponent.cpp:5953) unrelated to RecordPanel/PerformanceRecorder — no naming or state collision with the disabled UI. No menu item, MIDI binding, or keyboard binding targets RecordPanel.

5. Stop/Play buttons are correctly non-meaningful (both disabled, no reachable state transitions) — this is consistent given #1/#2 are cleaned up.
```

---

VERDICT: PASS-WITH-FIXES

1. MUST-FIX — `Output Folder...` (src/ui/RecordPanel.cpp:80-92) and the `JSON Events` format selector (lines 62-66) stay fully enabled with no tooltip explaining they do nothing yet. A performer can pick a folder, see it reflected in `outputDirLabel_`, and reasonably conclude recording is configured and will use it — that is exactly the false-confidence the disabled/dimmed buttons are trying to prevent. Fix: add `.setTooltip("Saved for when recording is enabled; no files are written yet.")` to both `browseOutputBtn_` and `formatSelector_`.
2. NICE-TO-HAVE — In after-full.png, the Play tooltip ("Playback of a recorded performance is coming in a later build.") renders directly over the status label ("Performance recorder coming in a later build"), producing overlapping near-duplicate text (visible fragment "...oming in a later build" peeking out beside the tooltip box). Content isn't contradictory so it's not a MUST-FIX, but it's visually noisy. Consider tooltip offset/positioning below the button row.
3. NICE-TO-HAVE — `formatSelector_` includes a "Video (Future)" item (line 64) alongside the only-real option "JSON Events"; selecting it silently does nothing (no tooltip, no visual cue it's inert), same class of issue as finding 1 — covered by the same fix.
4. PASS — Status line ("Performance recorder coming in a later build") is visible by default without any hover, satisfies the core honesty requirement (performer never has to discover the truth by accident). Whole-word tooltip text confirmed correct on all 5 buttons (RecordPanel.cpp:11,26,41,53,59) — no abbreviations, matches project UI rule.
5. PASS — Disabled/dimmed buttons (alpha 0.4, RecordPanel.cpp:10,25,40,52,58) read clearly as inactive in after-full.png vs before-full.png; no risk of a performer believing Record/Stop/Play/Save/Load currently work.

---

REVIEW VERDICT: PASS-WITH-FIXES

FILE: /Users/boriskarpman/projects/RealTimeAudio/src/ui/RecordPanel.cpp (+ .harmony/.reports/s-rta-0923/l5-visual/{before,after}-full.png, after-crop-hover-play.png)

1. MUST-FIX — Contrast: `recordBtn_` alone gets a dark maroon tint (`0xff442222`, RecordPanel.cpp:7) before `kDisabledAlpha=0.4` is applied (line 10), while Stop/Play/Save/Load keep the default (lighter, neutral) button colour at the same 0.4 alpha. In the after-crop image the "Record" label reads visibly dimmer/lower-contrast than its four siblings — it nearly disappears into the panel background where the other four are still comfortably legible. This breaks the "uniformly disabled" read the honesty fix is going for: a performer scanning the row sees four clearly-dim buttons and one that's almost invisible, which draws the eye rather than reading as "equally off." Fix: either drop the maroon tint (match the other four disabled buttons) or compute `kDisabledAlpha` per-button so perceived luminance matches across the row.

2. OK — Dimming vs before: before-full shows the five buttons at full, non-dimmed brightness (the bug); after-full shows a clear, uniform muting of the whole row against the still-bright "Output Folder..." button and "JSON Events" dropdown directly below. Hierarchy correctly signals "this row is off, that row still works."

3. OK — Tooltip text: whole-word, sentence-style, and states an honest fact ("Playback of a recorded performance is coming in a later build.") rather than implying partial function — matches CLAUDE.md's whole-word rule and the project's honesty goal.

4. NICE-TO-HAVE — Status line: "Performance recorder coming in a later build" (kTextSecondary grey) sits directly above the disabled row and is itself only medium contrast; consider bumping it slightly or bolding since it's the one line a user reads before ever hovering a button, and hover-tooltips won't reach a non-technical user who doesn't mouse over each control.

5. OK — Consistency: the JUCE default light tooltip bubble is the app-wide `TooltipWindow` (per RecordPanel.h comment) — not a regression introduced here, so not counted against this change.

SUMMARY: 1 file reviewed (RecordPanel.cpp) + 3 screenshots, 2 issues (1 blocking: Record-button contrast inconsistency vs its 4 siblings; 1 suggestion: status-line legibility). Confidence: VERIFIED against source (colour/alpha values read directly from RecordPanel.cpp/.h) + visual read of the three provided screenshots; the exact rendered contrast ratio is INFERRED from screenshot pixels, not measured with a contrast tool.

---

REVIEW VERDICT: PASS-WITH-FIXES

FILE: src/ui/RecordPanel.cpp / RecordPanel.h (graphic-design pass only)

1. MUST-FIX — Tooltip collides with the status line. In after-full.png / after-crop-hover-play.png, hovering Play pops the tooltip directly on top of `statusLabel_` (RecordPanel.cpp:70, y-position set by `resized()` at RecordPanel.cpp:116-117). The status text's tail ("...oming in a later build") visibly peeks out to the right of the tooltip box, producing two overlapping near-identical gray strings at once — reads as a rendering glitch, not "coming in a later build" x2. Fix: widen the vertical gap between `statusLabel_` and the button row (`kRowSpacing = 6` at RecordPanel.h:51 is too tight given JUCE's default tooltip appears just above the cursor), or accept the default below-button tooltip offset so it never sits flush against the row directly above.

2. MUST-FIX — Large unlabeled blank gap under the buttons, unverified by any visible label. `resized()` reserves three separate spacer blocks with only code comments, not real Labels: `eventCountLabel_` (RecordPanel.cpp:133, never given text anywhere in the file — confirmed dead), `// "Format" label space` (RecordPanel.cpp:138), and `// "Output" label space` (RecordPanel.cpp:145). Visually this is the ~90px dead zone between the button row and "JSON Events" in before/after-full.png with no section header to explain it — looks unfinished/broken rather than deliberate. Either draw real "Format"/"Output" section labels or collapse the reserved height.

3. NICE-TO-HAVE — Record button's disabled dark-red tint (`0xff442222` at `kDisabledAlpha=0.4`, RecordPanel.cpp:7,10) reads almost identical in muted gray-brown to the neutral Stop/Play/Save/Load buttons in the crop; bump its alpha or base saturation slightly above the other four so "Record" still visually signals its identity without implying active/enabled state.

4. NICE-TO-HAVE — `statusLabel_` uses 11pt (RecordPanel.cpp:71) while `eventCountLabel_`/`outputDirLabel_` use 10pt (RecordPanel.cpp:76,96); minor inconsistent caption sizing within one panel.

SUMMARY: 2 files reviewed (RecordPanel.cpp, RecordPanel.h) + 3 screenshots, 4 issues (2 blocking, 2 suggestions). Alignment/left-margin of status line vs. buttons is otherwise clean and consistent.