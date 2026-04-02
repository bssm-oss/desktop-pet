#include "AnimationPlayer.h"
#include <algorithm>

AnimationPlayer::AnimationPlayer() = default;

AnimationPlayer::~AnimationPlayer() { stop(); }

void AnimationPlayer::setSpeed(double speed) {
    speed_ = std::clamp(speed, 0.1, 8.0);
}

void AnimationPlayer::load(const FrameSequence& seq) {
    stop();
    sequence_ = seq;
    playbackTime_ = 0;
    lastFrameIndex_ = -1;
    if (sequence_.count() > 0 && delegate_) {
        delegate_->onFrame(0, sequence_.frames[0]);
    }
}

void AnimationPlayer::play() {
    if (playing_ || sequence_.count() == 0) return;
    playing_ = true;
}

void AnimationPlayer::pause() {
    playing_ = false;
}

void AnimationPlayer::stop() {
    playing_ = false;
    playbackTime_ = 0;
    lastFrameIndex_ = -1;
}

void AnimationPlayer::tick(double deltaSeconds) {
    if (!playing_ || sequence_.count() == 0) return;

    playbackTime_ += deltaSeconds * speed_;
    int idx = sequence_.frameIndexAt(playbackTime_);
    if (idx == lastFrameIndex_) return;
    lastFrameIndex_ = idx;

    if (delegate_) {
        delegate_->onFrame(idx, sequence_.frames[idx]);
    }
}
