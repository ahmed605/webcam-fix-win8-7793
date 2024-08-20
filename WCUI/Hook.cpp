#include "pch.h"
#include "Hook.h"
#include <wrl.h>

using namespace WCUI;
using namespace Microsoft::WRL;

decltype(&CoCreateInstance) Hook::CoCreateInstanceOriginal = CoCreateInstance;
decltype(&Hook::RemoveAllEffectsHook) Hook::GetAvailableDeviceMediaTypeOriginal;
decltype(&Hook::GetAvailableDeviceMediaTypeHook) Hook::SetCurrentDeviceMediaTypeOriginal;
decltype(&Hook::PreviewAddStreamHook) Hook::PreviewAddStreamOriginal;
decltype(&Hook::PhotoAddStreamHook) Hook::PhotoAddStreamOriginal;
decltype(&Hook::RecordAddStreamHook) Hook::RecordAddStreamOriginal;

bool Hook::IsMediaFoundationHooked = false;

#if ENABLE_DUI_HOOK
decltype(&Hook::InitProcessPrivHook) Hook::InitProcessPrivOriginal;
bool Hook::IsDirectUIHooked = false;
#endif

static decltype(&RegQueryValueExW) OriginalRegQueryValueExW = RegQueryValueExW;

HRESULT WINAPI Hook::CoCreateInstanceHook(_In_ REFCLSID rclsid, _In_opt_ LPUNKNOWN pUnkOuter, _In_ DWORD dwClsContext, _In_ REFIID riid, LPVOID* ppv)
{
	if (rclsid == CLSID_MFCaptureEngine)
	{
		ComPtr<IMFCaptureEngineClassFactory> factory;
		HRESULT hr = CoCreateInstanceOriginal(CLSID_MFCaptureEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
		if (FAILED(hr))
			return hr;

		ComPtr<IMFCaptureEngine> engine;
		hr = factory->CreateInstance(CLSID_MFCaptureEngine, IID_PPV_ARGS(&engine));
		if (FAILED(hr))
			return hr;

		if (!IsMediaFoundationHooked)
		{
			hr = InstallInternal(engine.Get());
			if (FAILED(hr))
				return hr;

			IsMediaFoundationHooked = true;
		}

		if (riid == IID_IMFCaptureEngine)
			return engine.CopyTo(riid, ppv);
		else if (riid == IID_IMFCaptureEngineClassFactory)
			return factory.CopyTo(riid, ppv);
		else
			return factory->QueryInterface(riid, ppv);
	}

	return CoCreateInstanceOriginal(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

HRESULT WINAPI Hook::RemoveAllEffectsHook(IMFCaptureSource* thisPtr, DWORD dwSourceStreamIndex, DWORD dwMediaTypeIndex, IMFMediaType** ppMediaType)
{
	return GetAvailableDeviceMediaTypeOriginal(thisPtr, RedirectStreamIndex(dwSourceStreamIndex), dwMediaTypeIndex, ppMediaType);
}

HRESULT WINAPI Hook::GetAvailableDeviceMediaTypeHook(IMFCaptureSource* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType)
{
	return SetCurrentDeviceMediaTypeOriginal(thisPtr, RedirectUnfriendlyStreamIndex(dwSourceStreamIndex), pMediaType);
}

HRESULT WINAPI Hook::SetCurrentDeviceMediaTypeHook(IMFCaptureSource* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType** ppMediaType)
{
	return thisPtr->GetCurrentDeviceMediaType(RedirectUnfriendlyStreamIndex(dwSourceStreamIndex), ppMediaType);
}

HRESULT WINAPI Hook::PreviewAddStreamHook(IMFCaptureSink* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType, IMFAttributes* pAttributes, DWORD* pdwSinkStreamIndex)
{
	return PreviewAddStreamOriginal(thisPtr, RedirectStreamIndex(dwSourceStreamIndex), pMediaType, pAttributes, pdwSinkStreamIndex);
}

HRESULT WINAPI Hook::PhotoAddStreamHook(IMFCaptureSink* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType, IMFAttributes* pAttributes, DWORD* pdwSinkStreamIndex)
{
	return PhotoAddStreamOriginal(thisPtr, RedirectStreamIndex(dwSourceStreamIndex), pMediaType, pAttributes, pdwSinkStreamIndex);
}

HRESULT WINAPI Hook::RecordAddStreamHook(IMFCaptureSink* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType, IMFAttributes* pAttributes, DWORD* pdwSinkStreamIndex)
{
	return RecordAddStreamOriginal(thisPtr, RedirectStreamIndex(dwSourceStreamIndex), pMediaType, pAttributes, pdwSinkStreamIndex);
}

#if ENABLE_DUI_HOOK
HRESULT WINAPI WCUI::Hook::InitProcessPrivHook(uint32_t dwExpectedVersion, HINSTANCE hModule, bool fRegisterControls, bool fEnableUIAutomationProvider, bool fInitCommctl)
{
	auto result = InitProcessPrivOriginal(14, hModule, fRegisterControls, fEnableUIAutomationProvider, fInitCommctl);
	if (!SUCCEEDED(result))
	{
		result = InitProcessPrivOriginal(9, hModule, fRegisterControls, fEnableUIAutomationProvider, fInitCommctl);

		if (!SUCCEEDED(result))
		{
			result = InitProcessPrivOriginal(8, hModule, fRegisterControls, fEnableUIAutomationProvider, fInitCommctl);

			if (!SUCCEEDED(result))
			{
				for (uint32_t i = 10; i <= MAX_DUI_VERSION; i++)
				{
					result = InitProcessPrivOriginal(i, hModule, fRegisterControls, fEnableUIAutomationProvider, fInitCommctl);
					if (SUCCEEDED(result))
						return result;
				}
			}
		}
	}

	return result;
}
#endif

LSTATUS WINAPI Hook::RegQueryValueExHook
(
	_In_ HKEY hKey,
	_In_opt_ LPCWSTR lpValueName,
	_Reserved_ LPDWORD lpReserved,
	_Out_opt_ LPDWORD lpType,
	_Out_writes_bytes_to_opt_(*lpcbData, *lpcbData) __out_data_source(REGISTRY) LPBYTE lpData,
	_When_(lpData == NULL, _Out_opt_) _When_(lpData != NULL, _Inout_opt_) LPDWORD lpcbData
)
{
	if (lpValueName && wcscmp(lpValueName, L"RemoteFontBootCacheFlags") == 0)
	{
		if(lpType)
			*lpType = REG_DWORD;

		if (lpcbData)
			*lpcbData = 4;

		if (lpData)
			*(DWORD*)lpData = 4111;

#if ENABLE_DUI_HOOK
		if (!IsDirectUIHooked)
		{
			HMODULE dmod = GetModuleHandle(L"dui70.dll");
			if (!dmod) dmod = LoadLibrary(L"dui70.dll");

			InitProcessPrivOriginal = (decltype(&InitProcessPrivHook))GetProcAddress(dmod, "InitProcessPriv");
			if (InitProcessPrivOriginal)
			{
				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());
				DetourAttach(&(PVOID&)InitProcessPrivOriginal, Hook::InitProcessPrivHook);
				DetourTransactionCommit();

				IsDirectUIHooked = true;
			}
		}
#endif

		return ERROR_SUCCESS;
	}

	return OriginalRegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

HRESULT Hook::InstallInternal(IMFCaptureEngine* engine)
{
	ComPtr<IMFCaptureSource> source;
	HRESULT hr = engine->GetSource(&source);
	if (FAILED(hr))
		return hr;

	ComPtr<IMFCaptureSink> previewSink;
	engine->GetSink(MF_CAPTURE_ENGINE_SINK_TYPE_PREVIEW, &previewSink);
	if (FAILED(hr))
		return hr;

	ComPtr<IMFCaptureSink> photoSink;
	engine->GetSink(MF_CAPTURE_ENGINE_SINK_TYPE_PHOTO, &photoSink);
	if (FAILED(hr))
		return hr;

	ComPtr<IMFCaptureSink> recordSink;
	engine->GetSink(MF_CAPTURE_ENGINE_SINK_TYPE_RECORD, &recordSink);
	if (FAILED(hr))
		return hr;

	auto sourceVtbl = *(void***)source.Get();
	auto removeAllEffects = decltype(GetAvailableDeviceMediaTypeOriginal)(sourceVtbl[8]);
	GetAvailableDeviceMediaTypeOriginal = decltype(GetAvailableDeviceMediaTypeOriginal)(sourceVtbl[9]);
	SetCurrentDeviceMediaTypeOriginal = decltype(SetCurrentDeviceMediaTypeOriginal)(sourceVtbl[10]);

	auto previewSinkVtbl = *(void***)previewSink.Get();
	PreviewAddStreamOriginal = decltype(PreviewAddStreamOriginal)(previewSinkVtbl[5]);

	auto photoSinkVtbl = *(void***)photoSink.Get();
	PhotoAddStreamOriginal = decltype(PreviewAddStreamOriginal)(photoSinkVtbl[5]);

	auto recordSinkVtbl = *(void***)recordSink.Get();
	RecordAddStreamOriginal = decltype(RecordAddStreamOriginal)(recordSinkVtbl[5]);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)removeAllEffects, Hook::RemoveAllEffectsHook);
	DetourAttach(&(PVOID&)GetAvailableDeviceMediaTypeOriginal, Hook::GetAvailableDeviceMediaTypeHook);
	DetourAttach(&(PVOID&)SetCurrentDeviceMediaTypeOriginal, Hook::SetCurrentDeviceMediaTypeHook);
	DetourAttach(&(PVOID&)PreviewAddStreamOriginal, Hook::PreviewAddStreamHook);
	DetourAttach(&(PVOID&)PhotoAddStreamOriginal, Hook::PhotoAddStreamHook);
	DetourAttach(&(PVOID&)RecordAddStreamOriginal, Hook::RecordAddStreamHook);
	DetourTransactionCommit();
}

void Hook::Install()
{
	HMODULE amod = GetModuleHandle(L"advapi32.dll");
	OriginalRegQueryValueExW = (decltype(&RegQueryValueExW))GetProcAddress(amod, "RegQueryValueExW");

	HMODULE cmod = GetModuleHandle(L"combase.dll");
	CoCreateInstanceOriginal = (decltype(&CoCreateInstance))GetProcAddress(cmod, "CoCreateInstance");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)OriginalRegQueryValueExW, Hook::RegQueryValueExHook);
	DetourAttach(&(PVOID&)CoCreateInstanceOriginal, Hook::CoCreateInstanceHook);
	DetourTransactionCommit();
}
