# RULING bf2 GATES RE-STATED -- ruling-bf2-delta section 5 after Boris's two overrides (lane bf2, s-rta-1003b)

ROUND 3, THE FINAL FOLD. Edited in place on the round-2 text. Folded in: Harmony's rulings H-2 .. H-7 (HR:); the
merged lane tree c45b579, with every line number of sections 1-3 re-pinned to it; a ruling for a venue store that is
full; the second review R2: (five SHOULD, five NIT), each finding checked at its source before anything was changed.
Section 8 says what was folded, which INFERRED facts are now VERIFIED, which changed, and gives each finding with
ACCEPT / REJECT. Section 7 is the record of round 2 and keeps the line numbers of that round.

Restates, from .harmony/.reports/s-rta-1003/ruling-bf2-delta.md: section 5 rows R11 and G7 and G1's S5a / S5b suite
lists; amendments D13, D14, D15, D16, D20 (and, outside the asked range, D21, D22, the S5a stage row and five plan B6
sentences); sections 6-7. Every row, amendment and sentence this file does not restate stands as written there.
Read-only: nothing was built, run, launched or probed. Trees read ONLY through `git show`: C: = lane/bf2 at c45b579
(the merged tree: M0 is in it), M: = main at 34179a2, L: = lane/bf2 at 4a1f240 (round 2; cited in section 7 only).
No file of the bf2 worktree was read.
Paths: RD: = .harmony/.reports/s-rta-1003/ruling-bf2-delta.md. PD: = .harmony/.reports/s-rta-1003/plan-bf2-delta.md.
P: = .harmony/.reports/s-rta-1002b (plan-bf2.md, ruling-bf2.md). BD: = .harmony/binding-decisions.md.
FN: = .harmony/.reports/s-rta-1003/facts-notices.md. HR: = .harmony/.reports/s-rta-1003b/rulings-bf2.md.
RV: = .harmony/.reports/s-rta-1003b/review-bf2-gates-restated-r1.md. R2: = the same folder's
review-bf2-gates-restated-r2.md.
J: = build/_deps/juce-src/modules/juce_gui_basics (the JUCE checkout in the main build folder: a dependency, part of
neither tree).
Labels: VERIFIED (read at the cited line), INFERRED (reasoned from verified lines), ASSUMED (not checked).
"The tree before S5a" = the lane after M0, S3f, S4 and S6. C: holds M0; S3f, S4 and S6 are still to be built
(RD:514-516). What a row reads before S5a is VERIFIED wherever a C: line is cited. That the three stages leave it so
is INFERRED from their scope (F40), and lines they add above a cited line will move its number. "The tree before
S5b" = the same plus S5a.

## 0 VERDICT

QUESTION: which gate rows, amendments and Boris-page lines still expect the load notice or the key-less "Default at 0"
save that Boris withdrew on 2026-10-03, and what does each say now.

APPROACH: every clause that measured a withdrawn thing is removed or turned into its opposite ("no text"); every other
clause is kept word for word. The replaced value (D15) is DROPPED. Round 2 makes the re-stated rows able to fail: a
wait wherever a POST answers before its work is done; "ok": true on every on-screen-text read; a second no-text
detector that sees every text widget, compared against a control load; one clause for a store that holds only
"Default"; the modal bar made passable in C9; no app-log line. Round 3 closes what the second review found (a load
that did not happen, a file left from an earlier run, a text list that reads nothing, a text that names neither
word) and one hole the check turned up (a control that itself carries the text); rules the full venue store (the
adoption is refused whole: nothing changes, nothing is written on screen); and folds Harmony's rulings in.

TRADEOFFS CONSIDERED
 - The notice clauses of R11 (b)-(d). Delete them and put nothing in their place: rejected -- nothing would then
   fail a builder who writes the sentence into the file label. Turn them into "no text": taken.
 - The second detector (round 2). A word list over every text in the window: rejected -- "Resync" and the fixture's
   own file name hold "sync" (F31). A whole-window pixel diff inside R11: rejected -- S5a takes no captures, and the
   picture, the wheel and the meters move. The dump lists every text widget and the row compares it with a control
   load of the same file at the same final dial: taken. In S5b, where captures exist, the uncropped captures of C13
   and of its control go to the logic seat as well.
 - The wait for R11 (g). The file label ("Loaded deck: <name>"), as the review proposed: rejected as the witness --
   that text is an event text of the removal inventory and leaves with that lane (F28). The deck tab count, read on
   the message thread: taken.
 - The app-log line of round 1: dropped. Nobody asked for it, no gate held it, and adoption item 7 withdraws "any
   other event text S5a / S5b would add".
 - "modalComponents" in C9 (round 2). Leave C9 out of the bar: rejected -- the panel could turn modal there unseen.
   Count only modal components that are not menus: rejected -- JUCE's menu window type is private, a builder would
   have to guess. 0 without a menu and exactly 1 in C9: taken.
 - The replaced value. A recall button in the panel: runner-up (section 4). Undo: not available (F14, F15). Drop:
   taken.
 - C11. Keep the refusal caption and call it a state: rejected (F17, a failure report). A silent refusal: taken, and
   CONFIRMED by Harmony (H-2) on Boris's fourth line (F33). An OK button greyed while the name is empty or taken: a
   follow-up, not ruled here; H-2 calls such a control a state display, so it is admissible.
 - The critic seats. Three, as RD has them: rejected (adoption item 5 names five). Five, the two new ones judging
   the pictures against the dump while D20's unit tests stay: taken.
 - A new lint on guessed names: declined (K3).
 - The full venue store (round 3; section 8.3). Put the show's number on the venue he is on: rejected -- it writes
   over a room's number without a sign, and it is what the most natural wrong build does by accident (a refused
   createVenue followed by setMs). Drop a venue to make room: rejected -- the same loss, and no venue carries a
   last-used stamp to choose by. Refuse the whole adoption, change nothing, write nothing: taken.
 - The whole-text comparison of C13 with its control (round 3). Let the dump mark the texts that move, as R2:
   proposed: rejected -- it needs an audit of every label in the app and hands a builder the switch that hides a
   text from the comparison. Replace every number by "#" and compare the two lists as counted sets: taken.
 - The file-name cut (round 3). Fixed long stems and a cut tied to a space or a colon, as R2: proposed: not taken.
   Only one loaded file has a name that holds "sync", the fixture; the cut is now that one fixed name and no
   scratch stem is ever cut, so no stem can take letters out of "Warehouse" or "sync".
 - The detector's live proof (round 3). A remark under RED, as round 2 had it: rejected -- Harmony copies gate
   strings from the row only. A line of the row, as R7's mutant app is (RD:573-575): taken.

WHAT CHANGED
 1. R11: (a3) now expects the key ({"venue":"Default","ms":0}); (b), (c), (d) expect NO text, on two detectors; (d)
    expects no "replacedByLoad"; (h) is new (a show saved on "Default" 0 sets the dial to "Default" 0); (i) is new
    (the same on a store that holds only "Default"); (k) is new (the control load the text clauses compare with); (j) is
    new in round 3 (a full venue store: nothing changes, nothing is written; section 8.3).
    (a) keeps its words and gains one sentence (the copy A). (a2), (e), (f), (g) keep their words; what (g) and the
    saves wait for is in the row's WAITS paragraph. Two defects of the row as written are fixed without touching a
    bar: (a2) overwrote "the file of (a)" before (c), (d), (e), (g) loaded it (F18); three of its POSTs answer
    before their work is done (F23).
 2. G7: C13 is "exactly like the same value set by hand", with a control state C13k; C11 is "a refused name leaves
    the panel as it was" -- the refusal caption is a failure report on screen, which Boris removed app-wide (F4,
    F17). C11 was NOT in the dispatch's list; Harmony has CONFIRMED it (H-2; K2 is closed). The critic panel has five
    seats again. The "modalComponents" bar is stated so that C9 can pass it (F29); that change comes from JUCE, not
    from Boris; Harmony has ADMITTED it (H-5; K10 is closed: the S5b builder measures the count on the first C9
    dump).
 3. G1: "the LoadNotice::forLoad combinations" and "replacedByLoad" leave the S5a list. A builder could not have
    passed G1 with them anyway: on main and in the merged lane the identifier LoadNotice anywhere in src/ fails a
    lint (F7). One suite is added: the unit test of the text list the dump carries.
 4. "replacedByLoad": DROP (section 4). A show that selects ANOTHER venue leaves the old venue and its number in the
    venue list. A show that names the venue he is ON with another number replaces the number, and nothing keeps the
    old one. For a user who never made a venue everything is on "Default", so that second case is his usual one.
 5. The Boris page: both questions are answered and removed; one question is left, with a default (a way back to a
    replaced number: default no). One check is added for the "Default"-only case (6.12). The question is printed
    directly under that check (H-6); nothing is built for it.
 6. Round 3. R11: every load must answer "ok": true; P, Z and A must not exist before they are written; the sync
    POSTs are waited for like the others; the cut is the fixture's name only; (k) proves the text list reads the
    window and is itself held against a reading taken before any load; (j) is new; the mutant app is a line of the
    row. G7: the modal bar in H-5's words; the whole text list of C13 against C13k, numbers aside; the same anchor
    against the hand-set states C3 and C8; two seat questions (H-2's, and one whole-window reading). G1: the
    full-store unit cases. Every one of these adds a way to fail; none removes one.

## 1 FACTS RE-READ

Boris (quoted only from BD:)
 F1  BD:696-697 (section recorded 2026-10-03 14:55:32, BD:687). Boris: "every show remembers it's sync". Recorded
     reading: every saved composition carries its venue and sync value, 0 included; opening it sets the dial to that
     value; supersedes ruling-bf2-delta's default for a show saved at "Default" 0. VERIFIED.
 F2  BD:704-707 (recorded 2026-10-03 15:30:49). Boris: "We don't need any text indicating what has happened or what
     has happened. That is something that happens online and is not necessary in this application. It is extra
     overhead and bloat. Please remove it cleanly and completely." Recorded reading: no on-screen notice announces an
     event; supersedes, by name, "ruling-bf2-delta's composition-open sync notice". VERIFIED.
 F3  BD:698-700 (recorded 2026-10-03 14:58:10). Boris: "I don't wanna see an under removed button at all. We just use
     control Z. The only place that we will see undo remove, will be in the top edit menu." VERIFIED. This is his one
     recorded answer about a control that appears after an event to bring something back.
 F4  BD:708-711 (recorded 2026-10-03 15:42:01). Boris: "remove the list entirely and cleanly". Recorded reading: every
     event-announcing on-screen text is removed, "pre-existing ones and failure reports included". VERIFIED.
 F5  BD:695. Gain -- Boris: "yes" (twice as long, same scale). BD:676-677: Q10, opening a composition loads its saved
     venue and sync value -- Boris: "yes". VERIFIED.
 F6  Adoption items 5-7, PD:684-696. Item 5 names five critic seats: "visual-design, UX, graphic-design, logic,
     interaction-logic" (PD:685). Item 6: every save writes the key, "Default" at 0 included; a file without the key
     never touches the dial (PD:687-690). Item 7: no notice text, "together with any other event text S5a / S5b would
     add"; the SYNC button's current value is a state display and stays; the replaced value "stays only as a control
     (not a message), or is dropped if it needs a text" (PD:691-696). Item 6's "the SYNC note shows the change" is
     the SYNC button itself: PD:375-376 calls the button "the note of page question 12". VERIFIED.
 F33 BD:718-723 (recorded 2026-10-03 20:59:50, s-rta-1003b). Asked whether the two "Save failed" boxes stay as the one
     exception, Boris: "ok. the only fail message will be a failed save. remove all others". Recorded reading: the
     save-failed alert stays (FN: B06, B08); every other failure text and every event text is removed. VERIFIED.
     (HR:10 stamps this line 21:03:06, the time HR: itself was written, HR:3. BD: is the source: 20:59:50.)

What a load notice is on main today, and which code shows it
 F7  Main has NO load notice, and neither has the merged lane. `git grep -i 'LoadNotice\|showLoadNotice\|forLoad'`
     over src/ tests/ docs/ returns one line on M: and the same line on C:: the regex of a lint that forbids the
     name. tests/test_render_thread_lint.cpp:444-480 (the same lines on both trees), case "bf9b B4j: no source-deck
     badge, tab dot, Undo Remove button or load notice identifier left in src/" [lint][bf9b]: any line of any .h /
     .cpp / .mm under src/, comments included, that matches
     `undoHint|UndoHint|undoRemoveHint|Undo Remove|loadNotice|LoadNotice|load_notice|\bNoticeLabel\b|Removed deck`
     (:450-451) fails `CHECK(count == 0)` (:479). VERIFIED on M: and on C:. So RD:160-163's F26 ("shows ONE notice
     built by LoadNotice::forLoad", read in the recon worktree a7491d4) describes neither tree, and RD:397-399's
     "LoadNotice::forLoad takes a third argument" cannot be built without failing G1.
 F8  What a composition open DOES write on screen, on main and on the merged lane alike: one write to the file
     label, `setFileLabel(done)` (C:src/MainComponent.cpp:3407; Load Deck / Duplicate: :3445), text "Loaded: <name>"
     / "Loaded deck: <name>" / "Duplicated deck: <name>" from stagedload::doneLabel (C:src/core/StagedLoad.h:60-68).
     An old show's conversion goes to the app log only: `logLine(composition_.migrationNote)` (C:src/
     MainComponent.cpp:3403-3404; BD:609 "one app-log line, nothing on screen"). VERIFIED. (On M: the same three
     places are :3360, :3398 and :3355-3357.)
 F9  GET /api/debug/ui_text returns exactly five strings: "file_label", "audio_notice", "inspected_layer",
     "inspected_clip", "inspector_tab" (C:src/api/ApiServer.cpp:2125-2129). "file_label" is fileLabel_.getText()
     (C:src/MainComponent.cpp:2158), the text of a juce::Label the window shows (`addAndMakeVisible(fileLabel_)`,
     :213). "inspected_layer" and "inspected_clip" are names read from the MODEL (`juce::String(l->name)`, :2163-
     2170) and "inspector_tab" is a word made from an enum (:2171-2178): none of those three is read from a text
     widget. VERIFIED. The route cannot see a label a builder adds anywhere else.
 F10 The merged lane has no load notice: the Composition branch of finishStagedLoad is the swap, the log line and
     `setFileLabel(done)` (C:src/MainComponent.cpp:3394-3410). `git grep -i 'LoadNotice\|replacedByLoad\|forLoad'`
     over C: src/ tests/ docs/: the lint regex of F7 and nothing else. VERIFIED.

