<!-- Primary→Project down-channel inbox (route-down.sh). Verbatim ROUTED-ITEM records (C-boundary-ruling.md s148 §2b). Appended ONLY by the Harmony primary via scripts/route-down.sh; this repo's own session flips status: SENT -> TAKEN/DONE/DECLINED by hand (single-writer-per-direction: primary appends whole records, project edits only status lines of existing records). Read at boot via hooks/session-start.sh's [inbox] banner (session-lifecycle-startup.md MINIMAL Step 1b). -->
--- ROUTED-ITEM ---
id:          down-2026-08-30-RealTimeAudio-17881102275772810473
raw:         RealTimeAudio norm drift (HEAVY: 116 feature-affecting files / 121 commits) is FAILing Harmony's integration score. Ruled out-of-scope for the primary twice (s127/s128) and it recurred; it needs this repo's own session.
origin:      memory/loose-ends-ledger.md:235,3491
why-routed:  project norm drift — only an RTA session can fix it
source-idea: 
routed-by:   harmony-13732     date: 2026-08-30
status:      DONE
status-note: s-rta-0904: HEAVY drift CLOSED. 127 commits re-traced by a 7-lane read-only swarm + critic + fenced writer + independent verifier. 6 CRITICAL corrections (largest found by 3 lanes independently: the doc claimed undo/redo was a no-op while a full Undo v1 had shipped). Every count re-derived from source. state.md last_normalized_sha -> d93e6ba. Reports in .audit/renorm-2026-09-05/.
--- /ROUTED-ITEM ---

--- ROUTED-ITEM ---
id:          down-2026-08-30-RealTimeAudio-1788110227577472721
raw:         6 gotcha entries in this repo's .harmony/gotchas.md have schema drift invisible to the promotion scan. Harmony owns the schema and the scan; the entries are this repo's file content.
origin:      memory/session-handoff.md:52
why-routed:  per-repo gotcha file content
source-idea: 
routed-by:   harmony-13732     date: 2026-08-30
status:      DONE
status-note: s-rta-0904 commit a4e2efd. The "6 entries" is right for one failure mode and understates the truth: hygiene.sh greps LITERAL "Promoted: no"/"Scope: universal" and every entry used bold markdown, so ALL 31 were invisible, not 6. 136 field labels converted, 0 content changed. 8 entries still lack Scope/Promoted outright and were NOT guessed — listed for Boris.
--- /ROUTED-ITEM ---

--- ROUTED-ITEM ---
id:          down-2026-08-30-RealTimeAudio-17881102275776627736
raw:         USAGE QUERY (reply via scripts/idea-capture.sh): which of graphify-rta / codegraph-rta / clangd-rta does your workflow actually need? Harmony was about to remove all three as 'never called', but config/mcp-catalogue.md:13 marks clangd-rta the AUTHORITATIVE impact/blast-radius source for this repo — 'never called' means no RTA session ran in the window, not dead. Removal of clangd-rta is DEFERRED pending your answer.
origin:      memory/session-handoff.md:31 (b) L407
why-routed:  the repo that uses the servers decides
source-idea: 
routed-by:   harmony-13732     date: 2026-08-30
status:      DONE
status-note: s-rta-0904 answered up-channel, idea-2026-09-04-harmony2-17885803531987214855. clangd-rta KEEP (sole authority for C3/blast-radius), codegraph-rta REMOVE (stale, no refresh, catalogue-admitted overlap), graphify-rta KEEP entry. Note: graphify DATA PIPELINE (post-commit hook + launchd) is alive; the MCP SERVER is what was never called — the scan conflates them.
--- /ROUTED-ITEM ---

--- ROUTED-ITEM ---
id:          down-2026-08-30-RealTimeAudio-17881102275778519984
raw:         Re-normalize flagged due since 08-10 by the s147 EOD (kill-date 2026-08-01; all registered projects flagged). Verify against this repo and run the normalize skill in your own session.
origin:      memory/session-handoff.md:31 (c)
why-routed:  per-repo normalize
source-idea: 
routed-by:   harmony-13732     date: 2026-08-30
status:      DONE
status-note: s-rta-0904: normalize-check.sh --framework game now reads STATUS: CURRENT ("1 commits since normalize (all chore/docs - no code changes)") + validator COMPLIANT PASS=265 FAIL=0. PRECISION NOTE: the tool never emits the literal word "NORMALIZED" - its vocabulary is CURRENT / NEEDS FIXES / NOT NORMALIZED - so the acceptance wording as written is unsatisfiable; CURRENT is the achievable best and that is what it reads. Under the tool DEFAULT (--framework web) it still reads NEEDS FIXES on 3 false-positive FAILs; fix filed up-channel as idea-2026-09-04-harmony2-17885803732009828310.
--- /ROUTED-ITEM ---

