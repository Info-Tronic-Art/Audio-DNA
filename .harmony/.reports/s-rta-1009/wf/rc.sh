# usage: bash rc.sh <max chars of output to show> <command> [args ...]
# Runs the command with its whole output in a scratch file, prints "rc=<its return code>" on a line of its own, then the END of the output cut to the limit.
# Why: "cmd | tail; echo rc=$?" prints the rc of tail, not of cmd (RIG-RULES A: a return code on its own line, never after a pipe). Broken by hand in four sessions running; this makes the right way the short way.
n=$1; shift
o=$(mktemp "${TMPDIR:-/tmp}/rc.XXXXXX")
"$@" > "$o" 2>&1
rc=$?
echo "rc=$rc"
tail -c "$n" "$o"
rm -f "$o"
exit $rc
