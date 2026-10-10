// usage: node dry.js <workflow.js> '<args json>' [dumpDir]  -- runs the workflow body against stub agents; prints type, model, effort, prompt length, "undefined" hits
const fs = require('fs')
const src = fs.readFileSync(process.argv[2], 'utf8').replace(/^export const meta/m, 'const meta')
const args = JSON.parse(process.argv[3] || '{}')
const dump = process.argv[4]
let n = 0
const val = (k, p) => {
  if (p.enum) return p.enum.includes('DONE') ? 'DONE' : (p.enum.includes('FAIL') ? 'FAIL' : (p.enum.includes('SOUND_WITH_CORRECTIONS') ? 'SOUND_WITH_CORRECTIONS' : p.enum[0]))
  if (k === 'findings') return [{ severity: 'MUST', issue: 'stub issue', fix: 'stub fix' }]
  if (k.indexOf('commit') >= 0) return 'abc1234'
  if (k === 'lint') return 'stub lint line OK'
  if (p.type === 'string') return 'stub-' + k
  if (p.type === 'number') return 1
  if (p.type === 'boolean') return false
  if (p.type === 'array') return []
  return {}
}
const agent = async (prompt, o = {}) => {
  n++
  const u = (prompt.match(/undefined|\[object Object\]|NaN/g) || []).length
  console.log(`#${n} type=${o.agentType} model=${o.model} effort=${o.effort} phase=${o.phase} label=${o.label} len=${prompt.length} bad=${u} schema=${o.schema ? 'y' : 'n'}`)
  if (dump) fs.writeFileSync(`${dump}/prompt-${n}.txt`, prompt)
  if (!o.schema) return 'stub text'
  const out = {}
  for (const [k, p] of Object.entries(o.schema.properties)) out[k] = val(k, p)
  return out
}
const parallel = async ts => Promise.all(ts.map(t => t().catch(() => null)))
const pipeline = async (items, ...st) => Promise.all(items.map(async (it, i) => { let r = it; for (const s of st) r = await s(r, it, i); return r }))
const AF = Object.getPrototypeOf(async function () {}).constructor
new AF('args', 'agent', 'parallel', 'pipeline', 'phase', 'log', 'budget', 'workflow', src)(args, agent, parallel, pipeline, t => console.log('phase:', t), m => console.log('log:', m), { total: null, spent: () => 0, remaining: () => Infinity }, async () => null)
  .then(r => { const s = JSON.stringify(r); console.log('RETURN keys:', r && Object.keys(r).join(','), 'len', s.length, 'bad', (s.match(/undefined/g) || []).length) })
  .catch(e => { console.log('THREW', e && e.stack || e); process.exit(1) })
