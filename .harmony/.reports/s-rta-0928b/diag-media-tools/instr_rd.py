W='/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928b-media'
p=W+'/src/render/Renderer.cpp'
s=open(p).read()
def rep(a,b,cnt=1):
    global s
    assert s.count(a)==cnt, (a[:60], s.count(a))
    s=s.replace(a,b)
rep('#include "Renderer.h"\n', '#include "Renderer.h"\n#include "diag/DiagTrace.h"\n')
rep('''    } callbackPeak{ peakCallbackMs_ };
''','''    } callbackPeak{ peakCallbackMs_ };
    // DIAGMEDIA: one [GF] line per frame (every return path), the per-frame media accumulators.
    struct DgFrame
    {
        double t0 = diag::nowMs();
        ~DgFrame()
        {
            auto& a = diag::gl();
            std::fprintf(stderr, "[GF] %.3f %.3f %.3f %.3f %d | v %d %d %d %d %.3f %.3f %.3f %.3f %d %.3f %.3f %.3f | e %d %.3f | s %d %d %.3f %.3f | p %.3f\\n",
                         t0, diag::nowMs() - t0, a.fwMs, a.compMs, a.deckActive,
                         a.vidCalls, a.vidDec, a.vidSeek, a.vidConv, a.vidAdvMs, a.vidDecMs, a.vidSeekMs, a.vidConvMs,
                         a.vidUpl, a.vidUplMs, a.vidFlipMs, a.vidLockMs,
                         a.exists, a.existsMs, a.seqCalls, a.seqUpl, a.seqMs, a.seqUplMs, a.pumpMs);
            a.reset();
        }
    } dgFrame;
''')
rep('''    uploadBudget_.reset();
    compositor_.pumpImages();
''','''    uploadBudget_.reset();
    { const double dgP0 = diag::nowMs(); compositor_.pumpImages(); diag::gl().pumpMs = diag::nowMs() - dgP0; }
''')
rep('''    Deck* deck = activeDeck_.load(std::memory_order_acquire);
    bool deckActive = (deck != nullptr);
''','''    Deck* deck = activeDeck_.load(std::memory_order_acquire);
    bool deckActive = (deck != nullptr);
    diag::gl().deckActive = deckActive ? 1 : 0;
''')
rep('''        compositor_.setLatestSnapshot(snap);
        sourceTexture = compositor_.compositeDeck(*deck, shaderMgr_, quad_, time, realDt,
                                                   static_cast<int>(renderW),
                                                   static_cast<int>(renderH));
''','''        compositor_.setLatestSnapshot(snap);
        const double dgC0 = diag::nowMs();
        sourceTexture = compositor_.compositeDeck(*deck, shaderMgr_, quad_, time, realDt,
                                                   static_cast<int>(renderW),
                                                   static_cast<int>(renderH));
        diag::gl().compMs = diag::nowMs() - dgC0;
''')
rep('''    double frameMs = std::chrono::duration<double, std::milli>(renderEnd - renderStart).count();
''','''    double frameMs = std::chrono::duration<double, std::milli>(renderEnd - renderStart).count();
    diag::gl().fwMs = frameMs;
''')
# getVideoPlayer lock wait on the caller (message thread)
rep('''VideoPlayer* Renderer::getVideoPlayer(uint32_t clipId)
{
    std::lock_guard<std::mutex> lock(videoPlayerMutex_);''','''VideoPlayer* Renderer::getVideoPlayer(uint32_t clipId)
{
    DIAG_SCOPE_MIN("rd.getVideoPlayer.lockwait", 0.2);
    std::lock_guard<std::mutex> lock(videoPlayerMutex_);''')
rep('''    {
        std::lock_guard<std::mutex> lock(videoPlayerMutex_);
        auto it = videoPlayers_.find(clipId);
        if (it != videoPlayers_.end())
        {
            it->second->close();''','''    {
        DIAG_SCOPE_MIN("rd.closeMedia.video", 0.2);
        std::lock_guard<std::mutex> lock(videoPlayerMutex_);
        auto it = videoPlayers_.find(clipId);
        if (it != videoPlayers_.end())
        {
            it->second->close();''')
rep('''    closeMediaForClip(clipId);

    std::lock_guard<std::mutex> lock(videoPlayerMutex_);
    videoPlayers_[clipId] = std::move(player);''','''    closeMediaForClip(clipId);

    DIAG_SCOPE_MIN("rd.openVideo.insertLock", 0.2);
    std::lock_guard<std::mutex> lock(videoPlayerMutex_);
    videoPlayers_[clipId] = std::move(player);''')
# video branch of syncMedia
rep('''    if (clip->mediaType == Clip::MediaType::Video)
    {
        std::lock_guard<std::mutex> lock(videoPlayerMutex_);
        auto it = videoPlayers_.find(clip->id);
        if (it == videoPlayers_.end())
            return 0;

        auto* player = it->second.get();
''','''    if (clip->mediaType == Clip::MediaType::Video)
    {
        const double dgL0 = diag::nowMs();
        std::lock_guard<std::mutex> lock(videoPlayerMutex_);
        diag::gl().vidLockMs += diag::nowMs() - dgL0;
        auto it = videoPlayers_.find(clip->id);
        if (it == videoPlayers_.end())
            return 0;

        auto* player = it->second.get();
        // DIAGMEDIA: one [GV] line per decoding call (the per-call split), from the accumulator deltas.
        struct DgVid
        {
            const Clip* c; const VideoPlayer* p; bool decode; diag::GlAcc before = diag::gl(); double t0 = diag::nowMs();
            ~DgVid()
            {
                auto& a = diag::gl();
                const double adv = diag::nowMs() - t0;
                if (decode) { a.vidCalls++; a.vidAdvMs += adv; }
                std::fprintf(stderr, "[GV] %.3f %u %dx%d %d %.3f %d %d %.3f %.3f %.3f %.3f %d %.3f %.4f\\n", t0, c->id,
                             p->getWidth(), p->getHeight(), decode ? 1 : 0, adv, a.vidDec - before.vidDec,
                             a.vidSeek - before.vidSeek, a.vidDecMs - before.vidDecMs, a.vidSeekMs - before.vidSeekMs,
                             a.vidConvMs - before.vidConvMs, a.vidFlipMs - before.vidFlipMs, a.vidUpl - before.vidUpl,
                             a.vidUplMs - before.vidUplMs, p->getPlayheadPosition());
            }
        } dgVid{ clip, player, decode };
        static const bool dgNoDecode = diag::flag("DIAG_VID_NODECODE");
''')
rep('''        if (decode)
            player->advanceFrame(static_cast<double>(dt));
        else
            player->advanceClock(static_cast<double>(dt));   // plan4 T4: the clock only, no decode''','''        if (decode && !dgNoDecode)
            player->advanceFrame(static_cast<double>(dt));
        else
            player->advanceClock(static_cast<double>(dt));   // plan4 T4: the clock only, no decode''')
# sequence
rep('''        bool seqPending = false;
        const GLuint tex = seq->getCurrentTexture(imageDecoder_, uploadBudget_, &seqPending);''','''        bool seqPending = false;
        const double dgS0 = diag::nowMs();
        const GLuint tex = seq->getCurrentTexture(imageDecoder_, uploadBudget_, &seqPending);
        diag::gl().seqCalls++; diag::gl().seqMs += diag::nowMs() - dgS0;''')
open(p,'w').write(s)
print('ok')
