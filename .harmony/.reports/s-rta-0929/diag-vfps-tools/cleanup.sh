# cleanup.sh -- save the TEMPORARY instrumentation, revert it (apply -R of my own diff + delete my new files), rebuild
# build-lane from the clean tree against the main checkout's (read-only) JUCE copy, strings check.
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/diag-vfps
W=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-vfps
set -u
git -C $W diff > $S/instr-tracked.diff
cp $S/instr-tracked.diff $S/instr.diff
for f in src/diag/DiagVfps.h src/diag/DiagVfps.mm; do git diff --no-index /dev/null $W/$f >> $S/instr.diff; done
git -C $S/juce-src diff > $S/juce-instr.diff
echo "saved: $(wc -l < $S/instr.diff) lines instr.diff, $(wc -l < $S/juce-instr.diff) lines juce-instr.diff"
git -C $W apply -R $S/instr-tracked.diff && echo "reverted tracked files"
rm -f $W/src/diag/DiagVfps.h $W/src/diag/DiagVfps.mm && rmdir $W/src/diag && echo "removed src/diag"
echo "--- status:"; git -C $W status --porcelain
JUCE_DIR=/Users/boriskarpman/projects/RealTimeAudio/build/_deps/juce-src bash $S/tools/build.sh cfg
B=$W/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA
echo "strings markers (DIAG-VFPS|ADNA_VFPS_|DiagVfps): $(strings $B | grep -c -E 'DIAG-VFPS|ADNA_VFPS_|DiagVfps')"
grep FETCHCONTENT_SOURCE_DIR_JUCE $W/build-lane/CMakeCache.txt
