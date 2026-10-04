# ATTACK PAPERS -- blind council on plan-nudge-row.md (lane "nudge-row", the delta on Boris's answers 61, 62, 111; s-rta-1004, 2026-10-04)

Verbatim seat papers, 2 of 2 seats returned. Source: the workflow task text handed to the architect (one JSON array;
the dispatch states 16389 characters; as transcribed here: 16389 characters; the array is closed).
Pretty-printed only (json.dumps indent=2, ensure_ascii=False); no word was changed. Seats were blind to each other.
Plan attacked: .harmony/.reports/s-rta-1004/plan-nudge-row.md. Ruling: .harmony/.reports/s-rta-1004/ruling-nudge-row.md.

| seat | attacks | ids | citations rechecked by the seat |
|---|---|---|---|
| gates | 7 | GA-1 (MUST), GA-2 (MUST), GA-3 (SHOULD), GA-4 (SHOULD), GA-5 (SHOULD), GA-6 (SHOULD), GA-7 (NIT) | yes |
| stage-hands | 8 | ST-1 (MUST), ST-2 (MUST), ST-3 (SHOULD), ST-4 (SHOULD), ST-5 (SHOULD), ST-6 (SHOULD), ST-7 (SHOULD), ST-8 (SHOULD) | yes |

Total: 15 attacks.

## SEAT: gates

