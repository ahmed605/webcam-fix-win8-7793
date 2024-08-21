#include "pch.h"
#include "roapi.h"
#include <winrt/base.h>
#include "TranscodeOperation.h"

typedef DWORD(WINAPI* RegisterGadgetMessage_t)(const GUID*);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" HRESULT WINAPI WinRTGetActivationFactory(HSTRING activatableClassId, REFIID iid, void** factory)
{
	winrt::hstring hstring;
	winrt::copy_from_abi(hstring, activatableClassId);
	if (hstring == L"Windows.Media.TranscodeOperation")
	{
		auto tfactory = winrt::make<winrt::WCUI::WinRT::factory_implementation::TranscodeOperation>();
		return tfactory.as(iid, factory);
	}

	return RoGetActivationFactory(activatableClassId, iid, factory);
}

extern "C" HRESULT WINAPI WinRTRegisterActivationFactories(HSTRING* activatableClassIds, PFNGETACTIVATIONFACTORY* activationFactoryCallbacks, UINT32 count, RO_REGISTRATION_COOKIE* cookie)
{
    return RoRegisterActivationFactories(activatableClassIds, activationFactoryCallbacks, count, cookie);
};

extern "C" void WINAPI WinRTRevokeActivationFactories(RO_REGISTRATION_COOKIE cookie)
{
	return RoRevokeActivationFactories(cookie);
}

extern "C" HRESULT WINAPI WinRTInitialize(RO_INIT_TYPE initType)
{
	return RoInitialize(initType);
}

extern "C" void WINAPI WinRTUninitialize()
{
	return RoUninitialize();
}

extern "C" HWND WINAPI CreateImmersiveWindow(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, HWND hWndParent, HINSTANCE hInstance, LPVOID lpParam)
{
	HMODULE umod = GetModuleHandle(L"duser.dll");
	if (!umod) umod = LoadLibrary(L"duser.dll");

	RegisterGadgetMessage_t RegisterGadgetMessage = (RegisterGadgetMessage_t)GetProcAddress(umod, "RegisterGadgetMessage");
	if (RegisterGadgetMessage)
	{
		GUID guid0 = { 0x9FB1E2B1, 0x0DC32, 0x43D3, { 0xA0, 0xBC, 0x3A, 0xA8, 0x2A, 0xD1, 0x13, 0xE0 } };
		GUID guid1 = { 0x0B500F4A9, 0x433D, 0x489E, { 0xBD, 0x1D, 0x32, 0x37, 0x7A, 0x28, 0x23, 0x5F } };

		RegisterGadgetMessage(&guid0);
		RegisterGadgetMessage(&guid1);
	}

    WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };
    if (!GetClassInfoExW(hInstance, lpClassName, &wcex))
	{
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = DefWindowProcW;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = NULL;
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = lpClassName;
		wcex.hIconSm = NULL;

		if (!RegisterClassExW(&wcex))
			return NULL;
	}

	auto hwnd = CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWndParent, NULL, hInstance, lpParam);

	if (hwnd)
		ShowWindow(hwnd, SW_SHOWNORMAL);

	return hwnd;
}