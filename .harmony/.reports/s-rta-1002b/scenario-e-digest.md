# scenario e digest (s-rta-1002b, tsan-analyze fix)

Data: the s-rta-1002 G3 sweep (31 launches, scenarios a-e, both arms; incl. the 3 extra main-d launches 29-31), re-keyed with the fixed analyzer. Arm trees: main = git 575deca, lane = the merged tree (b844b65).
Selection: every unique of scenario e (both arms) + every unique in scenarios a-d now keyed R5 / R7 / UNCLASSIFIED (none of the latter: a-d have no R5 / R7 / UNCLASSIFIED report).
'Racing field' is read from the printed source lines of the two top src frames (INFERRED, not from the memory address). Thread names are the TSan creation records (TSan does not carry the JUCE thread name); GL render thread = the juce OpenGLContext RenderThread.

## F1  arm=main  scenario=a/b/c/d/e  new class=A  (old family: A)
launches seen: tsan-1(a), tsan-3(b), tsan-5(c), tsan-7(d), tsan-9(a), tsan-11(b), tsan-13(c), tsan-15(d), tsan-17(a), tsan-19(b), tsan-21(c), tsan-23(d), tsan-25(e), tsan-27(e), tsan-29(d), tsan-30(d), tsan-31(d)
racing field: not inferable from source lines
  - Read of size 8 [httplib server thread (T28, main thread)]
    top src line: ``
    frames: (none)
  - Previous write of size 8 [JUCE worker thread (T27, main thread)]
    top src line: `<< ", bandwidth " << static_cast<int>(resampler_.inputBandwidthHz()) << " Hz\n";`
    frames: AnalysisThread.cpp:84 AnalysisThread::run

## F3  arm=main  scenario=b/c/d/e  new class=B  (old family: B)
launches seen: tsan-3(b), tsan-5(c), tsan-7(d), tsan-11(b), tsan-13(c), tsan-21(c), tsan-23(d), tsan-27(e), tsan-30(d)
racing field: activeClipColumn (one side only)
  - Write of size 4 [main thread (JUCE message thread)]
    top src line: `triggerClipImmediate(column);`
    frames: Layer.h:250 Layer::triggerClip <- MainComponent.cpp:4865 MainComponent::handleColumnTrigger <- Main.cpp:99 main
  - Previous read of size 4 [GL render thread (T45, main thread)]
    top src line: `if (!layer || layer->activeClipColumn < 0) continue;`
    frames: Renderer.cpp:593 Renderer::renderOpenGL

## F4  arm=main  scenario=b/c/d/e  new class=C  (old family: C)
launches seen: tsan-3(b), tsan-5(c), tsan-7(d), tsan-11(b), tsan-13(c), tsan-15(d), tsan-19(b), tsan-21(c), tsan-23(d), tsan-25(e), tsan-27(e), tsan-29(d), tsan-30(d), tsan-31(d)
racing field: playheadPosition (one side only)
  - Atomic read of size 8 [main thread (JUCE message thread)]
    top src line: `const auto tv = transportViewOf(layer_, transportBounds_);`
    frames: LayerStrip.cpp:763 LayerStrip::updateTransportView <- LayerStrip.cpp:783 LayerStrip::timerTick <- LayerStrip.cpp:None non-virtual thunk to LayerStrip::timerCallback <- Main.cpp:99 main
  - Previous write of size 8 [GL render thread (T45, main thread)]
    top src line: `clip->playheadPosition = player->getPlayheadPosition();`
    frames: Renderer.cpp:1823 Renderer::syncMedia <- CompositorEngine.cpp:1103 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL

## F5  arm=main  scenario=c/e  new class=A  (old family: A)
launches seen: tsan-5(c), tsan-25(e)
racing field: not inferable from source lines
  - Read of size 8 [JUCE worker thread (T22, main thread)]
    top src line: `std::cerr << "[VideoPlayer] Opened: " << path`
    frames: VideoPlayer.cpp:314 VideoPlayer::open <- MediaOpener.cpp:33 MediaOpener::OpenJob::runJob
  - Previous write of size 8 [JUCE worker thread (T21, main thread)]
    top src line: `<< " (" << width_ << "x" << height_`
    frames: VideoPlayer.cpp:315 VideoPlayer::open <- MediaOpener.cpp:33 MediaOpener::OpenJob::runJob

