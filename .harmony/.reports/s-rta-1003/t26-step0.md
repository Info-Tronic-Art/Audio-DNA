# T26 STEP 0 - first-call prefix measurement (RTA main session + RTA builder)

Measured 2026-10-03 from Claude Code transcripts (read-only; nothing in the repo touched except this file).
Labels: VERIFIED = measured from a transcript / file on disk; INFERRED = derived, with the derivation stated.
Token estimates come in two columns: `tok@3.6` = bytes / 3.6 (the rule you asked for) and `tok@fit` = bytes x 0.451 (fitted, see section 3). Section 3 shows 3.6 under-counts by about 1.6x on this content, so read `tok@fit` as the better number and `tok@3.6` as the floor.

## 0. Answer in five lines

1. The "54k" is the RTA MAIN session, not a builder. VERIFIED: today's RTA main first call = 54,114 tokens. Today's RTA builder first call = 29,598 tokens (about the "30k"). Yesterday's RTA builders (old 43 KB spec) were 37.7k-43.7k, mean 40.4k over 32 builders.
2. The "primary 30k" does not reproduce for current sessions. VERIFIED: the Harmony_Main primary main-session first call is 39,520 (today 15:34Z); the five sessions since 10-02 are 43,331 / 40,867 / 38,668 / 39,774 / 39,520; 09-21..09-30 were 40.6k-43.3k; only early September (09-05/06) sat at 33.6k-34.1k (58 KB of identified text) and one odd 09-13 session at 31.8k. The measured RTA-main minus primary gap is 14.6k tokens, not 24k.
3. The global `~/.claude/CLAUDE.md` is NOT loaded twice. VERIFIED: it is loaded once in the main session and once in the builder; `layer0-block.md` appears once (as the expanded @-import). "Duplicate" can only mean overlap of content with the kernel / RTA CLAUDE.md. The primary avoids it with `claudeMdExcludes` in `Harmony_Main/.claude/settings.json`; RTA has none.
4. `.harmony/HANDOFF.md` (291,403 B) is NOT in any first-call prefix. VERIFIED. What IS in the main prefix is a 10,132 B pasted copy of its top "BIRTH PROMPT" section (9,943 B in the file, same text re-wrapped).
5. Biggest single piece in the RTA builder's first call is the RTA CLAUDE.md: 23,961 B = about 10.8k of its 29.6k tokens (36%).

## 1. Per-transcript first-call totals (usage of the first assistant record; VERIFIED)

| Transcript | agent / model | input | cache_creation | cache_read | first-call prefix (sum) |
|---|---|---|---|---|---|
| T-main: a868a537...jsonl (this session, 17:24Z) | harmony main, opus 5.5 1M | 2 | 30,805 | 23,307 | **54,114** |
| T-builder: wf_657aa03c-c9c/agent-a2685b00116d9e478 (build:bf9b-merge:M1, 17:37Z) | builder, opus | 2 | 29,596 | 0 | **29,598** |
| Y-builder: 73d4f54c.../wf_d50aebe5-9c9/agent-a30c924eeb36526bb (build:bf9b:S4, 10-03 01:49Z, old 42.9 KB spec) | builder, opus | 2 | 20,450 | 20,700 | **41,152** |
| Y-builder range, all 32 builders of 73d4f54c | builder | | | | 37,680 - 43,698, mean 40,431 |
| Y-main: 73d4f54c...jsonl (yesterday) | harmony main | 2 | 31,085 | 25,843 | 56,930 |
| P: Harmony_Main primary ebd1a1f6 (10-03 15:34Z) | harmony main (reference) | 2 | 39,518 | 0 | 39,520 |
| P older: Harmony_Main 9dc554c6 / 67e04d9d | harmony main | | | | 39,774 / 38,668 |
| HM builder (cwd Harmony_Main, old spec) a4858591, 16:15Z | builder, sonnet | | | | 25,654 |
| this measuring agent (general-purpose, sonnet) | all tools + deferred list | 2 | 36,877 | 0 | 36,879 |

Notes: cache_read in a first call is a cross-session cache hit on an identical earlier prefix (T-main 23,307; Y-builder 20,700); it is part of the prefix. The sum is what the call carries.

Calls per session (VERIFIED, distinct assistant message ids with usage, session 73d4f54c, main + all subagent jsonl): total 6,716 = builder 3,070, architect 2,116, reviewer 892, MAIN 297, Explore 226, council seats 106, general-purpose 9. Every agent type except Explore (226 calls) loads the three instruction files, so 6,490 calls carry CLAUDE.md. The plan's "~2,360 calls/session" is lower than this one session; both are shown in section 6.

