# CHECK D -- blind re-check of apply-D.md (topic D, Actions in a show; page 2)
Checker: read-only; nothing built, run or launched. Stamp: see end.

## MY OWN READING
(written before apply-D.md was opened; only answers-by-item.md, boris-page-2.txt and slice3-D.md had been read)

BOX 226 (BF250) "tempo stop stops all actions, not just global".
- Decides: the tempo bar's stop reaches every action, not only the global ones (layer actions; and, "all", clip actions). With BF271 ("Stop removes all clips from all layers") a clip's actions end with the clip anyway.
- Leaves open: "stops" is not a letter. It can mean switched off (the item's text: buttons go dark, you switch on again) or stand still (way b: buttons stay on, start again with the beat). Also whether the CLIP actions' buttons go dark.
- Overturns: old 180 default A (buttons stay ON, new start with the beat), and any old rule that the stop reaches only layer+global actions (180, R161, R224).

BOX 227 (BF251) "neither. It's button stays on and that action plays again only when that clip is re-triggered".
- Decides: for a play-once action ON A CLIP: button stays on after its pass (rejects b); it is not "only after you switch off and on" (rejects c); it plays again when its clip is triggered again, and in no other case.
- Leaves open: the layer/global half of the item text (plays once, button goes off by itself). NOTE: way c said "always stays on"; he rejected c, which is some evidence that the layer/global half of the item stands. Also: switching it on by hand under a playing clip; re-trigger during a pass.
- Overturns: old D-4 as an open question; nothing else.

BOX 228 (BF252) "b".
- Decides: preset loaded under an action: the moving sliders keep moving with the action and, when it lets go, glide back to the value they had BEFORE the action; the preset's value for them is lost. (The other sliders taking the preset at once is the page's answer text, which he did not argue with.)
- Overturns: Harmony's recommendation (answer R129-4, assumption D-5 default) and point (4) of old R129.

Boxes elsewhere that bear on D:
- BF271 (273) "Stop removes all clips from all layers": clip actions end with their clip at a tempo stop (A owns the clip removal).
- BF256 (234) "Pasting over a clip or deleting a clip, removes it from the layer strip, and it does not play": the clip's actions end with it (R161/D11); "If a clip is playing, and I change the deck, that does not change the clip" fits 182 b (next fire only).
- BF247 (220) preview by name is not on the beat: overturns R158 (g) "triggered on the 1" (his L36).
- BF246 (218 b) "app should always try to find the 1": the 1 may now move by the app's doing -> R181 b (Resync glide) must also cover that.
- BF245/BF263 Studio: every "Review" in D's old blocks.
- BF260 (240): keying and blend stay -> old R193 TODAY line "Keying ... being removed" is stale (a TODAY line only, not a rule).
- BF243 (snapshot = save of the show as it stands): touches R132 (a control an action moves is saved at the pre-action value). F's.
- BF262 (244 b): a MIDI knob belongs to the cell -> old R134 "a key or pad put on an action stays with that action" is J's to rule.
- BF241 (241) two kinds of envelope: I's; no D rule says actions are beat-only apart from R144 (an action is counted in beats), which stands.
Empty items of D and whether any box goes against them:
- 224 (one Global glide): no box against; BF252 b uses it.
- 225 (one layer switch against global actions): no box against; BF250 "all actions" must not be read as the switch protecting a layer from the stop (the paper rules that it does not).
- 254 (clip an action triggered stays): BF271 (stop takes every clip) and BF256 (delete/paste-over) are the only exceptions; no clash if the stop is named as the exception.
- 255 (Ignore Actions toggle sends control back to before the action): agrees with BF252 b ("the value it had before the action").

## LINT OUTPUT
blocks: 7 ITEM (page 2 has 7 here), 14 AMEND, 8 ASSUME (2 YES, 2 LINE, 4 NO), 0 ANSWER, 5 NAME, 0 DROP
LINT: OK -- 7 ITEM, 14 AMEND, 8 ASSUME (2 YES, 2 LINE, 4 NO), 0 ANSWER, 5 NAME, 0 DROP; 0 problem(s) that must be fixed, 0 soft note(s)