## F7  arm=main  scenario=c/d/e  new class=B  (old family: B)
launches seen: tsan-5(c), tsan-7(d), tsan-13(c), tsan-15(d), tsan-21(c), tsan-23(d), tsan-27(e), tsan-29(d), tsan-30(d), tsan-31(d)
racing field: activeClipColumn (one side only)
  - Read of size 4 [GL render thread (T45, main thread)]
    top src line: `if (!layer || layer->activeClipColumn < 0) continue;`
    frames: Renderer.cpp:593 Renderer::renderOpenGL
  - Previous write of size 8 [main thread (JUCE message thread)]
    top src line: `void execute() override { apply(after_, targetPlayingAfter_); }`
    frames: TriggerCommands.h:68 TriggerClipCmd::execute <- UndoManager.cpp:15 UndoManager::perform <- MainComponent.cpp:5158 MainComponent::pushCommands <- MainComponent.cpp:4789 MainComponent::handleClipTrigger

## F8  arm=main  scenario=c/d/e  new class=B  (old family: B)
launches seen: tsan-5(c), tsan-7(d), tsan-13(c), tsan-15(d), tsan-21(c), tsan-23(d), tsan-27(e), tsan-29(d), tsan-30(d), tsan-31(d)
racing field: crossfadeProgress, previousClipColumn (one side only)
  - Read of size 4 [GL render thread (T45, main thread)]
    top src line: `if (layer.crossfadeProgress >= 1.0f || layer.previousClipColumn < 0)`
    frames: CompositorEngine.cpp:401 CompositorEngine::incomingImagePending <- CompositorEngine.cpp:1074 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL
  - Previous write of size 4 [main thread (JUCE message thread)]
    top src line: `void execute() override { apply(after_, targetPlayingAfter_); }`
    frames: TriggerCommands.h:68 TriggerClipCmd::execute <- UndoManager.cpp:15 UndoManager::perform <- MainComponent.cpp:5158 MainComponent::pushCommands <- MainComponent.cpp:4789 MainComponent::handleClipTrigger

## F11  arm=main  scenario=c/d/e  new class=B  (old family: B)
launches seen: tsan-5(c), tsan-7(d), tsan-21(c), tsan-27(e), tsan-29(d)
racing field: not inferable from source lines
  - Write of size 4 [main thread (JUCE message thread)]
    top src line: `triggerClipImmediate(column);`
    frames: Layer.h:250 Layer::triggerClip <- MainComponent.cpp:4659 MainComponent::handleClipTrigger <- Main.cpp:99 main
  - Previous read of size 4 [GL render thread (T45, main thread)]
    top src line: `advanceCrossfade(layer, dt);`
    frames: CompositorEngine.cpp:1075 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL

## F21  arm=main  scenario=b/c/d/e  new class=B  (old family: B)
launches seen: tsan-7(d), tsan-11(b), tsan-15(d), tsan-21(c), tsan-27(e), tsan-29(d), tsan-30(d)
racing field: activeClipColumn (one side only)
  - Read of size 4 [GL render thread (T45, main thread)]
    top src line: `if (!layer || layer->activeClipColumn < 0) continue;`
    frames: Renderer.cpp:593 Renderer::renderOpenGL
  - Previous write of size 8 [main thread (JUCE message thread)]
    top src line: `void execute() override { apply(after_, targetPlayingAfter_); }`
    frames: TriggerCommands.h:68 TriggerClipCmd::execute <- CompositeCommand.h:35 CompositeCommand::execute <- UndoManager.cpp:15 UndoManager::perform <- MainComponent.cpp:5165 MainComponent::pushCommands

