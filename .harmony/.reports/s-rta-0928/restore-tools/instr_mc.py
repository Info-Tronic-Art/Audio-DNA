W='/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928-w4/src/'
def edit(path, pairs):
    p=W+path; s=open(p).read()
    for old,new in pairs:
        n=s.count(old); assert n==1,(path,old[:60],n); s=s.replace(old,new)
    open(p,'w').write(s)

edit('MainComponent.cpp', [
('#include "model/AppSettings.h"\n', '#include "model/AppSettings.h"\n#include "diag/DiagTrace.h"\n'),
('''    addKeyListener(this);
    setSize(1280, 800);
}''','''    addKeyListener(this);
    setSize(1280, 800);
    diag::heartbeat().start();
    std::fprintf(stderr, "[DG] heartbeat started at %.3f\\n", diag::nowMs());
}'''),
('''MainComponent::~MainComponent()
{
''','''MainComponent::~MainComponent()
{
    diag::heartbeat().stop();
'''),
# dispatch.fire
('''        const bool immediate = (f.p.origin == Origin::Preamble);
        if (control == "activeClip")
        {''','''        const bool immediate = (f.p.origin == Origin::Preamble);
        diag::Scope dgFire(immediate ? "mc.fire.preamble" : "mc.fire.replay", -1);
        std::fprintf(stderr, "[EV] %.3f dispatch.fire %s layer=%d col=%d v=%d action=%s\\n", diag::nowMs(),
                     control.c_str(), f.target.layer, f.target.col, static_cast<int>(f.p.v), f.p.action.c_str());
        if (control == "activeClip")
        {'''),
# manualWrite etc
('''    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    auto ref = resolveControl(composition_, globalMacroBank_, p);
    bool ok = ref.has_value() && manualWriteCore(''','''    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    DIAG_SCOPE_MIN("mc.manualWrite", 0.1);
    auto ref = resolveControl(composition_, globalMacroBank_, p);
    bool ok = ref.has_value() && manualWriteCore('''),
('''void MainComponent::manualRelease(const ControlPath& p, Origin o)
{
''','''void MainComponent::manualRelease(const ControlPath& p, Origin o)
{
    DIAG_SCOPE_MIN("mc.manualRelease", 0.1);
'''),
('''    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    auto ref = resolveControl(composition_, globalMacroBank_, p);
    bool ok = ref.has_value() && manualTouchCore(''','''    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    DIAG_SCOPE_MIN("mc.manualTouch", 0.1);
    auto ref = resolveControl(composition_, globalMacroBank_, p);
    bool ok = ref.has_value() && manualTouchCore('''),
# tickFeaturePipeline
('''void MainComponent::tickFeaturePipeline()
{
    const FeatureSnapshot snap''','''void MainComponent::tickFeaturePipeline()
{
    DIAG_SCOPE_MIN("mc.tickFeaturePipeline", 1.0);
    const FeatureSnapshot snap'''),
('''    signalRegistry_.evaluateAll(snap);
''','''    { DIAG_SCOPE_MIN("tfp.signalRegistry", 0.5); signalRegistry_.evaluateAll(snap); }
'''),
('''    previewPanel_.getMappingEngine().processFrame(snap, previewPanel_.getEffectChain(), signalDepth);
''','''    { DIAG_SCOPE_MIN("tfp.mappingEngine", 0.5); previewPanel_.getMappingEngine().processFrame(snap, previewPanel_.getEffectChain(), signalDepth); }
'''),
('''    globalMacroBank_.updateValues(signalRegistry_, signalDepth);
''','''    { DIAG_SCOPE_MIN("tfp.macroBank", 0.5); globalMacroBank_.updateValues(signalRegistry_, signalDepth); }
'''),
('''        recorderHost_.tick(snap, now, audioEngine_.getDeliveredSamples(),''','''        DIAG_SCOPE_MIN("tfp.recorderHost", 0.5);
        recorderHost_.tick(snap, now, audioEngine_.getDeliveredSamples(),'''),
('''    connectionEngine_.tick(composition_, ctx);
''','''    { DIAG_SCOPE_MIN("tfp.connectionEngine", 0.5); connectionEngine_.tick(composition_, ctx); }
'''),
('''    if (inspectorPanel_) inspectorPanel_->tickModulation();
}''','''    { DIAG_SCOPE_MIN("tfp.tickModulation", 0.5); if (inspectorPanel_) inspectorPanel_->tickModulation(); }
}'''),
# timerCallback
('''void MainComponent::timerCallback()
{
''','''void MainComponent::timerCallback()
{
    DIAG_SCOPE_MIN("mc.timerCallback", 1.0);
'''),
('''        if (browserPanel_)
            browserPanel_->getRecordPanel().refresh(recorderHost_.status(),''','''        DIAG_SCOPE_MIN("tc.recordPanelRefresh", 0.5);
        if (browserPanel_)
            browserPanel_->getRecordPanel().refresh(recorderHost_.status(),'''),
('''    if (uiUpdateCounter_ % 3 == 0 && inspectorPanel_)
        inspectorPanel_->refresh();''','''    if (uiUpdateCounter_ % 3 == 0 && inspectorPanel_)
    { DIAG_SCOPE_MIN("tc.inspectorRefresh", 0.5); inspectorPanel_->refresh(); }'''),
('''    if (deckView_)
    {
        std::vector<juce::String> deckNames, layerNames;''','''    if (deckView_)
    {
        DIAG_SCOPE_MIN("tc.routineView", 0.5);
        std::vector<juce::String> deckNames, layerNames;'''),
# routine notify
('''    routineEngine_.dispatch.notify  = [this](const std::string& msg) {
        std::cerr << "[Routine] " << msg << std::endl;
        if (browserPanel_)''','''    routineEngine_.dispatch.notify  = [this](const std::string& msg) {
        std::cerr << "[Routine] " << msg << std::endl;
        DIAG_SCOPE_MIN("mc.routineNotice", -1);
        if (browserPanel_)'''),
# handleClipTrigger
('''    const LayerRuntimeSnapshot rtBefore = captureLayerRuntime(*layer);
    std::optional<bool> playBefore;''','''    DIAG_SCOPE_MIN("mc.handleClipTrigger", -1);
    const LayerRuntimeSnapshot rtBefore = captureLayerRuntime(*layer);
    std::optional<bool> playBefore;'''),
('''        if (layer->getClipAt(column))
            layer->triggerClipImmediate(column);
        else
            layer->clearActiveClip();''','''        DIAG_SCOPE_MIN("hct.triggerClipImmediate", -1);
        if (layer->getClipAt(column))
            layer->triggerClipImmediate(column);
        else
            layer->clearActiveClip();'''),
('''        if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
        {
            previewPanel_.getRenderer().clearActiveSource();
            previewPanel_.loadImage(clip->mediaFile);''','''        if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
        {
            DIAG_SCOPE_MIN("hct.imagePreview", -1);
            previewPanel_.getRenderer().clearActiveSource();
            previewPanel_.loadImage(clip->mediaFile);'''),
('''        else if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty())
        {
            previewPanel_.getRenderer().setActiveSource(clip->sourceType);''','''        else if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty())
        {
            DIAG_SCOPE_MIN("hct.sourcePreview", -1);
            previewPanel_.getRenderer().setActiveSource(clip->sourceType);'''),
('''    if (deckView_)
        deckView_->refresh();
    }

    // Push the trigger command''','''    if (deckView_)
    { DIAG_SCOPE_MIN("hct.deckViewRefresh", -1); deckView_->refresh(); }
    }

    // Push the trigger command'''),
])