## FINDINGS
(0 MUST, 6 SHOULD. The paper is sound in the places that matter most: every quote of his and every BF number was checked letter by letter and is right; every OLD line of the 14 amendments was found verbatim in spec-D.md; 228 b is carried as he wrote it.)

F1 | SHOULD | ITEM 226 (STATUS, RULE a) and AMEND 180 3 | The RULE says the buttons of layer and global actions go off, and AMEND 180 3 NEW writes "his words ... say it again ... so the buttons go off". His words are only "tempo stop stops all actions, not just global" (BF250); "stops" fits way b (buttons stay on, actions stand still) as well as "switched off". The OLD sentence was honest ("they do not say whether their buttons go off"); the NEW one turns a reading into his word. The doubt is carried by D3-1, but D3-1's WHY and IF-WRONG speak only of the CLIPS' buttons. | FIX: AMEND 180 3 NEW: "say that they stop, and his words of 2026-10-09 say it again for every kind of action: \"tempo stop stops all actions, not just global\" (BF250). That 'stops' means the buttons go off, as at Stop actions, is the item's own reading, which his words do not go against (assumption D3-1)." In D3-1 WHY add: "BF250 gives no letter; 'stops' can also mean the actions stand still with their buttons on (way b of the page), for layer and global actions as well."

F2 | SHOULD | ASSUME D3-1 (ASK YES, ALT b) and NOT DONE bullet 2 | (a) The paper itself says the clip buttons go off "exactly as at Stop actions" (ITEM 226 b) and R224 c, which he accepted ("R224 yes. I like that.", binding-decisions.md:1155), already says Stop actions switches off "every action of every clip, every layer and global ... One press switches them all off". He then used the same words for the tempo stop: "The tempo stop button stops all actions as well as everything else." (same line) and "tempo stop stops all actions, not just global" (BF250). So the clips' buttons at the tempo stop are largely settled by his own rule "If I have explained something, use it to answer questions not answered"; what is left is a real choice he would most likely wave through = LINE, not YES. (b) ALT b ("Only layer and global action buttons go off; a clip's action buttons stay on") does not say which stop it is about, and NOT DONE says "the question as put covers both" stops; the TEXT names only the tempo stop. If ALT b is meant for Stop actions it contradicts his accepted R224 c and must not be offered as an open way for it. | FIX: make D3-1 a LINE; ASK: "LINE Stop actions, which you accepted, already switches off every clip's actions; I give the tempo stop the same reach; you can strike it." ALT b: "b) At the tempo stop only layer and global action buttons go off; a clip's buttons stay on." Delete or correct the NOT DONE sentence "If he answers D3-1 with way b, R224 c needs the same change".

F3 | SHOULD | ASSUME D3-2 (ASK YES, WHY) | WHY says the layer/global half "is neither confirmed nor rejected". Way c of the page said "Its button ALWAYS stays on; it plays again only after you switch it off and on." and he answered "neither"; rejecting "always" is real evidence that on a layer or on global the item's sentence (plays once, button goes off by itself) stands, and "It's button stays on" is about the clip he named. The doubt is smaller than YES: it is a LINE. | FIX: ASK: "LINE his 'neither' rejects 'always stays on', so the layer and global half stays as the page read; he can strike it." Put the observation into WHY. TEXT is fine.

F4 | SHOULD | ASSUME D3-5 (TEXT) | TEXT says "a layer's X". His own words for it are not "X" (he says "clear the layer", L98); NAMES.md lists the button as "Clear (the strip button, now the letter X)". TEXT is meant for him and must use the NAMES.md name. | FIX: "I assume a layer's Clear button takes only the clip off. The layer's own actions stay on and keep playing, and one that triggers clips puts a clip on that layer again at its next trigger." (Same word in AMEND 180 4 / R161 2 NEW if they are ever shown to him.)

F5 | SHOULD | ITEM 228 RULE (g) | "(g) This is the same rule as for the hand (R129 point 2): nothing that is set on a control while an action moves it lasts." That is a general rule written as if it followed from his "b". His "b" decides only the preset; the hand is already ruled (R129 2, with the Ignore Actions toggle as the exception, R131 b); a MIDI knob, a macro or a reset set on a control while an action moves it are not ruled by anything he said. | FIX: reduce (g) to: "(g) This is the same way as for the hand (R129 point 2): what is set on a control while an action moves it does not last." and add in REACHES J: "does a MIDI knob move on a control an action holds count as the hand? Not said by him."; or drop (g).