## F22  arm=main  scenario=b/c/d/e  new class=B  (old family: B)
launches seen: tsan-7(d), tsan-11(b), tsan-15(d), tsan-21(c), tsan-23(d), tsan-27(e), tsan-29(d), tsan-30(d), tsan-31(d)
racing field: crossfadeProgress, previousClipColumn (one side only)
  - Read of size 4 [GL render thread (T45, main thread)]
    top src line: `if (layer.crossfadeProgress >= 1.0f || layer.previousClipColumn < 0)`
    frames: CompositorEngine.cpp:401 CompositorEngine::incomingImagePending <- CompositorEngine.cpp:1074 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL
  - Previous write of size 4 [main thread (JUCE message thread)]
    top src line: `void execute() override { apply(after_, targetPlayingAfter_); }`
    frames: TriggerCommands.h:68 TriggerClipCmd::execute <- CompositeCommand.h:35 CompositeCommand::execute <- UndoManager.cpp:15 UndoManager::perform <- MainComponent.cpp:5165 MainComponent::pushCommands

## F25  arm=main  scenario=d/e  new class=B  (old family: B)
launches seen: tsan-7(d), tsan-15(d), tsan-23(d), tsan-27(e), tsan-29(d), tsan-30(d), tsan-31(d)
racing field: not inferable from source lines
  - Write of size 4 [GL render thread (T45, main thread)]
    top src line: `advanceCrossfade(layer, dt);`
    frames: CompositorEngine.cpp:1075 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL
  - Previous write of size 4 [main thread (JUCE message thread)]
    top src line: `void execute() override { apply(after_, targetPlayingAfter_); }`
    frames: TriggerCommands.h:68 TriggerClipCmd::execute <- UndoManager.cpp:15 UndoManager::perform <- MainComponent.cpp:5158 MainComponent::pushCommands <- MainComponent.cpp:4789 MainComponent::handleClipTrigger

## F26  arm=main  scenario=d/e  new class=B  (old family: B)
launches seen: tsan-7(d), tsan-15(d), tsan-23(d), tsan-25(e), tsan-29(d), tsan-30(d)
racing field: not inferable from source lines
  - Write of size 4 [GL render thread (T45, main thread)]
    top src line: `advanceCrossfade(layer, dt);`
    frames: CompositorEngine.cpp:1075 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL
  - Previous read of size 4 [main thread (JUCE message thread)]
    top src line: `const LayerRuntimeSnapshot rtBefore = captureLayerRuntime(*layer);`
    frames: MainComponent.cpp:4632 MainComponent::handleClipTrigger <- Main.cpp:99 main

## F27  arm=main  scenario=d/e  new class=B  (old family: B)
launches seen: tsan-7(d), tsan-23(d), tsan-27(e), tsan-30(d)
racing field: not inferable from source lines
  - Write of size 4 [main thread (JUCE message thread)]
    top src line: `triggerClipImmediate(column);`
    frames: Layer.h:250 Layer::triggerClip <- MainComponent.cpp:4865 MainComponent::handleColumnTrigger <- Main.cpp:99 main
  - Previous write of size 4 [GL render thread (T45, main thread)]
    top src line: `advanceCrossfade(layer, dt);`
    frames: CompositorEngine.cpp:1075 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL

## F30  arm=main  scenario=c/d/e  new class=B  (old family: B)
launches seen: tsan-7(d), tsan-21(c), tsan-25(e), tsan-30(d), tsan-31(d)
racing field: not inferable from source lines
  - Write of size 4 [main thread (JUCE message thread)]
    top src line: `triggerClipImmediate(column);`
    frames: Layer.h:250 Layer::triggerClip <- MainComponent.cpp:4659 MainComponent::handleClipTrigger <- Main.cpp:99 main
  - Previous read of size 4 [GL render thread (T45, main thread)]
    top src line: `const Clip* clip = layer.getActiveClip();`
    frames: CompositorEngine.cpp:1072 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL

## F42  arm=main  scenario=d/e  new class=B  (old family: B)
launches seen: tsan-15(d), tsan-23(d), tsan-25(e)
racing field: activeClipColumn (one side only)
  - Write of size 4 [main thread (JUCE message thread)]
    top src line: `triggerClipImmediate(column);`
    frames: Layer.h:250 Layer::triggerClip <- MainComponent.cpp:4659 MainComponent::handleClipTrigger <- Main.cpp:99 main
  - Previous read of size 4 [GL render thread (T45, main thread)]
    top src line: `if (!layer || layer->activeClipColumn < 0) continue;`
    frames: Renderer.cpp:593 Renderer::renderOpenGL

## F61  arm=main  scenario=e  new class=A  (old family: A)
launches seen: tsan-25(e)
racing field: not inferable from source lines
  - Write of size 8 [JUCE worker thread (T21, main thread)]
    top src line: `<< ", " << duration_ << "s"`
    frames: VideoPlayer.cpp:317 VideoPlayer::open <- MediaOpener.cpp:33 MediaOpener::OpenJob::runJob
  - Previous write of size 8 [JUCE worker thread (T20, main thread)]
    top src line: `std::cerr << "[VideoPlayer] Opened: " << path`
    frames: VideoPlayer.cpp:314 VideoPlayer::open <- MediaOpener.cpp:33 MediaOpener::OpenJob::runJob

## F62  arm=main  scenario=e  new class=B  (old family: B)
launches seen: tsan-25(e), tsan-27(e)
racing field: playing
  - Write of size 1 [GL render thread (T44, main thread)]
    top src line: `clip->playing = player->isPlaying();`
    frames: Renderer.cpp:1826 Renderer::syncMedia <- CompositorEngine.cpp:1103 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL
  - Previous read of size 1 [main thread (JUCE message thread)]
    top src line: `clipRT.playing = clip.playing;`
    frames: PerfStateCapture.cpp:91 capturePerfState <- RecorderHost.cpp:381 RecorderHost::disarm <- MainComponent.cpp:5766 MainComponent::perfStop <- Main.cpp:99 main

## F63  arm=main  scenario=e  new class=B  (old family: B)
launches seen: tsan-25(e)
racing field: not inferable from source lines
  - Read of size 4 [main thread (JUCE message thread)]
    top src line: `const auto tv = transportViewOf(layer_, transportBounds_);`
    frames: LayerStrip.cpp:763 LayerStrip::updateTransportView <- LayerStrip.cpp:783 LayerStrip::timerTick <- LayerStrip.cpp:None non-virtual thunk to LayerStrip::timerCallback <- Main.cpp:99 main
  - Previous write of size 4 [GL render thread (T44, main thread)]
    top src line: `triggerClipImmediate(column);`
    frames: Layer.h:250 Layer::triggerClip <- Autopilot.cpp:288 Autopilot::advanceClip <- Autopilot.cpp:105 Autopilot::processFrame <- Renderer.cpp:547 Renderer::renderOpenGL

## F64  arm=main  scenario=e  new class=R5  (old family: - unkeyed)
launches seen: tsan-25(e), tsan-27(e)
racing field: visible
  - Write of size 1 [main thread (JUCE message thread)]
    top src line: `if (flag == "visible") layer->visible = value;`
    frames: MainComponent.cpp:6348 MainComponent::applyLayerFlag <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous read of size 1 [GL render thread (T44, main thread)]
    top src line: `if (!layer.visible || layer.bypassed || (anySolo && !layer.solo))`
    frames: CompositorEngine.cpp:1069 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL

## F65  arm=main  scenario=e  new class=R5  (old family: B)
launches seen: tsan-25(e), tsan-27(e)
racing field: autopilotEnabled
  - Write of size 1 [main thread (JUCE message thread)]
    top src line: `else if (flag == "autopilot") layer->autopilotEnabled = value;`
    frames: MainComponent.cpp:6352 MainComponent::applyLayerFlag <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous read of size 1 [GL render thread (T44, main thread)]
    top src line: `if (!layer.autopilotEnabled || !layer.autopilotEndOfVideo)`
    frames: Autopilot.cpp:11 Autopilot::processFrame <- Renderer.cpp:547 Renderer::renderOpenGL

