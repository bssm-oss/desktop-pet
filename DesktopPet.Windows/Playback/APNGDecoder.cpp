#include "APNGDecoder.h"
#include <windows.h>
#include <wincodec.h>
#include <algorithm>

static IWICImagingFactory* getWICFactory() {
    static IWICImagingFactory* factory = nullptr;
    if (!factory) {
        CoCreateInstance(CLSID_WICImagingFactory2, nullptr,
            CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    }
    return factory;
}

namespace APNGDecoder {

FrameSequence* decode(const std::wstring& filePath) {
    IWICImagingFactory* factory = getWICFactory();
    if (!factory) return nullptr;

    IWICBitmapDecoder* decoder = nullptr;
    HRESULT hr = factory->CreateDecoderFromFilename(
        filePath.c_str(), nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) return nullptr;

    UINT frameCount = 0;
    decoder->GetFrameCount(&frameCount);
    if (frameCount == 0) {
        decoder->Release();
        return nullptr;
    }

    auto* seq = new FrameSequence();
    seq->frames.resize(frameCount);
    seq->delays.resize(frameCount, 1.0 / 24.0);

    for (UINT i = 0; i < frameCount; ++i) {
        IWICBitmapFrameDecode* frame = nullptr;
        if (FAILED(decoder->GetFrame(i, &frame))) continue;

        IWICFormatConverter* converter = nullptr;
        factory->CreateFormatConverter(&converter);
        converter->Initialize(frame, GUID_WICPixelFormat32bppBGRA,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);

        UINT w = 0, h = 0;
        converter->GetSize(&w, &h);

        auto& f = seq->frames[i];
        f.width = w;
        f.height = h;
        f.stride = w * 4;
        f.pixels.resize(f.stride * h);
        converter->CopyPixels(nullptr, f.stride, (UINT)f.pixels.size(), f.pixels.data());

        // Premultiply alpha once at decode time for UpdateLayeredWindow AC_SRC_ALPHA
        for (int p = 0, n = (int)f.pixels.size(); p < n; p += 4) {
            BYTE a = f.pixels[p + 3];
            if (a > 0 && a < 255) {
                f.pixels[p]     = (BYTE)(f.pixels[p]     * a / 255);
                f.pixels[p + 1] = (BYTE)(f.pixels[p + 1] * a / 255);
                f.pixels[p + 2] = (BYTE)(f.pixels[p + 2] * a / 255);
            }
        }

        BITMAPINFOHEADER bi = {};
        bi.biSize = sizeof(bi);
        bi.biWidth = w;
        bi.biHeight = -(LONG)h;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = (DWORD)f.pixels.size();

        HDC hdc = GetDC(nullptr);
        f.bitmap = CreateDIBitmap(hdc, &bi, CBM_INIT,
            f.pixels.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        ReleaseDC(nullptr, hdc);

        IWICMetadataQueryReader* meta = nullptr;
        if (SUCCEEDED(frame->GetMetadataQueryReader(&meta))) {
            PROPVARIANT prop;
            PropVariantInit(&prop);
            double delay = 1.0 / 24.0;
            if (SUCCEEDED(meta->GetMetadataByName(L"/acTL/num_frames", &prop))) {
                // APNG has multiple frames
            }
            PropVariantClear(&prop);

            PropVariantInit(&prop);
            if (SUCCEEDED(meta->GetMetadataByName(L"/fcTL/DelayNumerator", &prop)) && prop.vt == VT_UI2) {
                UINT16 num = prop.uiVal;
                PropVariantClear(&prop);
                PropVariantInit(&prop);
                UINT16 den = 100;
                if (SUCCEEDED(meta->GetMetadataByName(L"/fcTL/DelayDenominator", &prop)) && prop.vt == VT_UI2) {
                    den = prop.uiVal;
                }
                if (den > 0) delay = (double)num / den;
            }
            PropVariantClear(&prop);
            seq->delays[i] = std::max(delay, 0.02);
            meta->Release();
        }

        converter->Release();
        frame->Release();
    }

    decoder->Release();

    if (seq->frames.size() == 1) {
        seq->delays[0] = 1.0;
    }

    seq->updateTotalDuration();

    if (seq->frames.empty()) {
        delete seq;
        return nullptr;
    }
    return seq;
}

}
