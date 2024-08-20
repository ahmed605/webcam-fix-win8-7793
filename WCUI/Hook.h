#pragma once

#include <Windows.h>
#include <Objbase.h>
#include <detours/detours.h>
#include <mfapi.h>
#include <mfcaptureengine.h>
#include <cstdint>

#define ENABLE_DUI_HOOK false

namespace WCUI
{
	class Hook
	{
	private:
		static decltype(&CoCreateInstance) CoCreateInstanceOriginal;
		static HRESULT WINAPI CoCreateInstanceHook(_In_ REFCLSID rclsid, _In_opt_ LPUNKNOWN pUnkOuter, _In_ DWORD dwClsContext, _In_ REFIID riid, LPVOID* ppv);
		
		static HRESULT WINAPI RemoveAllEffectsHook(IMFCaptureSource* thisPtr, DWORD dwSourceStreamIndex, DWORD dwMediaTypeIndex, IMFMediaType** ppMediaType);
		static HRESULT WINAPI PreviewAddStreamHook(IMFCaptureSink* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType, IMFAttributes* pAttributes, DWORD* pdwSinkStreamIndex);
		static HRESULT WINAPI PhotoAddStreamHook(IMFCaptureSink* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType, IMFAttributes* pAttributes, DWORD* pdwSinkStreamIndex);
		
		static decltype(&RemoveAllEffectsHook) RemoveAllEffectsOriginal;
		static decltype(&PreviewAddStreamHook) PreviewAddStreamOriginal;
		static decltype(&PhotoAddStreamHook) PhotoAddStreamOriginal;

#if ENABLE_DUI_HOOK
		static constexpr auto MAX_DUI_VERSION = 40;
		static bool IsDirectUIHooked;

		static HRESULT InitProcessPrivHook(uint32_t dwExpectedVersion, HINSTANCE hModule, bool fRegisterControls, bool fEnableUIAutomationProvider, bool fInitCommctl);
		static decltype(&InitProcessPrivHook) InitProcessPrivOriginal;
#endif

		static LSTATUS WINAPI RegQueryValueExHook
		(
			_In_ HKEY hKey,
			_In_opt_ LPCWSTR lpValueName,
			_Reserved_ LPDWORD lpReserved,
			_Out_opt_ LPDWORD lpType,
			_Out_writes_bytes_to_opt_(*lpcbData, *lpcbData) __out_data_source(REGISTRY) LPBYTE lpData,
			_When_(lpData == NULL, _Out_opt_) _When_(lpData != NULL, _Inout_opt_) LPDWORD lpcbData
		);
		static bool IsMediaFoundationHooked;

		static HRESULT InstallInternal(IMFCaptureEngine* engine);

	public:
		static void Install();
	};
}