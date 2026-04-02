#pragma once
#include "FrameSequence.h"
#include <string>

class OverlayWindow;

class AnimationPlayer {
public:
    class Delegate {
    public:
        virtual ~Delegate() = default;
        virtual void onFrame(int frameIndex, const FrameSequence::Frame& frame) = 0;
    };

    AnimationPlayer();
    ~AnimationPlayer();

    void setDelegate(Delegate* d) { delegate_ = d; }
    void setSpeed(double speed);
    void load(const FrameSequence& seq);
    void play();
    void pause();
    void stop();
    bool isPlaying() const { return playing_; }

    void tick(double deltaSeconds);

private:
    Delegate* delegate_ = nullptr;
    FrameSequence sequence_;
    double speed_ = 1.0;
    double playbackTime_ = 0.0;
    int lastFrameIndex_ = -1;
    bool playing_ = false;
};