Saving and loading
 F11 Three direct `composition_.saveToFile(` calls in C:src/MainComponent.cpp: :3598 (saveComposition, :3593), :3621
     (saveCompositionTo, :3618), :6741 (Collect media). VERIFIED. So test_save_path_lint reads three before S5a, as
     RD:389-390 says.
 F12 POST /api/debug/save_composition is in the merged lane (route C:src/api/ApiServer.cpp:333; handler :2199-2218;
     wired to saveCompositionTo, C:src/MainComponent.cpp:2157). A body without an absolute "path" answers 400 "path
     (an absolute file in an existing folder) required" (:2203-2208). VERIFIED. (Round 2 had the route on main only
     and INFERRED that M0 would bring it.)
 F13 No composition sync key exists: C:src/model/Composition.h has no "sync" (grep -i, no hit); `git grep
     'adoptFromComposition\|SyncSetting\|applyCompositionSync'` over C: src/ tests/: no hit; no route of C:src/api/
     ApiServer.cpp holds "sync_ui" (grep). The one widget named for it is an unrelated `syncButton_{"Sync"}`
     (C:src/MainComponent.h:392), hidden (C:src/MainComponent.cpp:2786). VERIFIED.
 F14 A composition open is not undoable and clears the undo history: "Loading/replacing/appending is not itself
     undoable", `undoManager_.clear()` (C:src/MainComponent.cpp:3203-3207, inside swapCompositionModel, :3166).
     VERIFIED.

The dial
 F15 C:src/sync/SyncOffsetController.h: "A refused op changes nothing and lands in lastError(). The offset is a
     machine setting: not undoable" (:17-18); ops setMs, nudge, selectVenue, createVenue, renameVenue, removeVenue
     (:63-68); the dial IS the current venue's number: `targetMs()` returns `venues_.currentMs()` (:73). GET
     /api/sync carries "venue", "venues" [{name, ms}] and "lastError" (C:src/MainComponent.cpp:2332-2351; statusVar,
     C:src/sync/SyncOffsetController.h:83-84). VERIFIED. So "the store's "Warehouse" is 42" is read from GET
     /api/sync "venues".
 F16 C:src/sync/SyncVenues.h: at most 64 venues (`kMaxVenues = 64`, :29), names up to 40 characters (:30), unique
     case-insensitively (:9-10), the built-in name "Default" (:32). POST /api/sync/venue {"name", "create"}: create
     -> createVenue, else selectVenue (C:src/api/ApiServer.cpp:2804-2824; C:src/MainComponent.cpp:2324-2329). POST
     /api/sync/venue/remove {"name"} -> removeVenue (route C:src/api/ApiServer.cpp:367; handler :2847-2865;
     C:src/MainComponent.cpp:2331). VERIFIED.
 F17 The refusal caption. P:plan-bf2.md:416-417: "a refusal (duplicate / empty name, last venue) shows in the caption
     line in the warning colour until the next change". PD:415 keeps it ("Behaviour = plan I11 unchanged"); RD:475-476
     (D20) and RD:646-647 (G7 C11) gate it. The removal inventory's classes (FN:17-22): A event notice, B "failure
     report: same, but it reports a refusal, error or fault" -- both removed; C state display, D decision dialog,
     E tooltip / caption -- kept; an event text INSIDE a tooltip is removed too (FN:154, "The pad tooltip PREFIX is
     B21; the rest stays"). VERIFIED. By F4 and F6 item 7 the refusal caption is a text S5b must not add. That was my
     classification in round 2 (the dispatch did not list it); Harmony has CONFIRMED it (H-2, HR:19-23), on Boris's
     fourth line (F33).

The gate list
 F18 R11 as written cannot be run in order. (a) saves P with ms 42; (a2) plain-saves the SAME file with ms 43; (a3)
     saves again; then (c), (d), (e), (g) load "the file of (a)" and expect 42 (RD:600-614). VERIFIED (reading).
 F19 Five seats: PD:685 (adoption) and P:ruling-bf2.md:499. RD:659-665 lists three; RD:477-480 (D20) moved the
     interaction-logic list to unit tests; the plan's own logic and interaction-logic lists are PD:574-578.
     VERIFIED.
 F20 G1-RED (kept by id): "for every new test, a first run against stub APIs on the base behaviour that compiles and
     FAILS on its assertions ... A compile-only RED is not accepted" (P:ruling-bf2.md:402-404). VERIFIED.
 F21 Top bar: Gain is 70 px (C:src/ui/TopBar.cpp:538), range 0..4 (:26). No sync indicator in C:src/ui/TopBar.h
     (grep). Suites that exist at C:: test_composition, test_staged_load, test_topbar_model,
     test_sync_offset_controller, test_sync_venues (ls-tree). Not there yet: test_topbar_wheel, test_save_path_lint,
     test_visible_texts, test_sync_panel, test_sync_popover, test_topbar_sync_button, and the fixture tests/fixtures/
     composition_pre_sync.json. VERIFIED.
 F22 RD:426-427 (D16) cites "the load notice label" (R:src/MainComponent.cpp:493-494) as the existing example of a
     widget that does not take the keyboard when clicked. That label is in neither tree (F7). The examples that
     exist: `revealBtn_.setWantsKeyboardFocus(false);   // a click never parks the keyboard here` (C:src/ui/
     ClipInspector.cpp:445) and C:src/ui/OutputWindow.cpp:65. `setMouseClickGrabsKeyboardFocus` is nowhere in
     C: src/ (grep). VERIFIED.

Routes and waits
 F23 Three TEST routes answer BEFORE their work is done: each calls juce::MessageManager::callAsync and answers ok on
     the next line -- load_deck (C:src/api/ApiServer.cpp:2152-2153), duplicate_deck (:2173-2174), save_composition
     (:2216-2217). The save then runs in one message-thread call (saveCompositionTo, C:src/MainComponent.cpp:3618-
     3632). A Load Deck / Duplicate is staged and ends later, in finishStagedLoad (:3365), whose last lines finish
     the ticket (:3449-3453). POST /api/load_composition is different: it answers only when that ticket is finished
     (C:src/api/ApiServer.cpp:1174-1175, :1199). VERIFIED.
 F24 GET /api/sync never asks the message thread: "GET reads only the controller's lock-free status copy + the
     analysis thread's atomics -- never the model, never the message thread" (C:src/MainComponent.cpp:2319-2321; the
     provider, :2332-2351; the handler, C:src/api/ApiServer.cpp:2714-2724). VERIFIED. So a GET /api/sync sent right
     after a load_deck's 200 can be answered before finishStagedLoad has run. INFERRED (nothing was run).
 F25 GET /api/debug/deck_tabs is read ON the message thread (C:src/api/ApiServer.cpp:2299-2305) and answers {"ok":
     false, ...} after 2 s without it (:2306-2313). Its answer holds "tabs", one entry per deck tab (C:src/ui/
     DeckView.cpp:810, :819). finishStagedLoad adds the deck (pushCommands, C:src/MainComponent.cpp:3439) and
     rebuilds the tabs (:3442) inside its append branch, before its shared last lines (:3449-3453); a refused append
     returns before both (:3420-3432). VERIFIED. INFERRED: an answer with one tab more was produced after
     finishStagedLoad had returned -- one message-thread call does not run inside another.
 F26 GET /api/debug/ui_text answers {"ok": false, "reason": ...} and no other string when the message thread does not
     answer within 2 s (C:src/api/ApiServer.cpp:2117-2121). VERIFIED. "No string holds X" is then true of nothing.
 F27 Load Deck takes a composition file. Its shape check refuses only a file without a top-level "layers": `if (!obj
     || !obj->hasProperty("layers"))` (C:src/MainComponent.cpp:3704; the comment above it, :3694-3698, says a
     composition is refused). A composition file has that key: Composition::toVar writes "layers" ("The shared layer
     stack", C:src/model/Composition.h:798-802) and "decks" (:804-808). Deck::fromVar then makes one row for every
     entry of "layers" (C:src/model/Deck.h:137-156), and validateDeck refuses only a deck with no rows or with more
     than 10,000 columns (C:src/core/CompositionLoad.h:34-44). VERIFIED (the four places). INFERRED: "the file of
     (a)", saved from a composition with at least one layer, is appended as a deck of empty rows (ClipRow::fromVar on
     a layer-settings entry was not read).
 F28 The removal inventory lists the file label's "Loaded: <name>" / "Loaded deck: <name>" / "Duplicated deck:
     <name>" as A07 and "Saved: <file>" as A08, class A, removed (FN:48, FN:50). The widget, setFileLabel and the
     ui_text readout stay (FN:132). Log-only lines are kept by that ruling (FN:164). VERIFIED. A gate that waits for
     one of those label texts stops working the day that lane merges. (H-7: the sync dial merges before that lane.)
 F29 JUCE makes a shown PopupMenu modal: `window->enterModalState (false, userCallbackDeleter.release());`
     (J:menus/juce_PopupMenu.cpp:2162). `int getNumModalComponents() const` (J:components/
     juce_ModalComponentManager.h:95) counts the active items of the modal stack (J:components/
     juce_ModalComponentManager.cpp:155-163). RD:428: the dump's "modalComponents" is "the count of modal
     components". VERIFIED (the four places). INFERRED: with the venue menu shown (C9) that count is 1, not 0; H-5
     has the S5b builder measure it on the first C9 dump.
 F30 A store nobody added a venue to holds exactly one: `SyncVenues();   // one venue "Default" at 0, current`
     (C:src/sync/SyncVenues.h:34; "at least one venue", :9). VERIFIED. The adopt rule (PD:451-455): the store has the
     name with a different number -> "the FILE's value replaces the store's". So on such a store every show saved on
     "Default" is the same-venue case. INFERRED from those lines.
 F31 Texts that hold "sync" in some case and are not about the dial: the top bar's "Resync" button (C:src/ui/
     TopBar.h:108; made visible at C:src/ui/TopBar.cpp:81); the Clip tab's transport box while its item "BPM Sync"
     is the chosen one (C:src/ui/ClipInspector.cpp:10; "Timeline" is the default, :11); the fixture's own name,
     composition_pre_sync.json, which the file label shows after a load (F8). VERIFIED.
 F32 Existing probes write their own deck files and wait on a deck count: C:.harmony/probe-async-load.py:61 ("POST
     load_deck {deck16.json}; ... poll numDecks == 2"), :203 (it writes deck16.json itself), :957 (the POST).
     VERIFIED.

Read in round 3
 F34 POST /api/load_composition answers HTTP 200 with {"ok": false, "reason": ...} -- no 4xx -- when the file is
     missing (C:src/api/ApiServer.cpp:1146-1150), cannot be read (:1156-1161), is refused by the validator (:1162-
     1166), or the load failed, was superseded, was cancelled or timed out (:1199-1206). The body's key is "path"
     (:1127). VERIFIED. A load that did not happen leaves the dial and every text as they were.
 F35 The sync POSTs answer before their change is made: callAsync, then the answer (set, C:src/api/
     ApiServer.cpp:2761-2764; venue, :2822-2823; rename, :2843-2844; remove, :2863-2864). VERIFIED for those four;
     the nudge handler was not read (R11 no longer uses it). With F24, a GET /api/sync sent right
     after one can still show the old state. INFERRED (nothing was run). Two calls posted to the message thread run
     in the order they were posted, so a change POSTed before a load is made before that load starts. INFERRED
     (JUCE's message queue was not read).
 F36 How the controller changes the store: `apply` runs the op on a COPY of the venues. Refused -> `lastError_ =
     r.error;   // a refusal changes nothing: no push, no save`. Accepted -> lastError is cleared, the copy becomes
     the store, the sink is pushed, a save is scheduled. Either way the status copy is published and the listeners
     are called (C:src/sync/SyncOffsetController.cpp:69-93; "also a refused one", C:src/sync/
     SyncOffsetController.h:39). VERIFIED. So an op written as ONE `apply` call changes the store whole or not at
     all.
 F37 The cap. `SyncVenues::create` refuses a 65th venue with "There can be at most 64 venues." (C:src/sync/
     SyncVenues.cpp:117-127; the refusal, :122-123), after it has checked the name (:120-121; an empty name is
     "A venue needs a name.", :98-99). The lock-free status copy has room for exactly kMaxVenues names and numbers
     (C:src/sync/SyncOffsetController.h:52-53; the count is cut to it, C:src/sync/SyncOffsetController.cpp:205).
     VERIFIED.
 F38 The app's own alerts are not shown in TEST mode: each sits behind `if (!testMode_)` (C:src/MainComponent.cpp:
     3604-3610, "Save failed"; :3706-3710 and :3723-3727, "Load Deck"). VERIFIED. An alert added in that pattern is
     seen by no TEST-mode row.
 F39 The JUCE calls the text list needs exist: `Desktop::getNumComponents()` and `getComponent (int)` (J:desktop/
     juce_Desktop.h:242, :251); `TextEditor::getText()` (J:widgets/juce_TextEditor.h:367); `ComboBox::getText()`
     (J:widgets/juce_ComboBox.h:245); `Label::getText` (J:widgets/juce_Label.h:82); `Button::getButtonText()`
     (J:buttons/juce_Button.h:76). VERIFIED. (Round 2 had the first three ASSUMED.)
 F40 What lies between C: and S5a: S3f (probe rows, BeatLead, the replay end, [timing]), S4 (keys and MIDI), S6
     (taps through the dial; "Engine only.") (RD:514-516). "S3f and S4 touch neither the top bar nor the composition
     file" (RD:487). "S6, S5a and S5b reach main together" (RD:490). VERIFIED (the ruling's words; what the builders
     will really touch is not known yet).

