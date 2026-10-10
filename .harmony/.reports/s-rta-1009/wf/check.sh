# usage: bash check.sh file.js ...  -- wraps the workflow body in an async fn and runs node --check
for f in "$@"; do
  t=$(mktemp /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/1f72c63c-b1d2-4e28-8f18-bcce65a97add/scratchpad/wf/.chk.XXXX)
  { echo 'const args={}, agent=async()=>null, parallel=async()=>[], pipeline=async()=>[], phase=()=>{}, log=()=>{}, budget={}, workflow=async()=>null;'; echo '(async()=>{'; sed 's/^export const meta/const meta/' "$f"; echo '})()'; } > $t.js
  node --check $t.js && echo "OK $f" || echo "SYNTAX FAIL $f"; rm -f $t $t.js
done
