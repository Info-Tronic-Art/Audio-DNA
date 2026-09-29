W='/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928b-media'
def edit(path, pairs):
    p=W+path; s=open(p).read()
    for a,b in pairs:
        assert s.count(a)==1, (path, a[:70], s.count(a))
        s=s.replace(a,b)
    open(p,'w').write(s)

edit('/src/MainComponent.cpp', [
('#include "model/AppSettings.h"\n', '#include "model/AppSettings.h"\n#include "diag/DiagTrace.h"\n'),
('''    addKeyListener(this);
    setSize(1280, 800);
}
''','''    addKeyListener(this);
    setSize(1280, 800);
    diag::heartbeat().start();
    std::fprintf(stderr, "[DG] heartbeat started at %.3f (DIAGMEDIA)\\n", diag::nowMs());
    // DIAGMEDIA temporary hook: /api/diag/media (DIAG_MEDIA_HOOK=1) -> the same message-thread handlers a Finder
    // drop reaches (DeckView::onFileDropped / onMultiFileDropped), a grid refresh or a grid repaint.
    diag::mediaHook() = [safe = juce::Component::SafePointer<MainComponent>(this)](const std::string& body) {
        juce::MessageManager::callAsync([safe, body] {
            auto* self = safe.getComponent();
            if (self == nullptr) return;
            auto v = juce::JSON::parse(juce::String(body));
            const auto op = v.getProperty("op", "").toString();
            const int L = static_cast<int>(v.getProperty("layer", 0));
            const int C = static_cast<int>(v.getProperty("col", 0));
            std::vector<juce::File> files;
            if (auto* arr = v.getProperty("files", juce::var()).getArray())
                for (auto& f : *arr) files.push_back(juce::File(f.toString()));
            std::fprintf(stderr, "[EV] %.3f hook %s %d %d %d 0\\n", diag::nowMs(), op.toRawUTF8(), L, C, (int) files.size());
            diag::Scope dgHook("hook.total", -1);
            if (op == "drop" && files.size() == 1) { self->handleFileDrop(L, C, files[0]); if (self->deckView_) self->deckView_->clearSelection(); }
            else if (op == "drop" && files.size() > 1) { self->handleMultiFileDrop(L, C, files); if (self->deckView_) self->deckView_->clearSelection(); }
            else if (op == "refresh" && self->deckView_) { for (int i = 0; i < std::max(1, C); ++i) self->deckView_->refresh(); }
            else if (op == "repaint" && self->deckView_) { self->deckView_->repaint(); }
        });
    };
}
'''),
('''MainComponent::~MainComponent()
{
''','''MainComponent::~MainComponent()
{
    diag::mediaHook() = nullptr;
    diag::heartbeat().stop();
'''),
('''void MainComponent::swapCompositionModel(const std::function<void()>& mutation)
{
''','''void MainComponent::swapCompositionModel(const std::function<void()>& mutation)
{
    DIAG_SCOPE_MIN("mc.swapCompositionModel", -1);
'''),
('''void MainComponent::loadComposition(const juce::File& file)
{
''','''void MainComponent::loadComposition(const juce::File& file)
{
    DIAG_SCOPE_MIN("mc.loadComposition", -1);
'''),
('''void MainComponent::openMediaForDeck(Deck& deck)
{
''','''void MainComponent::openMediaForDeck(Deck& deck)
{
    DIAG_SCOPE_MIN("mc.openMediaForDeck", -1);
    static const bool dgNoThumb = diag::flag("DIAG_NO_THUMB");
'''),
('''                if (!clip.mediaFile.existsAsFile()) continue;   // non-fatal: skip, continue
                if (renderer.openVideoForClip(clip.id, clip.mediaFile))
                {
                    if (auto* p = renderer.getVideoPlayer(clip.id))
                    {
                        clip.hasAlpha = p->hasAlpha();
                        clip.clipWidth = p->getWidth();
                        clip.clipHeight = p->getHeight();
                        clip.thumbnail = p->getThumbnail(90, 72);''','''                if (!clip.mediaFile.existsAsFile()) continue;   // non-fatal: skip, continue
                diag::Scope dgClip("omd.videoClip", -1);
                if (renderer.openVideoForClip(clip.id, clip.mediaFile))
                {
                    if (auto* p = renderer.getVideoPlayer(clip.id))
                    {
                        clip.hasAlpha = p->hasAlpha();
                        clip.clipWidth = p->getWidth();
                        clip.clipHeight = p->getHeight();
                        if (!dgNoThumb) clip.thumbnail = p->getThumbnail(90, 72);'''),
('''                if (clip.sequenceFiles.empty()) continue;
                renderer.openImageSequenceForClip(clip.id, clip.sequenceFiles, clip.sequenceFps);
                auto first = juce::ImageFileFormat::loadFrom(clip.sequenceFiles[0]);
                if (first.isValid())''','''                if (clip.sequenceFiles.empty()) continue;
                diag::Scope dgClip("omd.seqClip", -1);
                renderer.openImageSequenceForClip(clip.id, clip.sequenceFiles, clip.sequenceFps);
                if (dgNoThumb) continue;
                diag::Scope dgT("omd.seqThumbDecode", -1);
                auto first = juce::ImageFileFormat::loadFrom(clip.sequenceFiles[0]);
                if (first.isValid())'''),
('''void MainComponent::handleClipTrigger(int layerIndex, int column, Origin origin, int deckIndex, bool immediate)
{
''','''void MainComponent::handleClipTrigger(int layerIndex, int column, Origin origin, int deckIndex, bool immediate)
{
    DIAG_SCOPE_MIN("mc.handleClipTrigger", -1);
'''),
('''MainComponent::applyFileDrop(int layerIndex, int column, const juce::File& file)
{
''','''MainComponent::applyFileDrop(int layerIndex, int column, const juce::File& file)
{
    DIAG_SCOPE_MIN("mc.applyFileDrop", -1);
    static const bool dgNoThumb = diag::flag("DIAG_NO_THUMB");
'''),
('''            if (player)
            {
                clip.hasAlpha = player->hasAlpha();
                clip.clipWidth = player->getWidth();
                clip.clipHeight = player->getHeight();
                clip.thumbnail = player->getThumbnail(90, 72);''','''            if (player)
            {
                clip.hasAlpha = player->hasAlpha();
                clip.clipWidth = player->getWidth();
                clip.clipHeight = player->getHeight();
                if (!dgNoThumb) clip.thumbnail = player->getThumbnail(90, 72);'''),
('''MainComponent::applyMultiFileDrop(int layerIndex, int column, const std::vector<juce::File>& files)
{
''','''MainComponent::applyMultiFileDrop(int layerIndex, int column, const std::vector<juce::File>& files)
{
    DIAG_SCOPE_MIN("mc.applyMultiFileDrop", -1);
    static const bool dgNoThumb = diag::flag("DIAG_NO_THUMB");
'''),
('''        // Thumbnail from first image
        auto firstImg = juce::ImageFileFormat::loadFrom(clip.sequenceFiles[0]);
        if (firstImg.isValid())''','''        // Thumbnail from first image
        diag::Scope dgT("amfd.thumbDecode", -1);
        auto firstImg = dgNoThumb ? juce::Image() : juce::ImageFileFormat::loadFrom(clip.sequenceFiles[0]);
        if (firstImg.isValid())'''),
])
edit('/src/core/UndoService.cpp', [
('#include "core/UndoService.h"\n', '#include "core/UndoService.h"\n#include "diag/DiagTrace.h"\n'),
('''    renderer_->getContext().executeOnGLThread([](juce::OpenGLContext&) {}, true);

    if (mutation)
        mutation();
''','''    { DIAG_SCOPE_MIN("us.fence.wait", -1);
    renderer_->getContext().executeOnGLThread([](juce::OpenGLContext&) {}, true); }

    DIAG_SCOPE_MIN("us.fence.mutation", -1);
    if (mutation)
        mutation();
'''),
])
edit('/src/ui/DeckView.cpp', [
('''void DeckView::refresh()
{
''','''void DeckView::refresh()
{
    DIAG_SCOPE_MIN("dv.refresh", -1);
'''),
])
edit('/src/ui/ClipCell.cpp', [
('''void ClipCell::paint(juce::Graphics& g)
{
''','''void ClipCell::paint(juce::Graphics& g)
{
    // DIAGMEDIA: one [CP] line per paint -- duration and the missing-file stat's cost.
    static const bool dgNoPaintStat = diag::flag("DIAG_NO_PAINTSTAT");
    struct DgPaint { double t0 = diag::nowMs(); int n = 0; double statMs = 0; int kind = 0;
        ~DgPaint() { std::fprintf(stderr, "[CP] %.3f %.4f %d %.4f %d\\n", t0, diag::nowMs() - t0, n, statMs, kind); } } dgPaint;
    if (clip_ != nullptr) dgPaint.kind = static_cast<int>(clip_->mediaType);
    auto dgStat = [&](const juce::File& f) {
        if (dgNoPaintStat) return true;
        const double s0 = diag::nowMs(); const bool r = f.existsAsFile();
        dgPaint.n++; dgPaint.statMs += diag::nowMs() - s0; return r; };
'''),
('''        clip_->mediaFile != juce::File() && !clip_->mediaFile.existsAsFile())''','''        clip_->mediaFile != juce::File() && !dgStat(clip_->mediaFile))'''),
])
print('ok')
