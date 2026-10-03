#!/bin/bash
# MU1 / MU2 / MU3: each mutant is an uncommitted edit of a COPY-backed src file, run once, then restored byte-for-byte.
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b; S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a868a537-8200-4641-a63f-f3029b3cf7ee/scratchpad/bf9b-fix-FIX-1
FILES="src/ui/LayerInspector.cpp src/ui/ClipInspector.cpp src/ui/InspectorRepoint.h src/MainComponent.cpp"
mkdir -p $S/orig
for f in $FILES; do cp $WT/$f $S/orig/$(basename $f); done
(cd $WT && shasum -a 256 $FILES) > $S/sha-before.txt
restore() { for f in $FILES; do cp $S/orig/$(basename $f) $WT/$f; done; }
LINT=$WT/build-lane/tests/test_render_thread_lint
ASANBIN=$WT/build-asan/tests/test_show_model

echo "=== MU1: the forget-first statements removed from setLayer / setClip $(date '+%T')"
python3 - <<'PY'
W='/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b/'
for f in ('src/ui/LayerInspector.cpp','src/ui/ClipInspector.cpp'):
    s=open(W+f).read()
    import re
    lines=[l for l in s.split('\n') if not l.strip().startswith('forgetScalarBindings();   // FIRST')]
    assert len(lines)==len(s.split('\n'))-1, f
    open(W+f,'w').write('\n'.join(lines))
PY
ADNA_JOBS=6 bash $WT/.harmony/probe-asan-unit.sh > $S/mu1-asan.log 2>&1; echo "exit=$?" >> $S/mu1-asan.log
ASAN_OPTIONS=abort_on_error=0:halt_on_error=1 $ASANBIN "N1*" > $S/mu1-n1.log 2>&1; echo "exit=$?" >> $S/mu1-n1.log
$LINT "bf9b B4h*" > $S/mu1-lint.log 2>&1; echo "exit=$?" >> $S/mu1-lint.log
restore

echo "=== MU2: step (1) removed from repointInspectorsAfterStackMove $(date '+%T')"
python3 - <<'PY'
W='/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b/'
f='src/ui/InspectorRepoint.h'
s=open(W+f).read()
old='''{
    clearClipInspectorIfUnowned(clipInspector, comp);
    Layer* fresh'''
assert s.count(old)==1
open(W+f,'w').write(s.replace(old,'''{
    Layer* fresh'''))
PY
ADNA_JOBS=6 bash $WT/.harmony/probe-asan-unit.sh > $S/mu2-asan.log 2>&1; echo "exit=$?" >> $S/mu2-asan.log
restore

echo "=== MU3: the wiring line deleted (lint B4h, no build needed: the lint reads src) $(date '+%T')"
python3 - <<'PY'
W='/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b/'
f='src/MainComponent.cpp'
s=open(W+f).read()
a=s.index('    undoService_.onLayerStackMoved = [this]() {')
b=s.index('    undoService_.onFencedEdit = [this]() {')
open(W+f,'w').write(s[:a]+s[b:])
PY
$LINT "bf9b B4h*" > $S/mu3-lint.log 2>&1; echo "exit=$?" >> $S/mu3-lint.log
restore

(cd $WT && shasum -a 256 $FILES) > $S/sha-after.txt
cmp $S/sha-before.txt $S/sha-after.txt && echo "RESTORED: sha256 of the 4 mutated files equal before / after"
echo "=== rebuild build-asan from the restored src $(date '+%T')"
ADNA_JOBS=6 bash $WT/.harmony/probe-asan-unit.sh > $S/after-mutants-asan.log 2>&1; echo "exit=$?" >> $S/after-mutants-asan.log
tail -2 $S/after-mutants-asan.log
