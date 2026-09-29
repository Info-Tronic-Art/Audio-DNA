import sys
W='/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928b-media'
p=W+'/src/media/VideoPlayer.cpp'
s=open(p).read()
def rep(a,b,cnt=1):
    global s
    assert s.count(a)==cnt, (a, s.count(a))
    s=s.replace(a,b)
rep('#include "VideoPlayer.h"\n', '#include "VideoPlayer.h"\n#include "diag/DiagTrace.h"\n')
# open(): message-thread sub-scopes
rep('    // Open input\n    if (avformat_open_input', '    diag::Scope dgOpen("vp.open.total", -1);\n    // Open input\n    if (avformat_open_input')
rep('    // Decode first frame\n    currentTime_ = 0.0;', '    // Decode first frame\n    diag::Scope dgFirst("vp.open.firstFrame", -1);\n    currentTime_ = 0.0;')
# seek
rep('''    auto* stream = formatCtx_->streams[videoStreamIndex_];
    int64_t timestamp = static_cast<int64_t>(timeSec / timeBase_);
''','''    auto* stream = formatCtx_->streams[videoStreamIndex_];
    int64_t timestamp = static_cast<int64_t>(timeSec / timeBase_);
    const double dgT0 = diag::nowMs();
    struct DgSeek { double t0; ~DgSeek() { diag::gl().vidSeek++; diag::gl().vidSeekMs += diag::nowMs() - t0; } } dgSeek{ dgT0 };
''')
# decodeNextFrame
rep('''    if (!formatCtx_ || !codecCtx_ || !decodedFrame_ || !packet_)
        return false;

    while (true)''','''    if (!formatCtx_ || !codecCtx_ || !decodedFrame_ || !packet_)
        return false;
    struct DgDec { double t0; ~DgDec() { diag::gl().vidDec++; diag::gl().vidDecMs += diag::nowMs() - t0; } } dgDec{ diag::nowMs() };

    while (true)''')
# convert: sws + flip
rep('''    // Convert to RGBA
    sws_scale(''','''    const double dgC0 = diag::nowMs();
    // Convert to RGBA
    sws_scale(''')
rep('''    // Flip vertically for OpenGL (bottom-to-top)
    // rgbaFrame_ data is top-to-bottom, we need bottom-to-top
    int rowBytes''','''    const double dgC1 = diag::nowMs();
    diag::gl().vidConv++; diag::gl().vidConvMs += dgC1 - dgC0;
    struct DgFlip { double t0; ~DgFlip() { diag::gl().vidFlipMs += diag::nowMs() - t0; } } dgFlip{ dgC1 };
    // Flip vertically for OpenGL (bottom-to-top)
    // rgbaFrame_ data is top-to-bottom, we need bottom-to-top
    int rowBytes''')
# upload
rep('''    if (frameBuffer_.empty() || frameBufferWidth_ <= 0 || frameBufferHeight_ <= 0)
        return 0;

    if (!textureCreated_)''','''    if (frameBuffer_.empty() || frameBufferWidth_ <= 0 || frameBufferHeight_ <= 0)
        return 0;
    static const bool dgOnlyNew = diag::flag("DIAG_VID_UPLOAD_ONLY_NEW");
    if (dgOnlyNew && textureCreated_ && !dgNewFrame_)
        return texture_;   // counterfactual arm: re-upload only a newly decoded frame
    dgNewFrame_ = false;
    struct DgUpl { double t0; ~DgUpl() { diag::gl().vidUpl++; diag::gl().vidUplMs += diag::nowMs() - t0; } } dgUpl{ diag::nowMs() };

    if (!textureCreated_)''')
# mark new frame whenever convertFrameToRGBA runs
rep('''    if (!decodedFrame_ || !rgbaFrame_ || !swsCtx_)
        return;
''','''    if (!decodedFrame_ || !rgbaFrame_ || !swsCtx_)
        return;
    dgNewFrame_ = true;
''')
# thumbnail
rep('''    if (!open_.load(std::memory_order_relaxed) || frameBuffer_.empty())
        return {};

    // Create JUCE Image''','''    if (!open_.load(std::memory_order_relaxed) || frameBuffer_.empty())
        return {};
    diag::Scope dgThumb("vp.getThumbnail", -1);

    // Create JUCE Image''')
open(p,'w').write(s)
h=W+'/src/media/VideoPlayer.h'
t=open(h).read()
a='    bool frameReady_ = false;\n'
assert t.count(a)==1
t=t.replace(a, a+'    bool dgNewFrame_ = true;   // DIAGMEDIA temporary\n')
open(h,'w').write(t)
print("ok")
