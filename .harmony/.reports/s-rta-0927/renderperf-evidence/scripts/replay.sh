# Replays the lane's patch scripts on HEAD's files in scratch trees: replay/c0, c1, c2, c3 (each cumulative).
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/renderperf
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w8
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
FILES="src/render/Renderer.cpp src/render/Renderer.h src/render/CompositorEngine.cpp src/render/CompositorEngine.h tests/CMakeLists.txt .harmony/probe-render-state.py .harmony/probe-render-state.json docs/claude/pitfalls.md docs/claude/rendering.md .harmony/HANDOFF.md"
R=$SP/replay; rm -rf $R; mkdir -p $R/base
for f in $FILES; do mkdir -p $R/base/$(dirname $f); git -C $WT show dc7adf9:$f > $R/base/$f; done
ap() { sed "s|$WT|$R/$1|g" $SP/$2 > $R/$2.$1.py; $PY $R/$2.$1.py > /dev/null || echo "APPLY FAIL $2 on $1"; }
cp -R $R/base $R/c0; ap c0 c0.py
cp -R $R/c0 $R/c1; ap c1 c1code.py; ap c1 c1probe.py; ap c1 c1docs.py; [ -f $SP/c1extra.py ] && ap c1 c1extra.py
cp -R $R/c1 $R/c2; ap c2 c2code.py
cp -R $R/c2 $R/c3; ap c3 c3code.py; [ -f $SP/c3extra.py ] && ap c3 c3extra.py
for f in $FILES; do cmp -s $R/c3/$f $WT/$f && echo "same  $f" || echo "DIFF  $f"; done
