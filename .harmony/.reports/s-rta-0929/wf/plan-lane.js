export const meta = {
  name: 'rta-0929-plan-lane',
  description: 'Audio-DNA s-rta-0929: Fable architect authors one plan (opus fallback only on a Fable hard-fail, logged), then blind council seats attack it',
  phases: [{ title: 'Plan' }, { title: 'Attack' }],
}
// args: { name, packet, seats: [{ key, temperament }] }
const A = args
const MAIN = '/Users/boriskarpman/projects/RealTimeAudio'
const RPT = '.harmony/.reports/s-rta-0929'
const PLAN = `${RPT}/plan-${A.name}.md`

const P = `${A.packet}

AUTHORING RULES: you are the planner (Law #11 row 2); an opus builder executes your plan in a worktree and Harmony adopts or overrides it after blind attackers read it. Re-derive every claim about today's code from source (file:line) at main HEAD in ${MAIN}; label anything not re-derived INFERRED. Plans that go through: exact changes by file:line, RED-first gates that drive real code (ctests) and live probe rows whose RED on current main you predict with the number, must-not-change list, risks (strongest counterargument first), commit sequence (each commit builds, passes ctest, keeps every existing probe GREEN), docs text (docs/claude/*.md; pitfall text as "NN" — Harmony assigns the number; CLAUDE.md is 24,980 B of its 25,000 B cap — pay for any addition by moving text into docs/claude/*.md), and a COMPACT at the end. Rig constraints the builder lives under (write gates that respect them): one live app at a time via a lock; open -g launches only (a background, non-key window); the Output window is NEVER opened by any gate; no synthetic input (UI states via REST / composition files / TEMPORARY env-var hooks); no debuggers or samplers; render gates decode pixels; >= 5 runs per arm for any flake verdict; perf bars only from quiet numbers; TEST_SERVER-only REST endpoints for counters; the machine is an M1 Pro (8P+2E), 32 GiB, 120 Hz display, no Xcode (Command Line Tools only), no Bluetooth audio.
Write the plan to ${MAIN}/${PLAN} via a Bash heredoc (you have no Write tool), then return a <= 40-line manifest: path, the design in <= 6 lines, gates, open questions for Harmony.
REPORT_FILE: ${PLAN}`

const bad = r => !r || /reached your Fable limit|Fable limit/i.test(String(r)) || String(r).length < 200
phase('Plan')
let plan = await agent(P, { agentType: 'architect', model: 'fable', phase: 'Plan', label: 'plan:' + A.name })
let tier = 'fable'
if (bad(plan)) {
  log(`plan ${A.name}: fable attempt 1 failed (${String(plan).slice(0, 120)}); retrying once`)
  plan = await agent(P, { agentType: 'architect', model: 'fable', phase: 'Plan', label: 'plan-retry:' + A.name })
}
if (bad(plan)) {
  log(`plan ${A.name}: FABLE HARD-FAIL x2 — Law #11 fallback: same stage as opus (logged deviation)`)
  plan = await agent(P, { agentType: 'architect', model: 'opus', phase: 'Plan', label: 'plan-opus-fallback:' + A.name })
  tier = 'opus-fallback'
}
if (bad(plan)) return { plan, tier, seats: [] }

phase('Attack')
const seats = await parallel((A.seats || []).map(s => () => agent(`BLIND COUNCIL SEAT — ATTACK the plan ${MAIN}/${PLAN} (read it in full, then the source it cites, at main HEAD in ${MAIN}). Your temperament: ${s.temperament}.
Find what is WRONG, MISSING or UNGATEABLE: a claim about today's code that does not match the source (cite file:line); a gate that cannot fail on current main or could not catch the regression it names; a case the design mishandles (stutter, black frame, leak, race, a Boris ruling in BORIS_DECISIONS.md "Playback Behaviour" crossed); a thread / GL-context / real-time violation (CLAUDE.md Sacred Rules). Rank findings MUST / SHOULD / NIT, each with the concrete fix. Do not rewrite the plan. <= 60 lines. Write the paper to ${MAIN}/${RPT}/attack-${A.name}-${s.key}.md via a Bash heredoc and return a <= 25-line summary (every MUST in one line each, SHOULD count).
REPORT_FILE: ${RPT}/attack-${A.name}-${s.key}.md`, { agentType: 'authored-council-seat', model: 'sonnet', effort: 'medium', phase: 'Attack', label: `seat:${A.name}:${s.key}` })))
return { plan, tier, seats }