## 2 THE RE-STATED ROWS (copy from here)

### R11 (replaces RD:599-614)

R11 (new; S5a; TEST mode, scratch settings and scratch files in a scratch folder made new and empty for the run;
    on-screen text read through GET /api/debug/ui_text and through the "texts" of GET /api/debug/sync_ui_dump; no
    scratch folder, scratch file, deck, layer or clip name holds "sync" in any case or "Warehouse", and no venue
    name but "Warehouse" does; the fixture of (b) is not a scratch file)
    LOADS. "Load <file>" is POST /api/load_composition {"path": <file>}. In every clause that loads -- (k), (b),
    (c), (d), (e), (h), (i), (j) -- the answer is HTTP 200 with "ok": true; any other answer is a FAIL of that
    clause.
    WAITS. Three POSTs answer before their work is done. After POST /api/debug/save_composition the probe reads the
    file again for up to 5 s until the clause holds; not holding by then is a FAIL. P, Z and A do not exist before
    the step that writes them (the probe checks it just before that step). After POST /api/debug/load_deck and
    after POST /api/debug/duplicate_deck it reads GET /api/sync only once GET /api/debug/deck_tabs has answered
    "ok": true with one entry more in "tabs" than before that POST; no such answer within 10 s is a FAIL. A POST
    /api/sync/set, /api/sync/venue or /api/sync/venue/remove answers before its change is made too: after each one
    the probe reads GET /api/sync for up to 5 s until it shows the state the step names (the venue, targetMs, and
    "venues" where the step names the store); not showing it by then is a FAIL. "Unchanged" compares the GET
    /api/sync after the step with that state. A venue is added with POST /api/sync/venue {"name": ..., "create":
    true} and taken out of the store with POST /api/sync/venue/remove {"name": ...}.
    TEXTS. On-screen text is read 1 s after the load has answered. In the ui_text clauses "Sync" and "Warehouse"
    are matched as written (capital S). S(x) = the strings of the dump's "texts", read after step x, that still
    hold "sync" in any case or "Warehouse" once every "composition_pre_sync" (the fixture's name, in any case) is
    cut out of them, each counted as often as it occurs. S(0) is read before the first save and the first load of
    the run, the dial already at "Warehouse" +42.
    (a) venue "Warehouse" at +42, POST /api/debug/save_composition {"path": P} -> the file's "sync" equals
        {"venue":"Warehouse","ms":42}. Once that holds the probe copies P to A; A is "the file of (a)" below.
    (a2) set +43, POST /api/debug/save_composition {"plain": true} -> the same file's "sync" has ms 43.
    (a3) on "Default" at 0, POST /api/debug/save_composition {"path": Z} -> the file's "sync" equals
        {"venue":"Default","ms":0}. Z is "the file of (a3)" below.
    (k) the control: dial at "Warehouse" +42, load the file of (a) -> GET /api/sync unchanged (venue, targetMs); GET
        /api/debug/sync_ui_dump answers 200; its "texts" holds "Resync" and, when that string is not empty, the
        "file_label" string GET /api/debug/ui_text returns in the same step; the probe keeps S(k); no string occurs
        more often in S(k) than in S(0).
    (b) dial at "Warehouse" +42, load tests/fixtures/composition_pre_sync.json -> GET /api/sync unchanged (venue,
        targetMs); GET /api/debug/ui_text answers "ok": true and none of its strings holds "Sync" or "Warehouse";
        S(b) = S(k).
    (c) select "Default" 0, load the file of (a) -> venue "Warehouse", targetMs 42; GET /api/debug/ui_text answers
        "ok": true and none of its strings holds "Sync" or "Warehouse"; S(c) = S(k).
    (d) set "Warehouse" to +55, load the file of (a) -> targetMs 42; the store's "Warehouse" is 42; GET
        /api/debug/ui_text answers "ok": true and none of its strings holds "Sync" or "Warehouse"; S(d) = S(k); GET
        /api/debug/sync_ui_dump answers 200 with "wheel" and without a "replacedByLoad" property.
    (e) a store without "Warehouse", load the file of (a) -> the venue is created at 42 and selected.
    (f) the real ~/Library/Audio-DNA/settings.json sha256 is unchanged around the run.
    (g) dial at "Warehouse" +55, POST /api/debug/load_deck with the file of (a), then POST /api/debug/duplicate_deck
        -> GET /api/sync unchanged after each.
    (h) dial at "Warehouse" +42, load the file of (a3) -> venue "Default", targetMs 0; the store's "Warehouse" is
        still 42; then POST /api/sync/venue {"name":"Warehouse"} -> targetMs 42.
    (i) a store whose only venue is "Default", set to +55; load the file of (a3) -> venue "Default", targetMs 0; GET
        /api/sync "venues" equals [{"name":"Default","ms":0}].
    (j) a full store: 64 venues, none of them "Warehouse", the dial on one of them at +55; the probe keeps GET
        /api/sync and S; load the file of (a) -> GET /api/sync unchanged (venue, targetMs, and "venues" entry for
        entry); its "lastError" is not empty; GET /api/debug/ui_text answers "ok": true and none of its strings
        holds "Sync" or "Warehouse"; S after the load = S before it.
    LIVE RED (required, as R7's): the mutant app -- a scratch copy with (1) the adopt hook call moved out of the
        Kind::Composition branch of finishStagedLoad into the function's shared last lines, and (2) a juce::Label
        with the text "Sync changed" added to MainComponent and made visible when the hook changes the venue or the
        number -- run through R11 -> (g) FAILS (targetMs 42 after the Load Deck) and (c) and (d) FAIL on S (S(c) and
        S(d) hold "Sync changed", S(k) does not); `grep -c MUTANT` on the real tree = 0 afterwards. A mutant app
        that passes (g), or passes (c) or (d) on S, shows a row that cannot fail: R11 is then NOT passed.

What changed, clause by clause
 - Header: "the load notice read through GET /api/debug/ui_text" -> on-screen text read through that route and
   through the dump's "texts"; the names rule added. The word "Sync" in the ui_text clauses is matched as written
   (capital S): the fixture's own name holds "sync" and is in the file label (F31). Round 3: the scratch folder is
   new and empty for the run; the names rule says in so many words that it covers the folder and the venue names
   and does not cover the fixture (R2: NIT-1).
 - LOADS (round 3; R2: SHOULD-3): POST /api/load_composition answers 200 with "ok": false when it did not load
   (F34). Without this sentence a load that did not happen passes every "unchanged" clause and every text clause.
 - WAITS (round 2): F23, F24. The save clauses are polled because the POST answers before the file is written. (g)
   waits on the deck tab count because the file label's text leaves with the removal lane (F28) and because that
   route's answer comes from the message thread, after finishStagedLoad has returned (F25). Round 3: P, Z and A must
   not exist before they are written -- a file left by an earlier run would satisfy the poll on a tree with no
   writer (R2: SHOULD-2). The sync POSTs the row uses answer early as well (F35), so the state a step names is
   confirmed by GET /api/sync before the step goes on, and "unchanged" has a confirmed state to be compared with.
   The routes that add a venue and take one out are named (F16; R2: NIT-5).
 - TEXTS (round 2): the second detector. ui_text sees five strings (F9); "texts" sees every text widget (D21 below).
   It is compared with the control (k), not searched for words alone, because texts that hold "sync" are on screen
   for other reasons (F31) and, from S5b on, the SYNC button itself is. Round 3: what is cut out of a string is the
   fixture's name and nothing else. It is the one loaded file whose name holds "sync"; a scratch stem holds neither
   word (the names rule) and is never cut, so a short stem cannot take letters out of "Warehouse" or "sync" (R2:
   SHOULD-4). The name is cut out of the string, the string is not dropped: a notice that also names the file is
   still seen. S(0) is new: a reading taken before any load (see (k)).
 - (a): words kept; one sentence added -- the copy A, taken only once the clause holds (F18, F23). (a2): word for
   word.
 - (a3): "the file has no "sync" key" -> the key with "Default" and 0 (F1). Its own path Z, so A stays what (a) wrote.
 - (k): NEW (round 2). The same file, loaded when the dial already holds its value: nothing changes, so nothing could
   be announced. Every string it shows is state. Round 3, two clauses added. (1) Its "texts" must hold "Resync" and
   the file label's text: a walk that reaches no window, or skips buttons or labels, returns a list without them,
   and every S() would then be empty and equal (R2: SHOULD-1). The review asked for all five ui_text strings; three
   of them are not read from a text widget (F9), so a correct tree could fail that. The two strings named are
   VERIFIED to sit in a juce::TextButton and a juce::Label the window shows (F9, F31). (2) S(k) may hold nothing
   that S(0) does not: a text EVERY keyed load shows, or one that stays on screen once shown, is in the control
   too, and an equality with the control cannot see it. S(0) is taken before any load and cannot hold such a text.
 - (b): "the load notice holds no "Sync:" sentence" -> the same check on the route that is left, with "ok": true
   (F26), plus S(b) = S(k). (c), (d): "the notice begins ..." -> its opposite, on both detectors (F2).
 - (d): "the dump's "replacedByLoad" equals ..." and "then POST /api/sync/nudge +1 -> "replacedByLoad" is gone" ->
   removed with the thing they measured; the absence is asserted instead. The authority is adoption item 7's "or is
   dropped if it needs a text" plus the ruling of section 4 -- Boris's words remove its three displays, not the
   memory itself. If Harmony overturns section 4, both clauses come back as written. "targetMs 42; the store's
   "Warehouse" is 42" kept word for word.
 - (e), (f), (g): word for word. (h): NEW (round 1) -- the load half of F1, and the test that a venue the load did
   not name keeps its number. (i): NEW (round 2) -- the same load on a store that holds only "Default" (F30): the
   show's number replaces the room's and no second venue appears.
 - (j): NEW (round 3) -- the ruling of section 8.3. It compares S after the load with S just before it, not with
   S(k): from S5b on the SYNC button shows this clause's own number (+55).
 - LIVE RED (round 3; R2: SHOULD-1): in round 2 the two mutants stood as remarks under RED, where no gate string is
   copied from. R7 carries its mutant app inside the row (RD:573-575); R11 now does too, one scratch build for both.

RED -- what each reads on the tree before S5a. VERIFIED at C: that nothing writes or reads a key and that the dump
route does not exist (F11-F13); what a probe reads there is INFERRED -- nothing was run.
 S(0) GET /api/debug/sync_ui_dump answers 404 (the route is created in S5a, D21) -> the run FAILS at its first read.
 (a)  the saved file has no "sync" property; after 5 s -> FAIL.
 (a2) the POST answers 400 "path (an absolute file in an existing folder) required" (F12) -> FAIL.
 (a3) no "sync" property -> FAIL. It ALSO fails on a tree built to the withdrawn rule `present = !(venue is "Default"
      and ms == 0)` (RD:385): that is what this clause is for.
 (k)  the dump answers 404 -> FAIL. After the stage it is the control. Its GET /api/sync bar fails only on a hook
      that moves the dial when the key equals it. Its "texts" clause fails on a walk that reaches nothing ("texts":
      [] holds no "Resync"). Its S(0) clause fails on a text every keyed load shows and on one that stays on screen.
 (b)  S(b) cannot be read (404) -> FAIL. Its other clauses pass before the stage (nothing reads a key): they are
      guards. The trees they fail on: a fixture that is missing or refused ("ok": false, F34); an adopt hook that
      does not test `present` (the dial goes to "Default" 0); one that writes a sentence into the file label; one
      that shows a label anywhere that names the venue or holds "sync". The unit RED is the seam case "no call when
      present is false" (PD:467-468), run first on a stub hook that always calls.
 (c)  venue stays "Default", targetMs 0 -> FAIL.
 (d)  targetMs stays 55 -> FAIL; the dump route answers 404 -> FAIL.
 (e)  no venue "Warehouse" after the load -> FAIL.
 (f)  PASSES before the stage: a rig guard. It fails on a run launched without the scratch settings file.
 (g)  PASSES before the stage: a guard. With the wait it can fail. The tree it fails on: the hook placed in
      finishStagedLoad's shared last lines (C:src/MainComponent.cpp:3449-3453) instead of the Composition branch
      (:3394-3410) -- after (e) the model holds "Warehouse" 42, the dial is at +55, the Load Deck ends, the hook
      runs, and the read that waited for the new tab sees 42. No unit test can hold this (finishStagedLoad is the
      app's own method), so its RED is the row's LIVE RED line. If the app refuses the file of (a) as a deck, no tab
      is added and the wait ends in a FAIL: that goes to the architect (K7), it is not waived.
 (h)  venue stays "Warehouse" +42 -> FAIL. It also fails on the withdrawn rule (Z has no key, the dial is not
      touched) and on an adopt hook that skips "Default" at 0.
 (i)  targetMs stays 55 -> FAIL. It also fails on the withdrawn rule, and on a tree that tries to save the +55 by
      adding a venue (the list is then not exactly one entry).
 (j)  FAILS before the stage on "lastError" alone: nothing adopts, and the last op (the set to +55) was accepted and
      cleared it (F36). After the stage it fails on: an adopt written as createVenue then setMs -- the create is
      refused, the setMs is accepted, the venue he is on reads 42 and lastError is empty again; an adopt that drops
      a venue to make room ("venues" differs); a refusal written on screen.
 The text detector's own RED: unit, test_visible_texts on a stub that returns nothing (G1 below). Its live teeth are
 the row's LIVE RED line.

### G7 (replaces RD:640-666)

G7  (changed; S5b) VISUAL WORK GATE, before Boris sees anything. TEST mode, scratch settings, states set by REST and
    POST /api/debug/sync_ui; the main window captured by Quartz window id at 1728 x 1000 and at 1280 x 720, cropped
    to the top bar (and to the panel when open); C13 and C13k are kept uncropped as well; the dump read at each
    capture.
    States: C1 first launch, 1728: "SYNC 0" dim, Gain 140, Auto mode; C2 the same in Manual mode (BPM field shown,
    Gain still 140, Master Signal label whole); C3 +42 on "Warehouse": "SYNC +42" bright; C4 -30; C5 +500 and -500;
    C7 the panel open at 0 (caption "In step with the sound coming in"); C8 the panel at +42 and at -30 (bar filled
    from the centre, captions), the +42 capture taken on "Warehouse"; C9 the venue menu open (3 venues, tick, Delete
    in warning red); C11 a refused venue name: the panel open at "Warehouse" +42 captured, POST /api/sync/venue
    {"name":"Warehouse","create":true} (the name is taken), captured again -- the panel is as it was; C12 the panel
    open while REST changes the value; C13 just after loading a composition that changed the dial ("Warehouse" set to
    +55, then a show saved at "Warehouse" +42 loaded): the button, then the panel opened -- both exactly as the same
    value set by hand; C13k the control for C13: the same show loaded once more, the dial already at "Warehouse"
    +42, captured the same two ways; C14 1280 x 720: Gain 70, the button present, the panel fully inside the window.
    (C6, C10 and C15 of the plan are removed, D20.)
    MEASURABLE BARS (dump; no waiver): at 1728 in Auto AND Manual every right-group widget has its wanted width and
    the Master Signal label is not cut; Gain is 140 at 1728 in Auto and in Manual and 70 at 1280; the SYNC button is
    62 x 26 inside the tempo slot; its text rows (62 x 12) meet neither the BPM number nor the tracker-state label;
    the tracker-state label's bounds equal the pre-stage ones; every text fits its bounds, "SYNC -500" included;
    panel widgets do not overlap and lie inside the panel; the panel lies inside the window at both sizes;
    "modalComponents" is 0 in every state without a menu (C1 - C5, C7, C8, C11, C12, C13, C13k, C14) and exactly 1
    in C9 (the venue menu itself: JUCE makes a shown menu modal); every state's text equals the model (dump against
    GET /api/sync), C12 and C13 included; C11's second dump equals its first in the button part and the panel part,
    field for field (the wheel and the clock values move and are not compared), and GET /api/sync "lastError" is not
    empty within 200 ms of the POST (the second capture is taken after that); the loads of C13 and of C13k answer
    "ok": true; C13's dump equals the dump of the same value set by hand -- the button's text, colour and tooltip
    against C3, every panel widget's text against C8 at +42; in C11, C13 and C13k GET /api/debug/ui_text answers
    "ok": true and none of its strings holds "Sync" or "Warehouse" (names and matching as in R11); S(C13) = S(C13k)
    at the button capture and at the panel capture, and S at C11's second capture = S at its first (S as in R11);
    no string occurs more often in S(C13k) at the button capture than in S(C3), nor at the panel capture than in
    S(C8 at +42); N(C13) = N(C13k) at the button capture and at the panel capture, and N at C11's second capture = N
    at its first (N(x) = every string of the dump's "texts" at x with each number in it -- [+-]?[0-9]+([.][0-9]+)?
    -- replaced by "#", each string counted as often as it occurs). INFO: Tap and Resync x at 1728 before and after
    the stage (expected +70).
    CRITIC PANEL in parallel, five seats (pixels and dump only), PASS / FAIL per item. No seat may ask for a text, a
    mark or a control that tells that something happened; such an item is void, not a FAIL. A caption that describes
    the value in use ("Visuals 42 ms later") is a state display and stays. A control that is greyed while its input
    cannot be accepted is a state display too, and a seat may ask for one (H-2). A text that tells that something
    happened and was in the app before this lane (the file label's "Loaded: <name>") is reported by the seat that
    sees it and recorded by Harmony as pre-existing, for the app-wide removal lane: it is neither void nor a bf2
    FAIL -- unless that lane has merged first, in which case it must be gone and is a FAIL. (H-7: the sync dial
    merges first.)
     visual-design: is "SYNC 0" clearly quieter than "SYNC +42" yet readable? does the word above the tracker state
      crowd the BPM number? is the doubled Gain balanced against its neighbours? palette colours only, no orange
      #ff4500, red only for Delete?
     UX: can a first-time user find the control? is the sign obvious without the caption? is one click to reach the
      number acceptable for a set-once control?
     graphic-design: grammar widths, 9 pt / 11 pt hierarchy, caption #888, panel alignment with the house inspector
      rows, no round control.
     logic: does each capture SHOW what its dump says -- the button's text and its dim / bright state, the box, the
      caption, the venue button, the menu rows? does the sign read the same way in every place of one capture (+42:
      "+42", "Visuals 42 ms later", the bar filled to the right of the centre; -30: "-30", "Beats 30 ms earlier ...",
      the bar filled to the left)? in C9 is there exactly one tick, on the venue the dump names? in C12 do the box,
      the caption, the bar and the button all show the REST value? does C13 look exactly like C3 (button) and like
      C8 at +42 (panel), and C11's second capture exactly like its first -- no word, mark or colour that tells what
      happened? do the uncropped captures of C13 and C13k show the same words and marks everywhere outside the moving
      parts (the picture, the wheel, the meters)? does the uncropped C13 show, anywhere outside the SYNC button and
      the panel, a word about Sync, an offset, a venue or a change (it must not)?
     interaction-logic: read the captures in the order they were driven (the manifest gives it). After each step
      does every widget show the same value as the others, none one step behind (C7 -> C8 +42 -> C8 -30 -> C12)?
      with the venue menu open (C9) is the panel still open and unchanged behind it, and does the dump say
      "modalComponents" 1? after the refused name (C11) does the next step (C12) change the value normally? does
      the panel after the refused name read as a fault -- frozen or broken -- or as a panel that did not take the
      name (H-2)? in every capture with the panel open and no menu shown does the dump say "modalComponents" 0 and
      the "focusOwner" of before the panel opened, and does the picture show no text caret in the value box unless
      that step typed into it? after the load (C13) is the panel an ordinary panel -- every widget on the new value,
      nothing left of +55? at 1280 (C14) is the button uncovered and the panel whole?
      (What no picture can show stays a unit test with a RED stub, D20: hold-repeat of - / +, one panel instance and
      closing, the box's commit rules, deleting the current venue, the glide.)
    A critic FAIL blocks unless Harmony records why it is taste-only. Then the page Boris opens (section 5 below).

What changed
 - C11: "a refusal in the caption" -> "the panel is as it was" (F17). CONFIRMED by Harmony (H-2) on Boris's fourth
   line: "ok. the only fail message will be a failed save. remove all others" (F33) -- a refusal caption is a fail
   message that is not a failed save. C13: "(button, load notice, the panel's "was" caption)" -> "exactly as the same
   value set by hand" (F2, section 4). C8: the venue of its +42 capture is named, so C13 has something to be
   compared with. C13k: NEW (round 2), the control of C13 -- the same load without the change. Every other state:
   word for word.
 - MEASURABLE BARS: the ruled sentence word for word but for one clause, plus the C11 and C13 bars. They are bars,
   not critic items: a dump comparison has no opinion in it.
 - The one clause (round 2, NOT from Boris's words): ""modalComponents" is 0 with the panel open" could not be met in
   C9, where the venue menu is shown and JUCE makes it modal (F29). ADMITTED by Harmony (H-5). Round 3 states it in
   H-5's words (R2: NIT-2): 0 in EVERY state without a menu, the states with the panel closed (C1 - C5) included,
   and exactly 1 in C9. That is tighter than leaving C9 out: a panel that turned modal behind the menu would read 2.
   "Exactly 1" is INFERRED (F29): the S5b builder MEASURES it on the first C9 dump; a count other than 1 with only
   the venue menu shown is a STOP and a report -- a builder does not adjust the bar.
 - Round 3, three bars added. (1) The loads of C13 and of C13k answer "ok": true: a C13k whose load did not happen
   is C13 read twice and compares equal to it whatever is on screen (F34; the hole of R2: SHOULD-3, here). (2)
   N(C13) = N(C13k): the two states end the same, so every text must be the same. S() looks only at strings that
   hold "sync" or the venue name; "Offset changed", or a bare "+55", holds neither (R2: SHOULD-5). Numbers are
   replaced by "#" so that a readout that moves by itself (a frame rate, a level) does not differ, and nothing has
   to be marked by hand. The same N holds C11 against a refusal text that names nothing. (3) S(C13k) against C3 and
   C8: the control is itself a keyed load, so a text every keyed load shows is in C13 and in C13k alike; C3 and C8
   were set by hand before any load.
 - UX: "does the load notice say what changed in plain words?" removed -- Boris removed the notice it judged. The
   other three UX questions, visual-design and graphic-design: word for word.
 - logic, interaction-logic: seated again (F6 item 5, F19). They judge the pictures against the dump and the order
   of the captures. The machine compares the dump with the model; only a reader of the picture can see a text that
   is painted wrong, a bar filled on the wrong side, a tick on the wrong row, or a word outside every text widget.
   D20's unit tests stay. Round 3: the logic seat reads the whole uncropped C13 for any word about Sync outside the
   button and the panel; the interaction-logic seat judges whether the silent refusal of C11 reads as a fault (H-2).
 - Pre-existing event texts are handled in ONE place now, the CRITIC PANEL paragraph (the file label's "Loaded:
   <name>" is on main today, F8, and is row A07 of the removal inventory, F28). H-7: the sync dial merges before
   the removal lane, so the label is in every bf2 capture and is recorded, not failed.

RED -- what G7 reads on the tree before S5b (INFERRED)
 - POST /api/debug/sync_ui answers 404 (the route is created in S5b, D21): no state from C3 on can be set. The dump
   has no button and no panel part. Gain at 1728 is 70 (F21) -> "Gain is 140 at 1728" FAILS.
 - The C11 bar fails on a panel built to P:plan-bf2.md:416-417 (the caption turns into the refusal: the second dump
   differs). The C13 bars fail on a panel or a button built to D15 as first written (a "was +55" caption or
   tooltip: the dump differs from C8 / C3) and on a notice label anywhere in the window (S(C13) differs from
   S(C13k)).
 - N(C13) = N(C13k) fails on a text that names neither word and shows only when the load changed the dial ("Offset
   changed", a bare "+55"). The C3 / C8 clause fails on a text every keyed load shows.
 - The C9 clause fails on a panel that is itself modal (the count is 2) and on a menu that was not shown (0).

### G1 -- the S5a and S5b suite lists (replace RD:542-546; the rest of G1 stands)

    S5a: test_composition (the sync key, written by every save, "Default" at 0 included; composition_pre_sync.json),
    test_sync_offset_controller (adopt; adopt changes no venue but the one the file names; a refused adoption
    changes nothing), test_topbar_model (musicBeat, afterRealign, advance), test_topbar_wheel, test_save_path_lint,
    test_staged_load (an old file leaves a +42 dial at +42), test_visible_texts (the text list of the dump);
    S5b: test_sync_panel (a refused op changes no widget's text; after an adoption the panel reads as after the same
    value typed), test_sync_popover, test_topbar_sync_button, test_topbar_model (gainSliderWidth), the 1280 and 1728
    layout cases, the re-anchored top-bar layout tests.
    Unchanged and green: test_render_thread_lint "bf9b B4j" (no load-notice identifier anywhere in src/).

What changed
 - REMOVED: "the LoadNotice::forLoad combinations" (F2; and F7: it cannot be built past the lint). REMOVED:
   "replacedByLoad" from test_sync_offset_controller's bracket (section 4).
 - ADDED to the list, not to the work: test_staged_load. RD:405-406 (D14) and PD:468-469 already name its case; the
   ruled list left the suite out.
 - ADDED (round 2): test_visible_texts -- the function behind the dump's "texts" (D21 below).
 - ADDED cases: "Default" at 0 is written (test_composition); adopt changes only the named venue, also when that
   venue is the only one (test_sync_offset_controller); the two test_sync_panel cases.
 - ADDED (round 3): "a refused adoption changes nothing" (test_sync_offset_controller) -- the three cases of
   section 8.3.

RED per suite (G1-RED, F20: each new case first runs on a stub that compiles and fails on its assertion)
 - test_composition: round trip of the key -- stub toVar writes nothing -> present is false after the reload. "Default"
   at 0 is written -- the stub IS the withdrawn rule (skip the key when "Default" and 0) -> no "sync" property.
   Malformed -> absent, clamp, name cut to 40 -- stub fromVar copies raw. The fixture has no "sync" property and
   loads with present == false -- stub: SyncSetting defaults to present = true.
 - test_sync_offset_controller: the three adopt cases, the notification, the saved JSON -- stub adoptFromComposition
   does nothing. adopt("Default", 0) from "Warehouse" +42 selects "Default" at 0 and leaves "Warehouse" at 42 -- same
   stub. Adopt changes only the named venue (three venues, adopt one, the other two equal before and after) -- stub
   adopt = setMs(ms) on whatever venue is current. adopt("Default", 0) on a store whose only venue is "Default" +55
   -> one venue, "Default" at 0 -- stub: adopt does nothing.
   A refused adoption changes nothing (round 3), three cases. (1) A full store and a new name: 64 venues, the
   current one at +55, adopt("Warehouse", 42) -> venues() equal before and after, targetMs() 55, lastError() not
   empty, no save scheduled by it, one notification -- stub A: adopt does nothing (lastError() stays empty); stub B, the
   natural wrong build: createVenue(name) then setMs(ms) (targetMs() reads 42 and lastError() is empty again).
   (2) A full store and a name it holds: adopt(a venue of the 64, another number) -> that venue is selected with
   the file's number, the other 63 are equal before and after, lastError() empty -- stub: adopt refuses whenever
   the store is full. (3) A name the store cannot take (spaces only): adopt("   ", 42) -> nothing changes,
   lastError() not empty -- stub B.
 - test_staged_load: "an old file leaves a +42 dial at +42", through the seam applyCompositionSync (PD:467-469) --
   stub: a hook that ignores `present`. (That Load Deck and Duplicate never adopt is R11 (g), with a live mutant.)
 - test_visible_texts: a visible juce::Label, juce::Button, juce::TextEditor and juce::ComboBox under one parent are
   listed with their texts; a hidden one is not; the children of a hidden parent are not; an empty text is left out
   -- stub: the function returns an empty list. (Pitfall 34: the test calls setVisible(true) itself.)
 - test_topbar_model, test_topbar_wheel, test_save_path_lint: as ruled (RD:372-378, :389-390; the lint reads three
   today, F11).
 - test_sync_panel: plan I11's list and D17's cases as ruled. A refused op changes no widget's text, a refused
   adoption included -- stub: the caption shows lastError (the withdrawn P:plan-bf2.md:416-417). After an adoption
   the panel reads as after the same value typed -- stub: the caption gains a "was" clause (the withdrawn D15).
 - test_sync_popover (D16), test_topbar_sync_button (D18), gainSliderWidth and the layout cases (D19): as ruled.

## 3 AMENDMENT DELTAS (the sentence the builder builds from; OLD is withdrawn wherever it is quoted)

D12  NOT TOUCHED. The wheel shows a state, not an event; no notice, no save rule.

D13  (one writer) -- TOUCHED: the key-less save.
  OLD (RD:385): "File key and model as plan :443-446, kept. present = !(venue is "Default" and ms == 0), kept (see
  D14)."
  NEW: "File key and model as plan :443-446, kept. EVERY save writes the key: writeCompositionFile copies the
  controller's current venue and value into the model and sets present = true -- always, "Default" at 0 included
  ({"venue":"Default","ms":0}) -- then composition_.saveToFile. `present` is false only on a model read from a file
  without a valid "sync" property, or never read from a file; it decides the adopt hook (D14) and nothing about
  saving."
  The rest of D13 (the three callers, test_save_path_lint, {"plain": true}) stands.

D14  (loading) -- TOUCHED: the notice; "Default at 0 writes no key".
  Title NEW: "Loading: a composition open only; nothing says so; a real old file".
  RD:395-396 NEW: "The adopt hook runs ONLY inside the Kind::Composition branch of finishStagedLoad, after the model
  swap (at C: the branch is src/MainComponent.cpp:3394-3410, the swap :3400). Load Deck and Duplicate Deck never
  read the file's sync."
  RD:397-401 (LoadNotice::forLoad, the sentence) NEW: "The adoption writes NOTHING: no label, no sentence in the file
  label, no tooltip, no alert; no app-log line is asked for (H-4). LoadNotice is not re-created (neither tree has
  one; its name fails test_render_thread_lint). What shows the change is state: GET /api/sync, and from S5b the SYNC
  button's value."
  RD:402-403 NEW: "WITHDRAWN (Boris: "every show remembers it's sync"). Every file that holds the key adopts,
  {"venue":"Default","ms":0} included: no special case for "Default" and none for 0. A file without the key never
  calls the hook: the dial, the venue and the store are untouched. docs/claude/integration.md says both."
  RD:404-406 (the real fixture): stands word for word, plus: "No deck, layer or clip name in the fixture holds "sync"
  in any case or "Warehouse" (R11's names rule)."
  ADDED (round 3; the ruling of section 8.3): "adoptFromComposition is ONE op through SyncOffsetController::apply
  (C:src/sync/SyncOffsetController.cpp:69-93): the store changes whole or not at all, with one push, one
  notification and one scheduled save. The plan's three cases stand (PD:451-455). A fourth: when the store refuses
  -- it already holds 64 venues (kMaxVenues, C:src/sync/SyncVenues.h:29) and the file names one it does not hold,
  or the name is one the store cannot take -- NOTHING changes: the venue, the number, the list and the settings
  file are as before; lastError() holds the store's reason, for REST; nothing is written on screen. A full store
  that HOLDS the named venue adopts as usual. docs/claude/integration.md says it."
  For the reviewer of the S5a diff, not for the builder's text: an alert behind `if (!testMode_)` is seen by no
  live row (F38). The reviewer greps the diff: no added line holds `AlertWindow` or `showMessageBox`.

D15  (the replaced value) -- TOUCHED: all of it but its first sentence.
  KEPT: ""The file wins" stands (Q10 "yes"; plan :453-455)."
  NEW, in place of RD:410-416: "Nothing remembers the value a load replaced: no replacedByLoad(), no Replaced type,
  no dump property, no caption, no tooltip, no log line. adoptFromComposition changes only the venue the file names,
  so a load that selects a DIFFERENT venue leaves the venue he was on, and its number, in the venue list. A load
  that names the venue he is on with another number replaces that number, and the old one is kept nowhere -- on a
  store that holds only "Default" that is every show saved on "Default" with another number. Tests:
  test_sync_offset_controller "adopt changes no venue but the one the file names" and "adopt("Default", 0) on a
  store whose only venue is "Default" +55 -> one venue, "Default" at 0"; no replacedByLoad case; test_sync_panel has
  no "was" caption case."

D16  (the panel) -- TOUCHED: one dead citation and one definition, no behaviour.
  OLD (RD:426-427): "(the load notice label already works this way, R:src/MainComponent.cpp:493-494)".
  NEW: "(as the Clip tab's Reveal button does: `revealBtn_.setWantsKeyboardFocus(false);`, C:src/ui/
  ClipInspector.cpp:445 -- the load notice label the ruling pointed at is in neither tree)".
  OLD (RD:428): "The dump carries "modalComponents" (the count of modal components)".
  NEW: "The dump carries "modalComponents" = juce::ModalComponentManager::getInstance()->getNumModalComponents()
  (the count of modal components; a shown menu is one, so it reads 1 while the venue menu is shown and 0 in every
  state without a menu). H-5: that it reads exactly 1 is INFERRED -- the S5b builder MEASURES it on the first C9
  dump; a count other than 1 with only the venue menu shown is a STOP and a report. A builder does not adjust the
  bar."

D17, D18, D19  NOT TOUCHED. D18's tooltip (PD:389-390: the value in use and the venue) is a state text and stays; it
  gains nothing about a load. D19: Gain "yes" (F5) is what D19 already builds.

D20  (G7) -- TOUCHED: C11, C13, the seats, the bars.
  OLD (RD:475-476): "C11 (a refusal in the caption) is reached by REST: POST /api/sync/venue with a duplicate name
  while the panel is open."
  NEW: "C11 (a refused venue name leaves the panel as it was) is reached by REST: POST /api/sync/venue {"name": a
  name that is taken, "create": true} while the panel is open. The panel shows NO refusal: plan I11's "a refusal ...
  shows in the caption line in the warning colour until the next change" (P:plan-bf2.md:416-417) is withdrawn -- a
  failure report on screen. A refused op changes nothing on screen; lastError keeps the reason for REST."
  CONFIRMED by Harmony (H-2), on Boris: "ok. the only fail message will be a failed save. remove all others" (F33).
  H-2 adds: a control greyed while its input cannot be accepted is a state display and may be built; a caption or
  any other text that reports the refusal may not.
  OLD (RD:477): "The interaction-logic list leaves the critic panel." NEW: "The critic panel has five seats
  (visual-design, UX, graphic-design, logic, interaction-logic). The logic and interaction-logic seats judge the
  captures against the dump and their order (section 5, G7). The stateful items stay named unit tests with a RED
  stub, as listed."
  ADDED: "C13k is the control of C13: the same show loaded once more with the dial already at its value. C13 and
  C13k are also kept uncropped. The show of C13 is saved by the probe from the composition on screen, at
  "Warehouse" +42. C3 and C8 are captured before the first load of the run (G7 holds C13k against them). The bar
  on "modalComponents" reads 0 in every state without a menu and exactly 1 in C9 (D16; H-5). N, the whole text list
  with every number replaced by "#", is computed by the probe from the dump's "texts"; the dump marks nothing."
  The rest of D20 stands.

Outside the asked range, same withdrawn behaviour (a builder reads these too)
 D21  OLD (RD:485-486): "created in S5a with "wheel" and "replacedByLoad"; S5b adds the button, panel and Gain parts".
      NEW: "created in S5a with "wheel" and "texts"; S5b adds the button (text, colour, bounds, tooltip), panel and
      Gain parts. "texts" = the text of every juce::Label, juce::Button, juce::TextEditor and juce::ComboBox reached
      from each visible top-level component of the app through visible children only, in tree order, empty texts
      left out, read on the message thread. The walk is one free function with its own unit suite
      (test_visible_texts)." VERIFIED: Label::getText and Button::getButtonText are in use (C:src/MainComponent.cpp:
      2158, C:src/ui/DeckView.cpp:810); TextEditor::getText, ComboBox::getText and juce::Desktop's list of top-level
      components exist (F39). The unit suite tests the walk on a parent it builds; that the walk reaches the app's
      own window is R11 (k): the list must hold "Resync" and the file label's text.
 D22  OLD (RD:496-497): "the "sync" key, the single writer (D13), "Default at 0 writes no key", the load rules (D14),
      the replaced value (D15), the test routes." NEW: "the "sync" key, the single writer (D13), every save writes
      the key ("Default" at 0 included), the load rules (D14: a composition open only; a file without the key never
      touches the dial; nothing says the dial changed; same venue name -> the file's number replaces the venue's and
      the old number is not kept; a full venue list -> a show that names a venue the list does not hold opens
      without changing Sync), the test routes."
 Stage table, S5a (RD:517): strike "the replaced value (D15),"; after "and its dump (D12)" add "with the text list
      (D21)". "No new visible widget." stays.
 Plan B6, withdrawn sentences: PD:447-448 ("present = !(current venue is "Default" and ms == 0) ... carries no key");
      PD:456-458 ("the load notice says ..."); PD:462 ("the notice with the old number covers that case"); PD:464
      ("the glide, the notice and the SYNC button make it visible" -> the glide and the SYNC button's value); PD:415
      ("Behaviour = plan I11 unchanged" -> unchanged except the refusal caption, D20).
 Plan B6, added: PD:451-455's three adopt cases gain a fourth (D14, ADDED): the store refuses -> nothing changes.
 Overtaken, no builder input: RD section 9's K2 (:756-757, "D15 keeps the old number in sight") -> K1 below; its K9
      (:772-773, Q2's default) -> closed by F1; the attack table's SO-9 / SO-10 rows (RD:192-193) -> sections 3-4.

## 4 "replacedByLoad" -- THE RULING

RULING: DROP. SyncOffsetController gets no memory of the value a load replaced. This settles adoption item 7's "the
S5a builder reports which": the builder builds none and reports nothing on it.

Why
 1. Every form D15 gave it is a text about an event: the notice, the caption "was +55 before this composition was
    opened", the tooltip (RD:413-414). F2 removes all three; an event text inside a tooltip is removed too (F17).
 2. A control without any text cannot say what its number is. A bare "+55" beside the value box means nothing until
    a caption or a tooltip explains it -- and that explanation is the event text again. Adoption item 7's own
    condition ("dropped if it needs a text") is met. INFERRED: a judgment, not a measurement.
 3. A control that appears only because something happened is what Boris turned down the same afternoon for Remove
    Deck (F3). His route there was Cmd+Z. That route does not exist here: the dial is "not undoable" by design (F15)
    and a composition open clears the undo history (F14). VERIFIED.
 4. What still keeps a number, with no new code: a load that selects ANOTHER venue leaves the venue he was on, and
    its number, in the list; he picks it again. INFERRED from the adopt rule (PD:451-455) and F15; pinned by R11 (h)
    and the unit case of D15.
 5. What loses a number: a load that names the venue he is ON with another number. Nothing keeps the old one. This
    is not a corner. A user who never made a venue has only "Default" (F30), so every show saved on "Default" with
    another number does it: the show saved at home on "Default" 0 sets the room's "Default" +55 to 0. Round 1 called
    this the rare case; that was wrong. It is still what "every show remembers it's sync" asks for (F1) and what his
    "yes" to Q10 already meant (F5). The +55 comes back by opening any show saved in the room, and it is never lost
    when the room has its own venue. Pinned by R11 (i); shown to him as check 6.12; asked as Q1.
 6. Why 5 does not switch the ruling: the only design that brings the number back is the runner-up, and it is a
    control that appears after an event (3) whose number needs an explanation (2). A builder, a critic or an
    architect cannot allow that. He can.

What R11 (d) asserts: "set "Warehouse" to +55, load the file of (a) -> targetMs 42; the store's "Warehouse" is 42; GET
/api/debug/ui_text answers "ok": true and none of its strings holds "Sync" or "Warehouse"; S(d) = S(k); GET
/api/debug/sync_ui_dump answers 200 with "wheel" and without a "replacedByLoad" property." The nudge clause is gone.

RUNNER-UP: a recall button in the SYNC panel.
 - Widget: one juce::TextButton in the panel's Offset row, between [+] and the bar, 40 px taken from the bar.
 - State display: its text is the stored number and nothing else ("+55"), in the secondary text colour; hidden while
   nothing is stored, so the resting panel is the panel of section 2.
 - Reached: click SYNC, click the button -> the controller selects the stored venue and sets the stored number. The
   memory is D15's: set by an adoption that changed the venue or the number, cleared by setMs, nudge, selectVenue,
   createVenue, renameVenue, removeVenue; never saved.
 - Tests: test_sync_offset_controller (set / not set / cleared by each of the six / recall restores venue and
   number); test_sync_panel (hidden when empty; text equals the number; click calls recall); R11 (d) takes back "the
   dump's "replacedByLoad" equals {"venue":"Warehouse","ms":55}; then POST /api/sync/nudge +1 -> "replacedByLoad" is
   gone"; R10 gains {"click":"recall"} -> targetMs 55; G7 gains a state "the panel after a load that replaced a
   number", and the C13 bars (the panel against C8; S and N of C13 against C13k) are re-written for it.
 - Cost: about 100 lines and one more stage (S5c, panel only).
SWITCH CONDITION: Boris answers Q1 (section 5) with "yes", or reports at check 6.6 or 6.12 that he lost a number he
needed. Only he can allow a control that appears after an event. Nothing a builder or a critic finds switches it.
H-6 (HR:31-32): the question goes on the Boris page under check 6.12, so that he sees the loss before he answers.
Nothing is built for it.

## 5 SECTIONS 6-7 RE-STATED (the page Boris opens)

WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like)

 6.1 Tap along to a track with Sync at 0. -> The circle at the top pulses on your taps. Open SYNC and type +150. ->
     The circle stays exactly on the music; the picture's beat motion now lands a little after it. Wrong: the circle
     drifts off the music, or the picture does not move later.
 6.2 With Sync still at +150, tap the tempo again. -> The circle answers each tap at once, with no wait, and the
     picture stays that same bit behind. Wrong: the circle waits before it follows your tap; or after tapping the
     picture is suddenly back on the beat.
 6.3 Type -150. -> Beat-locked motion lands a little BEFORE the music; loudness-driven motion does not run early (it
     cannot). The circle stays on the music. With BPM on Auto the circle may pause for a moment, together with the
     picture, when the beat tracker corrects itself. Wrong: the circle runs ahead of the music.
 6.4 Record a short take with music at Sync 0; set Sync to +300 and replay it. -> Everything plays a touch late, as
     set; in the last third of a second the final moves arrive together as the music stops. Wrong: the replay never
     ends, or the whole take is out of step.
 6.5 At -300: press Resync on the one, start a 4-bar effect, then tap the tempo several times without pressing
     Resync. -> The 4-bar effect stays on the bar you set. Wrong: after tapping it sits on a different bar, or it
     jumps ahead when you tap.
 6.6 Click SYNC, make a venue "Test room", set +42, save the composition with Cmd+S, quit, reopen, load it. ->
     SYNC +42, venue "Test room". Set it to +55 without saving and load the composition again. -> It glides back to
     +42 and the SYNC button reads +42. Nothing says that Sync changed, and the +55 is gone. Load a composition
     saved before this version. -> Sync does not move. Load a single deck from another show. -> Sync does not move.
     Wrong: any note or sentence about Sync appears; SYNC still reads +55; an old show or a single deck moves Sync.
 6.7 Gain: drag it in the part you use (below a quarter). -> Twice the room to move. Is that enough? Wrong: the
     slider is no longer than before.
 6.8 Look at the top bar in your usual window size, Manual on and off. -> Nothing is cut off on the right. Play,
     Stop, the circle, Tap, Resync and the buttons after them sit a little further right than before (Gain took the
     room). SYNC is the small word above the tracker word next to the BPM; clicking anywhere in that little block
     opens it. Is it easy enough to hit?
 6.9 Open SYNC and leave it open. -> Your keyboard still fires clips. Click a clip. -> The panel closes and the clip
     fires. Wrong: keys do nothing while the panel is open, or the first click only closes the panel.
 6.10 In the SYNC box type 60 and click + without pressing Enter. -> 61. Wrong: your 60 is gone.
 6.11 Put SYNC on the venue "Default" at 0 and save a show. Pick "Test room" (+42) again, then open that show. ->
     SYNC 0, venue "Default". Open SYNC and pick "Test room" in the venue list. -> +42 again. Wrong: Sync stays on
     +42 after the show opens; or "Test room" has lost its +42.
 6.12 Pick "Default" in the SYNC venue list and set it to +55 (make no new venue). Open the show of 6.11 again. ->
     SYNC 0, and the +55 is gone: nothing brings it back. That is "every show remembers it's sync" on the one venue
     you are on. Wrong: Sync stays on +55.
     QUESTION (the only one; nothing waits for it and nothing is built for it). When a show you open was saved on
     the venue you are on, with another number, the number you had is gone -- you have just seen it. DEFAULT: leave
     it so; a room keeps its number when it has its own venue. The other way: one small button in the SYNC panel
     that shows only the old number; a click sets it back. (It would be a button that appears after something
     happened, the kind you turned down for Remove Deck.)

 Said on the page, no answer needed:
  - With Sync at +100, a Tap or Resync reaches the picture 100 ms after you press. The circle shows it at once.
  - About 1 tap in 20 at a later Sync, the lit quarter of the circle may step once a moment after the tap.
  - A take recorded at a smaller Sync than you replay it at plays its very last moves together as the music ends.
  - Every show remembers its Sync. Opening a show switches Sync to the venue and number saved in it, 0 included.
  - If the show was saved on ANOTHER venue than the one you are on, your venue keeps its number -- pick it again in
    the SYNC list.
  - If the show was saved on the venue you are ON, with another number, the show's number replaces yours and the old
    number is not kept. If you never made a venue, everything is on "Default", so this happens with every show: one
    saved at home on "Default" 0 sets the room back to 0. To keep a room's number, give the room its own venue.
  - Nothing on screen says that Sync changed. The SYNC button always shows the value in use.
  - A show saved before this version does not touch Sync. Loading a single deck never does.
  - A venue name that is empty or already taken is not accepted: nothing changes, nothing is written.
  - The venue list holds 64 venues. When it is full, a show saved on a venue that is not in your list opens without
    changing Sync. Delete a venue you no longer need and open the show again.
 (Harmony, not for the page: the line about a venue name stands -- H-2 confirmed C11. The line about the full list
 is the ruling of section 8.3; it may be left off the page, the case needs 64 venues. The sync dial merges before
 the app-wide removal lane (H-7), so the top text line still reads "Loaded: <name>" after a load; that is about the
 file, not about Sync.)

BORIS QUESTIONS (each has a default; nothing waits)

 Q1 is the QUESTION printed under check 6.12 (H-6: there, so that he sees the loss before he answers). There is no
    other question.

 Closed, not asked again: the plan's 8.1, 8.2, 8.3 (RD:719-720). RD's Q1, Gain -- Boris: "yes" (BD:695). RD's Q2, a
 show saved on "Default" at 0 -- Boris: "every show remembers it's sync" (BD:696-697).

What changed against RD's sections 6-7
 - 6.1 - 6.5 (RD:675-689) and 6.8 - 6.10 (RD:695-701): word for word.
 - 6.6: the yellow note and the panel's "was +55" are gone; "Load an old composition" -> "a composition saved before
   this version" (every newer save holds the key); a Wrong clause added.
 - 6.7: RD:694 word for word, plus a Wrong clause. "Is that enough?" stays: his "yes" (F5) was given before he had
   dragged it. No answer = yes.
 - 6.11 (round 1) and 6.12 (round 2): new.
 - Said-lines: RD:704-706 word for word; RD:707-708 replaced by the lines after them (six in round 2, a seventh in
   round 3).
 - RD's Q1 and Q2: answered, removed. One new question.
 - Round 3: 6.12 begins "Pick "Default"" -- after 6.11 the dial is on "Test room" (R2: NIT-5); the question stands
   under 6.12 (H-6); one said-line is added (the full list, section 8.3); the Harmony note no longer waits on K2.

## 6 RISKS (the strongest counterargument first; the cheapest test that would refute each choice)

 K1 (strongest) DROP makes a quiet failure quieter, and it is the usual case, not a corner: on a store that holds
    only "Default", every show saved with another number resets the room's number and nothing keeps the old one
    (F30). The picture then sits some tens of ms off the beat and the only sign is "SYNC 0" where "SYNC +55" stood.
    The council's stage-operator seat asked for the opposite (RD:193), and a recall button costs about 100 lines.
    Why DROP still stands: Boris removed this class of thing twice in one afternoon, once as text (F2) and once as
    a control (F3); the control cannot explain itself without the text (section 4, reason 2); and the loss is the
    rule he chose, which opening any show saved in the room undoes. Cheapest refutation: check 6.12 and his answer
    to Q1, one line. Before him, nothing technical can refute a preference.
 K2 CLOSED by H-2 (HR:19-23). C11 is confirmed as re-stated: no refusal caption; a refused name leaves the panel as
    it was. Boris's fourth line settles it (F33). What is left of the risk is its cost: typing a taken name into New
    venue does nothing visible. H-2 gives that to the interaction-logic seat (G7: does the silent refusal read as a
    fault?) and admits the cure that needs no text -- a control greyed while its input cannot be accepted (OK greyed
    while the name is empty or taken). That is still a follow-up, not ruled here.
 K3 Both no-text detectors see text WIDGETS. A notice drawn by a component's own paint code, the message of an alert
    window, and a tooltip are in neither list. A text shown later than 1 s after the load, or for less than 1 s,
    escapes as well. An alert escapes every TEST-mode row, G7 included: the app's alerts sit behind `if (!testMode_)`
    (F38), and a builder who copies that pattern adds one that no probe and no capture can see. What covers the
    rest: in S5b the uncropped C13 / C13k captures read by the logic seat; in S5a the stage's own scope "no new
    visible widget" at the state review, the lint on the old names (F7), and, for the alert, the reviewer's grep of
    the diff (D14) and check 6.6 on the page. Cheapest test that the detector has teeth: R11's LIVE RED line.
    Runner-up, declined: a lint over the S5a diff (no added `juce::Label`, `setText(` or `setTooltip(` line) -- it
    gates the diff, not what the app shows. Declined as before: a lint on guessed names.
 K4 CLOSED by H-3 (HR:24-25) for the additions of round 2: R11 (h), (i), (k), the copy A, the path Z, the WAITS and
    TEXTS paragraphs, "texts" with test_visible_texts, the dump's "tooltip", C13k and the test_staged_load list
    entry are ADMITTED; each tightens a row. OPEN again for the additions of round 3, which H-3 has not seen
    (section 8.5 lists them). Each adds a way to fail and none removes one. If Harmony strikes one: without LOADS a
    load that did not happen passes (b) and (k); without the absent-file sentence a stale file passes (a) and (a3);
    without (k)'s two clauses a text list that reads nothing, or a text every keyed load shows, passes; without (j)
    the full store is held by unit cases alone; without N a text that names neither word reaches the seats
    unmeasured.
 K5 CLOSED in its first half: the merged tree c45b579 was read and sections 1-3 are pinned to it (section 8.2); the
    three routes are there (C:src/api/ApiServer.cpp:333, :341, :362) and so is the lint case (F7). What is left:
    S3f, S4 and S6 are built on top of C: before S5a (F40). By their scope they touch neither the save path, the
    staged load, the venue store nor the test routes -- INFERRED -- but the lines they add will move the numbers
    again. Cheapest test, on the tree S5a starts from: `git grep -n 'composition_.saveToFile(' -- src/
    MainComponent.cpp` still prints three lines and `git grep -c 'adoptFromComposition' -- src` prints nothing.
 K6 "Every save writes the key" reaches further than the Default-0 case. A plain Save of an OLD show stamps tonight's
    dial into it; from then on that show moves the dial wherever it is opened. That is F1's rule; 6.11, 6.12 and the
    page lines say it.
 K7 R11 (g)'s Load Deck half rests on an accident. Load Deck takes a composition file only because its shape check
    looks for "layers", and since the deck change a composition has that key too; the code's own comment says a
    composition is refused. Round 3 read the rest of the path at C: (F27): the parser makes one row for every
    "layers" entry and the validator refuses only a deck without rows -- so the append goes through, as a deck of
    empty rows. VERIFIED for the three checks, INFERRED for the row parser. If main tightens the shape check, no tab
    is added and the wait fails. The clause then needs a deck file: "POST /api/debug/load_deck with D, a deck file
    the probe writes (as C:.harmony/probe-async-load.py:203 does) with "sync": {"venue":"Warehouse","ms":42} added".
    Cheapest test, on the merged tree: POST /api/debug/load_deck with a saved composition, then GET
    /api/debug/deck_tabs -- one tab more or not. Side finding for Harmony: Load Deck taking a whole composition
    file is probably not meant.
 K8 RULED in round 3 (section 8.3): a full store and a new name -> the adoption is refused whole. Its own strongest
    counterargument: on that night "every show remembers it's sync" is not kept and nothing says so -- the show
    opens, the picture runs on the room's number, and a Cmd+S then writes the room's number into the show (K6). Why
    it still stands: every way to put the show's number on the dial with a full list destroys a number he stored,
    on disk, without a sign; the refusal destroys nothing and is undone by deleting one venue. Cheapest refutation:
    Boris, told the page line, says he would rather have the show's number.
 K9 The model's copy of the venue and number is fresh only at a save (D13). Any other reader of Composition::toVar
    sees the values of the last save or load. No such reader was looked for. ASSUMED harmless.
 K10 CLOSED by H-5 (HR:27-30): the bar is admitted -- 0 in every state without a menu, exactly 1 in C9. That the
    count is 1 in C9 is still INFERRED from F29; H-5 has the S5b builder MEASURE it on the first C9 dump and STOP
    and report if it is not 1 with only the venue menu shown. A 0 there can also mean the menu had already closed
    itself: JUCE dismisses a menu when no JUCE component has the focus (J:menus/juce_PopupMenu.cpp:1441-1445,
    VERIFIED lines, INFERRED reading) -- then the C9 capture is wrong too, and the clause has caught it.
 K11 The wait of (g) rests on F25's inference (the deck_tabs answer is produced after finishStagedLoad has
    returned). Cheapest test: R11's LIVE RED line -- the mutant app must FAIL (g). If it passes, the wait is wrong,
    not the mutant.
 K12 S() looks at strings that hold "sync" or the venue name. Round 2 accepted that with "every notice the withdrawn
    texts describe names both". That was wrong: the withdrawn panel caption, "was +55 before this composition was
    opened" (RD:413-414), names neither. In G7 the hole is closed by N(C13) = N(C13k) (R2: SHOULD-5). In R11 it
    stays: a text added in S5a that names neither word and shows only when the dial changed passes R11 and is caught
    one stage later, by G7's N, before anything merges (S6, S5a and S5b reach main together, F40). Runner-up: N(c) =
    N(k) and N(d) = N(k) in R11. Switch: S5a is to merge without S5b.
 K13 N can fail on a correct tree. It forgives numbers, not words: a label whose WORDS change by themselves between
    C13 and C13k (a tracker state, a detected genre) makes the two lists differ. In TEST mode with no sound coming
    in such labels should stand still. INFERRED; nothing was run. Cheapest test: the first G7 run prints the strings
    that differ. A string that differs by itself is exempted only by Harmony, by its exact text and the reason, in
    the gate record -- never by a builder and never by a pattern.
 K14 The two anchors read "no string occurs more often than before any load". That direction is chosen because a load
    empties the inspectors: strings go, none come. If a load on a correct tree BRINGS a string that holds "sync" --
    the Clip tab showing a clip whose transport is "BPM Sync" (F31) -- (k), or G7's C3 / C8 clause, fails on a
    correct tree. The probe inspects nothing, so it should not. INFERRED. Cheapest test: the first run of (k).
 K15 (k)'s "texts" clause names two strings. After the app-wide removal lane the file label no longer reads
    "Loaded: <name>" (F28); the clause then rests on "Resync" alone, which is why the label is asked for only "when
    that string is not empty". A top bar without its Resync button would need the clause re-stated.
 K16 The LIVE RED line costs one scratch build of the app and one run of R11 on it, as R7's does. If Harmony finds
    that too dear, what still shows the text list alive is (k)'s "texts" clause; the proof that a NEW label is seen
    would then rest on test_visible_texts alone.
 K17 Boris's fourth line keeps one fail message, a failed save. The venue list has a failed save of its own: "Could
    not save the sync venues to ..." lands in lastError (C:src/sync/SyncOffsetController.cpp:142) and is never on
    screen. I read his exception as the two "Save failed" boxes he was asked about (BD:718-720: B06, B08), so that
    reason stays REST-only and the panel shows no lastError at all (H-2). If Harmony reads "a failed save" wider,
    that is one more text for S5b and a change to C11's unit case. Not ruled here; it is Q-H1 of section 8.5.

## 7 REVISION (round 2; the review is RV:)

(Round 3 note. This section is the record of round 2. Its line numbers are the ones read then -- L: = 4a1f240, M: =
34179a2; section 1 holds the numbers of the merged tree. Harmony has since ruled on three of its items: S-3's bar is
ADMITTED (H-5); the log line dropped under S-4 stays dropped (H-4); N-6's "Harmony may leave it off the page" is
settled -- the question goes on the page, under check 6.12 (H-6). The K-numbers it names are those of section 6:
K10 and K2 are closed there.)

Each finding was checked at its source before the text was touched. ACCEPT = the finding is right and the text is
changed in place. Where the fix differs from the one proposed, the reason is given.

 MUST-1  R11 (g) passes vacuously -- ACCEPT the finding; the proposed fix is NOT taken as written.
         Checked: load_deck and duplicate_deck answer on the line after callAsync (F23); GET /api/sync never asks
         the message thread (F24). The race is real. The proposed wait ("file_label equals 'Loaded deck: <name>'")
         would work on main today, but that text is row A07 of the removal inventory and is removed app-wide (F28):
         the row would stop passing the day that lane merges. The wait is on the deck tab count instead, read on
         the message thread (F25). In place: R11 WAITS; (g)'s RED; K11.
 S-1     ui_text can answer {"ok": false} -- ACCEPT. Checked at M:src/api/ApiServer.cpp:2096-2100 (F26). Every
         ui_text clause of R11 (b), (c), (d) and of G7 now says "answers "ok": true and ...".
 S-2     the detector sees five strings -- ACCEPT. Checked (F9). Fix: the dump's "texts" (every text widget, D21) and
         a control load (k); the row compares S(x) with S(k). Of the two fixes proposed, the first is taken, with a
         comparison instead of a word search because "Resync" and the fixture's file name hold "sync" (F31). The
         second (a whole-window diff) is taken where captures exist: G7's uncropped C13 / C13k for the logic seat.
         It is not taken inside R11 (no captures in S5a; the picture moves). What is still unseen: K3, K12.
 S-3     "modalComponents" 0 cannot hold in C9 -- ACCEPT. Checked: J:menus/juce_PopupMenu.cpp:2162 and RD:428 (F29);
         that the count is 1 is INFERRED. Fix: 0 where no menu is shown, exactly 1 in C9 -- tighter than leaving C9
         out. D16 names the count. This changes a ruled bar for a reason that is not Boris's words: Harmony's call
         (K10). [Round 3: ADMITTED by H-5; K10 is closed.]
 S-4     page 6.6 against the app-log line -- ACCEPT; the log line is dropped, not the page softened. It was my
         addition in round 1; nobody asked for it, no gate held it, and adoption item 7 withdraws "any other event
         text S5a / S5b would add" (F6). In place: D14, D15, D22, section 0, section 4, the page; round 1's K7 is
         gone. (FN:164 shows the app-wide ruling keeps existing log lines, so a log line would not have been
         forbidden; it was unasked and ungated.)
 S-5     a store with only "Default" hits the same-venue case -- ACCEPT. Checked: L:src/sync/SyncVenues.h:34 and
         PD:451-455 (F30). Round 1's "only one case loses a number" understated it. In place: section 0 item 4;
         section 4 reasons 4-6; K1; R11 (i) and its unit case; check 6.12; the page lines; Q1.
 N-1     "(F6)" cited for the lint -- ACCEPT. It is F7.
 N-2     the tooltip cite -- REJECT the line given, ACCEPT that one line was too narrow. The tooltip's text is at
         PD:390 and the word "Tooltip:" ends PD:389; it is not at PD:388-389. The cite now reads PD:389-390.
 N-3     (a) "keeps its words" although a sentence was added; 6.7 rewritten without being listed -- ACCEPT. Section 0
         now says (a) gains a sentence. 6.7 is RD:694 word for word again, plus a Wrong clause; section 5 lists
         every change against RD's sections 6-7.
 N-4     main has its own no-focus-on-click example -- ACCEPT. Checked: M:src/ui/ClipInspector.cpp:445, M:src/ui/
         OutputWindow.cpp:65 (F22). D16 cites the first; the ASSUMED second API name is gone.
 N-5     a critic who reports "Loaded: <name>" is "right" while event-text requests are void -- ACCEPT. One rule, in
         G7's CRITIC PANEL paragraph: asking for an event text is void; reporting a pre-existing one is recorded for
         the removal lane. The page line is true either way (the label speaks of the file, not of Sync); the note
         under the page says what still shows until that lane merges.
 N-6     Q1 re-offers a control of the kind he turned down -- ACCEPT in part. The question stays: S-5 makes the loss
         his usual case, so it is his to decide. It now says what kind of button it is, and 6.12 lets him see the
         loss before he answers. Harmony may leave it off the page; the default then stands. [Round 3: settled by
         H-6 -- it goes on the page, under check 6.12.]
 N-7     DROP rests on a judgment -- noted, no change. Section 4 reason 2 is labelled INFERRED.

 Found while verifying (not in the review)
  V-1  My own round-1 sentence "The probe copies P to A at once" raced: save_composition answers before the save
       (F23). (a) now copies only once its clause holds, and all three save clauses are polled (WAITS).
  V-2  R11 (g) loads "the file of (a)", a composition file, as a deck. It passes the shape check only because a
       composition has a top-level "layers" key since the deck change (F27). With the old read ("unchanged after
       each") a refused Load Deck would have passed unseen; with the wait it fails. K7 holds the replacement clause.
  V-3  A string dropped because it holds the loaded file's name would also have dropped a notice that names the
       file. The name is cut out of the string instead (TEXTS).

## 8 FINAL FOLD (round 3)

8.1 Harmony's rulings (HR:), folded in place
 H-2  C11 CONFIRMED: no refusal caption; a refused name leaves the panel as it was; a greyed control is a state
      display and admissible; the interaction-logic seat judges whether the silence reads as a fault. Closes K2.
      Boris's fourth line is quoted, from BD:, where C11 is ruled: F33; G7 "What changed"; D20. In place: section 0
      (the C11 tradeoff, item 2), F17, G7 (the CRITIC PANEL paragraph, the interaction-logic seat), D20, K2, the
      page's note.
 H-3  The additions of round 2 are ADMITTED. Closes K4 for them. Nothing admitted is struck. One sentence of the
      admitted TEXTS paragraph is changed (the cut; 8.4 SHOULD-4) and the admitted WAITS paragraph gains sentences
      (8.4 SHOULD-2, NIT-5, V-4): 8.5 lists them for Harmony.
 H-4  No app-log line. In place: D14 ("no app-log line is asked for (H-4)"). Round 2 had already dropped it.
 H-5  The modal bar ADMITTED: 0 in every state without a menu, exactly 1 in C9; the S5b builder measures the 1 and
      STOPs if it is not. Closes K10. In place: G7's bar (now in H-5's words, the closed-panel states included), G7
      "What changed", D16, D20, F29.
 H-6  The one question goes on the page under check 6.12; nothing is built. In place: section 5 (the QUESTION under
      6.12; the BORIS QUESTIONS block points to it), section 4, section 0 item 5. Settles N-6 of section 7.
 H-7  The sync dial merges before the removal lane. In place: F28, G7's CRITIC PANEL paragraph and "What changed",
      the page's note. The clause "unless that lane has merged first" is kept: it costs nothing and holds if the
      order ever turns.
 Outside this file's range: H-1 (Pitfall NN in history files) and H-8 ([timing] under load) touch none of its rows.
 One thing in HR: that only Harmony can mend: HR:10 stamps Boris's fourth line 21:03:06; BD:718 records it at
 20:59:50 (21:03:06 is when HR: was written, HR:3). The words are the same. This file quotes BD:.

8.2 The merged tree c45b579 (C:)
 Every line number of sections 1-3 is now a C: line, read through `git show c45b579:<path>`; the M: numbers that
 remain are named as main's (F7, F8). Section 7 keeps round 2's numbers.
 INFERRED in round 2, VERIFIED now
  - The lint that forbids the load-notice names is in the lane (F7; round 2: "INFERRED for the lane after M0").
  - POST /api/debug/save_composition is in the lane (F12; round 2: "It reaches the lane with M0 (INFERRED)").
  - The routes that answer early, the deck-tab route and ui_text's "ok": false: read on main in round 2, read on
    the merged tree now (F23, F25, F26).
  - The ground of every RED line of R11: nothing in C: writes or reads a composition sync key, no adopt hook exists,
    the dump route does not exist (F11-F13). Round 2: "INFERRED from L: and M: together".
  - The JUCE calls behind "texts" (F39; round 2 had three of them ASSUMED).
  - The path a composition file takes through Load Deck: the parser and the validator were read (F27; round 2 had
    read the shape check only).
 STILL INFERRED
  - What a probe reads: nothing was run. That one message-thread call does not run inside another (F25) and that
    posted calls keep their order (F35). That the count of modal components is 1 in C9 (F29; H-5 has it measured).
  - That S3f, S4 and S6 leave all of this as it is (F40).
 CHANGED
  - Line numbers. MainComponent.cpp is 47 lines longer above the staged load than on main: finishStagedLoad M:3318
    -> C:3365; the Composition branch M:3347-3363 -> C:3394-3410; the three saves M:3551 / 3574 / 6694 -> C:3598 /
    3621 / 6741. ApiServer.cpp: ui_text's strings M:2104-2108 -> C:2125-2129; save_composition M:2178-2197 ->
    C:2199-2218; deck_tabs M:2266-2294 -> C:2289-2315; the sync venue route L:2484-2504 -> C:2804-2824.
  - F9 gained a fact that changes a fix: three of ui_text's five strings are not widget texts (8.4 SHOULD-1).
  - F31 gained a third text that holds "sync" and is not about the dial: "BPM Sync" (K14).
  - F21 now lists the suites and the fixture that do NOT exist yet.
  - New facts: F33 - F40.

8.3 RULING: adoptFromComposition when the venue store is full
 The case: the store holds 64 venues -- kMaxVenues, C:src/sync/SyncVenues.h:29; a 65th is refused with "There can be
 at most 64 venues." (C:src/sync/SyncVenues.cpp:122-123) -- and the opened show names a venue the store does not
 hold. PD:451-455's three cases do not cover it. Nobody has to mean it: every opened show that names a new venue
 ADDS one (the plan's first case), so a list fills from other people's shows and from one venue per night.
 RULING: the adoption is REFUSED WHOLE. The venue he is on, its number, the list and the settings file stay as they
 were. lastError() holds the store's own reason, read through GET /api/sync. Nothing is written on screen: no
 caption, no label, no alert; no log line is asked for. The show itself opens as any show does. The SYNC button goes
 on showing the value in use, which is the truth. The same holds for any adoption the store refuses (a name it
 cannot take). A full store that HOLDS the named venue is not this case: it adopts as usual.
 Why
  1. It is the controller's own contract: "A refused op changes nothing and lands in lastError()" (F15); `apply`
     works on a copy and keeps it only when the op is accepted (F36). Written as ONE `apply` op, the adoption gets
     this ruling for nothing.
  2. Every other choice destroys a number he stored, on disk, with no sign (see the runner-up).
  3. D15's sentence -- "adoptFromComposition changes only the venue the file names" -- stays true: a refused
     adoption changes none.
  4. It is undone by hand in two steps: delete a venue, open the show again.
 RUNNER-UP: put the show's NUMBER on the venue he is on (setMs), without the name. It keeps the letter of "every
 show remembers it's sync" (F1) -- the picture runs on the show's number -- and it is what the most natural wrong
 build does by accident: createVenue(name) is refused, the setMs(ms) that follows is accepted, and that accepted op
 clears lastError (F36). It writes over the number of the room he is standing in, saves that to the settings file,
 and the next Cmd+S writes the room's NAME into the show beside the show's number. Rejected. Also rejected: dropping
 a venue to make room (the same loss; no venue has a last-used stamp to choose by); a 65th venue for adoptions only
 (the status copy has room for 64, F37).
 SWITCH: Boris, told the page line of section 5, says he wants the show's number on the dial even when the list is
 full. Only he can choose to lose a stored number. Nothing a builder, a critic or a reviewer finds switches it.
 UNIT CASES (test_sync_offset_controller; their RED stubs are in G1): (1) a full store, a new name -> nothing
 changes, lastError() not empty, no save scheduled by it, one notification; (2) a full store, a name it holds, another
 number -> that venue selected with the file's number, the other 63 untouched; (3) a name the store cannot take ->
 nothing changes, lastError() not empty. test_sync_panel: a refused adoption changes no widget's text.
 WHAT R11 ASSERTS: clause (j). After the load of a show that names "Warehouse" into a full store without it: GET
 /api/sync is unchanged -- the venue, targetMs, the 64 "venues" entry for entry; "lastError" is not empty; ui_text
 answers "ok": true with no "Sync" and no "Warehouse"; S after the load equals S before it. No on-screen text under
 the ruling, and none under the runner-up either: the runner-up fails (j) on targetMs, not on a text.
 NOT RULED, admissible under H-2: the panel's "New venue" entry greyed while the list is full -- a state display
 that would let him see why. A follow-up for S5b, like the greyed OK of C11.
 ON THE PAGE: one said-line (section 5). No check: it would take 64 venues.

8.4 The second review (R2:), finding by finding
 Each was checked at its source first. ACCEPT = the finding is right and the text is changed in place.
 SHOULD-1 the second detector can be dead -- ACCEPT the finding; the first half of the fix is NOT taken as written.
   Checked: nothing in round 2's row says "texts" is there, is not empty, or reaches the window; S(b) = S(c) = S(d)
   = S(k) = nothing passes. The fix proposed -- "texts" holds every non-empty string ui_text returned -- would fail
   a correct tree: "inspected_layer" and "inspected_clip" are names read from the model and "inspector_tab" is a
   word made from an enum (C:src/MainComponent.cpp:2163-2178, F9); none is read from a text widget. Taken instead:
   "texts" holds "Resync" (a juce::TextButton the top bar shows, F31) and the file label's text (a juce::Label the
   window shows, F9). The second half is taken as proposed: the mutant app is a line of the row (LIVE RED), as R7's
   is. In place: R11 (k), LIVE RED, RED; D21; K15, K16.
 SHOULD-2 (a) and (a3) pass on a stale file -- ACCEPT. Checked: round 2's poll is satisfied by any file at P or Z
   that already holds the value. In place: R11's header (a scratch folder made new and empty for the run) and WAITS
   ("P, Z and A do not exist before the step that writes them").
 SHOULD-3 (b) and (k) pass if the load did not happen -- ACCEPT. Checked at C:src/api/ApiServer.cpp:1146-1166 and
   :1199-1206 (F34): HTTP 200 with "ok": false, no 4xx. (The review cites :1156-1163; the two refusals run to
   :1166.) In place: R11's LOADS sentence, naming (k), (b), (c), (d), (e), (h), (i) and (j). The same hole was in G7
   -- a C13k whose load did not happen is C13 read twice -- so: "the loads of C13 and of C13k answer "ok": true".
 SHOULD-4 the file-name cut is unanchored and the stems are not pinned -- ACCEPT the finding; the fix is NOT taken
   as proposed. Checked: round 2 cut "the name of the file just loaded"; a stem "e" takes the e's out of
   "Warehouse" in S(x) and in S(k) alike. Taken instead: nothing is cut but the fixture's name, the one loaded file
   whose name holds "sync" (F31). No scratch stem is cut, so there is no stem to pin and nothing to anchor. An
   anchored cut would also leave the fixture's name standing wherever it follows a bracket, a quote or a slash, and
   fail (b) on a correct tree. In place: R11 TEXTS, "What changed".
 SHOULD-5 S(C13) = S(C13k) compares only strings that hold "sync" or "Warehouse" -- ACCEPT the finding; the fix is
   taken in another form. Checked: the two states end the same; and round 2's K12 was wrong to say every withdrawn
   notice names both words -- the "was +55" caption names neither (RD:413-414). Proposed: compare all texts but
   those the dump marks as moving. Taken: N -- all texts, every number replaced by "#", compared as counted sets.
   Nothing is marked by hand, so no builder holds a switch that hides a text. The seat's uncropped reading is kept
   and gains one question. In place: G7's bars, "What changed", RED; D20; K12, K13.
 NIT-1 the names rule against the fixture's name -- ACCEPT. In place: R11's header ("the fixture of (b) is not a
   scratch file").
 NIT-2 H-5 says 0 "in every state without a menu" -- ACCEPT; the ruling wins. In place: G7's bar names C1 - C5 too.
 NIT-3 the stamp of Boris's fourth line -- ACCEPT. Checked at BD:718 (20:59:50), HR:10 (21:03:06) and HR:3. HR: is
   not mine to edit: 8.1 names it for Harmony. This file quotes BD:.
 NIT-4 the conditional about K2 on the page; K2 and K10 as open calls -- ACCEPT. In place: section 5's note, K2,
   K10, section 0 item 2.
 NIT-5 three parts. (1) 6.12 "Stay on "Default"" -- ACCEPT: after 6.11 the dial is on "Test room". In place: 6.12
   begins "Pick "Default"". (2) (e) and (i) need a venue removed and the row does not name the route -- ACCEPT. In
   place: R11 WAITS names POST /api/sync/venue/remove (C:src/api/ApiServer.cpp:367). (3) "R11 cannot see an alert;
   G7 C13 catches it" -- the first half ACCEPT, the second REJECT: G7 runs in TEST mode too, and the app's alerts
   are not shown in TEST mode (C:src/MainComponent.cpp:3604-3610, :3706-3710; F38). No row sees an alert built in
   that pattern. In place: D14 (the reviewer's grep of the S5a diff), K3.

 Found while verifying (not in the review)
  V-4  The sync POSTs the row uses (set, venue, remove) answer before their change is made (F35), as the three TEST
       routes do (F23), and GET
       /api/sync never waits for the message thread (F24). A step such as (h)'s "then POST /api/sync/venue ->
       targetMs 42", read at once, could fail on a correct tree; an "unchanged" compared with a state read too early
       could pass on a wrong one. In place: R11 WAITS.
  V-5  The control can carry the text. (k) is itself a load of a keyed file. A text EVERY keyed load shows, or one
       that stays on screen once shown, is in S(k) as well, and S(b) = S(c) = S(d) = S(k) holds. In place: S(0), a
       reading before any load, and (k)'s "no string occurs more often in S(k) than in S(0)"; in G7 the same
       against C3 and C8. The direction (nothing MORE than before) is chosen so that a load that empties the
       inspectors does not fail it (K14).
  V-6  Round 2's K12 said every notice the withdrawn texts describe names "sync" or the venue. The panel's "was +55
       before this composition was opened" names neither. K12 is re-written.
  V-7  An adoption written as two controller calls -- createVenue, then setMs -- turns the full store into the
       runner-up of 8.3 by accident and wipes its own trace (the accepted setMs clears lastError). D14 now says ONE
       `apply` op; unit case (1), stub B, is that build.

8.5 For Harmony
 Additions of round 3 that H-3 has not seen. Each adds a way to fail; none removes one (K4).
  R11:  the LOADS sentence; "P, Z and A do not exist"; the wait after a sync POST; the named add / remove routes;
        S(0); (k)'s two clauses; (j); the LIVE RED line; the header's empty scratch folder and wider names rule.
  G7:   "the loads of C13 and of C13k answer "ok": true"; N for C13 / C13k and for C11; the C3 / C8 clause; the modal
        bar in H-5's words; one logic question, one interaction-logic question; the greyed-control sentence.
  G1:   three unit cases in test_sync_offset_controller; "a refused adoption included" in test_sync_panel.
  Page: one said-line (the full list).
 Changed inside text H-3 admitted: the TEXTS paragraph's cut -- the fixture's name instead of "the name of the file
 just loaded" (R2: SHOULD-4).
 Questions, each with the default this file is written to:
  Q-H1 Does Boris's "the only fail message will be a failed save" cover a failed save of the venue list (K17)?
       DEFAULT: no -- its reason stays in lastError, off screen.
  Q-H2 Does the full-list line go on the Boris page? DEFAULT: yes, one line, no check.
  Q-H3 Is R11's LIVE RED line worth a scratch build (K16)? DEFAULT: yes, once, alongside R7's.
  Q-H4 HR:10's stamp (NIT-3): mend it to 20:59:50?
 A method worth keeping (V-5): a "no text" gate that compares with a control needs one reading taken before any
 event of that kind -- the control can carry the text.

STATUS: DONE -- ruling-bf2-gates-restated, round 3 (final fold), 2026-10-03; rows R11 (a)-(k) + LIVE RED, G7 (states incl. C13k, bars, five seats), G1 S5a / S5b; deltas D13, D14, D15, D16, D20 (+ D21, D22, stage row S5a, plan B6); replacedByLoad DROPPED; a full venue store REFUSES the adoption whole; H-2..H-7 folded (K2, K4, K10 closed; K5 half; K8 ruled); pinned to c45b579; review R2: 5 SHOULD + 5 NIT accepted as findings (3 fixes re-shaped, one half-claim of NIT-5 rejected); 1 Boris question with a default; 4 questions for Harmony.
