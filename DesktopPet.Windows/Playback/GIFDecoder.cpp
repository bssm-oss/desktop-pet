#include "GIFDecoder.h"
#include <windows.h>
#include <wincodec.h>
#include <comdef.h>
#include <algorithm>
#include <vector>
#include <cstring>

static IWICImagingFactory* getFactory() {
    static IWICImagingFactory* factory = nullptr;
    if (!factory) {
        CoCreateInstance(CLSID_WICImagingFactory2, nullptr,
            CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    }
    return factory;
}

namespace GIFDecoder {

FrameSequence* decode(const std::wstring& filePath) {
    IWICImagingFactory* factory = getFactory();
    if (!factory) return nullptr;

    IWICBitmapDecoder* decoder = nullptr;
    HRESULT hr = factory->CreateDecoderFromFilename(
        filePath.c_str(), nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) return nullptr;

    UINT frameCount = 0;
    decoder->GetFrameCount(&frameCount);
    if (frameCount == 0) { decoder->Release(); return nullptr; }

    IWICBitmapFrameDecode* firstFrame = nullptr;
    if (FAILED(decoder->GetFrame(0, &firstFrame))) { decoder->Release(); return nullptr; }
    UINT canvasW = 0, canvasH = 0;
    firstFrame->GetSize(&canvasW, &canvasH);
    firstFrame->Release();

    if (canvasW == 0 || canvasH == 0) { decoder->Release(); return nullptr; }

    auto* seq = new FrameSequence();
    seq->frames.resize(frameCount);
    seq->delays.resize(frameCount, 0.1);

    std::vector<BYTE> canvas(canvasW * canvasH * 4, 0);
    std::vector<BYTE> backup(canvasW * canvasH * 4, 0);

    for (UINT i = 0; i < frameCount; ++i) {
        IWICBitmapFrameDecode* frame = nullptr;
        if (FAILED(decoder->GetFrame(i, &frame))) continue;

        IWICFormatConverter* converter = nullptr;
        factory->CreateFormatConverter(&converter);
        converter->Initialize(frame, GUID_WICPixelFormat32bppBGRA,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);

        UINT fw = 0, fh = 0;
        converter->GetSize(&fw, &fh);

        UINT left = 0, top = 0;
        int disposal = 0;

        IWICMetadataQueryReader* meta = nullptr;
        if (SUCCEEDED(frame->GetMetadataQueryReader(&meta))) {
            PROPVARIANT prop;
            PropVariantInit(&prop);
            if (SUCCEEDED(meta->GetMetadataByName(L"/imgdesc/Left", &prop)) && prop.vt == VT_UI2)
                left = prop.uiVal;
            PropVariantClear(&prop);
            PropVariantInit(&prop);
            if (SUCCEEDED(meta->GetMetadataByName(L"/imgdesc/Top", &prop)) && prop.vt == VT_UI2)
                top = prop.uiVal;
            PropVariantClear(&prop);
            PropVariantInit(&prop);
            if (SUCCEEDED(meta->GetMetadataByName(L"/grctlext/Delay", &prop)) && prop.vt == VT_UI2) {
                int delayCs = prop.uiVal;
                seq->delays[i] = (delayCs > 0) ? delayCs / 100.0 : 0.1;
            }
            PropVariantClear(&prop);
            PropVariantInit(&prop);
            if (SUCCEEDED(meta->GetMetadataByName(L"/grctlext/Disposal", &prop)) && prop.vt == VT_UI1)
                disposal = prop.bVal;
            PropVariantClear(&prop);
            meta->Release();
        }

        std::vector<BYTE> framePixels(fw * fh * 4, 0);
        converter->CopyPixels(nullptr, fw * 4, (UINT)framePixels.size(), framePixels.data());

        memcpy(backup.data(), canvas.data(), canvas.size());

        for (UINT row = 0; row < fh; ++row) {
            UINT canvasY = top + row;
            if (canvasY >= canvasH) continue;
            for (UINT col = 0; col < fw; ++col) {
                UINT canvasX = left + col;
                if (canvasX >= canvasW) continue;
                BYTE* src = &framePixels[(row * fw + col) * 4];
                BYTE a = src[3];
                if (a == 0) continue;
                BYTE* dst = &canvas[(canvasY * canvasW + canvasX) * 4];
                if (a == 255) {
                    dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = 255;
                } else {
                    dst[0] = (BYTE)((src[0] * a + dst[0] * (255 - a)) / 255);
                    dst[1] = (BYTE)((src[1] * a + dst[1] * (255 - a)) / 255);
                    dst[2] = (BYTE)((src[2] * a + dst[2] * (255 - a)) / 255);
                    dst[3] = (BYTE)std::min(255, (int)((a * 255 + dst[3] * (255 - a)) / 255));
                }
            }
        }

        auto& f = seq->frames[i];
        f.width = canvasW;
        f.height = canvasH;
        f.stride = canvasW * 4;
        f.pixels = canvas;

        // Premultiply alpha once at decode time for UpdateLayeredWindow AC_SRC_ALPHA
        for (int p = 0, n = (int)f.pixels.size(); p < n; p += 4) {
            BYTE a = f.pixels[p + 3];
            if (a > 0 && a < 255) {
                f.pixels[p]     = (BYTE)(f.pixels[p]     * a / 255);
                f.pixels[p + 1] = (BYTE)(f.pixels[p + 1] * a / 255);
                f.pixels[p + 2] = (BYTE)(f.pixels[p + 2] * a / 255);
            }
        }

        seq->delays[i] = std::max(seq->delays[i], 0.02);

        converter->Release();
        frame->Release();

        if (disposal == 2) {
            for (UINT row = 0; row < fh; ++row) {
                UINT canvasY = top + row;
                if (canvasY >= canvasH) continue;
                for (UINT col = 0; col < fw; ++col) {
                    UINT canvasX = left + col;
                    if (canvasX >= canvasW) continue;
                    memset(&canvas[(canvasY * canvasW + canvasX) * 4], 0, 4);
                }
            }
        } else if (disposal == 3) {
            memcpy(canvas.data(), backup.data(), canvas.size());
        }
    }

    decoder->Release();
    seq->updateTotalDuration();

    if (seq->frames.empty()) { delete seq; return nullptr; }
    return seq;
}

}
