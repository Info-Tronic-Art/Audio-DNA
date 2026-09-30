# RULING -- lane bt2 (s-rta-0930): architect ruling on plan-bt2.md

Architect (Opus 5.5, max effort -- Fable unavailable this session, s-rta-0930-work.md 11:57 / 12:23), 2026-09-30 13:10 EDT,
main HEAD 655d232. Read in full: .harmony/.reports/s-rta-0930/plan-bt2.md. Tree untouched; scratch only
(SP = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad;
this ruling's rigs: SP/ruling/x_r13.cpp, rig_r13.sh, rig_r13p.sh). No app launched, no real device opened (mock types only).
Labels: VERIFIED (read / ran today) / INFERRED.

## 0. VERDICT FIRST

1. BLOCKED on the council part: the two attack papers the dispatch names do not exist, so no council attack is ruled.
   `attack-bt2-coreaudio.md` and `attack-bt2-gates.md` are absent from .harmony/.reports/s-rta-0930/ (polled every 5 s
   12:44-13:03 EDT). A find over /Users/boriskarpman, /private/tmp and /tmp returned 0 files named attack-bt2* and no .md
   written anywhere after 12:42. They will not appear later: the workflow starts this rule stage only after both seats
   resolved (SP/wf/plans.js:81-85, `parallel(seats).then(...)` -> rule). The seats read plan-bt2.md at 12:42:43 / 12:42:44
   EDT (session read log), this ruling started 12:43:56, and neither seat wrote a paper. A seat CAN write: its Bash
   heredoc is allowed (~/.claude/agents/authored-council-seat.md:7-8), and last session's seats did write
   (s-rta-0929b/attack-btguard-*.md). Whatever the seats returned is in the workflow result, which keeps only
   seat / verdict / MUST / SHOULD counts (plans.js:85).
2. In place of the council (labelled as such -- NOT a council substitute): I attacked the plan under the two seats' own
   briefs (plans.js:69 COREAUDIO / JUCE device-lifecycle; plans.js:70 GATES / PRODUCT), each attack checked against source
   or a scratch run and ruled. 22 attacks: 14 ACCEPT, 8 REJECT. The DESIGN is sound (no architecture or code-logic change).
   The GATES, TEST TEXT and DOCS need revision: 11 amendments (AM1-AM11). The plan is internally inconsistent in four
   places a builder would trip on (AM1, AM2, AM3, AM4).
3. ready_to_build = false ONLY because of item 1. It unblocks in one of two ways (Harmony decision H5):
   (a) the workflow result's counts show 0 MUST from both seats -> build the plan as amended below;
   (b) any seat reports a MUST -> re-run the two seats with papers persisted, then re-dispatch this ruling (the AM rulings
   stand; only the seats' new items need a ruling).

## 1. EVIDENCE RUN BY THIS RULING (today)

- E1 SP/bt2/x_bt2_red on main's objects: 6 cases, 3 failed, 7 failed assertions (XA1 / XA0 / XA2 fail, XK1 / XM / XO pass)
  -- plan S1 reproduced.
- E2 prototype (SP/bt2/p3): TDP 36/36 (201 assertions), x_bt2_red 6/6 (37), x_bt2_new 6/6 (38) -- plan S3 reproduced.
- E3 mutants (SP/bt2/m_*): noAdoptInput 9 / noInputLost 4 / noRestore 1 / noLaunchSeq 5 / noSettle 1 failed assertions
  in the new-API set. switchWorking survives that set but, relinked, is killed by TDP R2 (TDP:805-817) and by XK1 + XA2
  (x_bt2_red.cpp:54-92) -- plan S4 reproduced.
- E4 NEW: R13 (the adoption storm, written in MAIN's API: `initialiseWithDefaultDevices` + `reapplies()`; SP/ruling/x_r13.cpp)
  on 655d232's objects: FAIL, 2 assertions (reapplies 0; input ""). On the prototype (compiled against the prototype's
  headers): 6/6 pass.
- E5 NEW, a gate trap seen live: the same R13 TU compiled against MAIN's DeviceGuard.h, then linked to the PROTOTYPE's
  objects, printed "re-applying the device policy" and ended on "USB Mic", yet `reapplies()` read 0 -- the inline accessor
  read main's member layout. A RED or mutation run that mixes headers and objects from different trees gives a silently
  wrong verdict (AM5).

## 2. ATTACKS AND RULINGS (architect self-attack under the seats' briefs; council papers: 0 ruled, section 0)

### 2.1 COREAUDIO / JUCE device-lifecycle lens (plans.js:69)

- X-CA1 Change-notification timing: the reconciler may act before JUCE's own follow-ups settle, or on a stale default.
  REJECT. Every JUCE follow-up runs on the message thread: per-device detail timers of 100 ms (CA:848-852) and the
  combiner's restart timer (CA:1637-1652). Every device start / stop sends a change message (ADM:1074-1101), and the new
  `changeListenerCallback` always re-arms the 250 ms settle. The TYPE rescans only on kAudioHardwarePropertyDevices
  (CA:2119-2126, listener CA:2301-2304), so a default flip alone never rescans. That stale-default case is exactly the
  plan's R11 (allowed devices only, and adoption only when the app has no input).
- X-CA2 The known 8.0.4 overflow: bt2 opens a WIRED interface MID-SHOW (adoption, or the InputLost fallback). btguard's
  plan section 9 made "the first wired interface" the trigger for its frame-size check.
  ACCEPT -> AM7 (a precondition on Boris check B2 + a Harmony ledger line). No code change: a launch with that interface
  plugged in runs the same `CoreAudioInternal::reopen` (pitfalls.md:131 "Why"); bt2 changes only WHEN it runs.
- X-CA3 Re-entrancy: `openDefaultDevices()` inside `timerCallback` meets the in-start list change (CA:1945-1946).
  REJECT. `sendChangeMessage` is asynchronous (juce_ChangeBroadcaster.cpp:77-81), so the reconciler is never re-entered
  synchronously. The in-start rescan happens INSIDE the open (`audioDeviceAboutToStart` is called synchronously,
  CA:686-693), and `lastAttemptSeq_` is read after the open returns. TDP R5 (TDP:891-904) is green on the prototype (E2).
- X-CA4 Aggregate devices. REJECT. An aggregate is one name per list (same device ID both ways -> a plain
  CoreAudioIODevice, CA:2250). A member loss that makes BG2 deny the aggregate removes the name from the FILTERED lists,
  which JUCE's own list handler reads (ADM:193-233 via the guarded type) -> close + re-init (the NoDevice path); for the
  input half, InputLost. Nothing new.
- X-CA5 The startup open after C3. REJECT. The launch is `initialiseWithDefaultDevices` alone. The input channels are the
  defaults (ADM:711-728). Analysis reads input channel 0, and the tap sizes itself from the active OUTPUT channels
  (CC:87-121, CC:191-196) -- unchanged. The one side effect (no explicit-settings XML at launch -> the mono mic's vanished
  device takes the reconciler path) is M5c, green on main (E1: XM passes).
- X-CA6 The Bluetooth guard on every NEW path. ACCEPT (test rule only) -> AM1 last bullet. All three new open paths cross
  the decorator: `openDefaultDevices` (the launch open through the guarded type); the restore (`setAudioDeviceSetup` ->
  `deviceListContains` + `createDevice`, ADM:759-774 / DG:58-61, names pre-filtered against the scan); the runtime deny
  (`rebuild` through `dp::filter`, DG:71-95). But the plan states the spy assert only for R6.
- X-CA7 InputLost after a REAL unplug closes a DEAD combiner on the message thread: up to 2 s stall, plus the HAL-thread
  `restartAsync` race window. REJECT as a blocker (keep R1 / R2 as named). JUCE's own output-unplug path already closes a
  dying device on the message thread (ADM:219 -> CA:734-757). Today's alternative is permanent deafness with state "ok"
  (V10 / V11). The destroy lands >= 250 ms after the last change message, after the HAL's notification burst (INFERRED).
  Boris check B2(b) stays.
- X-CA8 Alternative: key InputLost on liveness (`isPlaying`) instead of the name, which would also cover R12 and error
  shutdowns. REJECT. `isPlaying` is not a liveness signal in JUCE 8.0.4: the combiner answers `callback != nullptr`
  (CA:1511), but a plain device stopped with a pending callback keeps `playing` true -- `stopWithPendingCallback`
  (CA:1342) uses `stop(true)`, and `playing = false` runs only when `!leaveInterruptRunning` (CA:734-757, :755). The probe
  also cannot exercise it (a runtime deny keeps the device alive). R12 stays a named residual.
- X-CA9 The restore is a by-name open, which Pitfall 61 forbids ("Never open a device by name from app code",
  pitfalls.md:131 -- OUTSIDE the span the plan's section 6 replaces, so after bt2 the pitfall would contradict the code).
  ACCEPT -> AM6.
- X-CA10 Adopting a mic while the performer plays a FILE restarts the working device, which also pulls the file through
  its callback (CC:124-134): a short analysis gap. ACCEPT as a named risk (AM10 R15), no code change. The only
  alternative (defer adoption until mic mode) puts a device call back into `setSourceMode`, which C3 removes. The converse
  also holds: a dead input stops BOTH combiner wrappers (CA:1735-1752), so File-mode analysis is ALREADY dead after a
  pulled wired-mic cable today, and InputLost repairs it.
- X-CA11 A persistently failing allowed input keeps AdoptInput true, so every later device-list change retries once,
  each retry restarting the working output. ACCEPT as a named risk (AM10 R16); the design change it suggests
  (content-keyed instead of scan-keyed gating) is REJECTED. It would break the merged R4b semantics (TDP:867-889: "a
  failed re-apply waits for a NEW device scan"). The bound holds (<= 1 per 10 s), and the scan seq advances only on real
  device-list changes (CA:2119-2126), never on default flips.

### 2.2 GATES / PRODUCT lens (plans.js:70)

- X-GP1 The RED cases cannot fail on 655d232 as specified. Plan I3 says "TDP: appLaunch -> reconciler.openDefaultDevices()
  wherever a reconciler exists (M5b, M5c, R1-R5, the new cases)". `openDefaultDevices` does not exist on 655d232, so R6 /
  R7 / R8 do not compile there and M5c cannot "PASS on main" (plan I2). The RED evidence (x_bt2_red.cpp) is main-API text
  the committed file would not contain. ACCEPT -> AM1.
- X-GP2 G3's method (SP/bt2/rig.sh compiles against $ROOT/src) mixes headers and objects when run from the lane tree:
  a silent wrong verdict (E5). ACCEPT -> AM5.
- X-GP3 "TDP +11" names only ten cases (M5c, M8, P14, R6-R12). The adoption storm the mutation table relies on (S4:
  "storm" kills noAdoptInput and noSettle) is unnamed. ACCEPT -> AM2 (R13; RED / GREEN measured, E4).
- X-GP4 G6's RED (the pre-merge 655d232 copy, 25 / 12 / 2) fails 10 of its 12 rows on a missing route (404), not on
  behaviour, and contradicts the plan's own I1 RED (the K1 build, 26 / 11 / 2). ACCEPT -> AM3.
- X-GP5 A12's bar is absolute ("reapplies still 2", plan I1), yet the plan says A12 "passes vacuously" at K1, where
  reapplies is 0. ACCEPT -> AM4 (a delta bar).
- X-GP6 A12 needs both POSTs' list changes to land inside the 250 ms settle; nothing checks that they did, so a slow
  message-thread hop turns it into a false FAIL. ACCEPT -> AM4 (a measured gap guard, one retry).
- X-GP7 The opens counter's teeth. A6's RED exists only with a MONO mic: a stereo mic's startup re-open is an early return
  (ADM:735-738), so main reads 1 there. This rig reads 2 (btguard.md:29), mono INFERRED from that. A6b fails on main for
  both shapes (mono 4; stereo 2: File's `clear()` differs, the mic's `setRange` does not). ACCEPT -> AM11 (a gate note).
- X-GP8 The hot-plug stimulus never reaches the HAL side (a dead combiner, the detail timers). REJECT as a MUST. It is
  already named (I5, R4), and the alternatives are forbidden (real hardware, a system setting, a driver install).
  A TEST-ONLY "stop the device" route to mimic V10 was considered and rejected: more TEST-ONLY surface, and a
  stopped-but-alive device still does not reproduce the dead-device close.
- X-GP9 G5's TCC tripwire covers one launch, but B7 and C8 open an INPUT in the middle of a run. ACCEPT -> AM8.
- X-GP10 Product: is the notice now true? REJECT (no change). "Plug one in" is now what happens in both states
  (AdoptInput for "No wired mic found"; NoDevice for "No audio device found" -- live C7 / C8). The one misleading edge (a
  plugged mic that fails to open keeps the note up, and JUCE's error reaches the label through `onError`, AE:16-17) is
  the plan's R13 NIT.
- X-GP11 Product: "never switch away from a working input" hides the most common gig case. A MacBook on its own mic, a
  wired mic plugged in mid-show (macOS usually makes it the default): the app stays on the MacBook mic with no sign the
  wired mic is ignored. Plan Q1 asks only about the re-plug after a drop-out. ACCEPT -> AM9 (Q1 covers both, default
  unchanged; B2 gains a step so Boris sees it). What a performer sees mid-show is then covered by B2 (a)-(d) + R2 / R15.

## ARCHITECT RULING (s-rta-0930)

Numbered amendments. Each OVERRIDES the plan body where they differ; everything not named here stands as written in
plan-bt2.md (design, files, K1 -> K4 order, the I1 row bars except A12, section 6 docs text except where AM6 edits it).

AM1 -- the test text runs on 655d232 (X-GP1, X-CA6). Replaces plan I3's "TDP: appLaunch -> reconciler.openDefaultDevices()
     wherever a reconciler exists (M5b, M5c, R1-R5, the new cases)" and refines I2's TDP bullet.
     - `appOpenSequence` (TDP:721-728) becomes `appLaunch(adm)` = `REQUIRE(adm.initialiseWithDefaultDevices(2, 2).isEmpty());`
       only. M5b and R1-R5 call `appLaunch`. This is behaviour-neutral: their fixtures use a STEREO mic, where the old
       re-open was an early return (ADM:735-738). M5 keeps its explicit-settings step inline (as the plan says).
     - R6, R7, R8, R9, R13 and M5c use MAIN's API ONLY: `initialiseWithDefaultDevices(2, 2)`,
       `DeviceReconciler(*rig.manager, 2, 2)`, `reapplies()`, `getAudioDeviceSetup()`, the spy. Their bodies are those
       of SP/bt2/x_bt2_red.cpp -- XA1 -> R6, XA0 -> R7, XA2 -> R8, XK1 -> R9, XM -> M5c -- and SP/ruling/x_r13.cpp ->
       R13. None of `openDefaultDevices`, `lastAction`, `dp::reconcile` appears in these six.
     - Only M8, P14, R10, R11 and R12 use the new API (their bodies: x_bt2_new.cpp XC, XP, XL, XA1f, XR).
     - Every new manager case whose fixture contains `kDenied` ends with `CHECK_FALSE(rig.spy.saw(kDenied));`.
AM2 -- name the eleventh TDP case (X-GP3): R13 "adoption storm: 20 list changes while a mic appears (output-only) ->
     exactly one adoption, never before the settle" = SP/ruling/x_r13.cpp verbatim. Output-only launch; "USB Mic"
     appears; fire; pump 100 -> reapplies 0; 19 more fires 20 ms apart; pump 800 -> reapplies 1, input "USB Mic",
     `!spy.saw(kDenied)`. RED on 655d232 (E4: 2 failed), GREEN on the prototype (E4). TDP +11 = M5c, M8, P14, R6, R7,
     R8, R9, R10, R11, R12, R13.
AM3 -- the live RED of record (X-GP4). Replaces plan section 0 item 5's and G6's "RED on the pre-merge copy 25 / 12 / 2".
     The RED of record is the lane's K1 build (the TEST-ONLY route present, 655d232's behaviour): 26 PASS / 11 FAIL /
     2 SKIP; FAIL set exactly {A6 (reads 2), A6b (4), A10, A11a, A11, B7, B7b (3), B8, C7, C8a, C8}; A12 PASSES there
     (AM4's delta bar). SHOULD, one run: the K2 build (C3 only) = 28 / 9 / 2, FAIL set = K1's minus {A6, A6b}, B7b reads
     1. The 655d232 copy may be run as INFO only -- its POST rows fail on a 404 and prove nothing about behaviour.
AM4 -- A12 (X-GP5, X-GP6). Replaces plan I1's A12 bar.
     - Start >= 10.5 s after A11's re-apply (unchanged; the no-settle mutant must be outside the 10 s bound).
     - Record r0 = `reapplies` and o0 = `opened{}` just before the first POST. POST deny [IN], then POST deny [] with no
       sleep between; gap = wall time between the two 200 responses. At +1.5 s:
       PASS iff reapplies == r0 AND `opened{}` is byte-identical to o0 AND gap <= 0.15 s.
     - If gap > 0.15 s, repeat A12 once after a fresh >= 10.5 s wait. A second overrun = FAIL "the two list changes did
       not land inside the 250 ms settle" (investigate the message thread; never a pass).
AM5 -- the RED / mutation method (X-GP2). Replaces G3's "(the SBX/rig.sh method)" and qualifies G4.
     - G3 runs in a clean worktree: `git worktree add "$TMPDIR/wt655" 655d232`. There, compile DeviceGuard / DevicePolicy
       / CoreAudioDeviceInfo (and AudioEngine / AudioCallback / AudioTap for AE1) with main's compile_commands flags and
       `-I$TMPDIR/wt655/src` FIRST.
     - The scratch TU = the lane TDP's fixture prelude (everything before the first TEST_CASE; TDP:1-266 today) + the
       six AM1 cases extracted verbatim by name. The extraction script prints `diff` against the lane file = empty.
     - Link each TU only to objects built from the same tree's headers -- never lane headers with 655d232 objects, or
       the reverse (E5). G4 likewise: each mutant's TUs and objects compile against that mutant's own copy.
AM6 -- Pitfall 61 (X-CA9, X-GP3); docs/claude/pitfalls.md:131, in addition to the plan's section 6 edit.
     - Replace "Never open a device by name from app code, never add a picker that bypasses the manager, never read the
       manager on the HTTP thread." with "Never open a device by name from app code -- the one exception is
       `DeviceReconciler`'s restore after a failed re-apply, which re-opens through the guarded manager (its type refuses
       a hidden name) only the still-listed half of the device the policy itself had opened -- never add a picker that
       bypasses the manager, never read the manager on the HTTP thread."
     - In the plan's "Guards:" append, after "R12 (lost -> output-only -> re-plugged -> adopted)", insert "; R13 (20 list
       changes while a mic appears -> one adoption, after the settle)", and make "R6 / R7 / R8 (... -- RED on 655d232)"
       read "R6 / R7 / R8 / R13 (... -- RED on 655d232)".
AM7 -- the 8.0.4 overflow precondition (X-CA2). Plan section 8 B2 gains, before (a):
     "Before B2, with THAT wired mic / interface: Harmony runs btguard plan section 9's check (bt-crash-mechanism.md E1,
     .harmony/.reports/s-rta-0924b/bt-crash-mechanism.md:129-141 -- `ca_probe_set 512 input|output`, read-back at
     +0 / +50 / +500 / +2000 ms and frames delivered). E1 reads the macOS DEFAULT device, so Boris makes the interface the
     default for those two minutes; Harmony never changes a system setting. Read-back == 512 and delivered == 512 -> B2
     proceeds; otherwise B2 waits for btguard section 9 option (a) or (c)."
     Harmony carries the same line in the handoff loose-ends ledger ("bt2 adds mid-show opens of a wired interface").
AM8 -- the TCC / dialog tripwire covers every live run (X-GP9). Replaces G5's "one launch". Before the first live run AND
     for each live run window (AM3's K1 run, the K2 run if made, each GATE-6 run): `log show --start "<run start>"
     --predicate 'process == "tccd"'` shows no new AUTHREQ for com.audiodna.app; 0 UserNotificationCenter windows
     (kCGWindowListOptionAll) at >= 15 s after each quit. Any prompt -> STOP (Boris's screen; never dismiss it).
AM9 -- the switch question covers the common case (X-GP11). Plan section 9 Q1 becomes:
     "If the app is listening to the MacBook's own mic -- because your wired mic dropped out mid-show, or because you
     plug a wired mic in while the app is already running -- what should happen when the wired mic is plugged in?
     (a) DEFAULT: stay on the MacBook mic until you relaunch -- the app never switches away from a mic that works;
     (b) switch to the wired mic by itself within about a second (like an output window that comes back when its
     display is plugged back); (c) for the drop-out only: never fall back to the MacBook mic -- show the 'plug one in'
     note and wait for the wired mic."
     Plan section 8 B2 gains "(d) with the app on the MacBook mic, plug the wired mic in -> by default nothing changes
     (the text line still reads 'Mic: MacBook Pro Microphone ...') -- this is the Q1 default you are judging." No code
     change: Keep stays the build default.
AM10 -- the risk register (X-CA10, X-CA11). Append to plan section 7:
     - R15 Adopting a mic while a FILE plays restarts the working device, which also pulls the file (CC:124-134): one
       device restart's worth of no analysis. Bounded to a mic plugged while the notice shows, <= 1 per 10 s. Conversely,
       a pulled wired-mic cable ALREADY stops file analysis today (both combiner wrappers stop, CA:1735-1752), and
       InputLost restores it.
     - R16 A persistently failing allowed input keeps AdoptInput true: each later device-list change (any device, a
       denied one included) retries once, <= 1 per 10 s, each retry restarting the working output. Kept -- the merged
       R4b semantics (TDP:867-889). The scan seq advances only on kAudioHardwarePropertyDevices (CA:2119-2126), never on
       a default flip.
AM11 -- the opens teeth (X-GP7). Added to plan G6 / G7 as a note, not a new row: A6's RED needs a mono default mic (this
     rig: 2 on main, btguard.md:29); A6b is the unconditional live C3 witness (main: mono 4, stereo 2); AE1 is
     rig-independent (no device).

### FINAL GATE LIST (replaces plan section 5; bars pre-registered)

- GATE-1 Build clean; no new warning in a touched file.
- GATE-2 `ctest --test-dir build -j1`: 100% pass. 1061 cases = 1049 + TDP 11 (AM2 list) + AE1; 112 targets; the
  builder records the real `ctest -N`. One failure = RED; a flake verdict needs >= 5 runs.
- GATE-3 Unit RED on 655d232 (AM5 method): R6, R7, R8, R13 FAIL (reapplies 0); R9 and M5c PASS; AE1 FAIL (the
  explicit-settings XML is set and >= 1 change message arrives; S2: 3 failed assertions). A predicted-RED case that
  passes = toothless -> back to the builder.
- GATE-4 Mutation table on the lane's real code (AM5 method). Each mutant must be killed by >= 1 case of its set:
  no AdoptInput {R6, R11, R12, R13, P14}; no InputLost {R8, R12, P14}; switch a working mic {R9, R2};
  no restore {R11}; launch seq not recorded {R10, R11, R4b}; no settle {R1, R13}. A survivor -> back to the builder.
- GATE-5 Live RED of record (AM3): K1 build = 26 / 11 / 2 with exactly the AM3 FAIL set, A12 PASS. SHOULD: K2 = 28 / 9 /
  2. Another FAIL = rig problem -> investigate before merging; a predicted FAIL that passes = toothless row -> fix the row.
- GATE-6 Live GREEN on merged main, 5 of 5 runs: 37 PASS / 0 FAIL / 2 SKIP; every quit clean (0 .ips, 0 dialogs); A12's
  gap guard never exhausted. The plan G7 flake rule for A6 stands (a C3 regression reads 2 on EVERY run; a JUCE detail
  restart only sometimes -> 5 more runs before a verdict).
- GATE-7 TCC / dialog tripwire per live run (AM8).
- GATE-8 Audio battery on merged main = plan G8 verbatim (orphaned-burner check first; step3 94/0, manual-bpm 22/0,
  resync 16/0, downbeat-level 14/0, onset-render 13/0, tempo-start 10/0, finalize-loop 8/0 (40 cycles), routines 109/0,
  async-load audio witness rows; a FAIL -> 5 interleaved merged-vs-pre-merge runs before attributing it to bt2).
- GATE-9 Production absence = plan G9. The `strings` check also covers "setTestDeniedNames" / "debugSetDeniedDevices"
  -> 0 hits; every new debug symbol sits inside `#if AUDIODNA_TEST_SERVER`.
- GATE-10 Sacred rules = plan G10 (CombinedCallback.h / AudioCallback.cpp diff empty; no new std::mutex outside the
  TEST-ONLY block; reconciler + hook message-thread only).
- GATE-11 Docs:
  - CLAUDE.md still 23,999 B.
  - pitfalls.md: "The startup `setSourceMode` re-open stays" occurs 0 times.
  - The AM6 exception sentence occurs exactly once; "R13 (20 list changes" occurs once.
- INFO only (no bar): launch -> /api/health time, interleaved A/B >= 5 runs per arm.

### Boris / Harmony

- Boris questions (defaults keep the build unblocked): Q1 as AM9; Q2 as plan section 9 (default: the Mic text line +
  the yellow note are enough).
- Boris checks: B1 as the plan; B2 with AM7's precondition and AM9's (d); B3 as the plan.
- Harmony decisions: H1-H4 as the plan (defaults yes). H5 NEW: how to treat the missing council papers (section 0 item 3).

### COMPACT

- Council: both attack papers absent; the attack stage had already resolved (plans.js:81-85) -> 0 council attacks ruled
  (BLOCKED). Self-attack under both seat briefs: 22 attacks, 14 ACCEPT / 8 REJECT. Design sound; gates / test text /
  docs revised.
- AM1 the RED cases in main's API (as written, the plan's RED cases could not compile on 655d232); AM2 R13 adoption storm
  (RED on 655d232 / GREEN on the prototype, measured); AM3 RED of record = K1 build 26 / 11 / 2 (not the 404-driven
  655d232 copy); AM4 A12 delta bar + settle-gap guard; AM5 same-tree headers and objects for RED / mutants (a mixed pair
  reads wrong fields -- seen); AM6 Pitfall 61 by-name exception + R13; AM7 8.0.4 E1 check before Boris's wired-mic
  test; AM8 TCC tripwire every live run; AM9 Q1 covers "plugged while on the MacBook mic"; AM10 R15 / R16; AM11 A6 teeth
  rig-conditional.
- Gates: 1061 ctest cases / 112 targets; unit RED {R6, R7, R8, R13, AE1}, guards {R9, M5c}; 6 mutants with pre-registered
  kill sets; live RED 26 / 11 / 2 (K1), GREEN 37 / 0 / 2 on 5 of 5 runs.
- Learnings for Harmony (not logged from here -- outside this dispatch's write fence):
  (1) a RED or mutation re-check must build the test TU and the objects under test from ONE tree; a mixed pair runs,
  prints plausible logs and reads wrong fields.
  (2) a workflow whose seats return StructuredOutput can leave no paper on disk while the rule stage still runs --
  persist each seat's output to its REPORT_FILE in the workflow before dispatching the ruling.

STATUS: BLOCKED (council papers absent; 0 council attacks ruled) -- self-attack ruling complete: 22 attacks (14 ACCEPT / 8 REJECT), 11 amendments, final gate list; ready_to_build=false pending Harmony H5.
