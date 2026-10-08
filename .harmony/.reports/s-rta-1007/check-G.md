# CHECK G -- blind re-check of apply-G.md
Written 2026-10-07 22:56:45 by the checker seat. Read-only; nothing built or run.

## MY OWN READING OF HIS LINES (written before opening the paper)
- L96 (R191): the show remembers which outputs are connected; opening it with them connected brings them back with no re-connecting. Changes part (g) of R191 (the old "app never opens an output by itself") only. Says nothing about the settings of the outputs, and nothing about Syphon by name.
- L94 (topic F, lands here by one edge): the app opens to the very last show. With L96 it implies outputs can come on at launch.
- L1: defaults of 199 (A) and 200 (A) are taken; "today" must not be in what he reads; ask focused questions on assumptions.
- L8: new functions laid out by Harmony where they fit; P27 (where settings open) is a look/place question, dropped.
- L9: earlier explanations answer repeats: for 199 his earlier "every show remembers it's sync" (BD:696) and "syphon is an output and treated with same output settings as a screen" (BD:834) are on file.
- L88 (R186 E): recording is the composition, not the screen output, because screens are tuned to a projector's colour and timing. Confirms R191(e): recording never has a screen's Delay/colour. It does NOT say where a Delay is stored.
- L55 (R133): "composition and global are interchangeable but lets move to global": the word for the tab. Q210 default A (L1) says the same for the tab.
- L35 (R170): "the preview window under the output window" is the in-app preview monitor; it shows cue or a previewed clip. Touches R191(e) (what the in-app monitors show) and the word "output window".
- L79 (R138), L91/L92 (186 b, 195): review picture to a monitor / to outputs: topic E, only a consistency edge for R209.
- L117 (R197): one list of names for everything: every NAME block is a candidate for that list.
- Nothing else of his message lands on 199, 200, R209, R226, D25-D28.

## FINDINGS
- MUST | D28 | STATUS names "the different projector, the twins, hold and skip" as Harmony's own, but only the first two are carried by an @@ASSUME (G-12). "Raising a screen's Delay holds its picture still for the added time; lowering it skips ahead; never goes black" has NO @@ASSUME. It is what the audience sees when he moves a Delay during a show; no word of his settles it (L130). Fix: add @@ASSUME G-13 ABOUT D28: "When you raise a screen's Delay its picture holds still for the added time; when you lower it, it skips ahead; it never goes black." ALT: b) the screen goes black briefly / the change applies only on release. IF-WRONG: STAGE. ASK: LINE (or YES: seen on stage, about what the app does).
- SHOULD | G-8 (D26) | IF-WRONG is STAGE (the audience sees a half-second flash) and no word of his settles it, yet ASK is LINE. By the three tests (not settled; seen on stage; about what the app does) this is ASK: YES. Also G-8's TEXT leaves out the second half of the RULE (opening a show of another size while recording asks first, R187 b). Fix: ASK: YES, TEXT: "...With screens on, the size can be changed at any time and they may flash for about half a second. Opening a show of another size while recording asks first."
- SHOULD | 199 | "Tested against his whole message" skips his earlier words on file: BD:696 "every show remembers it's sync" (-> a show carries its sync value) leans toward option C; BD:797 says it is superseded only "as far as it names the dial" and leaves where a Delay is kept OPEN. The paper cites instead "L88 leans the same way", but L88 ("screens can be modified to fit a projectors color and timing issues") is about what a recording contains, not where a setting is stored. Fix: HIS: "L1; BD:696 and BD:797 noted (open, superseded for the dial)"; drop the L88 claim; add one line in CHANGED: "L96 names which outputs, not their settings; with option A on the same computer the result is the same."
- SHOULD | R191 (g) / missing ASSUME | Two memories now exist: the show's set of outputs (L96) and Restore Last Outputs, "the computer's own memory" (i). What Restore Last Outputs brings back when they differ (after Cmd+Shift+Esc during a show, or after opening another show) and what an opened show does after he has just panicked all off, is written nowhere. A builder must guess. Fix: add @@ASSUME G-14 (ASK LINE): "Restore Last Outputs brings back what was last on, on this computer, not what the open show holds; opening a show after All Outputs Off switches its remembered outputs on again." ALT: opening a show never overrides an All Outputs Off made this session.
- SHOULD | R191 (g) | "A remembered screen that is not plugged in is passed over without a message" is a function choice (no message on stage when the projector is not found) carried by no @@ASSUME (G-6 covers only the late plug-in and the save moment). Fix: add to G-5 TEXT: "If a remembered screen is not there, nothing is said." ALT: a short note in the Outputs list.
- SHOULD | R191 (e) | "the output monitor and the preview monitor ... show the picture as it is now" -- per his L35 the preview monitor shows the cue or a clip previewed by name, not the composition. Fix: "The output monitor shows the picture as it is now; the preview monitor shows the cue or the previewed clip; neither carries any screen's Delay or colour."
- SHOULD | 200 | RULE "It counts in the number the Outputs button shows" is a decision beyond option A and beyond his words, written as rule with no @@ASSUME (A says only "one more line ... with its own tick"). Fix: delete the sentence, or add it to G-2 as a stated part of the assumption.
- SHOULD | R226 (STATUS, HIS, NAME "Global tab") | STATUS "only the tab's name changes (L55)" and NAME SOURCE "his words L55" present the tab rename as his word; L55 (R133) speaks of the global action, and the CHANGED line itself says INFERRED. The settled basis is question 210 default A (L1: "Global" for the tab). Fix: HIS: "L1 (210 default A); L55"; NAME SOURCE: "question 210 default A (L1), his L55 'lets move to global'".
- SHOULD | R226 title / (d) | Title "the output window" and (d) "no output window that can be moved" collide with his L35 "the preview window under the output window" (the in-app output monitor). Fix: title "Picture size, the Transform and the output screen"; (d) "an output screen is always...".
- SHOULD | G-6 | One assumption carries three separate choices (when the set is taken; save-prompt on switching; late plug-in). The ALT b) and c) are about different halves, so a single strike by Boris is ambiguous. Fix: split G-6 into three lines or give three one-line alternatives labelled by half.

