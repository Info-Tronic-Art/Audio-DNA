# UNCLEAN-CLOSE STAMP (deterministic close-gate-omission marker)

> Auto-written by close_omission_stamp_check() (lib/close-omission-stamp.sh)
> on SessionEnd when the calling hook could not confirm a passing close-gate
> artifact for this session. DETECTION ONLY (memory/DECISION_LOG.md
> "ENFORCEMENT RECON" 2026-08-17, RULE 1) — a CHECKPOINT.md snapshot (where
> one exists) nets any data loss regardless of whether EOS ran; this stamp
> only means the session ended without a VERIFIED EOS. Consumed (deleted) by
> lib/unclean-close-detect.sh the next time it is surfaced at boot.

instance: harmony-22658
date: 2026-09-06
status: closed without EOS
reason: EOS-COMPLETE.md receipt is for a different instance (harmony-68183)
repo: /Users/boriskarpman/projects/RealTimeAudio
