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
status:      DONE
status-note: s-rta-0923: codegraph-rta block deleted from .mcp.json; codegraph entry removed from .harmony/knowledge-tools.yml; graphify re-probed 7254N/11265E/373C (graph had been a 0-node file since 2026-09-07). Primary may now re-render mcp-catalogue.
--- /ROUTED-ITEM ---

--- ROUTED-ITEM ---
id:          down-2026-09-10-RealTimeAudio-17890807759054610371
raw:         Decide the disposition of this repo's graphify git hooks (.git/hooks/post-commit + post-checkout, installed by 'graphify hook install'). Context: a prior primary-seat session let these rebuild the code graph on RTA commits and one wrote a 0-node graph over the void. They are project-local tooling — Harmony's primary no longer touches this repo (see charter). Options: (a) keep + fix the empty-graph failure mode, (b) 'graphify hook uninstall' to remove them. Related tool bugs already filed as Harmony candidates: graphify-commit-hook-writes-empty-graph, graphify-staleness-blind-to-missing-outputs.
origin:      memory/session-handoff.md MUST-1(a)
why-routed:  git hooks live in the RTA project repo; graphify is a project-local tool integration, not a Harmony daemon — the owning session decides
source-idea: 
routed-by:   harmony-98613     date: 2026-09-10
status:      DONE
status-note: s-rta-0923: KEEP (a). The hook runs `graphify update` which refuses to overwrite a graph with fewer nodes unless --force; the 0-node graph came from the Harmony launchd wrapper deleting graph.json BEFORE extracting (void, so no guard applies) — that wrapper defect is Harmony-side, filed up-channel. Hook behaviour re-checked on this session's first commit (see HANDOFF).
--- /ROUTED-ITEM ---

--- ROUTED-ITEM ---
id:          down-2026-09-10-RealTimeAudio-1789082994142806700
raw:         Deregister the stale MCP server 'codegraph-rta' from this repo's .mcp.json. It is TODO-purpose / long-lived and never differentiated from graphify-rta (the two overlap on symbol/reference scope; graphify-rta + clangd-rta cover C++ symbol/impact analysis authoritatively per knowledge-tools.md). Once removed from .mcp.json, a future Harmony primary re-renders config/mcp-catalogue.md and the entry drops automatically. Verify: 'codegraph-rta' no longer appears in ~/projects/RealTimeAudio/.mcp.json.
origin:      memory/session-handoff.md MUST-1(c)
why-routed:  the server is registered in RTA's own .mcp.json (a project repo file) — deregistering it is a project-repo action; the primary only maintains Harmony's catalogue, which reflects disk truth
source-idea: 
routed-by:   harmony-98613     date: 2026-09-10
status:      DONE
status-note: s-rta-0923: duplicate of the 2026-09-06 item — codegraph-rta no longer in .mcp.json (grep -c = 0).
--- /ROUTED-ITEM ---

--- ROUTED-ITEM ---
id:          down-2026-09-11-RealTimeAudio-17891312102907024262
raw:         One repo's gitignore hides the unclean-close mechanism from itself: ~/projects/RealTimeAudio/.gitignore:62 ignores .harmony/, suppressing UNCLEAN-CLOSE-STAMP-*.md in this repo only (Clean Copy and t shirt 2 do not suppress it). Pre-existing since 2026-05-18. Also self-contradictory: the repo both ignores .harmony/ AND force-tracks ~20 files inside it. Decide deliberately whether .harmony/ artifacts are tracked; either way the stamp pattern should be visible.
origin:      memory/system-upgrade-candidates.md:7320 (rta-gitignore-hides-unclean-close-stamp)
why-routed:  project-config fix scoped to the RealTimeAudio repo's own .gitignore; routed s174 backlog reduction
source-idea: 
routed-by:   harmony-4288     date: 2026-09-11
status:      DONE
status-note: s-rta-0923: .gitignore `.harmony/` -> `.harmony/*` + `!.harmony/UNCLEAN-CLOSE-STAMP-*.md`. Proven: a probe stamp file shows as ?? in git status; other new .harmony files stay ignored; force-tracked files unaffected. Tracking policy kept as-is (ignore + force-track deliberate artifacts).
--- /ROUTED-ITEM ---

