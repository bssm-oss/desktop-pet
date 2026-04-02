#include "PNGSequenceDecoder.h"
#include <windows.h>
#include <wincodec.h>
#include <algorithm>
#include <vector>
#include <string>

static IWICImagingFactory* getFactory() {
    static IWICImagingFactory* factory = nullptr;
    if (!factory) {
        CoCreateInstance(CLSID_WICImagingFactory2, nullptr,
            CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    }
    return factory;
}

namespace PNGSequenceDecoder {

FrameSequence* decode(const std::wstring& directory, double fps) {
    std::wstring searchPath = directory;
    if (!searchPath.empty() && searchPath.back() != L'\\')
        searchPath += L'\\';
    searchPath += L"*.png";

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return nullptr;

    std::vector<std::wstring> files;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wstring fullPath = directory;
            if (!fullPath.empty() && fullPath.back() != L'\\') fullPath += L'\\';
            fullPath += fd.cFileName;
            files.push_back(fullPath);
        }
    } while (FindNextFileW(hFind, &fd));
    FindClose(hFind);

    std::sort(files.begin(), files.end());

    if (files.empty()) return nullptr;

    IWICImagingFactory* factory = getFactory();
    if (!factory) return nullptr;

    auto* seq = new FrameSequence();
    double delay = 1.0 / std::max(fps, 1.0);

    for (auto& filePath : files) {
        IWICBitmapDecoder* decoder = nullptr;
        if (FAILED(factory->CreateDecoderFromFilename(filePath.c_str(), nullptr,
            GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)))
            continue;

        IWICBitmapFrameDecode* frame = nullptr;
        if (FAILED(decoder->GetFrame(0, &frame))) {
            decoder->Release();
            continue;
        }

        IWICFormatConverter* converter = nullptr;
        factory->CreateFormatConverter(&converter);
        converter->Initialize(frame, GUID_WICPixelFormat32bppBGRA,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);

        UINT w = 0, h = 0;
        converter->GetSize(&w, &h);

        FrameSequence::Frame f;
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

        seq->frames.push_back(std::move(f));
        seq->delays.push_back(delay);

        converter->Release();
        frame->Release();
        decoder->Release();
    }

    seq->updateTotalDuration();

    if (seq->frames.empty()) {
        delete seq;
        return nullptr;
    }
    return seq;
}

}
