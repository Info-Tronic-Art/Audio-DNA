#pragma once
#include "model/Clip.h"

// ClipTransportSync: the GL thread's transport sync between a clip's model fields and its media player (VideoPlayer
// or ImageSequence) -- lane tsan (s-rta-1002; plan T4, ruling amendment 8). Pure, templated on the player, so
// tests/test_clip_transport_sync.cpp drives it with a fake player. Renderer::syncMedia's video AND sequence branches
// both call it (test_render_thread_lint case 1 pins that Renderer.cpp never stores `playing` plainly).
//
// `playing` is the message thread's INTENT (a trigger's auto-play, the transport UI's play / pause, REST, undo) and
// the GL thread's write-back of the player's state (a OneShot that stopped itself). Today's write-back was a plain
// store of player->isPlaying(): a trigger or a pause landing between the sync's read and that store was overwritten
// (the F5 / F13 lost update -- an auto-played video stuck paused). So:
//   1. pushIntent: read the intent ONCE and push it to the player;
//   2. writeBack: store the player's playhead, then compare-exchange `playing` from the intent read in step 1 to the
//      player's state (skipped when the intent changed since the read: the new intent is pushed next frame), and
//      test the out-point on the LOCAL playhead (never a re-load of the field). A OneShot stop at the out-point
//      compare-exchanges the value this sync just wrote, so it never erases a newer intent either.
//
// ABA (ruling M-A8; grep-verified on the lane tree): an intent that flips and flips back inside the window (pause +
// play) makes the CAS succeed against a value the player never saw paused. That cannot hurt: outside src/media the
// player's play state is written ONLY here (syncMedia's two branches), and the UI transport writes only the model
// (LayerStrip / ClipInspector / MainComponent), so inside the window the player holds `wanted` or its own OneShot
// stop, and after an ABA the CAS writes exactly what it would have written with no UI activity.
//
// Player concept: bool isPlaying(); void setPlaying(bool); double getPlayheadPosition(); void seekTo(double).
namespace ClipTransportSync
{
// Step 1: read the intent once and push it to the player: start a stopped player when the clip wants to play (also
// one that stopped itself at a OneShot boundary -- the base did the same; writeBack's OneShot stop / CAS clears the
// intent in that sync, so the next push leaves it stopped), stop it when the clip does not. Returns the intent read,
// for writeBack.
template <class Player>
bool pushIntent(const Clip& clip, Player& player)
{
    const bool wanted = clip.playing.load();
    if (wanted && !player.isPlaying())
        player.setPlaying(true);
    else if (!wanted)
        player.setPlaying(false);
    return wanted;
}

// Steps 2-4, after the player advanced: the playhead, the CAS write-back of the play state, the in / out points.
template <class Player>
void writeBack(const Clip& clip, Player& player, bool wanted)
{
    const double ph = player.getPlayheadPosition();
    clip.playheadPosition.store(ph);

    // Propagate the player's state back to the model (OneShot stops, PingPong reverses) -- unless the intent changed.
    const bool now = player.isPlaying();
    bool expected = wanted;
    const bool wrote = clip.playing.compareExchange(expected, now);

    // Enforce in/out points on the playhead this sync read (no re-load of the field).
    if (clip.outPoint < 1.0f && ph >= static_cast<double>(clip.outPoint))
    {
        if (clip.loopMode == Clip::LoopMode::OneShot)
        {
            if (wrote)
            {
                bool justWritten = now;
                clip.playing.compareExchange(justWritten, false);
            }
            player.setPlaying(false);
        }
        else
        {
            player.seekTo(static_cast<double>(clip.inPoint));
            clip.playheadPosition.store(static_cast<double>(clip.inPoint));
        }
    }
}
} // namespace ClipTransportSync
