# Adversarial check of boris-open.html (session s-rta-1004b, 2026-10-04)
Verdict: UNRELIABLE (no mechanical defect; four substantive defects left, none edited). The page and the JSON were NOT changed by this check.
Labels: VERIFIED = I ran or read it (file:line / script); INFERRED = my reasoning. Scripts: scratchpad chk1..chk7.py (read-only).

## (1) Machine check -- PASS (VERIFIED, chk1.py/chk7.py)
- 20 questions on the page (150,151,154..171), numbers unique; 17 readings on the page (R109..R117, R119..R126), unique. JSON has the same 20 / 17.
- 101 strings (every question text, option text, "why", every reading text) appear on the page character for character (whitespace collapsed, tags stripped, entities unescaped) AND in boris-clarify-150-plus.md. 0 missing.
- All 20 "source" lines are shown on the page. The 4 void readings are in the filing record only, not on the page (by design; R118 is simply absent, no explanation on the page: cosmetic).
- Told / checks / request texts are all on the page too.

## (2) Questions 150 and 151 -- PASS (VERIFIED, chk2.py)
JSON verbatim_block == ruling-looks-answers2.md lines 865-872 (collapsed); both question texts and all four option texts are substrings of the page. Only difference: the ruling's "A (default)" is shown as an "A" with a DEFAULT tag.
Readings R109..R120 == ruling-nudge-row2.md:833-852 (11 texts, R118 void); R121 == ruling-looks-answers2.md:852-855 (chk3.py). All true.

## (3) Numbers -- PASS (VERIFIED)
No question 150..171 in any earlier boris-clarify file; boris-clarify-144-147.md:2 reserves 150-153 and "next free 154" (152 withdrawn, 153 unused: consistent). Earlier files hold readings only up to R108 (135-143.md:114); R109..R120 are named in 144-147.md:2 as the planned renumbering of the ruling's R103..R114 (same texts); R121..R126 occur nowhere else under .harmony.

## (4) His words -- PASS (VERIFIED, chk4.py)
Every phrase in quotes attributed to him is in binding-decisions.md or the backlog: "after I let go it catches up" (BD:1017), "ignore actions" (BD:1056), "I think the simpler solution is that we could just draw rather than drag" (BD:397-401, section 2026-09-05), "134 delete them too" (BD:996), "144 a"/"147 a"/"145 b"/"146 b", "139 a", the BL:389 sentence. The quoted strings not in his words are UI text or file names (e.g. "Search presets...", "test with harry.v0.json") and Q154's "the clip's Snap goes", which the page itself says is Harmony's reading. No mechanical fix needed.

## (5) Already answered -- two findings
D1 (Q158, glide or jump when he lets go of a slider): NEARLY SETTLED by his own earlier words. BD:254-256 ruling 7 "HAND-BACK GLIDES, IT DOES NOT SNAP", his words "smooth transition back" ("snap-to is not the behaviour he wants"); BD:340-347 has his other words "will snap back to the recorded track as soon as it is let go" (Harmony read both as glide). Q158 quotes only "catches up" and does not tell him either earlier sentence or that a glide default already exists. Either drop it or show him both sentences.
D2 (Q154 header contradiction): the page says "Every question has a default: if you say nothing, A is built." But ruling-nudge-row2.md:873-876 (HB-5) says the clip Snap fields are KEPT until he confirms in words, and the page's own source line for Q154 says "the Snap values are kept until he confirms in words". If he stays silent on 154, A is NOT built. Q154 must say it waits for an answer, or the header must except it.
Minor, not counted: Q150 B ("shows Presets, dim, after any change") sits against his 133 "no need to show that it was changed" (BD:988ish); Harmony's ruling asks it on purpose (RL2:884-886). Q163 option B (a layer action moves a clip's sliders) contradicts his rule that clip sliders belong to clip actions (BD:1044-1046): an option he already ruled out. Q167: his September "build." (BD:258) and ruling 23 already say the draw editor is wanted; the question is only first-build scope (the text says so).
Not settled (checked): 155, 156, 157, 159, 160, 161, 162, 164, 165, 166, 168, 169, 170, 171, 151.

## (6) Plain words -- mostly PASS
Visible question text has no code, file names or stage ids. The collapsed "source" lines under every question DO show file names and shorthand (ruling-looks-answers2.md, BD:1049-1053, C135, FA:312, open-sweep-actions.md item 2, RL2, RN2). They are hidden under a "source" fold; INFERRED acceptable but he cannot decode "BD", "C135", "FA". Q157 option C ("joined in its middle") is the least plain wording.

## (7) "What is in your app now" vs rulings-one-save.md gate block
D3 (line "Decided and filed today, not in the app: ... the first hidden parts of the per-screen Delay and of the nudge are built and tested but not switched in"): those parts live ONLY on unmerged branches lane/nudge (commit 3ba5ca1) and lane/outputs-core (aa7bc8e) (git branch --contains; main has one merge today, 4627c7f). "Not switched in" reads as if they are inside the app he opens. Say "built on a side branch, not in the app you open".
D4 (first line, "The app you open next keeps the show as it was when a Save cannot be finished"): the full-disk row OS-L21 passed on the LANE app at the merged code head; rulings-one-save.md (MERGE 1 block) says "NOT RE-RUN on merged main: OS-L21 ... owed again at M2". Tag VERIFIED is a little strong for the merged binary: INFERRED for main's build.
Other lines verified: backups folder name and v0 copy and 49,367 bytes (RS:25, RS:27); one Save only copies once (tests/test_show_backup.cpp:114-125, a unit test, VERIFIED); "Save failed" box text (src/MainComponent.cpp:3619-3620); Deck menu unprotected (RS:13 H-S8, HANDOFF.md:96-99); R126 (his show file holds routines [] and routineBank [], read by me); deck fade 0.3 = "globalTransitionSpeed" (ShowMigration.h:166). One omission: the first Save also drops "outputDisplay" from his show (RS:27); his value is -1 so nothing is lost; not mentioned on the page.

## What I changed
Nothing. No mechanical defect of (1), (2) or (4) was found.
