#pragma once
#include "FrameSequence.h"
#include <string>

namespace GIFDecoder {
    FrameSequence* decode(const std::wstring& filePath);
}
