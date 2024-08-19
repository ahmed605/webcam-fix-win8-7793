#include "pch.h"
#include "Hook.h"
#include <wrl.h>

using namespace WCUI;
using namespace Microsoft::WRL;

decltype(&CoCreateInstance) Hook::CoCreateInstanceOriginal = CoCreateInstance;
decltype(&Hook::RemoveAllEffectsHook) Hook::RemoveAllEffectsOriginal;
decltype(&Hook::PreviewAddStreamHook) Hook::PreviewAddStreamOriginal;
decltype(&Hook::PhotoAddStreamHook) Hook::PhotoAddStreamOriginal;

bool Hook::IsHooked = false;

static decltype(&RegQueryValueExW) OriginalRegQueryValueExW = RegQueryValueExW;

HRESULT Hook::CoCreateInstanceHook(_In_ REFCLSID rclsid, _In_opt_ LPUNKNOWN pUnkOuter, _In_ DWORD dwClsContext, _In_ REFIID riid, _COM_Outptr_ _At_(*ppv, _Post_readable_size_(_Inexpressible_(varies))) LPVOID  FAR* ppv)
{
	if (rclsid == CLSID_MFCaptureEngine && riid == IID_IMFCaptureEngine)
	{
		ComPtr<IMFCaptureEngineClassFactory> factory;
		HRESULT hr = CoCreateInstanceOriginal(CLSID_MFCaptureEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
		if (FAILED(hr))
			return hr;

		ComPtr<IMFCaptureEngine> engine;
		hr = factory->CreateInstance(CLSID_MFCaptureEngine, IID_PPV_ARGS(&engine));
		if (FAILED(hr))
			return hr;

		if (!IsHooked)
		{
			hr = InstallInternal(engine.Get());
			if (FAILED(hr))
				return hr;

			IsHooked = true;
		}

		return engine.CopyTo(riid, ppv);
	}

	return CoCreateInstanceOriginal(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

HRESULT Hook::RemoveAllEffectsHook(IMFCaptureSource* thisPtr, DWORD dwSourceStreamIndex, DWORD dwMediaTypeIndex, IMFMediaType** ppMediaType)
{
	return thisPtr->GetAvailableDeviceMediaType(MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD, dwMediaTypeIndex, ppMediaType);
}

HRESULT Hook::PreviewAddStreamHook(IMFCaptureSink* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType, IMFAttributes* pAttributes, DWORD* pdwSinkStreamIndex)
{
	return PreviewAddStreamOriginal(thisPtr, MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD, pMediaType, pAttributes, pdwSinkStreamIndex);
}

HRESULT Hook::PhotoAddStreamHook(IMFCaptureSink* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType, IMFAttributes* pAttributes, DWORD* pdwSinkStreamIndex)
{
	return PhotoAddStreamOriginal(thisPtr, MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD, pMediaType, pAttributes, pdwSinkStreamIndex);
}

LSTATUS Hook::RegQueryValueExHook
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

	auto sourceVtbl = *(void***)source.Get();
	RemoveAllEffectsOriginal = decltype(RemoveAllEffectsOriginal)(sourceVtbl[8]);

	auto previewSinkVtbl = *(void***)previewSink.Get();
	PreviewAddStreamOriginal = decltype(PreviewAddStreamOriginal)(previewSinkVtbl[5]);

	auto photoSinkVtbl = *(void***)photoSink.Get();
	PhotoAddStreamOriginal = decltype(PreviewAddStreamOriginal)(photoSinkVtbl[5]);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)RemoveAllEffectsOriginal, Hook::RemoveAllEffectsHook);
	DetourAttach(&(PVOID&)PreviewAddStreamOriginal, Hook::PreviewAddStreamHook);
	DetourAttach(&(PVOID&)PhotoAddStreamOriginal, Hook::PhotoAddStreamHook);
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