## 2. Per-piece decomposition (bytes = rendered text in the transcript, UTF-8; VERIFIED)

Columns: B = bytes. "main/bT/bY/P" = present and its bytes in T-main / T-builder / Y-builder / primary P (ebd1a1f6). `tok@3.6`, `tok@fit` are for the T-main size (or the T-builder size where main is 0).

| Piece | T-main | T-builder | Y-builder | Primary | tok@3.6 | tok@fit |
|---|---|---|---|---|---|---|
| Agent spec = system prompt (harmony.md body / builder.md body) | 34,468 | 15,046 | 42,921 | 34,468 | 9.6k (4.2k builder) | 15.5k (6.8k builder) |
| Harness boilerplate in system prompt (WebSearch note 423; launcher msg 340 + "Notes:" 899; Y +49 total_tokens) | 423 | 1,239 | 1,288 | 423 | 0.1-0.3k | 0.2-0.6k |
| **RTA project CLAUDE.md** (23,961 B on disk 23,962) | 23,961 | 23,961 | 24,005 | (Harmony_Main CLAUDE.md 1,972) | 6.7k | **10.8k** |
| **~/.claude/CLAUDE.md** (global; symlink to Harmony_Main/global-config/CLAUDE.md) | 5,134 | 5,134 | 5,134 | 0 (excluded) | 1.4k | **2.3k** |
| layer0-block.md (via the global's @-import; primary gets it via global-config/layer0-block.md) | 2,255 | 2,255 | 2,255 | 2,255 | 0.6k | 1.0k |
| instructions wrapper text ("Codebase and user instructions are shown below...") | 576 | 576 | 576 | 468 | 0.2k | 0.3k |
| Task prompt: Boris's pasted birth prompt (main) / workflow packet (builder) | 10,132 | 13,664 | 10,646 | 25 | 2.8k (3.8k builder) | 4.6k (6.2k builder) |
| workflow-authoring skill (cmd 135 + text 17,141), auto-loaded at boot | 17,276 | 0 | 0 | 17,276 | 4.8k | 7.8k |
| skill_listing | 4,416 | 0 (builder has no Skill tool) | 4,639 | 5,949 | 1.2k | 2.0k |
| agent_listing_delta | 5,536 | 0 | 0 | 5,536 | 1.5k | 2.5k |
| deferred_tools_delta | 368 | 0 | 0 | 368 | 0.1k | 0.2k |
| mcp_instructions_delta (firecrawl) | 713 | 0 | 0 | 0 | 0.2k | 0.3k |
| SessionStart hooks (work-index-roll 113 + session-start 1,615 + hook-liveness 325) | 2,053 | 0 | 0 | 3,980 | 0.6k | 0.9k |
| UserPromptSubmit hook (gotcha-surface; fires again on EVERY user prompt) | 2,411 | 0 | 0 | 1,154 | 0.7k | 1.1k |
| ultra_effort_enter | 347 | 0 | 0 | 347 | 0.1k | 0.2k |
| session_context (email + gitStatus) | 2,103 | 2,174 | 2,504 | 1,887 | 0.6k | 0.9k |
| environment | 804 | 804 | 804 | 702 | 0.2k | 0.4k |
| model + date + attribution | 837 | 837 | 837 | 837 | 0.2k | 0.4k |
| **Identified bytes (sum)** | **113,813** | **65,690** | **95,609** | **77,647** | | |
| Identified tokens | 31.6k @3.6 | 18.2k @3.6 | 26.6k @3.6 | 21.6k @3.6 | | 51.3k / 29.6k / 43.1k / 35.0k @fit |
| Measured first-call total | 54,114 | 29,598 | 41,152 | 39,520 | | |
| REMAINDER = tool JSON schemas + estimator error (a remainder, NOT a measurement) | 22.5k @3.6 | 11.4k @3.6 | 14.6k @3.6 | 18.0k @3.6 | | 2.8k / 0.0k / -2.0k / 4.5k @fit |

Not in any transcript, so not measured directly: tool JSON schemas. The remainder row is the only handle. At 3.6 B/tok the remainder is 11.4k for a builder with 7 simple tools (Read, Write, Edit, MultiEdit, Bash, Glob, Grep), which is not credible; at the fitted rate it is about 0, with a +-1.7k fit error. INFERRED: tool schemas are a small term (about 0-3k) for these agent types; the first call is almost entirely identified text.

Other facts about what is NOT in the builder's first call (VERIFIED): no SessionStart/UserPromptSubmit hook output, no SubagentStart gotcha block (none among the first 9 records), no skill listing, no agent/deferred/MCP listing, no workflow-authoring text. Spec files on disk: builder.md 15,056 B (body 15,046 = prompt_snapshot block0), harmony.md 35,001 B (body 34,468). builder.md has no @-imports; its playbook (`harmony-references/builder-playbook.md`, 42,323 B) is on-demand only.

## 3. Calibration: why bytes/3.6 is not used for the conclusions

VERIFIED evidence that this model's tokenizer gives about 2.2-2.5 bytes/token on these inputs:
- Fit over 461 fresh first calls of reviewer / architect / council-seat agents (same 4-tool set, RTA, 09-30..10-03): tokens = 0.451 x identified_bytes + 279, residual sd 1.7k (about 6%). Same slope in three agent types separately (0.439-0.464).
- Out-of-sample checks: that fit predicts today's builder at 29,903 (measured 29,598, error 1%) and the RTA main at 51,605 (measured 54,114, error 4.6%, plausible extra for the main's larger tool set).
- Matched pair 1: RTA main (54,114; 113,813 B) vs primary (39,520; 77,647 B): +36,166 B for +14,594 tokens = 2.48 B/tok.
- Matched pair 2: yesterday's builder (41,152; 95,609 B) vs today's (29,598; 65,690 B): -29,919 B for -11,554 tokens = 2.59 B/tok.
- Incremental: across 58 call pairs (main + today's builder), added tokens vs tool-result bytes = 2.10 B/tok, R2 0.97.
Caveat (INFERRED): one global rate; prose (CLAUDE.md) may be a little cheaper per byte than path/ID-heavy packets. For CLAUDE.md-specific figures I show a range, `@3.6` floor to `@fit`, and the matched-pair rate (2.48) sits in between.

## 4. The gap: RTA main 54.1k vs primary 39.5k (and vs the builder)

Measured gap (VERIFIED) = 54,114 - 39,520 = **14,594 tokens**; identified byte difference = 36,166 B. Itemised (all bytes VERIFIED, tokens INFERRED):

| Piece in RTA main that the primary does not carry (or carries less of) | extra B | tok@3.6 | tok@fit |
|---|---|---|---|
| RTA CLAUDE.md 23,961 vs Harmony_Main CLAUDE.md 1,972 | +21,989 | 6.1k | 9.9k |
| ~/.claude/CLAUDE.md global (primary excludes it via claudeMdExcludes) | +5,134 | 1.4k | 2.3k |
| Birth prompt (Boris pasted the HANDOFF top section) 10,132 vs primary's 25 B "read handoff boot primary" | +10,107 | 2.8k | 4.6k |
| mcp_instructions_delta (RTA has the firecrawl block) | +713 | 0.2k | 0.3k |
| UserPromptSubmit hook +1,257, SessionStart hooks -1,927 (net) | -670 | -0.2k | -0.3k |
| session_context +216, environment +102, instructions wrapper +108 | +426 | 0.1k | 0.2k |
| skill_listing 4,416 vs 5,949 (smaller in RTA) | -1,533 | -0.4k | -0.7k |
| **Total** | **+36,166** | **10.0k** | **16.3k** |

Reading: @3.6 explains only 10.0k of the 14.6k measured gap (68%); @fit explains 16.3k (112%). The truth is between, close to the matched-pair rate. The ranking is robust to the rate: (1) the two CLAUDE.md files (27.1 KB, 75% of the extra bytes) then (2) the pasted birth prompt (28%; the remaining items net to -3%).

Pieces both carry (so NOT part of the gap, but big): harmony.md 34.5 KB, workflow-authoring text 17.3 KB, agent_listing 5.5 KB, skill_listing 4.4-5.9 KB.

Compared with the builder: the old-spec RTA builder (41.2k) vs an old-spec Harmony_Main builder (25.7k, instructions 4.7 KB) differ by 15.5k tokens for 35.0 KB: that is the "secondary premium" per builder call. After yesterday's spec slimming (43.5 KB -> 15 KB) the builder fell from 41.2k to 29.6k and the RTA instructions (31.9 KB, about 14.4k @fit = 49% of the first call, 8.9k @3.6 = 30%) became its largest piece.

Why the plan's "54k vs 30k" mismatch (INFERRED): the 54k is the main-session figure, the 30k matches the slimmed builder; they are different agents. I could not find a primary main-session first call near 30k in the last 3 weeks (lowest 31.8k on 09-13, an anomalous session; the next lowest 33.6k on 09-05).

## 5. Direct answers on STEP 2 and STEP 3

STEP 2 - is the global CLAUDE.md loaded twice?
- VERIFIED: no. `instructions` attachment of T-main and T-builder lists exactly 3 files: `/Users/boriskarpman/.claude/CLAUDE.md` (5,134 B), `/Users/boriskarpman/.claude/docs/layer0-block.md` (2,255 B, the global's @-import expanded), `/Users/boriskarpman/projects/RealTimeAudio/CLAUDE.md` (23,961 B). Phrase counts: "Real request first" 1, "Think before coding" 1, "Surgical changes" 1 in the whole instructions block; 0 in the system prompt. Same list in every RTA agent type except Explore (VERIFIED over 32+27+26+25+1 agents in 73d4f54c).
- Primary mechanism (VERIFIED): `Harmony_Main/.claude/settings.json` has `claudeMdExcludes` for both `/Users/boriskarpman/.claude/CLAUDE.md` and its symlink target `/Users/boriskarpman/Harmony_Main/global-config/CLAUDE.md`, and Harmony_Main/CLAUDE.md re-imports Layer 0 from an in-repo copy (a project CLAUDE.md cannot import outside the project). Primary subagents (builder, reviewer, architect in Harmony_Main) also show only the 2 project files, so the exclude reaches subagents (VERIFIED for project-level settings).
- RTA: `.claude/settings.local.json` holds only `disabledMcpjsonServers`; no exclude. Secondary sessions are launched `claude --agent harmony --settings <settings.secondary.json>` (`bin/harmony:777-778`); that file has no `claudeMdExcludes` either.
- Cut mechanics (INFERRED): add the same `claudeMdExcludes` (both paths) to `Harmony_Main/.claude/settings.secondary.json` or to RTA `.claude/settings.json`, AND supply Layer 0 in-project (copy the 2,255 B block into the RTA repo and @-import it, as Harmony_Main did), otherwise Layer 0 disappears with the global (it only arrives through the global's import). Whether `--settings`-supplied excludes reach subagents is not measured.
- What the global file would stop supplying: Core Rules, Layer 1 karpathy, LESSONS_LEARNED gate, Coding Standards, Communication (the primary runs without it). That is a content decision for Boris/Harmony, not a mechanical one.

STEP 3 - is HANDOFF.md in any first-call prefix?
- VERIFIED: no auto-load. Not in the instructions files, no @-import in either CLAUDE.md or the agent specs, SessionStart hook text (read in full) only says "Load this repo's own .harmony/HANDOFF.md". The builder never touches it (0 of 44 tool calls reference it); the builder's packet points at lane report files instead.
- VERIFIED: the main prefix DOES carry the file's top section: the birth prompt Boris pasted = 10,132 B, a re-wrapped copy of HANDOFF.md lines 5-87 ("NEXT-HARMONY - BIRTH PROMPT & PERSONA", 9,943 B). It is in main only (never in subagents), for every one of the main's calls (297 calls yesterday).
- VERIFIED: on demand, after call 1, the main read HANDOFF.md in ranges: Read offset 3682 limit 60 (3,255 B), Read offset 951 limit 75 (5,246 B), plus a `wc -c` and a grep (7,214 B and 8,403 B outputs that also cover other files). A whole-file read would be 291 KB (about 81k @3.6 / 131k @fit tokens, paid once and then carried by every later call of that agent).
- Consequence (VERIFIED arithmetic): trimming HANDOFF.md from 291 KB to <= 20 KB saves 0 tokens in any first call unless the top birth-prompt section also shrinks.

## 6. Savings per planned cut (per call; raw prefix tokens, cache-read dominated, not dollar-weighted)

| Step | Bytes removed | tok/call @3.6 | tok/call @fit | matched-pair rate (2.48) | Applies to | Per yesterday's 6,490 CLAUDE.md-carrying calls (@3.6 .. @fit) | Per plan's 2,360 calls (@3.6 .. @fit) |
|---|---|---|---|---|---|---|---|
| STEP 1: RTA CLAUDE.md 23,961 -> <= 8,000 B | 15,961 | 4.4k | 7.2k | 6.4k | main, builder, reviewer, architect, council, general-purpose (not Explore) | 28.8M .. 46.7M | 10.5M .. 17.0M |
| STEP 2: drop global ~/.claude/CLAUDE.md, keep Layer 0 in-project | 5,134 | 1.4k | 2.3k | 2.1k | same | 9.3M .. 15.0M | 3.4M .. 5.5M |
| STEP 2 if Layer 0 is lost too (not advised) | 7,389 | 2.1k | 3.3k | 3.0k | same | 13.3M .. 21.6M | 4.8M .. 7.9M |
| STEP 3: HANDOFF.md 291 KB -> <= 20 KB, birth-prompt section unchanged | 0 | 0 | 0 | 0 | none (not in a prefix) | 0 | 0 |
| STEP 3b (new): birth prompt 10,132 -> about 3,000 B | 7,132 | 2.0k | 3.2k | 2.9k | main only | 0.6M .. 1.0M (297 calls) | n/a |
| (new) workflow-authoring skill text not auto-loaded (17,276 B) | 17,276 | 4.8k | 7.8k | 7.0k | main only, primary too | 1.4M .. 2.3M (297 calls) | n/a |

Builder first call after STEP 1 + STEP 2: 29.6k -> about 20.1k (@fit: 29.6 - 7.2 - 2.3), i.e. -32%; at the 3.6 floor 29.6k -> 23.8k, -20%. Main first call: 54.1k -> about 44.6k @fit.

Caveats on the table: the per-session columns multiply a per-call prefix saving by call counts and assume the saving is a constant part of every call's cached prefix (true while the prefix is cached; a cache write costs more than a read, which this table ignores). The plan's "~9M" for STEP 1 equals about 2,360 calls x 3.8k tokens, between my @3.6 and @fit per-call figures; the real lever is the call count, which varies 2.8x between those two sources.

## 7. Ranked contributors

RTA main first call (54.1k): 1) harmony.md system prompt 34.5 KB = 9.6k..15.5k; 2) RTA CLAUDE.md 6.7k..10.8k; 3) workflow-authoring text 4.8k..7.8k; 4) birth prompt 2.8k..4.6k; 5) agent_listing 1.5k..2.5k (then global CLAUDE.md 1.4k..2.3k, skill_listing 1.2k..2.0k, hooks 1.3k..2.0k).
RTA builder first call (29.6k): 1) RTA CLAUDE.md 6.7k..10.8k; 2) builder spec 4.2k..6.8k; 3) task packet 13.7 KB = 3.8k..6.2k; 4) global CLAUDE.md 1.4k..2.3k; 5) session_context + env + date + attribution 1.1k..1.7k; layer0 0.6k..1.0k.

