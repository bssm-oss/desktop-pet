#include "VideoPlayer.h"
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <dshow.h>
#include <algorithm>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "mfreadwrite.lib")

struct VideoPlayer::Impl {
    IMFSourceReader* reader = nullptr;
    IMFSample* currentSample = nullptr;
    UINT32 width = 0;
    UINT32 height = 0;
    float speed = 1.0f;
    bool playing = false;
    LONGLONG duration = 0;
    bool useGdiPlus = true;
};

VideoPlayer::VideoPlayer() : impl_(new Impl()) {
    MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
}

VideoPlayer::~VideoPlayer() {
    stop();
    if (impl_->reader) impl_->reader->Release();
    if (impl_->currentSample) impl_->currentSample->Release();
    MFShutdown();
    delete impl_;
}

bool VideoPlayer::open(const std::wstring& filePath) {
    if (impl_->reader) {
        impl_->reader->Release();
        impl_->reader = nullptr;
    }

    HRESULT hr = MFCreateSourceReaderFromURL(filePath.c_str(), nullptr, &impl_->reader);
    if (FAILED(hr)) return false;

    IMFMediaType* mediaType = nullptr;
    hr = MFCreateMediaType(&mediaType);
    if (SUCCEEDED(hr)) {
        mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
        impl_->reader->SetCurrentMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, mediaType);
        mediaType->Release();
    }

    IMFMediaType* outputType = nullptr;
    impl_->reader->GetCurrentMediaType(
        (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &outputType);
    if (outputType) {
        MFGetAttributeSize(outputType, MF_MT_FRAME_SIZE, &impl_->width, &impl_->height);
        outputType->Release();
    }

    if (impl_->width == 0 || impl_->height == 0) {
        impl_->width = 300;
        impl_->height = 300;
    }

    return true;
}

void VideoPlayer::play() {
    impl_->playing = true;
}

void VideoPlayer::pause() {
    impl_->playing = false;
}

void VideoPlayer::stop() {
    impl_->playing = false;
}

void VideoPlayer::setSpeed(float speed) {
    impl_->speed = speed;
}

bool VideoPlayer::isPlaying() const {
    return impl_->playing;
}