--- ROUTED-ITEM ---
id:          down-2026-09-23-RealTimeAudio-1790207563485530421
raw:         Re-enable the knowledge-tool MCP servers this repo's own rulings say to KEEP. .claude/settings.local.json (mtime 2026-07-29) lists graphify-rta, codegraph-rta AND clangd-rta under disabledMcpjsonServers, so none load — yet the 2026-09-04 ruling (this inbox, s-rta-0904 DONE record) makes clangd-rta the SOLE authority for C3/blast-radius impact and keeps graphify-rta. Do this together with the codegraph-rta removal items already SENT above: drop codegraph-rta from .mcp.json + knowledge-tools.yml, then remove clangd-rta and graphify-rta from disabledMcpjsonServers. If Boris disabled them deliberately (settings.local.json is user-local), ask him before re-enabling.
origin:      s228 audit wf_285b0069-cb3 gap G3 (confirmed 2/2 verifiers)
why-routed:  settings.local.json + .mcp.json are RTA repo config; the project session owns them (s172 charter)
source-idea: 
routed-by:   harmony-95977     date: 2026-09-23
status:      TAKEN
status-note: s-rta-0923: codegraph-rta dropped from .mcp.json, knowledge-tools.yml and the disabled list. Re-enabling clangd-rta + graphify-rta is HELD for Boris — settings.local.json is user-local and may be deliberate; asked in the close.
--- /ROUTED-ITEM ---

--- ROUTED-ITEM ---
id:          down-2026-09-23-RealTimeAudio-1790207563487422669
raw:         Stop using .harmony/idea-ledger.md as a notebook. It holds 3 canonical --- IDEA --- records (all delivered to Harmony) plus ~17 '### ' / 10 '## ' prose sections from s-rta-0904, s166, s167, s168 (rig facts, product defects, retractions, method notes). Prose sections NEVER transport to the Harmony primary. Triage each: project findings/defects -> .harmony/notebook.md or gotchas.md (or a fix task); anything genuinely aimed at changing HARMONY (method/process lessons) -> re-file via ~/Harmony_Main/scripts/idea-capture.sh so it reaches the primary. From Harmony s228 on, the secondary close gate WARNs on any off-schema heading in this file. Also: the eos-secondary close requires a session log in THIS repo (.harmony/sessions/ or memory/sessions/) — sessions s-rta-0904..s168 (Sep 4-7) wrote none; the close gate was wrongly counting Harmony's own logs and is being fixed in s228, so the next close without an RTA session log will BLOCK.
origin:      s228 audit wf_285b0069-cb3 gaps G12/G14 + follow-up M1/M2
why-routed:  idea-ledger.md and session logs are RTA repo content; the project session triages them (s172 charter)
source-idea: 
routed-by:   harmony-95977     date: 2026-09-23
status:      DONE
status-note: s-rta-0923: prose sections moved VERBATIM to .harmony/notebook.md (loss check: 507/507 non-blank lines present in new homes); Harmony-aimed lessons re-filed as canonical IDEA records; ledger now holds only --- IDEA --- records. Session log written: .harmony/sessions/2026-09-24-s-rta-0923-secondary.md.
--- /ROUTED-ITEM ---

--- ROUTED-ITEM ---
id:          down-2026-09-24-RealTimeAudio-17902484309157426959
raw:         s230 G5 live proof for the report-path fix (Harmony 93e969b1): run the two architect dispatches in ~/Harmony_Main/memory/.reports/s230/g5-relay-packet.md (+ probe to .harmony/.reports/, − negcontrol to .harmony/specs/), report both outcomes up, then remove the empty stray <repo>/memory/.reports/ dir.
origin:      
why-routed:  Fable ruling §4 G5: sandbox rc alone does not close this family; L3 harness delivery on CC 2.1.280 unproven
source-idea: 
routed-by:   harmony-18791     date: 2026-09-24
status:      DONE
status-note: s-rta-0924: (+) PASS, (-) INCONCLUSIVE for the hook (architect self-refused; hook never reached); reviewer Bash-bypass of MINIMAL write BLOCK found; stray memory/ removed. Reported up: memory/.pending s-rta-0924-g5-relay-result.md.
--- /ROUTED-ITEM ---
