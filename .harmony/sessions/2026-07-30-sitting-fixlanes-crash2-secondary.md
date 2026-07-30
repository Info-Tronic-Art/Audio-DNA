# Session log — 2026-07-30 PM (secondary, slim boot over ~/Harmony, work repo RealTimeAudio)

| field | value |
|---|---|
| role/profile | secondary · SLIM (~/Harmony) · primary seat held elsewhere |
| lane | undo-v1 TAIL: Boris sitting → 4 fix lanes → crash-#2 true root cause |
| HEAD at open | 303178c (appgate-drive chore) — src clean |
| HEAD at close | 9b74c7d + this close's chore commits (6 source commits this session) |
| gauge | boot ~2% → close ~30% (drain off-ramp respected) |

## Timeline (terse)

| t | event | verdict |
|---|---|---|
| boot | Receiver-verify: app HEALTHY (117fps, **port 7070** — 8888 was a bad guess); cert ABSENT | order-flip held (sitting first) |
| 12:1x | namebar-scout: cell geometry cracked (name bar = bottom 20px of 90x96; displayRow INVERTED; drag >5px continuous; Clip>Clear handler-gated) + fold-height MIRROR-INDEX bug found (DeckView.cpp:274) → queue (8) | scout-namebar-geometry.md |
| 12:3x | Drive STOOD DOWN — HID <1s, Boris live at machine; stale-active anomaly observed → queue (9) | no synthetic events into occupied seat |
| 12:4x | **BORIS SITTING RESULTS**: 6 PASS · 1 PASS* (drag-move; checklist column-clause corrected) · PARTIAL (FX drop targets) · BLOCKED ×2 (quantize/retrigger-ignored; MilkDrop empty) · X-clear-keeps-playing bug · process rule: taste checks ONE per ask | checklist annotated per item |
| 12:5x | Cert: created right, CSSMERR_TP_NOT_TRUSTED → user-domain add-trusted-cert (no sudo) → 1 valid identity | recipe → log-event |
| 12:54 | MilkDrop lane: 9229f87 (selectPreset null-deref PROVEN fixed; presets bundle-copy — build-system gap; wiring was correct). Review APPROVE | 176/176 |
| 13:0x | sitting-triage-scout: 4 findings root-caused (FX targets map; retrigger EMERGENT no-op; FilesBrowser sync full-res decode; clear-path TWO roots) | scout-sitting-triage.md |
| 13:02 | Behavioral gate on MilkDrop lane: relaunch (quit-crash discovered → later candidate 10), SIGKILL doctrine adopted, **zero TCC clicks (cert inherited mic grant)**, presets populated, SignalBar arrow probe survived — but probe ARMED the then-unknown UAF | PASS at the time; see 13:25 |
| 13:2x | Lane A (a718572): X-clear renderer purge (rescan-or-purge, mirrors handleColumnTrigger) + kClipClear→genuine nullopt + activeClipColumn reset in same composite. Review APPROVE-WITH-NOTES → HIGH: 2 sibling paths missed purge → fix round 20fe75d (+11) → delta APPROVE | 177/177 |
| 13:25 | **Boris live crash**: getCuratedPresets on the FIXED binary, presets populated → empty-state theory DEAD | .ips 132512 |
| 13:4x | Lane B (9c316e6): CompositionInspector panel-wide FX drop target (LayerInspector mirror). Review APPROVE 0 issues. Sweep: multi-select FX = ONE undo entry all scopes | 177/177 |
| 13:5x | Lane C (8c746ab): FilesBrowser async decode + LRU cache + instant toggle (+4 tests). Review APPROVE-WITH-NOTES → fix round 9b74c7d (explicit dtor drain + read-time mtime key, +1 test) → delta APPROVE | 182/182 |
| 14:0x | shutdown-crash-scout (candidate 10): SIGBUS = single-word corruption in live Label DISCOVERED at teardown; writer unidentified; 2 candidate writers (unsynced EffectChain GL-vs-timer; compacted-index :472); fixes ranked (detach-GL-first 1-liner) | scout-shutdown-sigbus.md |
| 14:1x | **milkdrop-race-scout: crash #2 TRUE root cause — deterministic UAF** (preview hide → sync GL detach → activeSources_.clear() → preset manager destroyed → browser interior pointer dangles). Disassembly-exact both .ips. My 13:02 probe = the arming event (INFERRED). Also explains presets-vanish. NEW: activeSources_ 3-thread no-mutex → queue (11) | scout-milkdrop-uaf.md |
| 14:2x | UAF fix 76594fd (keep releaseGL loop, delete clear(); 6 verifications). Review APPROVE 0 issues | 181/181 then 182/182 at 9b74c7d |
| 14:2x | Gate attempt: snapshotted Boris state, relaunched onto fixed binary (91888 healthy) — then ABORT: Boris returned silently, first click landed in his Firefox (no action taken; disclosed) → gotchas rule (11) SYNTHETIC-CLICK PREFLIGHT (fresh HID + frontmost check same-command) | doctrine written |
| close | Boris EOS + request: test-list for his hands, feedback next session | this close |

## Artifacts
- Source commits: 9229f87 · a718572 · 20fe75d · 9c316e6 · 8c746ab · 76594fd · 9b74c7d (all locally committed, NOT pushed; every one independently reviewed)
- Chore commit 2b37883: ledger PM2-PM20, checklist sitting annotations, 4 scout dossiers, gotchas (7)-(11), Boris state snapshot
- APP-INVENTORY reconciled (3 surface rows + test count 182) this close
- harmony2 writes: .events learning append ×2 ONLY (lane-A telemetry, sanctioned; same posture as prior session log); ZERO system files → eos-secondary correct
- Work-repo resolution note: touched-repos.sh returned 7 globally-dirty repos; REPO_ROOT fixed to RealTimeAudio on session-evidence (all 8 commits + all writes this session are here; birth prompt scoped here) — not a guess

## Meta-learnings (distilled from in-session captures)
- Boris directive: perceptual/taste checks ONE item per ask (batched 6-item list rejected) — division-of-judgement per-unit routing, now also a log-event row
- Self-signed cert trust recipe (find-identity WITHOUT -v to see reason codes; user-domain add-trusted-cert, no sudo) — log-event row
- SYNTHETIC-CLICK PREFLIGHT (gotchas 11): fresh HID + frontmost-app check in the SAME command before every burst; stale idle checks are worthless
- SIGKILL disposable app instances (gotchas 10): graceful quit traverses a corruption-discovery teardown path
- Explore agents go idle WITHOUT transmitting — every scout contract now needs an explicit "SendMessage to main when done" line (nudged twice before adopting)
- Green gate ≠ proof at the right layer: model-level oracles (api/composition) passed clears while the RENDERER kept playing — behavioral gates must watch the layer the user sees
- A "fixed" crash symbol recurring is EVIDENCE, not noise: the 13:25 recurrence on the guarded binary is what killed the wrong theory and forced the real root cause
