#pragma once
#include "../Playback/FrameSequence.h"
#include <vector>

namespace PlaceholderAnimation {
    FrameSequence* make(int size = 200, int frameCount = 24);
}