## F66  arm=main  scenario=e  new class=B  (old family: B)
launches seen: tsan-25(e), tsan-27(e)
racing field: playing (one side only)
  - Write of size 1 [main thread (JUCE message thread)]
    top src line: `layer->clearActiveClip();`
    frames: MainComponent.cpp:5583 MainComponent::applyClearActiveClip <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous write of size 1 [GL render thread (T44, main thread)]
    top src line: `clip->playing = player->isPlaying();`
    frames: Renderer.cpp:1826 Renderer::syncMedia <- CompositorEngine.cpp:1103 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL

## F67  arm=main  scenario=e  new class=B  (old family: B)
launches seen: tsan-25(e), tsan-27(e)
racing field: activeClipColumn, previousClipColumn (one side only)
  - Write of size 4 [main thread (JUCE message thread)]
    top src line: `layer->clearActiveClip();`
    frames: MainComponent.cpp:5583 MainComponent::applyClearActiveClip <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous read of size 4 [GL render thread (T44, main thread)]
    top src line: `if (crossfadeStart_[clipKey].observe(layer.previousClipColumn, layer.activeClipColumn,`
    frames: CompositorEngine.cpp:991 CompositorEngine::renderLayerStages <- CompositorEngine.cpp:1135 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL

## F68  arm=main  scenario=e  new class=B  (old family: B)
launches seen: tsan-25(e), tsan-27(e)
racing field: crossfadeProgress (one side only)
  - Write of size 4 [main thread (JUCE message thread)]
    top src line: `layer->clearActiveClip();`
    frames: MainComponent.cpp:5583 MainComponent::applyClearActiveClip <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous read of size 4 [GL render thread (T44, main thread)]
    top src line: `layer.crossfadeProgress))`
    frames: CompositorEngine.cpp:992 CompositorEngine::renderLayerStages <- CompositorEngine.cpp:1135 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL

## F69  arm=main  scenario=e  new class=B  (old family: B)
launches seen: tsan-25(e)
racing field: not inferable from source lines
  - Write of size 4 [main thread (JUCE message thread)]
    top src line: `layer->clearActiveClip();`
    frames: MainComponent.cpp:5583 MainComponent::applyClearActiveClip <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous write of size 4 [GL render thread (T44, main thread)]
    top src line: `advanceCrossfade(layer, dt);`
    frames: CompositorEngine.cpp:1075 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL

## F70  arm=main  scenario=e  new class=R5  (old family: - unkeyed)
launches seen: tsan-25(e), tsan-27(e)
racing field: visible (one side only)
  - Write of size 1 [main thread (JUCE message thread)]
    top src line: `if (flag == "visible") layer->visible = value;`
    frames: MainComponent.cpp:6348 MainComponent::applyLayerFlag <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous read of size 1 [GL render thread (T44, main thread)]
    top src line: `DeckClock::tick(other, realDt, [this](const Clip* c, float dt) { tickMediaClock(c, dt); });`
    frames: Renderer.cpp:771 Renderer::renderOpenGL

## F71  arm=main  scenario=e  new class=R5  (old family: B)
launches seen: tsan-25(e), tsan-27(e)
racing field: autopilotEnabled
  - Write of size 1 [main thread (JUCE message thread)]
    top src line: `else if (flag == "autopilot") layer->autopilotEnabled = value;`
    frames: MainComponent.cpp:6352 MainComponent::applyLayerFlag <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous read of size 1 [GL render thread (T44, main thread)]
    top src line: `if (!layer.autopilotEnabled || !layer.autopilotEndOfVideo)`
    frames: Autopilot.cpp:11 Autopilot::processFrame <- Renderer.cpp:772 Renderer::renderOpenGL

