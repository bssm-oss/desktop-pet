#pragma once
#include "FrameSequence.h"
#include <string>

namespace APNGDecoder {
    FrameSequence* decode(const std::wstring& filePath);
}