F6 | SHOULD | REACHES OTHER TOPICS / MISSING | Items 224 and 225 close assumptions that live in the loose lists (MADE FROM names X-27 and X-3), and old items of spec-X still carry the pointers ("One slider for both is X-27", "see X-3", spec-X.md lines 44, 53, 71). The paper writes no bullet for X, though its own rule asks for one when an old block elsewhere touches the matter. | FIX: add: "- X (items touching the glide, C5 and the loose rule on a layer under a global action): 'one slider for both is X-27' is closed by item 224 (one slider, accepted); 'a layer can be taken out of this by its own switch, see X-3' is closed by item 225 (one switch, against global actions only)."

## WHAT I CHECKED AND FOUND RIGHT
- Quotes and BF numbers: every HIS line and every quote of his in ITEM 224-255, AMEND 180 1-4, R161 1-3, R224 1-2, R172 1, R181 1-2, R129 1, R158 1, CONFLICTS and NOT DONE against answers-by-item.md and binding-decisions.md (lines 981, 1105, 1134, 1155 and 1249 read; 1249 is Harmony's consequence text and is cited as "the rule as applied", correctly). BF245, BF246, BF247, BF250, BF251, BF252, BF256, BF263, BF271 all match their boxes.
- ITEM 224, ITEM 225, ITEM 254, ITEM 255 (STATUS earned: boxes empty; no box among the 33 goes against any of them; each RULE adds only what the old blocks 183/213/R172/R147/R160/R181, G5/179/R160 and R131 already say).
- ITEM 227 RULE for a clip (a)-(h): matches his words; "neither" is not stretched to rule on layer and global (asked as D3-2); "re-triggered" read as any trigger is flagged INFERRED in CHANGED; item 266 correctly used.
- ITEM 228 RULE (a)-(f), (h): says exactly "b" of the page item; Harmony's recommendation and point (4) of R129 correctly replaced; no ANSWER owed.
- AMEND 180 1, 180 2, 180 4, R161 1, R161 2, R161 3, R224 1, R224 2, R172 1, R181 1, R181 2, R129 1, R158 1: OLD text found verbatim in spec-D.md; NEW says what the cited words say. R158 (g) correctly overturned by BF247 (L36 "triggered on the 1" against "Previewing a clip should not happen on the beat").
- I looked for sentences made false by his words that no amendment touches: R160 (layers' actions held back, buttons stay on: the stop's rule (a) explicitly switches those off), R181 (b)/(h), R132 (pre-action value saved), R131 b/c, R158 d/e (stopped/paused), D11/D13/D14/D15, R193 (TODAY line on keying is stale after BF260 but is a TODAY line, not a rule, and topic I owns BF260), G5/179 (one switch). None found that changes a RULE.
- INTERNAL assumptions D-13, D-14, D-15, D-17, D-18, D-19, D-20, D-21, D-22, D-23, D-24, D-25, D-27, D-28, D-29, D-30: tested against the 33 boxes and the 57 items; none settled or contradicted, so writing nothing is right. D-31 reappears changed (extra sentence), NO: right.
- ASSUME D3-3 (LINE), D3-4, D3-6, D3-7 (NO): triage right by the three tests; D3-1..D3-3 TEXT length and form (one assumption, "I assume", at most 40 words, no layout, his names; "Studio", never "Review") correct except F4.
- CONFLICTS: the tempo stop (both quotes, with file lines verified) and the preview by name (both quotes verified); the 228 note that no word of his is overturned is right.
- Names: Studio (BF245, BF263), Global glide, Stop actions, play-once action; "Ignore Global Actions" is correctly marked as Harmony's pick.
- Lint: 0 problems; counts (7/14/8) agree with the paper's own SUMMARY.
- Owner discipline: the stop's effect on the beat, the clips, the layers and an audio file is left to A and K; the preview, Studio's opening, presets' "changed from preset", delete/paste-over of clips, the MIDI knob/cell are passed on in REACHES.

Blocks compared: 7 ITEM + 14 AMEND + 8 ASSUME + 5 NAME + 3 CONFLICTS = 37.
