#pragma once
#include <windows.h>
#include <string>

class VideoPlayer {
public:
    class Delegate {
    public:
        virtual ~Delegate() = default;
        virtual void onVideoFrame(HBITMAP frame) = 0;
    };

    VideoPlayer();
    ~VideoPlayer();

    bool open(const std::wstring& filePath);
    void play();
    void pause();
    void stop();
    void setSpeed(float speed);
    bool isPlaying() const;

private:
    struct Impl;
    Impl* impl_ = nullptr;
};