## MISSING
- Panic then open: after Cmd+Shift+Esc during a show, opening another show switches the projector back on at once (screen-safety on stage). Put into G-1 IF-WRONG or G-14 above.
- Syphon exception like G-4: should a Syphon remembered by the show come on at launch with no screen present? Not said (G-2 covers on/off only).
- What a show remembers when saved while some output was switched on only by a replug (no gap, just confirm) -- not needed.
- R191 (g) says an output window never takes the keyboard: carried over from CLAUDE.md "Outputs", not in his words or the shown reading; harmless, but it is Harmony's rule written without a mark.

## WHAT I CHECKED AND FOUND RIGHT
- Quotes and L numbers: L1 "All defaults good except for these.", L96 (full sentence), L94 "When you open the application, it opens to the very last show.", L88 (screens/projector), L55 "lets move to global", BD:834 "syphon is an output and treated with same output settings as a screen": all verbatim, right L numbers.
- R191: STATUS CORRECTED, only (g) replaced; a-f, h, i otherwise carried; no "today" in RULE; L96 conflict with the adopted default (not a word of his) correctly shown, grep for "never opens" in binding-decisions gives no word of his.
- G-1, G-2 ASK YES: correct (not settled; stage-visible; function). G-2 conflict L1/200 against L96 correctly raised.
- G-3, G-4, G-5: LINE is right (follow from his lines / careful choice, cheap to strike). G-9, G-10, G-11, G-12 ASK NO: right (internal / fact / rare edge).
- 199 RULE restates option A in full; 200 RULE keeps option A and narrows "off at every launch" with an ASK.
- R209: STANDS, no word asks frozen; L35 consistent; no "today" in RULE.
- D25, D26, D27: STATUS OPEN with a matching ASSUME (G-7, G-8, G-11); D26 R187 b cross-reference matches slice-E R187 b.
- P27: DROPPED by L8 with the right RULE text.
- Items present: 199 200 R191 R209 R226 D25 D26 D27 D28 P27 (10 of 10).

## LINT OUTPUT
blocks: 10 ITEM (slice has 10 ids), 12 ASSUME (2 ask YES, 6 LINE), 0 ANSWER, 7 NAME
OK
