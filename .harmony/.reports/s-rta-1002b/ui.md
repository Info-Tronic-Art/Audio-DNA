# LANE ui -- stage U1 (BF3 data) -- builder report
STATUS: PENDING
Worktree: /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/ui  branch lane/ui  base eff2b1c

## Items
### U1.1 VideoInfo.h (+ AM9 codecLine / frameLine) -- DONE
- NEW src/media/VideoInfo.h (std only): struct VideoInfo {codec, width, height, fps; known()}; namespace videoinfo:
  fourcc(a,b,c,d) (MKTAG order, = FFmpeg codec_tag), codecLabel(ffName, fourcc, profile) (plan table: H.264 / HEVC +
  profile; ProRes by profile, else FourCC apco/apcs/apcn/apch/ap4h/ap4x, else "ProRes"; HAP by FourCC
  Hap1/Hap5/HapY/HapM/HapA, else "HAP"; DXV, Motion JPEG, MPEG-4 Part 2, MPEG-2, QuickTime Animation, PNG, Uncompressed,
  VP8, VP9, AV1, CineForm; DNxHD or "DNxHR <grade>"; else the FFmpeg name upper-cased; empty name -> ""),
  fpsText (integer milli-rounding, no locale; <= 0 / NaN / huge -> ""), codecLine, frameLine, describe
  (= codecLine + ", " + frameLine).
- NEW tests/test_video_info.cpp [videoinfo]: 8 TEST_CASEs / 61 assertions (every table row + fallbacks, fpsText incl.
  30000/1001, 24000/1001, 60000/1001, 25, 30, 85/4, 0, -1; codecLine / frameLine / describe with and without a rate).
- tests/CMakeLists.txt: target test_video_info appended (test_video_player_open's link recipe; TEST_FIXTURES_DIR).
- RED(stub) 2026-10-02 16:23:09 (stub bodies return {}), raw:
    test cases:  8 |  8 failed
    assertions: 61 | 6 passed | 55 failed
  (the 6 passes = the "" expectations: codecLabel("") x2, fpsText(0 / -1), known() true/false on the struct.)
- GREEN 16:23:35, raw: All tests passed (61 assertions in 8 test cases)
- ctest -N lists the 8 cases (#1114-#1121).
### U1.2 VideoPlayer open-time info + getter (+ AM11 fixtures)
(pending)

## Builds / tests

## Rig discipline

## Notes for .harmony/notebook.md

## PACKET QUALITY
