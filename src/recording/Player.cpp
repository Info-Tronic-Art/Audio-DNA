#include "recording/Player.h"
#include <juce_core/juce_core.h>
#include <algorithm>

Player::Player(std::shared_ptr<const Program> program)
    : prog_(std::move(program))
{
    jassert(prog_ != nullptr);
    cursors_.assign(prog_->continuous.size(), LaneCursor{});
}

void Player::start(double at)
{
    pos_ = at;
    running_ = true;

    nextDiscrete_ = 0;
    while (nextDiscrete_ < prog_->discrete.size() && prog_->discrete[nextDiscrete_].at < at)
        ++nextDiscrete_;

    cursors_.assign(prog_->continuous.size(), LaneCursor{});
    for (size_t i = 0; i < prog_->continuous.size(); ++i)
    {
        const auto& gestures = prog_->continuous[i].gestures;
        size_t idx = 0;
        while (idx < gestures.size() && gestures[idx].x1 <= at)
            ++idx;
        cursors_[i].gestureIndex = idx;
        // A start() landing INSIDE a gesture's span joins it on the next
        // advanceTo() call via the normal touch() path (row 1 does not
        // synthesize a "joined in progress" state -- not in this step's
        // test scope).
    }
}

void Player::advanceTo(double pos, Sink& sink)
{
    if (!running_) return;
    pos_ = pos;

    while (nextDiscrete_ < prog_->discrete.size() && prog_->discrete[nextDiscrete_].at <= pos)
    {
        sink.fire(prog_->discrete[nextDiscrete_]);
        ++nextDiscrete_;
    }

    for (size_t i = 0; i < prog_->continuous.size(); ++i)
    {
        const ContLane& lane = prog_->continuous[i];
        LaneCursor& cur = cursors_[i];

        // A while loop, not an if: a coarse dt can open AND close one or
        // more short gestures within a single advanceTo() call, and each
        // one still gets its touch()/release() pair (D3/D5) -- never
        // silently skipped.
        while (cur.gestureIndex < lane.gestures.size())
        {
            const ContLane::G& g = lane.gestures[cur.gestureIndex];

            if (!cur.inGesture)
            {
                if (pos < g.x0)
                    break;   // not there yet; try again next call

                const bool accepted = sink.touch(lane.key, g.grip);
                cur.inGesture = true;
                cur.displaced = !accepted;
            }

            if (pos >= g.x1)
            {
                if (!cur.displaced)
                    sink.release(lane.key);
                cur.inGesture = false;
                cur.displaced = false;
                ++cur.gestureIndex;
                continue;   // pos may already reach the NEXT gesture too
            }

            if (!cur.displaced)
            {
                const float v = g.curve.eval(pos);
                if (!sink.set(lane.key, v))
                    cur.displaced = true;   // TOUCH refusal: displaced for THIS gesture only
            }
            break;   // still inside this gesture; done for this call
        }
    }
}

void Player::stop(Sink& sink)
{
    for (size_t i = 0; i < cursors_.size(); ++i)
    {
        auto& cur = cursors_[i];
        if (cur.inGesture && !cur.displaced)
            sink.release(prog_->continuous[i].key);
        cur.inGesture = false;
        cur.displaced = false;
    }
    running_ = false;
}

void Player::swap(std::shared_ptr<const Program> newProgram, Sink& sink)
{
    // Snapshot which keys the OLD program actively HELD at pos_.
    std::vector<ControlPath> heldKeys;
    for (size_t i = 0; i < cursors_.size(); ++i)
        if (cursors_[i].inGesture && !cursors_[i].displaced)
            heldKeys.push_back(prog_->continuous[i].key);

    prog_ = std::move(newProgram);

    nextDiscrete_ = 0;
    while (nextDiscrete_ < prog_->discrete.size() && prog_->discrete[nextDiscrete_].at < pos_)
        ++nextDiscrete_;

    cursors_.assign(prog_->continuous.size(), LaneCursor{});
    for (size_t i = 0; i < prog_->continuous.size(); ++i)
    {
        const ContLane& cl = prog_->continuous[i];
        const auto& gestures = cl.gestures;
        size_t idx = 0;
        while (idx < gestures.size() && gestures[idx].x1 <= pos_)
            ++idx;
        cursors_[i].gestureIndex = idx;

        const bool coveredNow = idx < gestures.size() && pos_ >= gestures[idx].x0 && pos_ < gestures[idx].x1;
        const bool wasHeld = std::find(heldKeys.begin(), heldKeys.end(), cl.key) != heldKeys.end();
        if (coveredNow && wasHeld)
            cursors_[i].inGesture = true;   // re-seated, still held -- no fresh touch()
        // Not held before, covered now: the next advanceTo() touches it
        // normally (its x0 <= pos_ < x1, same as any fresh entry).
    }

    // Orphans (D5): held before, no longer covered by the new program at all.
    for (const auto& key : heldKeys)
    {
        bool stillCovered = false;
        for (const auto& cl : prog_->continuous)
        {
            if (cl.key != key) continue;
            for (const auto& g : cl.gestures)
                if (pos_ >= g.x0 && pos_ < g.x1) { stillCovered = true; break; }
            break;
        }
        if (!stillCovered)
            sink.release(key);
    }
}

void Player::reenable(const ControlPath& key)
{
    for (size_t i = 0; i < prog_->continuous.size(); ++i)
        if (prog_->continuous[i].key == key)
            cursors_[i].displaced = false;
}

void Player::reenableAll()
{
    for (auto& cur : cursors_)
        cur.displaced = false;
}
