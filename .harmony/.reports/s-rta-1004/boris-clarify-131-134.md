# Questions 131-134 to Boris (asked at the close of session s-rta-1004, 2026-10-04 16:09:01) — from the looks answers ruling (ruling-looks-answers.md section 7)
New numbers. The text below is the ruling's section 7 VERBATIM.

---------------------------------------------------------------------------------------------------------
131. An effect has a signal plugged into a slider. You load a look that was made with NO signal on that slider, or you load
     "Default".
     A (default) The signal is unplugged: the effect becomes exactly the look. A signal you plugged in by hand and never
       kept in a look is gone; Cmd+Z brings it back.
     B The signal stays plugged in: a look only ever plugs signals in. A look made without signals then does not look the
       same on an effect that has signals, and the button does not show its name.
132. `Save over "Look 2"` replaces Look 2 for good; there is no undo for it.
     A (default) A small window asks first. Only a click on "Save Over" saves; Return and Esc cancel.
     B It saves at once, with no window.
133. You loaded "Look 2" and moved a slider. The small button on the effect:
     A (default) reads "Looks", dim -- it shows a name only while the effect is exactly that look. The menu still says
       `Save over "Look 2"`.
     B keeps reading "Look 2" with a mark that says it was changed.
134. Still on your disk from before: nine quick FX saves (the numbered slots). When the old buttons go, the app no longer
     opens them.
     A (default) Leave them on the disk.
     B Delete them too.
What changes with each answer (so that either is a small change)
- 131 B: ONE line, the constant `looks::kUnwiredEntry` (Unplug -> Keep). Both policies are already built and unit-tested
  (LK-18, LK-24) and `matches` is the same under both. Pre-registered text that changes: GL-9's bar (the B string in 5.6);
  RA-10's table rows 2 and 5 read their "Keep" half; section 6 item 7 (c); V-17's manifest. Rows that do NOT change: LC-9
  (worded for the command), GL-3, GL-4 (both arms), GL-10. PL DA-5 rule 3 then applies only to a replaced connection.
- 132 B: the Save Over window is not opened (one branch in the view); V-21 is dropped; the window clauses leave LM-18 and
  LM-21; GL-10's bar loses "Return did not save; " and its two window steps; HD-15's hidden copy is built with one new store
  row.
- 133 B: the button's text rule in `refresh()` reads the loaded look's name when nothing matches, with a mark; LM-3 and
  MU-EL-17 are re-cut; one more visual state. The loaded look lives in the run only, so the mark is gone after a show is
  re-opened unless HD-14 is built with it. No store or format change.
- 134 B: Harmony moves that folder to the Trash at his word, as for 106. No code.

---------------------------------------------------------------------------------------------------------

## ANSWERS
(none yet)
