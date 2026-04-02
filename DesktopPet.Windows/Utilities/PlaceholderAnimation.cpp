#include "PlaceholderAnimation.h"
#include <windows.h>
#include <algorithm>
#include <cmath>

namespace PlaceholderAnimation {

FrameSequence* make(int size, int frameCount) {
    auto* seq = new FrameSequence();
    seq->frames.resize(frameCount);
    seq->delays.resize(frameCount, 1.0 / 24.0);

    HDC hdc = GetDC(nullptr);
    HDC memDC = CreateCompatibleDC(hdc);

    for (int i = 0; i < frameCount; ++i) {
        BITMAPINFOHEADER bi = {};
        bi.biSize = sizeof(bi);
        bi.biWidth = size;
        bi.biHeight = -size;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;

        void* bits = nullptr;
        HBITMAP bmp = CreateDIBSection(hdc, (BITMAPINFO*)&bi,
            DIB_RGB_COLORS, &bits, nullptr, 0);

        DWORD* pixels = (DWORD*)bits;
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                pixels[y * size + x] = 0x00000000;
            }
        }

        double t = (double)i / frameCount;
        double angle = t * 3.14159265 * 2;
        double bounceY = sin(angle) * 30.0;
        int cx = size / 2;
        int cy = (int)(size / 2 + bounceY);
        int radius = 50;

        double hue = 0.8 + t * 0.2;
        BYTE r = (BYTE)(128 + 127 * sin(hue * 6.28));
        BYTE g = (BYTE)(128 + 127 * sin(hue * 6.28 + 2.09));
        BYTE b = (BYTE)(128 + 127 * sin(hue * 6.28 + 4.18));
        DWORD bodyColor = (0xFF << 24) | (b << 16) | (g << 8) | r;

        for (int py = -radius; py <= radius; ++py) {
            for (int px = -radius; px <= radius; ++px) {
                if (px * px + py * py <= radius * radius) {
                    int sx = cx + px;
                    int sy = cy - py;
                    if (sx >= 0 && sx < size && sy >= 0 && sy < size) {
                        pixels[sy * size + sx] = bodyColor;
                    }
                }
            }
        }

        DWORD eyeColor = 0xE6FFFFFF;
        int eyeR = 8;
        for (int e = 0; e < 2; ++e) {
            int ex = cx + (e == 0 ? -14 : 10);
            int ey = cy + 10;
            for (int py = -eyeR; py <= eyeR; ++py) {
                for (int px = -eyeR; px <= eyeR; ++px) {
                    if (px * px + py * py <= eyeR * eyeR) {
                        int sx = ex + px;
                        int sy = ey + py;
                        if (sx >= 0 && sx < size && sy >= 0 && sy < size) {
                            pixels[(size - 1 - sy) * size + sx] = eyeColor;
                        }
                    }
                }
            }
        }

        DWORD pupilColor = 0xFF1A1A1A;
        int pupilR = 4;
        for (int e = 0; e < 2; ++e) {
            int ex = cx + (e == 0 ? -10 : 14);
            int ey = cy + 14;
            for (int py = -pupilR; py <= pupilR; ++py) {
                for (int px = -pupilR; px <= pupilR; ++px) {
                    if (px * px + py * py <= pupilR * pupilR) {
                        int sx = ex + px;
                        int sy = ey + py;
                        if (sx >= 0 && sx < size && sy >= 0 && sy < size) {
                            pixels[(size - 1 - sy) * size + sx] = pupilColor;
                        }
                    }
                }
            }
        }

        auto& frame = seq->frames[i];
        frame.width = size;
        frame.height = size;
        frame.stride = size * 4;
        frame.pixels.resize(frame.stride * size);
        memcpy(frame.pixels.data(), pixels, frame.stride * size);

        // Premultiply alpha once at decode time for UpdateLayeredWindow AC_SRC_ALPHA
        for (int p = 0, n = (int)frame.pixels.size(); p < n; p += 4) {
            BYTE a = frame.pixels[p + 3];
            if (a > 0 && a < 255) {
                frame.pixels[p]     = (BYTE)(frame.pixels[p]     * a / 255);
                frame.pixels[p + 1] = (BYTE)(frame.pixels[p + 1] * a / 255);
                frame.pixels[p + 2] = (BYTE)(frame.pixels[p + 2] * a / 255);
            }
        }

        frame.bitmap = bmp;

        SelectObject(memDC, bmp);
    }

    DeleteDC(memDC);
    ReleaseDC(nullptr, hdc);

    seq->updateTotalDuration();
    return seq;
}

}
