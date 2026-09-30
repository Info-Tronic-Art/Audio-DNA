# usage: bash check.sh file.js ...  — wraps the workflow body in an async fn and runs node --check
for f in "$@"; do
  t=$(mktemp /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/wf/.chk.XXXX)
  { echo 'const args={}, agent=async()=>null, parallel=async()=>[], pipeline=async()=>[], phase=()=>{}, log=()=>{};'; echo '(async()=>{'; sed 's/^export const meta/const meta/' "$f"; echo '})()'; } > $t.js
  node --check $t.js && echo "OK $f" || echo "SYNTAX FAIL $f"; rm -f $t $t.js
done
