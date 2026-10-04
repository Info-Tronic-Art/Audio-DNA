# Reviewer Verdict — bf2-gates-restated-r1
STATUS: PARTIAL
VERDICT: REQUEST_CHANGES (structured verdict: FAIL, one MUST)
FILES: .harmony/.reports/s-rta-1003b/ruling-bf2-gates-restated.md (D = this doc line numbers)
METADATA: reviewer=claude-sonnet-5-5, round=1, date=2026-10-03, trees read via git show (main 34179a2, lane 4a1f240)

## Checked and clean (VERIFIED)
- (v) Every Boris quote (F1-F5, adoption items 6/7) equals binding-decisions.md BD:696-697, 698-700, 704-707, 708-711, 695 character for character; programmatic substring check of all quoted strings found no drift. PD/RD/BD line cites spot-checked and correct (one 1-2 line slip, below).
- (i) No clause still expects an on-screen event text. R11 (b)-(d), G7 C11/C13, G1 and the page 6.6 all expect absence. The "Loaded: <name>" file label (D:272-274) is acknowledged as pre-existing, not expected.
- (vii) No bar is looser than the original; (a3) flips to the key, (c)/(d)/(b) flip to absence, seats go 3 -> 5, additions (h), A, Z, tooltip only tighten.
- Facts F7-F13, F15, F16, F21 re-derived from main/lane: correct (lint at test_render_thread_lint.cpp:450-451; saveToFile x3 at M 3551/3574/6694; no save_composition route on lane; controller clears lastError on success, SyncOffsetController.cpp:76-80).

## Findings
### MUST-1 (ii) R11 (g) passes vacuously (D:167-168, D:199-202) -- VERIFIED handlers / INFERRED race
handleDebugLoadDeck and handleDebugDuplicateDeck answer ok right after juce::MessageManager::callAsync (M:src/api/ApiServer.cpp:2120-2132, 2135-2150); the staged load finishes later (setFileLabel(done) at finishStagedLoad, M:src/MainComponent.cpp:3360, 3398). The row says only "POST ... then POST ... -> GET /api/sync unchanged after each". Tree on which it passes: the mutant the doc itself names (adopt hook in finishStagedLoad's shared tail, so Load Deck / Duplicate adopt) with GET /api/sync read between the POST's 200 and finishStagedLoad -- it reads 55, not 42. D:199-202 says the live mutant "reads targetMs 42 after the Load Deck", but the row text, which Harmony copies as the probe spec, has no sync point. (b)(c)(d)(e) are safe: /api/load_composition waits on a ticket (M:ApiServer.cpp:1153-1154).
Fix: add "the read waits until GET /api/debug/ui_text file_label equals 'Loaded deck: <name>' / 'Duplicated deck: <name>'" (stagedload::doneLabel, M:src/core/StagedLoad.h:60-68).

### SHOULD
- S-1 (ii) R11 no-text clauses can pass on ui_text {"ok":false,...} (D:158-164, 313-315). handleDebugUiText answers ok:false with only "reason" when the message thread is slow (M:ApiServer.cpp:2091-2097); "no string holds Sync or Warehouse" is then true. Fix: add "ok is true" to every ui_text clause. VERIFIED.
- S-2 (ii) The no-text detector sees five strings only (D:513-518, F9). A notice painted in any other Label passes (c)/(d), and (d)'s dump clause only forbids the property NAME "replacedByLoad". The B4j lint forbids old names only; a new name passes. S5a has no G7, so nothing but R11 guards S5a. Fix: have ui_text/the dump enumerate all visible Label/Button texts, or require a before/after crop diff of the whole window in R11 (b)-(d). VERIFIED gap, doc concedes it (K3).
- S-3 (ii) G7 "modalComponents is 0 with the panel open" (D:226) and the interaction-logic sentence "in every capture with the panel open" (D:253-255) cannot pass in C9: the venue menu is a PopupMenu and JUCE enters modal state for it (juce_PopupMenu.cpp:2162 enterModalState). INFERRED that the dump counts it. A bar nobody can pass invites a waiver or a loosening. Fix: exclude C9 from the bar, or count only non-menu modal components.
- S-4 (iii)/(vi) Page 6.6 says "Nothing is written anywhere about the change" (D:457-458) but D14 NEW (D:340-342) writes an app-log line "Sync: Warehouse +55 -> Warehouse +42 (...)". The page overclaims against the delta. The log line itself follows from neither Boris's words nor adoption item 7 (precedent: migrationNote, M:MainComponent.cpp:3355-3357); K7 admits it has no gate. Fix: either drop the log line (and the "old number goes to the app log" in D15, D:352-355, 413-414) or say "nothing on screen" on the page.
- S-5 (iii) The reassurance on the page (D:480-482) "The room's own venue keeps its number -- pick it again" and section 4 reasons 4-5 (D:410-414), K1 (D:505-506) assume the load selects ANOTHER venue. A user who never named the room has only "Default" (F16, SyncVenues.h:32); a show saved at home on Default 0 then hits the SAME-venue case: the room's Default +55 is replaced and the old number goes nowhere. 6.11 tests only "Test room". INFERRED from the adopt rule. Fix: say so on the page, and test (h) on a store whose only venue is Default at +55.

### NIT
- N-1 D:43-44 (WHAT CHANGED 3) cites "(F6)" for the B4j lint; the fact is F7. VERIFIED.
- N-2 D:391 / D:363 cite PD:390 for the SYNC tooltip; it is PD:388-389. VERIFIED.
- N-3 D:36-39 says R11 (a),(a2) "keep their words" but (a) gains the copy-A sentence (D:154-155); 6.7 (D:462-463) was rewritten ("Is that enough?" removed, a Wrong: clause added) and is not listed under "What changed". VERIFIED.
- N-4 D16 NEW (D:359-361) says no example of no-focus-on-click exists; ClipInspector.cpp:445 `revealBtn_.setWantsKeyboardFocus(false)` and OutputWindow.cpp:65 are in-repo idioms (the claim is scoped to two files, so true, but the builder could copy a real one). VERIFIED.
- N-5 D:272-274 lets a critic who reports "Loaded: <name>" be "right" while D:233-235 voids event-text requests; state in one place that a present pre-existing event text is recorded, not voided. Boris's APP-WIDE ruling (BD:708-711) removes it, so that lane must merge before or with bf2 for the page to be true.
- N-6 Q1 (D:492-494) re-offers an event-triggered control after Boris's BD:698-700; default is "no", so acceptable, but expect irritation.
- N-7 DROP of replacedByLoad rests on adoption item 7's "dropped if it needs a text" plus INFERRED reason 2 (D:404-406); Boris's words remove the displays, not the memory (the doc says so, D:181-182). Authority is adequate; recorded as a judgment call.

## SLIM
No code in scope; nothing to slim. The restated rows remove EXCESS_SPEC (replacedByLoad, forLoad combinations).

## Confidence
VERIFIED: everything labelled so above (read at source). INFERRED: S-3, S-5, the (g) race timing. Not run: nothing was built or launched.