## F72  arm=main  scenario=e  new class=R5  (old family: - unkeyed)
launches seen: tsan-25(e)
racing field: solo
  - Read of size 1 [GL render thread (T44, main thread)]
    top src line: `if (layer.solo) { anySolo = true; break; }`
    frames: CompositorEngine.cpp:1265 CompositorEngine::hasPersistentContent <- Renderer.cpp:744 Renderer::renderOpenGL
  - Previous write of size 1 [main thread (JUCE message thread)]
    top src line: `else if (flag == "solo") layer->solo = value;`
    frames: MainComponent.cpp:6349 MainComponent::applyLayerFlag <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play

## F73  arm=main  scenario=d/e  new class=C  (old family: C)
launches seen: tsan-25(e), tsan-27(e), tsan-29(d), tsan-30(d), tsan-31(d)
racing field: playheadPosition (one side only)
  - Write of size 8 [GL render thread (T44, main thread)]
    top src line: `clip->playheadPosition = player->getPlayheadPosition();`
    frames: Renderer.cpp:1823 Renderer::syncMedia <- CompositorEngine.cpp:1103 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL
  - Previous atomic read of size 8 [main thread (JUCE message thread)]
    top src line: `const auto tv = transportViewOf(layer_, transportBounds_);`
    frames: LayerStrip.cpp:763 LayerStrip::updateTransportView <- LayerStrip.cpp:737 LayerStrip::refresh <- DeckView.cpp:276 DeckView::refresh <- Main.cpp:99 main

## F74  arm=lane  scenario=e  new class=R5  (old family: - unkeyed)
launches seen: tsan-26(e), tsan-28(e)
racing field: visible
  - Write of size 1 [main thread (JUCE message thread)]
    top src line: `if (flag == "visible") layer->visible = value;`
    frames: MainComponent.cpp:6348 MainComponent::applyLayerFlag <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous read of size 1 [GL render thread (T45, main thread)]
    top src line: `if (!layer.visible || layer.bypassed || (anySolo && !layer.solo))`
    frames: CompositorEngine.cpp:1072 CompositorEngine::compositeDeck <- Renderer.cpp:735 Renderer::renderOpenGL

## F75  arm=lane  scenario=e  new class=R5  (old family: - unkeyed)
launches seen: tsan-26(e), tsan-28(e)
racing field: visible (one side only)
  - Write of size 1 [main thread (JUCE message thread)]
    top src line: `if (flag == "visible") layer->visible = value;`
    frames: MainComponent.cpp:6348 MainComponent::applyLayerFlag <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous read of size 1 [GL render thread (T45, main thread)]
    top src line: `const int adopts = DeckClock::tick(other, realDt,`
    frames: Renderer.cpp:778 Renderer::renderOpenGL

## F76  arm=lane  scenario=e  new class=R5  (old family: B)
launches seen: tsan-26(e), tsan-28(e)
racing field: autopilotEnabled
  - Write of size 1 [main thread (JUCE message thread)]
    top src line: `else if (flag == "autopilot") layer->autopilotEnabled = value;`
    frames: MainComponent.cpp:6352 MainComponent::applyLayerFlag <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous read of size 1 [GL render thread (T45, main thread)]
    top src line: `if (!layer.autopilotEnabled || !layer.autopilotEndOfVideo)`
    frames: Autopilot.cpp:28 Autopilot::processFrame <- Renderer.cpp:782 Renderer::renderOpenGL

## F77  arm=lane  scenario=e  new class=R5  (old family: - unkeyed)
launches seen: tsan-26(e)
racing field: solo
  - Read of size 1 [GL render thread (T45, main thread)]
    top src line: `if (layer.solo) { anySolo = true; break; }`
    frames: CompositorEngine.cpp:1272 CompositorEngine::hasPersistentContent <- Renderer.cpp:751 Renderer::renderOpenGL
  - Previous write of size 1 [main thread (JUCE message thread)]
    top src line: `else if (flag == "solo") layer->solo = value;`
    frames: MainComponent.cpp:6349 MainComponent::applyLayerFlag <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play

