# usage: bash commit.sh STATE MSGFILE path...   -- stages each path's content from replay/STATE (or the worktree for
# new files not in the replay set) via update-index, then commits the index. The working tree is never touched.
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/renderperf
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w8
ST=$1; MSG=$2; shift 2
[ -e $WT/.venv ] && { echo "REFUSE: .venv link present"; exit 1; }
for p in "$@"; do
  src=$SP/replay/$ST/$p; [ -f "$src" ] || src=$WT/$p
  blob=$(git -C $WT hash-object -w "$src") || exit 1
  git -C $WT update-index --add --cacheinfo 100644,$blob,$p || exit 1
  echo "staged $p <- ${src#$SP/}"
done
git -C $WT commit -q -F $MSG && git -C $WT log --oneline -1 && git -C $WT show --stat --format= HEAD
