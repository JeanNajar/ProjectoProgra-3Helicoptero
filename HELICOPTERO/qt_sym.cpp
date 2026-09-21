// Workaround symbols for Qt6 + MinGW linker (libQt6EntryPoint.a needs __imp___argc).
extern "C" {
    int __imp___argc = 0;
    char **__imp___argv = nullptr;
    int __imp___wargc = 0;
    wchar_t **__imp___wargv = nullptr;
}