## F78  arm=main  scenario=e  new class=B  (old family: B)
launches seen: tsan-27(e)
racing field: crossfadeProgress (one side only)
  - Write of size 4 [GL render thread (T45, main thread)]
    top src line: `advanceCrossfade(layer, dt);`
    frames: CompositorEngine.cpp:1075 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL
  - Previous read of size 4 [main thread (JUCE message thread)]
    top src line: `layerRT.crossfadeProgress = layer.crossfadeProgress;`
    frames: PerfStateCapture.cpp:72 capturePerfState <- RecorderHost.cpp:381 RecorderHost::disarm <- MainComponent.cpp:5766 MainComponent::perfStop <- Main.cpp:99 main

## F79  arm=main  scenario=e  new class=B  (old family: B)
launches seen: tsan-27(e)
racing field: crossfadeProgress, previousClipColumn (one side only)
  - Write of size 4 [main thread (JUCE message thread)]
    top src line: `layer->clearActiveClip();`
    frames: MainComponent.cpp:5583 MainComponent::applyClearActiveClip <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous read of size 4 [GL render thread (T45, main thread)]
    top src line: `if (layer.crossfadeProgress >= 1.0f || layer.previousClipColumn < 0)`
    frames: CompositorEngine.cpp:401 CompositorEngine::incomingImagePending <- CompositorEngine.cpp:1074 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL

## F80  arm=main  scenario=e  new class=B  (old family: B)
launches seen: tsan-27(e)
racing field: crossfadeProgress (one side only)
  - Write of size 4 [main thread (JUCE message thread)]
    top src line: `layer->clearActiveClip();`
    frames: MainComponent.cpp:5583 MainComponent::applyClearActiveClip <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous read of size 4 [GL render thread (T45, main thread)]
    top src line: `glUniform1f(glGetUniformLocation(pid, "u_crossfadeProgress"), layer.crossfadeProgress);`
    frames: CompositorEngine.cpp:1740 CompositorEngine::applyTransition <- CompositorEngine.cpp:1004 CompositorEngine::renderLayerStages <- CompositorEngine.cpp:1135 CompositorEngine::compositeDeck <- Renderer.cpp:728 Renderer::renderOpenGL

## F81  arm=lane  scenario=e  new class=R5  (old family: B)
launches seen: tsan-28(e)
racing field: autopilotEnabled
  - Write of size 1 [main thread (JUCE message thread)]
    top src line: `else if (flag == "autopilot") layer->autopilotEnabled = value;`
    frames: MainComponent.cpp:6352 MainComponent::applyLayerFlag <- RecorderHost.cpp:79 RecorderHost::HostSink::fire <- Player.cpp:86 Player::firePreamble <- RecorderHost.cpp:823 RecorderHost::play
  - Previous read of size 1 [GL render thread (T45, main thread)]
    top src line: `if (!layer.autopilotEnabled || !layer.autopilotEndOfVideo)`
    frames: Autopilot.cpp:28 Autopilot::processFrame <- Renderer.cpp:553 Renderer::renderOpenGL

## per-arm counts per class (uniques; scenario e rows / scenarios a-d R5-R7-UNCLASSIFIED rows)
- arm main: A: 3 (e), B: 20 (e), C: 2 (e), R5: 5 (e)
  scenario-e per class: A 3, B 20, C 2, R5 5
- arm lane: R5: 5 (e)
  scenario-e per class: R5 5
- scenarios a-d, any arm: R5 0, R7 0, UNCLASSIFIED 0 (R7 occurs nowhere in this sweep: no access stack or access-thread creation stack of a non-main thread names ApiServer / TestServer / httplib except F1, a cerr race keyed A)

## reassignments (old family -> new class; the other 81 uniques did not move: family and class identical, notes identical)
F64: - -> R5 | F65: B -> R5 | F70: - -> R5 | F71: B -> R5 | F72: - -> R5 (main arm, scenario e)
F74: - -> R5 | F75: - -> R5 | F76: B -> R5 | F77: - -> R5 | F81: B -> R5 (lane arm, scenario e)
