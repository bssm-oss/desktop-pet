#pragma once
#include "FrameSequence.h"
#include <string>

namespace PNGSequenceDecoder {
    FrameSequence* decode(const std::wstring& directory, double fps = 24.0);
}
