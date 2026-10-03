set -e
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-fix
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
H=$WT/.harmony
G="git -C $WT"
rm -f $WT/.venv
T="

Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>"
# A = R3
$G add -f .harmony/probe-vupload-ab.sh .harmony/probe-vupload-ab-selftest.sh
$G commit -q -m "fix(s-rta-1002b bf10): R3 -- probe-vupload-ab.sh clears \"\$LOG.done\" before every launch attempt's compiler sampler, so a re-run of a tainted launch is sampled too (gates-r1 SHOULD 3); .harmony/probe-vupload-ab-selftest.sh drives the real driver with stubs (no app, no lock): RED on e3c73a2 1 ok / 5 FAIL (re-run .compilers empty, accepted under a compiler), GREEN 6 ok / 0 FAIL x6; probe-vupload-ab.py --selftest PASS$T"
# B = R1
cp $S/inter/probe-milkdrop.sh.B $H/probe-milkdrop.sh
cp $S/inter/probe-milkdrop-selftest.sh.B $H/probe-milkdrop-selftest.sh
cp $S/old/.harmony/probe-milkdrop.py $H/probe-milkdrop.py
$G add -f .harmony/probe-milkdrop.sh .harmony/probe-milkdrop-selftest.sh
$G commit -q -m "fix(s-rta-1002b bf10): R1 -- probe-milkdrop.sh MILKDROP_ATTACH=1 attaches only to the harness's own test-mode app (the one running pid == MILKDROP_ATTACH_PID or the lock helper's start_app record, owns the 8080 listener, 8080 /api/health answers), else REFUSE exit 2 before any request (gates-r1 SHOULD 1); .harmony/probe-milkdrop-selftest.sh (PATH shims, stub probe body: no app, no request)$T"
# C = R2
cp $S/inter/probe-milkdrop.sh.C $H/probe-milkdrop.sh
cp $S/inter/probe-milkdrop.py.C $H/probe-milkdrop.py
cp $S/final/probe-milkdrop-selftest.sh $H/probe-milkdrop-selftest.sh
$G add -f .harmony/probe-milkdrop.sh .harmony/probe-milkdrop.py .harmony/probe-milkdrop-selftest.sh
$G commit -q -m "fix(s-rta-1002b bf10): R2 -- MILKDROP_P1 overrides the pinned P1 only with MILKDROP_MODE=pre (calibration); in lane / gate mode probe-milkdrop.sh and probe-milkdrop.py exit 2 before any request; every probe-milkdrop.py run prints 'P1 = <preset path>' first (gates-r1 SHOULD 2); selftest cases b1-b3 / c1-c4. Selftest RED on e3c73a2 15 ok / 36 FAIL, GREEN 51 ok / 0 FAIL x2, mutants M1-M4 each FAIL$T"
# D = NITs
cp $S/final/probe-milkdrop.sh $H/probe-milkdrop.sh
cp $S/final/probe-milkdrop.py $H/probe-milkdrop.py
$G add -f .harmony/probe-milkdrop.sh .harmony/probe-milkdrop.py
$G add docs/claude/rendering.md
$G commit -q -m "chore(s-rta-1002b bf10): r1 NITs -- probe-milkdrop.sh refuses MILKDROP_APP != VIDEO_APP (gates NIT-1); unused CB dropped from probe-milkdrop.py (gates NIT-4); rendering.md says MilkDrop's alpha is forced to 1 after every frame (gl NIT-1)$T"
for f in probe-milkdrop.sh probe-milkdrop.py probe-milkdrop-selftest.sh; do cmp $S/final/$f $H/$f && echo "$f == final"; done
cmp $S/final/rendering.md $WT/docs/claude/rendering.md && echo "rendering.md == final"
$G log --oneline -5
$G status --short