edit('ui/DeckView.cpp', [
('''void DeckView::refresh()
{
    if (!composition_) return;''','''void DeckView::refresh()
{
    DIAG_SCOPE_MIN("dv.refresh", 0.1);
    if (!composition_) return;'''),
('''        layerStrips_[static_cast<size_t>(displayRow)]->refresh();
''','''        { DIAG_SCOPE_MIN("dv.refresh.layerStrip", 0.1); layerStrips_[static_cast<size_t>(displayRow)]->refresh(); }
'''),
('''void DeckView::paint(juce::Graphics& g)
{
''','''void DeckView::paint(juce::Graphics& g)
{
    DIAG_SCOPE_MIN("paint.DeckView", 0.25);
'''),
('''void DeckView::setRoutineView(const RoutineDeckView& view)
{
''','''void DeckView::setRoutineView(const RoutineDeckView& view)
{
    DIAG_SCOPE_MIN("dv.setRoutineView", 0.25);
'''),
])
edit('ui/ClipCell.cpp', [
('''void ClipCell::paint(juce::Graphics& g)
{
''','''void ClipCell::paint(juce::Graphics& g)
{
    DIAG_SCOPE_MIN("paint.ClipCell", 0.25);
'''),
('''        auto img = juce::ImageFileFormat::loadFrom(clip_->mediaFile);''','''        DIAG_SCOPE_MIN("cc.updateThumbnail.decodeImage", -1);
        auto img = juce::ImageFileFormat::loadFrom(clip_->mediaFile);'''),
])
edit('ui/LayerStrip.cpp', [
('''void LayerStrip::paint(juce::Graphics& g)
{
''','''void LayerStrip::paint(juce::Graphics& g)
{
    DIAG_SCOPE_MIN("paint.LayerStrip", 0.25);
'''),
])
edit('ui/InspectorPanel.cpp', [
('''void InspectorPanel::refresh()
{
''','''void InspectorPanel::refresh()
{
    DIAG_SCOPE_MIN("ip.refresh", 0.5);
'''),
])
edit('ui/RecordPanel.cpp', [
('''void RecordPanel::setNotice(const std::string& text, const RecorderHost::Status& raisedIn)
{
''','''void RecordPanel::setNotice(const std::string& text, const RecorderHost::Status& raisedIn)
{
    DIAG_SCOPE_MIN("rp.setNotice", -1);
'''),
])
print('ok')