```json
{
  "seat": "gates",
  "attacks": [
    {
      "id": "GA-1",
      "target": "NR2 rate readers; T-G12; LR3(c)(d)",
      "claim": "The headline claim 'a BPM-synced clip holds while the timer is not running' (Boris 111: 'If most of the show is set up to BPM ... nothing moves') has no gate that can fail. The lint is file-granular, LR3(c) is allowed to degrade to INFO, and no mutant removes a clip gate.",
      "evidence": "PL:275-276 promises the lint is 'in the same function', but T-G12 (PL:574-576) tests 'is on the whitelist or contains `beatRunning(`' per FILE. Renderer.cpp holds both BPM-Sync branches (M:src/render/Renderer.cpp:1657-1666 video, :1718-1734 sequence), so one token in either branch, or in a comment, satisfies the lint for both. MR1-MR19 (PL:609-614) include no 'clip gate removed' mutant. LR3(c) (PL:637-638) prints 'INFO LR3c no position route' and is 'reported, not passed' if main has no route. LR3(d) tests only the free clip. A BPM-synced video is never measured.",
      "severity": "MUST",
      "proposed_change": "Make the lint function-scoped (parse the function body, as PL:275 says). Pull the hold into one pure seam, e.g. `bpmSyncAdvances(snap, clip)`, with a unit case per branch. Add mutants MR20 (video gate removed) and MR21 (sequence gate removed). S2r must build the position route, and LR3(c) must be PASS-or-FAIL for BOTH an image sequence and a video, never INFO."
    },
    {
      "id": "GA-2",
      "target": "S2r vs S3r; LR1, LR2, LR3, LR6 (debug route 'tempo_row')",
      "claim": "Every live row drives the timer through the debug route, which calls MainComponent functions. Nothing proves the three top-bar buttons call them. The old clip play, pause and routines-stop wiring can stay in place and every LR row still passes.",
      "evidence": "S2r (PL:497): the route calls 'the same MainComponent functions the buttons will'. PL:423 says the routes 'drive the buttons' own handlers'. The two statements disagree. M:src/MainComponent.cpp:603-628 today binds onPlay and onPause to applyClipPlaying for every layer and onStop to routineEngine_.stopAll(). S3r (PL:501) only 're-wires' them. The widget tests do not click them: T-RW2 (PL:597-598) injects the published byte, and T-RW4 (PL:602) clicks '+' only. No test asserts that play, pause or stop reach `setBeatTimer`.",
      "severity": "MUST",
      "proposed_change": "Add a widget case in test_topbar_row.cpp that calls `triggerClick()` in-process (not OS input) on play, pause, stop, '-', '+', '/2' and 'x2'. Assert each fires its own callback exactly once and that the clip play/pause callbacks are never called. Make the debug route call `button.triggerClick()` so LR1-LR3 reach the real wiring. Add a mutant that leaves onPlay on applyClipPlaying."
    },
    {
      "id": "GA-3",
      "target": "Stage S2r gate list; LR4",
      "claim": "LR4 is scheduled where it cannot run, and its disabled-button step cannot tell a guarded handler from an unguarded one.",
      "evidence": "PL:497 runs LR4 after S2r ('LR4's UI clauses wait for S3r'). LR4 (PL:641-647) starts with 'Manual 120 by `bpm_edit`' and uses `bpm_edit` '127.6', '500' and 'abc' enter. `bpm_edit` is built only in S3r (PL:501), and the label it edits exists only after S3r. So the numeric steps, the RED arm MR7 built in S2r and the print '13 of 13 steps' cannot happen at S2r. Step 2 ('double -> 240 and enabled.double false') presses a disabled control through a route that calls the handler directly, so a handler missing the `canDouble` guard also gives 240.",
      "severity": "SHOULD",
      "proposed_change": "Split LR4 into LR4a (S2r: Manual set by `/api/set_bpm` 120, then the tempo_row ops) and LR4b (S3r: the bpm_edit steps). State that the route returns refused for a disabled op, and assert that refusal at 240 for double and at 30 for half. Add a mutant that drops the `canDouble` guard."
    },
    {
      "id": "GA-4",
      "target": "'What a take records' (PL:294-299); B-R12; mutant list",
      "claim": "Boris-visible risk with no gate: if `setBeatTimer` ever captures into a take, a replayed take would stop or start the beat it replays on. The plan files this under 'only Boris can check' although a machine can test it.",
      "evidence": "PL:294-296: the three controls are 'NOT captured and NOT replayed', and `setBeatTimer` 'never calls `recorderHost_.capture`'. T-R2 (PL:570-573) tests only RecorderClock's byte-to-anchor behaviour. MR1-MR19 (PL:609-614) contain no 'capture added' mutant. LR rows never record a take. B-R12 (PL:704) is the only check, and it is Boris's.",
      "severity": "SHOULD",
      "proposed_change": "Add a unit or probe case: record a take, run tempo_row stop, play and pause, and assert that the take holds zero timer points while a '/2' or 'x2' tempo point is present. Replay it and assert 'beatTimer' is unchanged. Add a mutant MR20b where setBeatTimer calls capture. Drop B-R12 to a one-line confirmation."
    },
    {
      "id": "GA-5",
      "target": "VG named questions; NR1 glyph choice; RR5",
      "claim": "The known defect (two identical '>' glyphs in one row) is left to a SHOULD-level question that cannot fail. The plan's own mis-press reasoning is applied unevenly.",
      "evidence": "PL:163-167 rejects '-' / '+' for the nudge because two such pairs 'within 100 px is a mis-press on stage'. It then ships play '>' and nudge-forward '>' 150 px apart, with the fallback '<<' / '>>' pre-ruled. PL:664-667 lists 'can \">\" (play) and \">\" (nudge forward) be told apart' as 'a SHOULD unless a seat shows a mis-read'. The 'pre-registered MUSTs' (PL:661-666) do not include identical glyphs. That distinctness is machine-checkable from ui_text.",
      "severity": "SHOULD",
      "proposed_change": "Add a pre-registered MUST: no two buttons in the row share the same text, asserted from `ui_text.tempo_row`. Either ship '<<' / '>>' as the default, or make the duplicate a stated MUST-waiver that Boris signs (B-R9)."
    },
    {
      "id": "GA-6",
      "target": "T-RW1 / fitTopBar at his real width; RR6",
      "claim": "The 'fits at 1728 with 31 px to spare' claim rests on an ASSUMED 86 px label. No pre-VG gate measures it. T-RB4 passes by construction because the label width is a parameter.",
      "evidence": "PL:392 and PL:761-762 ('ASSUMED 86 px label'). T-RB4 (PL:585-588) feeds `fitTopBar` the number 86. T-RW1 (PL:594-596) at 1728 x 40 asserts only order, non-overlap and non-empty bounds. It never asserts the shed list is empty at 1728 with the real font. The nudge box is `max(88, measured + 8)` (PL:380), another run-time width. If either is larger, DSP and FPS vanish on Boris's own screen and every unit gate stays green until VG.",
      "severity": "SHOULD",
      "proposed_change": "Pre-register a T-RW1 clause. With the real LookAndFeel fonts and the real Master Signal label, at 1728 the `shed` list is empty and the kept widths sum to <= 1720. Report the margin as a number. Make the margin a bar stated before VG-0 runs."
    },
    {
      "id": "GA-7",
      "target": "LR2 stop clause; LR6 count; ui_text 'thirteen names'",
      "claim": "A tolerance masks a mutant, and two counts are left stale or ambiguous by the added cases.",
      "evidence": "LR2 (PL:627-628) accepts `totalBeatCount in {c, c+1}` during the stop. A stop that realigns from the second half (MR2: `realignPhaseToZero` adds +1) lands inside that window, so the live stop clause cannot fail for MR2. Only the unit case T-G2 catches it, and LR2's own RED arm is MR3 only (PL:632-633). PL:427 says ui_text lists 'the thirteen names', but T-RB3 (PL:582-584) and T-RW1 (PL:594) say fourteen. LR6 (PL:656-659) prints '10 of 10' for 8 ops plus a release plus a CC. The MIDI-learn refuse-and-attach clause is uncounted, and the op list names 8 ops for 7 targets.",
      "severity": "NIT",
      "proposed_change": "In LR2 take c from the poll after the applied one for the stop, or assert equality to the held count when the stop lands in the first half. Add MR2 as a second RED arm. Fix 'thirteen' to 'fourteen'. Restate the LR6 total as an explicit enumerated list."
    }
  ],
  "strongest_point": "GA-2 and GA-1 together: the plan's two load-bearing live claims are 'the buttons are the BPM timer' and 'BPM-synced clips hold'. Both pass today on routes and file-granular token checks that never reach the code under test: the button callbacks at MainComponent.cpp:603-628, and the two separate BPM-Sync branches in Renderer.cpp. LR3(c) is explicitly allowed to be INFO.",
  "citations_rechecked": true
}
```

## SEAT: stage-hands

```json
{
  "seat": "stage-hands",
  "attacks": [
    {
      "id": "ST-1",
      "target": "NR2 rate readers + T-G12 + LR3",
      "claim": "Boris's 111 rule (\"BPM goes stop ... nothing moves\") is not proven for a BPM-synced VIDEO, the commonest BPM content. The gate that should prove it cannot fail.",
      "evidence": "LR3 (PL:634-639) clause (c) covers a BPM-synced image sequence only, and prints \"INFO LR3c no position route ... reported, not passed\" if no route exists, so it can be skipped. Clause (d) is a NON-BPM video. T-G12 (PL:575-576) passes if the FILE contains `beatRunning(` anywhere, but the lint's own rule (PL:276) says \"in the same function\". Once the image-sequence branch has the token, the video branch (Renderer.cpp:1657-1666, no tempo term) can omit it and pass. The held-frame call is only INFERRED (PL:266-268). Boris would see a BPM video keep playing while stop is lit.",
      "severity": "MUST",
      "proposed_change": "Make T-G12 function-level, as PL:276 states. Add a LR3 clause for a BPM-synced video clip that fails, not INFOs, when no position route exists. The S2r builder adds the route if needed."
    },
    {
      "id": "ST-2",
      "target": "NR2 'WHERE THE STATE IS KEPT' (PL:300-302)",
      "claim": "The plan contradicts itself on what opening a show does to a stopped or paused timer. No test or gate row covers it.",
      "evidence": "PL:300-302: \"every launch and every opened show starts Running\" and, in the same paragraph, \"New and Open do not change it.\" Both cannot hold. Boris sees one of two things: his stop silently undone, or the new show's BPM clips frozen with the stop button lit and no text allowed. The plan's Open path (A7, nudge glide) and step 2c (held level) are never combined in a case. No LR row or T-G case opens a show while Stopped or Paused.",
      "severity": "MUST",
      "proposed_change": "Rule one sentence. My pick: Open and New leave the timer as it is, since only he undoes a stop and the lit button shows it. Add a T-G case plus an LR2 arm: stop, Open a show, then check beatTimer and that its nudge glides under 2c."
    },
    {
      "id": "ST-3",
      "target": "NR2 'THE NUDGE AT A STOP' (R74, PL:253-257)",
      "claim": "Pressing Stop silently wipes the nudge to 0. Boris never said that, and a mid-set pad hit would undo a calibration.",
      "evidence": "His only zeroing words are for Resync: BD:855-857 (\"If I press re-sync, then it does re-sync and that changes\") and BD:868 (\"49 default ... Resync is a fresh start\"). Stop is a different button, and the plan itself says stop-then-play is only \"like Resync\" (PL:254). The plan already has the keep path (steps 2b and 2c, T-N19). With Stop on a pad, a VJ who stops and replays on the \"1\" sees the picture land 12 ms off, and the number he set is gone with no text.",
      "severity": "SHOULD",
      "proposed_change": "Default to KEEP: delete the two statements (PL:255-256), so Stop never touches the number, the same as Pause. Make zeroing the alternative if he asks. Reword B-R6 to match."
    },
    {
      "id": "ST-4",
      "target": "NR1 tooltips and glyphs (PL:163-170)",
      "claim": "The sign is stated only on hover over the buttons. The text, where he reads the number, never says which way plus goes. The glyph also contradicts the usual timeline reading.",
      "evidence": "The text's tooltip (PL:169-170) is \"How far the beat is moved ... Click to type a number.\" Direction is on the \"<\" and \">\" tooltips only. \"<\" (back) means LATER and \">\" (forward) means EARLIER (PL:161-162, 168). The usual reading of back is earlier in time. He was only told R71 (boris-clarify-111.md) and answered with a row list. \"+12\" read the wrong way round is the exact trap, and no gate looks at it.",
      "severity": "SHOULD",
      "proposed_change": "Put the direction in the text's own tooltip: \"Plus moves the beat earlier, minus later.\" Add it to T-RB5 and LR5(f). Let B-R9 ask whether back = later reads right on his screen."
    },
    {
      "id": "ST-5",
      "target": "NR3 tempo steps (PL:338-346, 356-360) + LR4",
      "claim": "Minus, plus, /2 and x2 take their base from the \"published tempo\", and the plan never says from where. The label is refreshed at 15 Hz, so fast repeats can read a stale tempo. LR4 hides the race.",
      "evidence": "PL:338 \"base = the published tempo\"; PL:345-346 the held repeat is 50 ms. TopBar::timerCallback copies the bus into displaySnap_ at 15 Hz = 66 ms (TopBar.cpp:297-306). If the handler uses the displayed tempo, two quick x2 presses both read 128 (one doubling) and a held \"+\" repeats the same step. LR4 (PL:641-648) reads /api/bpm after each step has applied, so it can never see this. B-R7's \"/2 twice from 128 -> 64, then 32\" is a hand check only.",
      "severity": "SHOULD",
      "proposed_change": "Specify the base as the last tempo the row itself commanded (a TopBar model field), re-seeded from the bus when the published tempo differs and no command is pending. Add an LR4 arm of three presses with no wait (double, double, half) that expects 256, 400 (clamped), 200."
    },
    {
      "id": "ST-6",
      "target": "NR2 question 129 default A + LR2",
      "claim": "A quantised fired clip waits behind a stopped or paused timer. LR2 pins that dead-looking pad as a PASS.",
      "evidence": "Main already refuses to queue when the beat cannot drain: \"an honest immediate trigger (Off) beats a trigger that may never drain\" (MainComponent.cpp:36-43; quantizeModeToForcedSnap returns Off when not LOCKED). A held timer is the same case, and Paused has no end the clip can see. LR2 (PL:629-631) asserts \"for 1.0 s the layer's playing clip is NOT the new one\". Boris hits a pad and sees nothing, and no text may say why. His words (BD:862: \"unless it's set to be quantized\") support waiting, so this is a default he can see and correct.",
      "severity": "SHOULD",
      "proposed_change": "Keep A. Name this in B-R5 as the place to check. Add a lit-state cue only if a seat finds the wait unreadable. Keep B (forced Off while held) as the pre-built one-line alternative, already written at PL:291-293."
    },
    {
      "id": "ST-7",
      "target": "NR4 glyphs (PL:163-167, 188, 759-760)",
      "claim": "Two identical glyph pairs are shipped, with the fix left as a fallback. A mis-press has a real cost: the routines \"[]\" is 72 px from the beat \"[]\" and kills routines mid-set.",
      "evidence": "Play \">\" and nudge \">\" share a glyph (PL:165-166); routines \"[]\" and beat stop \"[]\" share one (PL:188, 759-760). The only cue is colour (kRoutineCue). The order is routines [] | Bar N | wheel | play pause stop (PL:378-379), so only the Bar text and the wheel separate them. A nudge \">\" mis-press changes his number with no text, a routines \"[]\" mis-press stops his performance routines. The plan leaves both as named questions for critics and Boris, who sees them only after the build.",
      "severity": "SHOULD",
      "proposed_change": "Ship the pre-ruled fallbacks now: nudge \"<<\" and \">>\", and the routines stop as a short word (for example \"Rtn\"). Both are one string each. Keep the VG question for any remaining confusion."
    },
    {
      "id": "ST-8",
      "target": "NR3 the hand range 30..400 (PL:319-337) + RR4",
      "claim": "One tempo x2 press, or a pad bound to \"Tempo x2\", can push the tempo to 400. Every u_beatPhase-driven effect then strobes, and the plan admits it did not read those readers.",
      "evidence": "RR4 (PL:755-758): shaders taking u_beatPhase \"at 400 BPM (it will strobe at 6.7 Hz -- a photosensitivity matter)\" are NOT read. Detector tempos stay at or below 200 (BPMTracker.h:41-42). The plan puts \"Tempo x2\" on a pad \"at a climax\" (PL:415-421), so in Auto at 160 one press gives 320, about 5.3 Hz flashes on the wall. The 400 ceiling is the architect's choice, not Boris's. Greying x2 and plus at the ceiling does not help below it.",
      "severity": "SHOULD",
      "proposed_change": "Set kHandMaxBPM = 200, the detector's own ceiling (one constant): x2 and plus grey above it. Keep the 30 floor. Move 400 to a later ruling once the beat-phase readers are read. Adjust T-G9, T-G11, LR4, R11 and B-R7 numbers."
    }
  ],
  "strongest_point": "ST-1 and ST-2. Boris's 111 rule is that when the BPM goes stop, everything set to BPM stops. The plan's only proofs for the clip half can be skipped or can pass vacuously: LR3(c) may print INFO, there is no BPM-video clause, and T-G12 passes on a file-level token even though its own rule says function-level. The same plan also contradicts itself on what Open does to a stopped timer.",
  "citations_rechecked": true
}
```
