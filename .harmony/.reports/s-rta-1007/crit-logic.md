# crit-logic: the script of boris-page-2.next.html (last <script>) + ta() + SEND in wf/render_page.py

## VERDICTS (one line per hat)
- LOGIC: PASS_WITH_FIXES. The page cannot go dead and Boris cannot lose typed text in a normal browser; two things must change first: a comment line that starts with "=== " breaks the file format, and the clipboard-only outcome tells him "Claude takes it from there" when no file exists.
- Collected boxes (verified by grep and a node stub run): 67 textareas class "cm" = 9 "answer <key>" (codec, lowres, 189, R129-4, R184, R188-snapshot, R221-b, review-name, 207-hold) + 57 numeric items + "general"; no data-k repeats; ids i<number> unique; every href="#iNNN" has a target. (grep shows "217"/"general" twice only because the selftest line in the script quotes them.)
- Download/clipboard/Cmd+Enter/listener order: sound (details below).

## FINDINGS

1. MUST | collect(), line "out.push('=== '+k+'\n'+v)" | A comment is written raw. If Boris types a line that begins "=== " (or "=== END"), the receiver cannot tell his text from a header; his words get cut or a fake box appears. Also a comment ending in blank lines is cut by the trim and runs into the "\n\n" separator, so blank lines in a body are unreliable without a rule. | FIX: indent every non-empty body line by two spaces, so a column-0 "=== " is only ever a header:
`out.push('=== '+t.getAttribute('data-k')+'\n'+v.split('\n').map(function(l){return l?'  '+l:l}).join('\n'));`
and add a format line to head: `"format: audio-dna-page2-answers v1 (a line at column 0 starting '=== ' is a header; comment lines are indented by 2 spaces)\n"`. Receiver rule is in the section below.

2. MUST | send()/done(), the "did" array | Outcome "clipboard only" (file blocked, e.g. Safari asking permission, Chrome "ask where to save", a download policy) gives "Sent (...): copied to the clipboard. Claude takes it from there." Next session looks for a FILE; none is there; it asks him to press Send again: he is misled. Also "saved to your Downloads folder" is claimed right after a.click(), which proves nothing (the browser may refuse, rename to "(1)", or save elsewhere). | FIX: in send() use `var f=false;` and set `f=true` instead of did.push in the try (keep the a.click()); replace done() by:
`function done(c){var w=r.n?r.n+(r.n===1?' comment':' comments'):'no comments: everything accepted';
 if(f&&c){msg.className='';msg.textContent='Sent ('+w+'): the file '+FILE+' went to your Downloads folder, and the same text is on the clipboard. Claude takes it from there.'}
 else if(f){msg.className='';msg.textContent='Sent ('+w+'): the file '+FILE+' went to your Downloads folder (the clipboard was blocked; that does not matter). Claude takes it from there.'}
 else if(c){msg.className='bad';msg.textContent='The browser did not save the file, but the text is on the clipboard. Next session, paste it (Cmd+V) into the prompt, or allow downloads for this page and press Send again.'}
 else{msg.className='bad';msg.textContent='The browser blocked both the file and the clipboard. The text is selected below: press Cmd+C, and paste it to Claude.';box.focus();box.select();return}
 document.getElementById('send').focus()}`
(Plus: in the none case the old code moved focus to the button right after oldCopy had selected the text, so Cmd+C copied nothing; the new else-branch keeps focus in the box.)

3. SHOULD | line "var saved={};try{saved=JSON.parse(...)||{}}" | A foreign or damaged value that is valid JSON but not an object ("abc", 5, [1]) is accepted; then `saved[k]=...` is silently ignored (string/number) or lost in JSON.stringify (array): nothing he types is ever saved again, with no sign. Verified by a node run (store stays "\"abc\"" / "5"). Parse errors and a throwing localStorage are already caught. | FIX: after the try add `if(!saved||typeof saved!=='object'||Array.isArray(saved))saved={};`

