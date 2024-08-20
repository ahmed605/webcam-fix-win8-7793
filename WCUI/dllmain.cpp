#include "pch.h"
#include "Hook.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            WCUI::Hook::Install();
            break;
        }
    }
    return TRUE;
}

static FARPROC GetSystemFunction(const wchar_t* module, const char* name)
{
    HMODULE mod = NULL;
    BOOL wow64 = FALSE;
    WCHAR path[MAX_PATH];

    if (IsWow64Process(GetCurrentProcess(), &wow64) && wow64)
        GetSystemWow64DirectoryW(path, MAX_PATH);
    else
        GetSystemDirectoryW(path, MAX_PATH);

    lstrcatW(path, L"\\");
    lstrcatW(path, module);
    mod = LoadLibraryW(path);

    return GetProcAddress(mod, name);
}

extern "C" HRESULT WINAPI DWriteCreateFactory(unsigned int factoryType, REFIID iid, IUnknown** factory)
{
    // LOAD_LIBRARY_SEARCH_SYSTEM32 doesn't work for some reason
    auto func = (decltype(&DWriteCreateFactory))GetSystemFunction(L"dwrite.dll", __FUNCTION__);
    return func(factoryType, iid, factory);
}