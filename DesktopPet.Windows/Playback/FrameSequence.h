#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>

struct FrameSequence {
    struct Frame {
        HBITMAP bitmap = nullptr;
        int width = 0;
        int height = 0;
        int stride = 0;
        std::vector<uint8_t> pixels;
    };

    std::vector<Frame> frames;
    std::vector<double> delays;
    double totalDuration = 0.0;

    int count() const { return static_cast<int>(frames.size()); }

    int frameIndexAt(double time) const {
        if (count() <= 1 || totalDuration <= 0) return 0;
        double looped = fmod(time, totalDuration);
        if (looped < 0) looped += totalDuration;
        double accumulated = 0;
        for (int i = 0; i < count(); ++i) {
            accumulated += delays[i];
            if (looped < accumulated) return i;
        }
        return count() - 1;
    }

    void updateTotalDuration() {
        totalDuration = 0;
        for (auto d : delays) totalDuration += d;
    }
};
