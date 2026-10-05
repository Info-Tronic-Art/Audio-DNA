# seat2 facts-CD (round 2): topics C Presets, D Actions in a show
Written: see end line. Read-only. Labels: VERIFIED = I read it (file:line / quoted words). INFERRED = from what is named. UNKNOWN = cheapest check named.
Items read: Q178 Q192 (C); Q179-Q185 Q193 Q194 Q213 (D); readings R136 R127 R154 R155 R171 R156 R157 R180 R207 (C), R133 R134 R172 R158 R144 R159 R129 R130 R131 R132 R160 R161 R145 R162 R146 R143 R147 R181 R182 (D); decided lines 8-13; design lines 4-14; chat blocks R114 R116 R120 R121; conflicts 2,3,4-8,11,19; intro.
Sheets leaned on: facts-presets-delta.md (incl. CORRECTIONS), facts-resolume-emulate.md (incl. CORRECTIONS), facts-actions-open.md (incl. CORRECTIONS), area-actions.md, area-effects-signals.md, area-clip-transport.md.

## A. Source lines I re-checked myself (more than five, all hold)
1. VERIFIED EffectStackView.cpp:97-98 bypassBtn "B" at x=2 (left of the name, which is drawn from x=30, :46-48), deleteBtn "X" at width-26; EffectStackView.h:122-123 the two buttons are juce::TextButton{"B"} / {"X"}. So today B and X are NOT adjacent: B is at the left end.
2. VERIFIED BrowserPanel.h:48 fxTabBtn_{"FX"}; BrowserPanel.cpp:18-23 six tabs. R207 "today the tab reads FX" holds.
3. VERIFIED LayerStrip.cpp:478-488 the F slider: setRange(0,4,0.1), default 0.3; Layer.h:361 "transitionSpeedSeconds" (seconds). Q183 "0 to 4 seconds" holds.
4. VERIFIED MainComponent.cpp:4123-4136 Cmd+X = Clear selected clips, comment "No clipboard concept exists at HEAD"; :7078-7092 kClipClear. R145(d) holds (the "no copy/paste of clips today" rests on a 2026-07-30 grep comment; kClipCopy/Paste found only in that comment now).
5. VERIFIED ClipCell.cpp:220-243 thumbnail click = trigger, name area click = select. R162 "select a clip by its name" holds.
6. VERIFIED ClipInspector.cpp:20-52 transport buttons are the glyphs back / pause / play (writing clip->reverse, clip->playing); :56-58 loop list = Loop / Ping Pong / One Shot; ClipInspector.h:124 a text button "Reverse" also exists today. Clip.h:120 LoopMode {Loop, PingPong, OneShot}.
7. VERIFIED ManualWrite.cpp:78 `path.control == "speed"` returns nullopt: clip speed is not addressable by a take or an action (Q193 "today a recording cannot keep Speed" holds).
8. VERIFIED Routine.h:29 `deckRelative = true` "keys resolve on the ACTIVE deck at fire time; layer = recorded". Q182 "today's routines fire the cell of the deck that is shown" holds (default setting).
9. VERIFIED docs/claude/recording.md:77-81 + area-actions O7: the earlier routine does not resume. R182 holds.
10. VERIFIED Composition.h:147-148 gripHoldMs 250, handBackGlideMs 120 (R199(e) "quarter of a second" holds; today a hand-back GLIDES 120 ms, 158 A makes it a jump).
11. VERIFIED DeckView.h:257-260 kLayerStripWidth 250, kRoutineRowHeight 22, kCellWidth 90; FA:186 the strip uses 242 of 250. Design 8 "about 8 points free" holds.
12. VERIFIED InspectorPanel.h:87-90 tabs read "Clip", "Layer", "Composition", "Signal". There is NO tab that reads "Global" (Boris's word "global tab", BD:1050). Q39 settles the word; D items do not point to it.
13. VERIFIED binding-decisions.md:254-256 ("smooth transition back."), :339-345 (ruling 18 "snap back ... as soon as it is let go"), :894-895, :925-934, :988-992 (131 in full), :1080 (150), :1083 (160). Quotes in Q178/181/192/213, R130 are verbatim; Q192's quote is cut (finding M2).
14. VERIFIED LayerInspector.cpp:155-160 "Ignore Column Trigger" toggle; layer strip has X, B, S buttons (FA:186). R181(g)(h) premises exist.

## B. Supported by a sheet line (not re-derived by me)
- R136 (a)(b)(c): facts-resolume-emulate A1.1, A1.3, A1.4, A5.1, A5.2 (VERIFIED, pictures re-read in its CORRECTIONS); "P holds the menu" and "no arrow when no presets" INFERRED (A1.6, A5.2) and the label says so. Right.
- R127: facts-presets-delta T6-T10, C5 (the 133 / 150 clash is INFERRED not VERIFIED); a Save button is NOT in Resolume (A1.3: Save is a menu item only); name field start / existing name UNKNOWN (A2.2, A2.3) and R127's label says so. Right.
- R154: A5.4 VERIFIED sentence; arriving values INFERRED (A5.5); double-click on a preset UNKNOWN (A5.8). Right.
- R155: A2.1 says Save OPENS the Manage Presets window; A3.1 Manage... INFERRED the same window. R155(a) gives Save = small name window and Manage... = a list: labelled inferred; but it begins "As in Resolume" (NIT).
- Q178: "Resolume shows no preset name on an effect; its manual says an effect does not remember which preset" = A4.1, A4.3 VERIFIED; correct.
- R171, R157, R156, R180: INFERRED/mine and labelled so; R180(a) "shapes of your own signals belong to the show (R188)" is new work: area-effects-signals 1.15 VERIFIED that nothing of a signal's shape is saved with a show today, R188(b) says it is added. Consistent.
- Q192 / R157 what a preset holds: BD:926-927 "102 B" VERIFIED.
- R133, R134, R130, R131, R132, R160, R161, R129: his words VERIFIED (BD:1044-1054, 1056-1058, 1082-1087); every sentence after "Mine" is INFERRED and marked. R134 room on screen VERIFIED (above).
- R143: Pitfall 68 (CLAUDE.md) VERIFIED; his one saved show has no routine VERIFIED (FO F2, parsed).
- Decided 8, 12: area-actions point 24 / FA:141-148 (positional keys) VERIFIED. Decided 9, 13: consistent with CLAUDE.md rules 15 / Pitfall 36; build design, no code today.

## C. FINDINGS
### MUST
M1  Q180 A contradicts R129 (3) / R161 on a clip's own sliders.
    Q180 A: "Sliders stay where they stood (a fader that an action had at 0 stays at 0)". R129 (3): "A clip's action that ends because its clip stops or is replaced: ... back to before, so the clip looks the same the next time you fire it". Stop takes every clip off its layer, so for the two clip actions of Q180's own situation A and R129 say opposite things; option C is exactly R129 (3). A builder cannot follow both.
    FIX, Q180 A text: "Every action stops with the beat and its button stays ON. A layer's and a global action's sliders stay where they stood (a fader that an action had at 0 stays at 0). A clip's own sliders go back to where they were before its actions, because the clip left its layer (R129, case 3). When you fire a clip or press play, the actions start again from their beginning on the \"1\"." Then B and C need no change except C: "... the layer's and global sliders go back as well".
M2  Q192 drops the part of his answer 131 that decides it.
    his_words holds only the last sentence ("If it is a look, there is no way to remove the signal unless ..."). His sentence before it (BD:988-989): "If this is a clip in the show, then ctrl-z brings it back. There is no other way." That is evidence for A (load unplugs, Cmd+Z brings it back; "no other way" = no keep-my-signal option), and it is what R157 / R198 (b) already build. Without it the question looks open and the A / B choice is lopsided.
    FIX: his_words = both quotes in full: "131 is the signal plugged into a slider in a clip in the show, or is it a look which is an effect preset in the effect library that can also be connected to a signal. If this is a clip in the show, then ctrl-z brings it back. There is no other way. If it is a look, there is no way to remove the signal unless the user drops it into the show (clip, layer or global) and then adds a signal and saves that look." And add to A's text: "Cmd+Z brings the bass back (your words)." Then reword "why": "I read your answer as A; tell me if you meant B."
### SHOULD
S1  Q180 A hides what a layer's clip-firing action does after a stop. With A the layer's action stays ON; when the beat starts again (any fire, or play) it fires its clips again on the "1", so one fire of one clip can bring back the clips of other layers. B and C do not.
    FIX, append to A: "A layer's or global action that fires clips fires them again as soon as the beat runs, so your first fire after a stop can bring back clips on other layers."
S2  Q178 B says "Exactly as in Resolume" and then adds the small Save button. Resolume has no such button (Save is only a menu item: facts-resolume-emulate A1.3); the button is his own addition (BD:1080).
    FIX, B: "As in Resolume: a small \"P\" with an arrow and no name anywhere; the effect does not remember which preset it came from. Your small Save button is kept (Resolume has none); it shows whenever the sliders match none of the effect's presets."
    Also say in the "why": "your Resolume picture and \"Please emulate this\" came after your 150 a" (BD:1089 after BD:1080), so he can weigh the order.
S3  R136 (b) and design 4 say the P sits "between B and X". Today B is at the LEFT end of the row, before the name (EffectStackView.cpp:97), X at the right; they are not neighbours. In his Resolume picture B, P and X are together at the right.
    FIX, R136 (b) end: "... between B and X in his picture. In the app today B sits at the left end of the row, before the name; whether B moves to the right, next to P, comes as a picture (design: preset button)." Add the same clause to design item 4.
S4  Q179: why_default tag "[your words]" is partly mine: his words do not say that only a global action reaches several layers; that is my reading of "saved separately" (BD:1086). A's text "holds back the layers' own actions" does not say that it is EVERY layer, layer 3 too (155 A, accepted by default).
    FIX: tag "[your words + mine]"; A: "... Like every global action, it holds back the actions of every layer while it is on, layers 3 and 4 included, though they have nothing to do with the two faders."
S5  Q183: tag "[your words]" for "counts seconds, 0 to 4": his words say only "sets the time" (BD:1051); the unit is the F slider's today (LayerStrip.cpp:478). Since "all defaults good" accepts [mine] and [as today] differently, a wrong tag matters.
    FIX: tag "[as today]"; why_default: "Your words say it \"sets the time\"; the layer's fade slider F counts seconds, 0 to 4, today, so I took the same."
S6  Q192: tag "[your words]" while the text says it rests on "how I read your answer to 131". FIX: tag "[your words + mine]" (after M2 it may be pure [your words]).
S7  Intro: "Fourteen readings hold something that you would see on stage and that is mine: R129, R131, ... R213". The list is incomplete for D: R158 (c)-(f) (a layer's / global action starts on the next "1"; a clip you paused holds its actions), R161 (a layer's X ends a clip's actions; a replaced clip's actions end), R159 (an action not a whole number of bars drifts), R145 (right-click marks an action, Cmd+C/X/V) are all "Mine" and all seen on stage. He may read only the listed fourteen.
    FIX: add R158, R159, R161 to the list (sixteen / seventeen) or say "at least fourteen".
S8  Q193 A leaves out the in and out points with no reason (why_default explains only play / pause / backwards). His words: "every parameter and button that can be adjusted". B and C do not say what they do with in / out points either. Today also: the clip tab has a trigger list (Restart / Continue) and a text button "Reverse" (ClipInspector.h:124; the ruled design drops it, area-clip-transport 67).
    FIX: A: "... It cannot hold the clip's play, pause and backwards buttons, or its in and out points (an action would cut the clip under the beat; tell me if you want them)." Say in B and C what happens to in / out points.
S9  Decided line 10: "An action's recorded tempo ... is how the app counts the action in beats." Not supported: an action is beat-native (R144; Routine.h has no tempo field today, area-actions item 3), and the saved tempo is read by nothing (FA:44, area-actions H19). He may expect it to do something.
    FIX: "An action's recorded tempo is saved inside it, as you asked. The action is measured in beats and plays in time at any tempo (R144); the saved tempo changes nothing you see, and there is no control for it."
S10 R172 / Q213 say "an action that starts always takes its first value on the \"1\"" (jumps); R160 (a) says that when a global action goes on the sliders GLIDE over the transition time. For a slider that only the global moves, nothing says jump or glide.
    FIX, R172 add: "A global action is the exception: its sliders glide to their first values over the transition time (R160). Say so if a slider that no layer action moves should jump." and Q213 "always" -> "(except a global action, R160)".
S11 NEW. A preset loaded while an action moves the effect's sliders is in no item. R129 (2) says a hand move never lasts on a slider an action moves; a preset load is one press with nothing to hold, and R124 / R157 / R198 (b) already count it as something you do by hand. On stage: you load "Blue" on an effect whose first two sliders an action moves.
    QUESTION (topic C or D; new number): "An action is moving the first two sliders of an effect. You load your preset \"Blue\" on that effect. A (default, [mine], as R129 (2)): Blue sets the other sliders; the two sliders go on following the action, and when it goes off they go back to their values from before the action: Blue's values on those two are lost. B: Blue's values on those two become the values they go back to when the action goes off. C: Blue is refused for those two sliders and the button says why. Why it matters: with A a click on a preset can look as if it did nothing on the sliders you care about."
S12 R181 (c) uses "Play Once and Hold"; today the clip's list reads Loop / Ping Pong / One Shot (ClipInspector.cpp:56-58). No other item introduces the new list, so he meets a setting he cannot find. FIX: "(c) A clip set to \"Play Once and Hold\" (today the list reads One Shot; the new list comes with the clip transport) ..."
S13 First use of "global tab" in D (R133, Q179, R160): the tab reads "Composition" on screen today (InspectorPanel.h:89); Q39 (topic F) asks the word. FIX: in R133 (d) add "(the tab reads \"Composition\" today; question 39 asks which word stays)".
### NIT
N1  Q184 A and C make the app "tell you so" / "a line that says what clip B lacks"; his rule on messages (BD:771-774) is settled in Q209. Add "(a line in the action's own row, not a pop-up; see 209)".
N2  R155 begins "As in Resolume" but in Resolume "Save" opens the Manage Presets window itself (A2.1) and "Manage..." probably the same window (A3.1); here Save = the small name window (R127). Say: "In Resolume Save and Manage... lead to one window; here they are two".
N3  R136 (c): "(its manual says so)" covers "presets are listed under the effect" (A5.2); the fold ARROW is from his picture only. Move the bracket.
N4  R155 (d) ("Default is always first...") sits under "only if the effect remembers its preset"; it holds either way.
N5  R207: "It will read Effects" is a design fact (tab width: six tabs); fine as a reading.
N6  The word "holds" means five things in D (an action holds a slider; held back; hold with the clip's pause; the loop style "Hold"; "your hand holds"). Q179, R130, R160, Q194 B would read clearer with "takes over" for the hand and the action.
N7  Q183: "the transition slider" collides with the 15 clip-to-clip transitions and the layer's F "transition" list. Option text says "like the F (fade) slider" so he can tell; consider "the glide slider" only if he agrees.
N8  Q213 B and C differ only by "transition slider" vs "about one beat"; fine.
N9  R198 (b) makes deleting / cutting / pasting an action an Undo step while a layer's action buttons sit in the layer strip, where his rule says Cmd+Z changes nothing (BD:764-766). R198 says it; fine, but R145 (f) could point to R198.

## D. Checked and right (no finding)
Q179 (the options match R133 d), Q181 (Pitfall 33 / ManualWrite chain: hand > action > signal, VERIFIED ManualWrite.h:21-25 per area-actions 6), Q182 (today = shown deck, VERIFIED), Q184 A/B/C, Q185, Q194, R144, R146, R130 (quote verbatim BD:1083; the alternative "R130 off" is offered), R131 (a)-(e), R132, R143, R147 (nine defaults = BD:1078 list; eight readings stand; R113 second half withdrawn by R198), R156, R171, R180, R182, decided 8, 9, 11, 12, 13; design 4-14 (variants distinct; geometry facts in 8 verified); chat blocks R114, R116, R120, R121; conflicts 2, 3, 4-8, 11, 19 are each handled by an item.

## E. Uncovered
1 point: S11 (preset load while an action moves a slider): no home in the list. S10 is a wording gap inside existing readings, not a missing home.
Points checked: 66 (12 questions, 9 + 19 readings, 6 decided, 11 design, 4 chat blocks, 5 conflict handlings), plus 14 source lines re-read.
Mon Oct  5 16:53:05 EDT 2026
