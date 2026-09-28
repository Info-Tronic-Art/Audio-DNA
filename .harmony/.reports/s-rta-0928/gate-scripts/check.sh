#!/bin/bash
# syntax-check a workflow script: drop 'export', wrap body in async fn, node --check
f="$1"; t="${f%.js}.check.js"
{ echo 'async function __wf(agent,parallel,pipeline,phase,log,args,budget,workflow){'; sed 's/^export const meta/const meta/' "$f"; echo '}'; } > "$t"
node --check "$t" && echo "SYNTAX OK: $f"
grep -n '`[^`]*\${[^}]*`[^`]*}' "$f" | head -3
