// Workaround MinGW: libQt6EntryPoint.a referencia simbolos MSVC
// (__imp___argc/__imp___argv); los proporcionamos para que el enlace funcione.
#include <windows.h>

extern "C" {
    int __argc = 0;
    char **__argv = nullptr;
    int __wargc = 0;
    wchar_t **__wargv = nullptr;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Qt6::EntryPoint proporciona el main real; esto solo satisface el enlace.
    return 0;
}