## 8. What I could NOT measure, and the cheapest way

- Tool JSON schemas per agent type: not in transcripts. Cheapest: two `claude -p "ok" --output-format json` runs in an empty scratch dir, same `--agent builder`, one with a stub spec, read `usage`; or the API count_tokens on a tools-only request. Cost: 2 tiny calls.
- Exact tokens per file (CLAUDE.md, global, layer0): all rates here are fits. Cheapest: in a scratch dir with only a CLAUDE.md, run `claude -p "ok"` with the RTA CLAUDE.md, then with an 8 KB cut, then with none (3 runs, read `usage`); gives an exact B/tok for CLAUDE.md prose.
- Whether `--settings`-supplied `claudeMdExcludes` (settings.secondary.json) reaches subagents and drops the file: same A/B with `--settings` and an `--agent builder`.
- Origin of the 30k primary figure in the plan: not found in the current configuration: 90 primary-session first calls, range 31.8k-43.3k, last 12 days 38.7k-43.3k, 30k only approached in early September. Ask the s243 author which session id / agent type it came from.
- What launches `/workflow-authoring` at boot (user message 18-19 of both main and primary; 17.3 KB): `ultra_effort_enter` precedes it and the primary's settings have `ultracode: true`, but RTA's settings do not set it, so the source is the launcher or the harness; not traced.
- Dollar weighting (cache write vs read vs uncached): not computed; all token figures are raw prefix sizes.
- Per-user-prompt costs: the gotcha-surface hook (2,411 B in main) re-fires on every user prompt; frequency not counted.

## 9. Reproduction

Scratch scripts (parse jsonl, never print whole files): `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a868a537-8200-4641-a63f-f3029b3cf7ee/scratchpad/` - `decomp.py` (per-piece bytes up to the first assistant usage record), `reg5.py` (the 461-point fit), `calc.py` (tables), `p13.py` (incremental fit). Source transcripts: main `/Users/boriskarpman/.claude/projects/-Users-boriskarpman-projects-RealTimeAudio/a868a537-8200-4641-a63f-f3029b3cf7ee.jsonl`; builder `.../a868a537.../subagents/workflows/wf_657aa03c-c9c/agent-a2685b00116d9e478.jsonl`; yesterday `.../73d4f54c-e9d5-409c-8b5d-694bfd57c171/subagents/workflows/wf_d50aebe5-9c9/agent-a30c924eeb36526bb.jsonl`; primary `/Users/boriskarpman/.claude/projects/-Users-boriskarpman-Harmony-Main/ebd1a1f6-95b2-4904-8aea-e23bc9976512.jsonl`.
