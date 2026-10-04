# Reviewer Verdict - bf2 gates r1 (S3f)
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS) -- no MUST. Reviewed head 101dcab51ce0279377f2644cd01f942059a8a383 (base c45b579), through git objects only. No build, no test run, no probe run (as instructed).
FILES: .harmony/probe-sync.py, .harmony/probe-sync.sh, .harmony/probe-tsan-unit.sh, tests/CMakeLists.txt, tests/test_analysis_sync_thread.cpp, tests/test_beat_lead.cpp, tests/test_recorder_host.cpp, tests/test_sync_witness.cpp, src/analysis/{AnalysisThread.cpp,BeatLead.cpp,BeatLead.h,SyncWitness.h}, src/api/ApiServer.cpp, docs/claude/{analysis,integration,recording}.md, .harmony/APP-INVENTORY.md, lane report bf2-delta.md (S3f section)

## Row by row (VERIFIED = read at the pinned head)
- R7 (D3) VERIFIED probe-sync.py:912-974. Drop rule = 21.333 ms from the arm's median (offs are in ms from the take's own rate, so same as 1,024 samples at 48k); MEAN of the rest; dbar = mean(10 x 100-arm) - mean(11 x 0-arm); SE = sqrt(var(H)/nH + var(Z)/nZ) with sample variance; VALID = every arm n >= 100 and drops <= 4 (:937); bar 1.0; extension test is |abs(dbar)-1.0| <= 2 SE or SE > 0.33 (:953), once, pooled 42 by the same formulas, pooled SE > 0.33 = INCONCLUSIVE; |dbar| < 2.67 FAIL = "never re-run". Arms = [0,100]x10+[0] (:1070); extension refuses a subset or a non-21 prior (:1076-1085). Empty arm -> n = 0 -> INVALID (cannot pass empty).
- R7b (D4) VERIFIED :1105-1195. Fires at markers 10/20/30 from perf/status "markers"; g_k = gesture - first sample - grid position nearest raw marker k; B = median over ALL 0-arm paired errors (:1179); bar lo/hi = -0.032/+0.080 x rate = -1536/+3840 at 48k; median of the three; all three printed; (i) INFO only. 3 fired + 3 found required; missing 0 arm / 100 arm / markers -> FAIL.
- R5 (D6) VERIFIED :851-872. Antecedent unchanged (leadApplied pair, consecutive, not re-base, same barsAdvance, bpm>0, raw advance >= aN-1e-4); count printed; bar qualifying >= 5000 and 2q >= applied; applied == 0 fails every R5 line. Default arms and 61 s unchanged (:761-768); knobs only add a subset and print "DEVELOPMENT SUBSET".
- R1a over -500 (:1244-1252 area, row_r1a label/key) VERIFIED: (ii) 99.5 % and (iii) bound unchanged, no iterations -> FAIL, R4 without a -500 window -> FAIL.
- G6 (D2) VERIFIED :977-1031. Windows >= 150 hops else FAIL (:993); missing pipelineUs on the first hop -> FAIL; bar (1) leadApplied >= 99 % of the -500 window via h.get (missing field = 0 -> FAIL); bar (2) mean <= 2000 in all 8 windows (5 R1 0 arms, R1 500, R4 0 step, R4 -500); over-budget fallback: bar (2) replaced by mean(-500) <= mean(R4 0)+300 and mean(+500) <= mean(2 neighbours)+300, the second only when the two neighbours differ < 100, else INCONCLUSIVE (a FAIL line). Exactly as written. Needs R1 and R4 in the same run or FAILs (:1036).
- pipelineUs (D2) VERIFIED: AnalysisThread.cpp:570 stores elapsedUs (the pipeline from :211 through BeatLead :447-461 to :468, so BeatLead's cost IS inside it), inside the AUDIODNA_TEST_SERVER block (:526...); SyncWitness member and header include are test-server only (AnalysisThread.h:13, :122, :159), production unaffected. Struct 144->152 with explicit pad; STATIC_REQUIRE'd; EXPECTED_TSAN_CASES 11->12 (test_sync_witness is a tsan target).
- D8 VERIFIED CMakeLists.txt:3932-3933: "~[tsan]~[timing]" + "[timing]" RUN_SERIAL; the case really carries the [timing] tag (test_analysis_sync_thread.cpp:553). Pair line printed on every run (:677); arm lines already printed; bars 0.7/1.3, sd 2.0, 148/152 untouched.
- D5/D6 unit VERIFIED test_beat_lead.cpp: T-L6b drives the real BeatLead + BPMTracker, asserts the fold step == tracker step +-1e-5, bsr K-1, totalBarCount unchanged, back to K after re-cross; T-L3 (v) restated per the ruling's text plus an every-hop origin==barX check; (iv) count printed and 2q >= nonRebase asserted. BeatLead.cpp: the cap and its helper removed, `originX_ += absorbed` only.
- D7 VERIFIED test_recorder_host.cpp:2752-2814: three tail points, replayed with audio at +300, fire at the end edge in stamp order then finished, second tick fires nothing. The existing tail case asserts count only, so a new case was warranted (as the lane says).
- Script safety: probe-sync.sh teardown is quit_ours / refuse_foreign_start (unchanged by S3f; only header lines and the default row list changed). Nothing in S3f can quit an app it did not start. Mutant app path goes through the same PROBESYNC_APP + own-pid rig.

## RED on record (lane report, S3f progress log) -- VERIFIED present, not re-run
- T-L6b on the capped code: "published fold step 3.703999 beat", 1 case / 3 assertions failed -> GREEN -0.296001. STOP RULE not triggered.
- R7 + R7b on the mutant app (build-mut-r7): R7 SUBSET +99.511 ms FAIL; R7b g_k - B +4736 / +4352 / +5504 (all > +4300) FAIL. `grep -rc MUTANT src tests` = 0 and no MUTANT in the head tree (re-grepped by me at the head: no hit).
- D7: mutant (tail advance removed) "1 == 4"; M3 (markers +4800) 3 assertions fail.
- R5 count: 12 s arms -> "1246 of the 1247 ... FAIL" while the three older hold lines PASS.
- pipelineUs: compile RED (no member / static assertion).
- G6 bars (1)/(2) and the over-budget / <150 / missing-field paths: RED on SYNTHETIC windows only (offline.py section D) -- see finding 2. Live GREEN printed the real fields (282 of 282; 8 x G6(2)).

## Findings
(see structured output; no MUST)
