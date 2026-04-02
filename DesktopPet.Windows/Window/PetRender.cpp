#include "PetRender.h"
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

namespace PetRender {

void drawFrame(HWND hwnd, HBITMAP bitmap, BYTE alpha) {
    if (!hwnd || !bitmap) return;

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, bitmap);

    BITMAP bm;
    GetObjectW(bitmap, sizeof(bm), &bm);

    RECT rc;
    GetWindowRect(hwnd, &rc);

    BLENDFUNCTION bf = { AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };
    POINT ptSrc = { 0, 0 };
    POINT ptDst = { rc.left, rc.top };
    SIZE sz = { bm.bmWidth, bm.bmHeight };

    UpdateLayeredWindow(hwnd, hdcScreen, &ptDst, &sz,
        hdcMem, &ptSrc, 0, &bf, ULW_ALPHA);

    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
}

}