--- ROUTED-ITEM ---
id:          down-2026-08-30-RealTimeAudio-17881102275780412232
raw:         Commit this repo's uncommitted .harmony/ edits (pointer rewrites + s147 promotion markers, plus this inbox file).
origin:      memory/session-handoff.md:52
why-routed:  project-scoped .harmony commit
source-idea: 
routed-by:   harmony-13732     date: 2026-08-30
status:      DONE
status-note: s-rta-0904 commit 5ab9010 (gotcha promotion markers + CHECKPOINT) and a4e2efd (the rest). Tree dirty-count was 268 at report time.
--- /ROUTED-ITEM ---

--- ROUTED-ITEM ---
id:          down-2026-09-02-RealTimeAudio-178839673230861865
raw:         Normalize Phases 1-2 (secrets scan + gap analysis) ran from the primary on 2026-09-02 (s158) — report at /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/N12-rta.report.md with a BORIS PICKS checklist. At your next boot: read it, tick the picks with Boris, run the normalize skill's Phase 2.5/3 for the ticked items, then normalize-check.sh must read NORMALIZED. Commit-or-clean the dirty tree first (report section DIRTY TREE).
origin:      core/WORK_INDEX.md (s158 BORIS mid-session task: update these secondaries)
why-routed:  Boris 2026-09-02: get the secondaries using the current Harmony template
source-idea: 
routed-by:   harmony-57856     date: 2026-09-02
status:      DONE
status-note: s-rta-0904: both REQUIRED picks done; RECOMMENDED done except the graphify cache-tracking call (recon agent never reported - carried). Backups: 12 found not 8, 11 byte-identical deleted, 1 divergent archived to .harmony/.archive/. UNCLEAN-CLOSE-STAMP deliberately KEPT as the live reproducer for a detector defect filed up-channel (idea-2026-09-04-harmony2-17885803732007819224).
--- /ROUTED-ITEM ---

--- ROUTED-ITEM ---
id:          down-2026-09-06-RealTimeAudio-17887267064132214072
raw:         REMOVE the codegraph-rta MCP server. Verdict is your own repo's (up-channel idea idea-2026-09-04-harmony2-17885803531987214855, from RTA session s-rta-0904): codegraph-rta = REMOVE (stale DB, no refresh mechanism, zero use, purpose never differentiated against graphify-rta by a live comparison); clangd-rta = KEEP (sole authoritative C3 blast-radius source); graphify-rta = KEEP (data pipeline is alive and self-maintaining via post-commit hook + launchd; only the MCP SERVER is unqueried — do not conflate those). TWO EDITS, BOTH IN YOUR REPO: (1) delete the codegraph-rta block from ~/projects/RealTimeAudio/.mcp.json; (2) remove codegraph from ~/projects/RealTimeAudio/.harmony/knowledge-tools.yml AND re-probe its stale graphify node/edge counts (GRAPH_REPORT.md reads 5297N/8466E; knowledge-tools.yml still cites a 2026-07-02 probe at 2416N/3779E/275C, and the on-disk graph is ~1 month / 3 commits behind HEAD). AFTER your edit lands, the primary re-runs scripts/mcp-catalogue.sh --render and the catalogue row disappears on its own.
origin:      memory/session-handoff.md:111 MUST-2(c)
why-routed:  config/mcp-catalogue.md is a GENERATED render (scripts/mcp-catalogue.sh --render) and --check asserts every disk server is catalogued. Deleting the row from the render while ~/projects/RealTimeAudio/.mcp.json still declares codegraph-rta creates an awareness gap that hygiene.sh flags as WARN (scripts/hygiene.sh:6329). The removal must happen at the SOURCE, which is your .mcp.json — a foreign repo the primary must not commit to. Handing it to its owner, not deferring it a fifth time.
source-idea: idea-2026-09-04-harmony2-17885803531987214855
routed-by:   harmony-21985     date: 2026-09-06
status:      SENT
status-note:
--- /ROUTED-ITEM ---
