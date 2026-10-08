# PCHECK COVER D -- was the triage right for topic D (actions in a show)

Written: 2026-10-08 00:58:51 EDT
Read: boris-msg-numbered.txt (all), assume-all.md topic D (D-1..D-31, 30 blocks; D-12 was dropped and folded into D-3, see rule-D.md), page2-items.md (the @@TRIAGE D-* blocks, page items 224-228, 235, 255-257, the answer R129-4), slice-D.md (the page text he answered: questions 179, 180, 182, 183; readings R131, R133, R158, R172, R129). Nothing built, run, launched or committed.

## MY OWN TRIAGE
(id | mine | the ruling's | agree?)
- D-1 | ASK (four names, one slider or two; starts look different on stage) | ASK 224 | agree (text gap: see F2)
- D-2 | ASK (L7 "ignore actions" vs L73 "global action bypass": reach differs on stage) | ASK 225 | agree
- D-3 | ASK (L72 against the default taken in L1 for 180) | ASK 226 | agree; D-12 properly folded in (226 carries "switches every action off" and the hold reading as C)
- D-4 | ASK (L64 gives play once, not the button) | ASK 227 | agree
- D-5 | ASK (he asked, L66) | ASK 228 | agree
- D-6 | LINE (L68 "that specific action is muted" most naturally = the whole pasted action) | LINE 255 | agree
- D-7 | ASK (deletion of his files, L69) | MERGED 235 | agree (235 carries scope, Trash, list first, B, C)
- D-8 | LINE or SETTLED (L73 answers 179, whose option A had the downside "holds back layers 3 and 4"; a bypass only makes sense with A) | SETTLED L73 | agree
- D-9 | LINE (R181 g was shown and not corrected, but L73 adds a switch) | LINE 256 | agree
- D-10 | SETTLED (182 b is his letter, L74: "the cells ... in the deck that is shown ... other pictures"; L11/L19 for empty cell and the 1) | SETTLED | agree
- D-11 | LINE (stage-visible, a real choice; no word of his is about lighting a lamp mid-action) | SETTLED L67 | DISAGREE (F1)
- D-13 | INTERNAL (recorded stays as recorded; nobody asked for smoothing a seam) | INTERNAL | agree
- D-14 | INTERNAL (R158/R172 as shown say "on the 1" and were not corrected; L70 only adds a fade) | INTERNAL | agree
- D-15 | INTERNAL (L64 "Actions will loop") | INTERNAL | agree
- D-16 | SETTLED (L65 "show a warning") | SETTLED | agree
- D-17 | INTERNAL | INTERNAL | agree
- D-18 | INTERNAL (follows 255) | INTERNAL | agree
- D-19 | INTERNAL (R125 stands, uncorrected, L1/L70) | INTERNAL | agree
- D-20 | INTERNAL | INTERNAL | agree
- D-21 | INTERNAL | INTERNAL | agree
- D-22 | INTERNAL (logic) | INTERNAL | agree
- D-23 | LINE (a mixed pick of global-tab rows and layer rows: one global action or two; L55 says the app "breaks them down") | INTERNAL | DISAGREE (F2, SHOULD)
- D-24 | INTERNAL (rare edge; the visible half is in D-10) | INTERNAL | agree
- D-25 | INTERNAL | INTERNAL | agree
- D-26 | LINE | LINE 257 | agree
- D-27 | INTERNAL (L70: nothing jumps) | INTERNAL | agree
- D-28 | INTERNAL (logic) | INTERNAL | agree
- D-29 | INTERNAL | INTERNAL | agree
- D-30 | INTERNAL (look, L8) | INTERNAL | agree
- D-31 | INTERNAL | INTERNAL | agree

## FINDINGS

### F1. MUST | D-11, no page item (TRIAGE says SETTLED)
What is wrong: the TRIAGE cites L67 ("R131 again, whenever an action is switched off and it goes back, it follows the master action fade back time"). L67 is about an action SWITCHED OFF and the TIME of its going back. D-11 is a different event: his own ignore lamp lit while the action is still on and moving the control. The assumption itself says "whether lighting the lamp counts as that is not said". The page R131 (b) he read said the opposite ("it stays where you put it"), and ALT b (the value stays where the action had it, so he can catch a value the action reached) is a real performer's use. A wrong guess is seen on stage (where one slider rests when he takes it from an action) and it is about what the app does. Nothing of his settles it, so it cannot be SETTLED.
Fix (exact): change the TRIAGE to
@@TRIAGE D-11
TO: <new LINE item number, after 284>
WHY: L67 is about an action switched off; lighting the lamp under a running action is not said.
@@END
and add the page item
@@PAGE-ITEM 285
TOPIC: D
KIND: LINE
TEXT: I assume that when you light a control's ignore lamp while an action is moving it, the control glides back to where it was before the action (over the Global glide time), and from then on it stays where you put it.
B: It stays where the action has it at that moment, so you can catch a value the action has reached.
C: none
FROM: D-11, R131
@@END

### F2. SHOULD | D-23, no page item (TRIAGE says INTERNAL)
What is wrong: D-23 assumes global-tab rows ticked together with layer rows become ONE global action. His words on mixed picks are L55: "After use picks rows of more than 1 kind, the app breaks them down into clean, concise actions that can be saved in the action save window". The split rule he accepted (R133 b and d) puts layer-tab rows in a layer action and global-tab rows in a global action; whether the app then merges them into one global action is not said, and the number of action buttons on stage differs (one against two). The ruling's reason, "he sees and can change the result in the window", is not carried by the assumption: its window only names an action and leaves one out; it does not split or merge. A choice of Harmony's that he would most likely wave through: a LINE, not internal.
Fix (exact): TRIAGE D-23 TO: <new LINE item number>, and the page item
TEXT: I assume that rows of the global tab ticked together with rows of a layer become ONE global action (a global action may hold layer sliders), and the save window lists it so you can name it or leave it out.
B: They become separate actions: one global action for the global-tab rows and one layer action for each layer's rows.
C: none
FROM: D-23, R133

### F3. SHOULD | D-1, page item 224 (text)
What is wrong: the assumption D-1 lists SIX glides, the last being "a global action taking over". Item 224's TEXT lists five and drops that one, although its FROM carries 183, the transition slider that times exactly that. He said "We will just keep the Global glide slider" (L64): "keep" says this is the slider that already exists, which is 183's. A yes to 224 as worded leaves the takeover glide unsaid.
Fix (exact): in item 224 TEXT replace "its start, its switching off, going back after playing once, your hand letting go, and a Resync." with "its start, its switching off, going back after playing once, your hand letting go, a Resync, and a global action taking over from a layer's action (the slider you have already)."

## COUNTS
Assumptions re-derived: 30 (D-1 to D-31 without D-12, which has no block: dropped and folded into D-3 on purpose, rule-D.md line 314; its point is carried by items 226 B/C).
Agree with the ruling: 28 (D-1 through D-10, D-13 through D-22, D-24 through D-31 as listed above; D-1 agrees on the triage, with the text fix F3). Disagree: 2 (D-11, D-23).
Findings: 1 MUST (F1), 2 SHOULD (F2, F3).
Page items checked: 224, 225, 226, 227, 228, 235 (merge of D-7), 255, 256, 257. 225 to 228, 235, 255 to 257 carry their assumptions and their B and C are the real other ways. No assumption without a TRIAGE block. No item that should be dropped.