4. SHOULD | send() | On a second Send the old green "Sent" text stays until the clipboard promise settles; if it never settles (a permission prompt) or the call throws synchronously (a policy), he reads a stale "Sent" and a throw leaves the new text unreported. | FIX: first line of send(): `msg.className='';msg.textContent='Sending...';` and wrap the clipboard line: `try{ if(navigator.clipboard&&navigator.clipboard.writeText){navigator.clipboard.writeText(r.text).then(function(){done(true)},function(){done(oldCopy(r.text))})}else{done(oldCopy(r.text))} }catch(e){done(oldCopy(r.text))}`

5. SHOULD | keydown handler | A held Cmd+Enter auto-repeats and starts a download per repeat (Chrome then asks "allow multiple downloads"). Enter alone in a box is fine (needs meta/ctrl; adds a line). One keydown = one send (no second listener; button has type=button). | FIX: `if((e.metaKey||e.ctrlKey)&&e.key==='Enter'&&!e.repeat){...}`

6. SHOULD | hook `if(location.hash==='#selftest')` | It cannot run for Boris unless the address ends in #selftest (his own links are #iNNN), and it never writes localStorage, but if it did run it would replace what is in three boxes (answer codec, 217, general) and a Send would carry "TEST first box". Dead test code in a file he uses. | FIX: render it only for tests: in render_page.py emit the selftest lines only when `os.environ.get('PAGE_SELFTEST')` is set, else drop them from JS.

7. SHOULD | grow() | The height is fixed once, while overflow is hidden. Resizing the window (or a font load) re-wraps the text and the last lines of a long comment are cut off from view. | FIX: after the boxes.forEach add `window.addEventListener('resize',function(){boxes.forEach(grow)});`

## RECEIVING SIDE: how a script reads audio-dna-page2-answers*.txt (with fix 1 applied)
1. Pick the newest by mtime of ~/Downloads/audio-dna-page2-answers*.txt (the browser may name repeats "... (1).txt"; do not insist on the exact name). Reject it if its "sent:" stamp (local time, no zone) is older than the page's last build or the last time the comments were filed; then ask him to press Send.
2. Read as UTF-8 (strip a BOM if any); newlines are \n.
3. The file must contain a last line exactly `=== END`; if not, it is truncated: ask him to press Send again.
4. Split into lines. Header block = lines before the first line that starts with "=== " at column 0 (title, "sent:", "comments: N of M boxes").
5. For each line at column 0 starting "=== " (except the exact line `=== END`): key = the rest of the line, whole, spaces kept (keys are "217", "general", "answer codec", "answer R129-4"). Body = following lines up to the next column-0 "=== " line; strip exactly two leading spaces from each indented line, leave empty lines empty, drop trailing empty lines.
6. Check: number of bodies == N in the "comments:" line; keys are unique; every key is a known item or answer id (an unknown key goes back to him, it is not guessed).
7. Every id of the page NOT in the file = accepted as written. A body that is only "b" or "c" (case-insensitive, optional "." or spaces) = he chose that alternative; anything else = his own words, filed verbatim as his words with the key as its address. "general" is not tied to an item.
8. Without fix 1 the same parse cannot be made safe (a body line "=== 217" would be read as a new box).

## WHAT IS RIGHT
- Every box has a unique key; the count is 9 + 57 + 1 = 67; the header "of 67 boxes" is true.
- Empty and whitespace-only boxes are skipped (trim of any \s), so "accepted" holds; multi-line text keeps its inner newlines (textarea gives \n only); very long text is fine (no limit anywhere; localStorage quota errors are caught and the page keeps working).
- UTF-8 Blob, no BOM needed; non-ASCII letters, quotes and emoji pass through unchanged (no quoting or escaping is used, so none is needed).
- localStorage: a throwing getItem/setItem, corrupt JSON and null are all caught; the page still works without it. The key audio-dna-page2-answers-v1 is page-specific (all file:// pages share one origin, so page 3 must use a different KEY).
- Download is tried first, the clipboard second, with a textarea fallback; all four outcomes end in a message (but see 2).
- Nothing outside try/catch can throw before the listeners are attached: all getElementById targets exist (count, send, sent, sendmsg), the script is last in the body, count() is safe.
- Enter alone adds a line; Cmd/Ctrl+Enter sends once and preventDefault stops the button's own click.
