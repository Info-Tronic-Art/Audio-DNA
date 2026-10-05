# Check of boris-all-questions.html against boris-all-items.json (re-check, written 2026-10-05 17:40:09, stamp from date)

VERDICT: SOUND. Nothing wrong found; the page was NOT edited; the items file and filing record were NOT touched. This replaces my 16:37 check, which was of an older page: the items file (17:32) and the page (17:35) were rewritten since (45 questions 172-216 and 102 readings R127-R228 now, were 44 and 87), so everything was re-run on the 17:35 page. Both nits of the 16:37 check are gone from the new text (no "architects"; the intro now explains the "+ mine" tags).

## (1) Item for item -- VERIFIED (python3: json load, html.parser, entity un-escape; scripts in the session scratchpad)
- VERIFIED 629 item texts tested as exact substrings of the page's visible text, no whitespace normalising: 0 misses. Those are: 102 readings, 45 "Changed:" lines (only the revised readings), 45 situations, 45 "why it matters" lines, 109 option texts, 45 default reasons, 65 his-words blocks, 36 decided lines, 35 design-page items with all variants, 4 chat blocks and their "what" lines, the chat note, the defaults line, 14 intro paragraphs, topic and part titles.
- VERIFIED structure, field by field (0 mismatches): every reading (id, about line, text, "Changed:" line exactly on the revised ones, LOOK FIRST tag exactly on the 26 starred, picture, right topic section, right part); every question (number, title from question_titles, situation, his-words blocks in order, each option letter and text, exactly one DEFAULT mark and badge per question on the flagged option with its why line, "Why it matters", picture, right topic and part); decided lines (36) under their topic in order; design items (35) with variants under their topic; 4 chat blocks; defaults line.
- VERIFIED reverse test: with all item texts removed, what is left on the page is only headings, labels (R numbers, about lines, DEFAULT, Your words, Why it matters, question titles), index counts and links. Nothing on the page is missing from the items file.
- VERIFIED the derived numbers: "45 questions and 102 readings", parts 26/16/3 questions and 70/24/8 readings, the 26 look-first readings in the intro = the 26 starred, each topic's question list and reading list in the index and in "Where each ... is", the copy line 172 a ... 216 a (every default is A, so the line is all defaults), the "questions that hold up each build" line.
- VERIFIED the same texts are all found in boris-clarify-all.md (whitespace-normalised): 0 misses.
- NOTE the question "about" lines (e.g. "R111 and the show start") and the decided "by" lines are in the items file but not on the page. They are internal labels (R numbers, file names); not showing them is correct for plain words.

## (2) His words -- VERIFIED
- VERIFIED all 65 his-words blocks (45 questions) are verbatim in binding-decisions.md, boris-feedback-backlog.md or boris-msg-raw-1/2/3.txt (whitespace and curly-quote normalised for line wraps); none is found only after Harmony's "->" text.
- VERIFIED 375 double-quoted phrases in the page texts were tested (per text block, no odd quote counts): 203 are in his files. The other 172 are NOT his words: app labels (menu entries, button names, "Export Bindings...", "Open Image"), answer examples ("R193 c no", "203 a, but keep Overlay and Difference"), Harmony's own labels ("Stays as it is", "Mine:"), Resolume manual quotes (3, found in facts-resolume-emulate.md / seat2-facts-HIJK.md), sample names ("Club A"), and "test with harry" (his show, named in HANDOFF.md:209).
- VERIFIED the unquoted attributions I spot-checked: "no bigger envelope window" (binding-decisions.md:643), "the bpm they were recorded with need to be saved" (:961), the sync dial 1 ms steps (:578), Render asked in September (:220-224), "R123 yes" (:1097), "R114 ... that pause is saved" (:1094), "ctrl-z to get it back" (:767).
- INFERRED (not all checked one by one): the ~35 unquoted "as you said / your words / you asked" phrases; the 8 I read all hold; the rest read as paraphrases of his blocks.

## (3) Numbers -- VERIFIED
- VERIFIED questions on the page = 172..216, 45, no gap, no duplicate (page order is by topic; the intro says so). Equal to first_question 172 and last_question 216 of the items file.
- VERIFIED readings on the page = R127..R228, 102, no gap, no duplicate; last_reading 228.
- VERIFIED earlier filing records (20 files in s-rta-1004, 3 in s-rta-1004b, plus their json and html): no question number 172-216 is used as a question there (150-plus ends at 171: "Next free question: 172. Next free reading: R127"). R127+ appear there only as Harmony's planned numbers ("NOT yet told to him"); R127 there is the Save button, the same as the page's R127.
- VERIFIED readings below R127 mentioned on the page (R76, R109-R117, R119-R126): none is restated with a different text; the restatements in R147, R143, R198, R171 agree with boris-open-items.json (1004b). R114, R116, R120, R121 are marked on the page as chat answers "not the text of this reading as first shown".
- VERIFIED the first-pass page of this session (boris-open2.html) used the same numbers differently, but the items file says "not yet shown to him". UNKNOWN beyond that; cheapest check: ask Harmony whether boris-open2.html was ever opened for him.

## (4) Pictures -- VERIFIED
- VERIFIED 15 img elements, every src (and its enclosing link) resolves to an existing file: 4 under boris-images/ (8 uses), 3 under ../s-rta-1004b/boris-images/ (7 uses). The picture field of every reading and question equals the page's img. No dangling in-page anchors, no duplicate ids.
- VERIFIED I looked at the 4 pictures under boris-images/: they match their alt texts (Resolume monitors, monitor menu, effects list with Blue/Green/Red, Arena Presets menu).
- INFERRED: the 3 older pictures come from the 1004b filing and I did not open them.

## (5) Plain words -- VERIFIED by regex over the visible text
- VERIFIED none of: "delta", "ruling", "lane", "quantize-out", "packet", stage ids (S1, SM-, G0), BF/BD ids, s-rta, file names or extensions, code identifiers, "architect", "seat", "sweep", "fact sheet", "EOS", "Pitfall", threads, GL words.
- VERIFIED the mapping is named "keyboard and MIDI mapping" everywhere; never "key / pad list"; "bindings", "shortcuts", "MIDI learn" occur only in R148 (and R199, R197 i) as today's app labels, said to go.
- LEFT (nit, not a defect, text is the items file's) "ruled" occurs 8 times ("as you ruled", "you ruled: build."): the noun "ruling" is not used. It describes his own decisions; fine for him. Cheapest fix if wanted: "as you decided".
- LEFT (nit) "(read in the code)" / "(read in the code, not run)" occurs 17 times, "the code" being the app's source. It is not a code name, but it is programmer talk; the label does tell him a line is not tested. Cheapest fix: "(from the app's workings, not run)".
- NOTE "Harmony's chat answer" labels (4) use the name he knows.

## (6) HTML -- VERIFIED
- VERIFIED parses with python3 html.parser: no unclosed or mismatched tag; 2890 "<" and 2890 ">"; starts <!doctype html>; no <script>, iframe, link, form, object, no on* attribute, no http(s) URL, no url(), no @import; the only hrefs are in-page anchors and the 7 local picture files (target=_blank, rel=noopener); entities are only #x27, amp, gt, lt, middot, nbsp (all valid).

## What I changed
Nothing. No mechanical defect of (1), (2), (4) or (6).
