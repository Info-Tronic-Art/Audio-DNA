# Check of boris-open2.html against boris-open2-items.json (written 2026-10-05, from date: see end)

VERDICT: UNRELIABLE by the letter (one plain-words defect left, not mine to edit); the content is exact. Page not edited. Items file and filing record not touched.

## (1) Item for item -- VERIFIED (python3: json load, tags stripped, html un-escaped, substring test, plus a structural parse of every block)
- VERIFIED 414 texts tested as substrings of the page text (intro 9, chat note, chat blocks 4, readings 47 x text/id/about/why_revised, questions 17 x number/about/situation/options/why-default/why/his words, decided 14, design items 20 + 54 variants, defaults line): 0 misses.
- VERIFIED structural parse, exact equal and same order as the items file: 47 readings (topic group, id, tag NEW/REVISED/AS FILED, "about" line, text, "Changed:" line only on the 20 revised ones); 17 questions (number, about, situation, every option letter and text, exactly one DEFAULT mark per question and it is on the option the items file marks, "Why this is the default" only on the default option, "Why it matters", YOUR WORDS blocks); 14 decided lines in the right topic; 20 design items with their variants; 4 chat blocks and their note; the defaults line; the 9 intro boxes.
- VERIFIED residual test (all item texts removed from the page): what is left is only headings, the labels (about, NEW, REVISED, Changed:, DEFAULT, Why this is the default:, Why it matters:, YOUR WORDS, Mine:, click: full size, the topic letters), the date line and the question numbers. Nothing on the page is outside the items file. The Harmony-side fields (conflicts, seeds, not_established, arena_checks, still_unsure, notes_for_harmony, draft_to_final, label, source, by) are NOT on the page, as the items file's own note says.
- VERIFIED the same 282 texts are also found in boris-clarify-172-plus.md (whitespace-normalised): 0 misses.

## (2) His words -- VERIFIED
- VERIFIED 153 quoted strings in the items texts tested against binding-decisions.md, boris-feedback-backlog.md and boris-msg-raw-1/2/3.txt (case-sensitive too). Every phrase the page attributes to him is verbatim, including all 31 YOUR WORDS blocks (172-188) and the quotes in R130, R133, R140, R146, R157, R160, R172, the defaults line, the intro and the build-hold lines. The strings not found are all not his: instructions to him ("172 a", "R130 off", "readings ok"), labels ("Mine:"), today's app texts, new names (Stop actions, Audio play / pause), Resolume's manual and menu texts, example names and option labels.
- VERIFIED today's menu texts in R148 are the app's own (src/ui/MenuBarModel.cpp:10, 155-160; src/ui/BindingOverlay.cpp:68; src/ui/MidiLearnOverlay.cpp:95; src/MainComponent.cpp:7342).
- VERIFIED the "your NNN" answer references checked exist in binding-decisions.md: 122 a (l.944), 63 (916), 101 (925), 102 b (926), 103 (928), 104 (931), 132 (993), 133 (994), 135 (1007), 136 b (1009), 140 a / 141 a / 142 a / 143 a (1017-1018), plus the 2026-10-05 section (l.1077-1101). "your words of September" = the 2026-09-05 section (l.401: "just draw rather than drag...").
- INFERRED (small): question 187 says "your words of September and 167 say you draw, and move pieces": 167 A is an accepted default, not his typed words (accepted by "All defaults good except for these.").

## (3) Numbers -- VERIFIED
- VERIFIED questions 172..188 run without a gap (17, matches first_question/last_question). None of 172-188 appears as a question number in s-rta-1004/boris-clarify-*.md or s-rta-1004b/boris-clarify-*.md; the last earlier record says "first free question 172" (boris-clarify-150-plus.md:2, :129).
- VERIFIED readings on the page: R127..R173 all present, none missing, none twice (47). Every R-number the text refers to exists on the page or is an earlier reading of the last page (R109-R113, R115, R117, R119, R122-R126; the standing ones are written out in R147 and agree with boris-clarify-150-plus.md lines 90-106).
- NOTE (needs your look, I do not call it a defect): the chat block "What I told you in chat" carries R114, R116, R120, R121, numbers below R127, with texts that differ from the same numbers in boris-clarify-150-plus.md (lines 95, 97, 100, 101). It is labelled as what he was told in chat, brought up to date, and "nothing here is asked again"; the earlier R121 (a preset renamed or deleted that several effects carry) is not in the R121 block but is carried by R155 / R171 (INFERRED, not checked line by line).

## (4) Pictures -- VERIFIED
- VERIFIED 9 img tags (5 files; the 5 links point to the same 5): all exist relative to s-rta-1005/ (4 in boris-images/, review-screen-reference.png in ../s-rta-1004b/boris-images/).
- VERIFIED by looking: the monitors picture and the monitor-menu picture show "Composition Monitor" / "Clip Monitor" and the green dot on "Selected Clip" (R153); the presets-menu picture shows P with arrow between B and X, Default / Blue / Green / Red, Manage..., Save, and the second P on the Blend Mode row (R136, R156); the effects-tab picture shows the fold arrow and the indented presets (R154).

## (5) Plain words
- DEFECT (left, report only): the header line reads "2026-10-05 15:23 . session s-rta-1005": a session id on a page he reads.
- MINOR (left): the five picture alt texts are file names ("your picture: resolume-monitors.jpg"); they show only if a picture fails to load.
- MINOR (left): R121 says "it was ruled twice on paper"; "ruled" is process wording (the flagged word "ruling" itself does not occur).
- VERIFIED none of: delta, ruling, lane, quantize-out, packet, stage, file names (.md/.json etc.), code names, BD:/BF/HB- ids, in any visible text.
- VERIFIED the keyboard and MIDI mapping is called "keyboard and MIDI mapping" everywhere (R120 block, R148, 166, design item, intro to the decisions). "shortcuts", "bindings" and "MIDI learn" occur only in R148 as the old names that go, inside today's menu texts, and in his own quoted sentence (185: "computer shortcuts"). "the key / pad list" does not occur.

## (6) HTML -- VERIFIED
- VERIFIED parses with python3 html.parser, no unclosed or mismatched tags; no script, link, iframe, form, event handler, @import, url() or http(s) text; the only hrefs are the 5 local picture links.

## What I changed
- Nothing. No mechanical defect of (1), (2), (4) or (6) was found.
Mon Oct  5 15:28:35 EDT 2026
