#include "AppController.h"
#include <windows.h>
#include <objbase.h>

int WINAPI WinMain(_In_ HINSTANCE hInstance,
                   _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine,
                   _In_ int nCmdShow) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    AppController app;
    app.run();

    CoUninitialize();
    return 0;
}
