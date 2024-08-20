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
		static HRESULT WINAPI GetAvailableDeviceMediaTypeHook(IMFCaptureSource* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType);
		static HRESULT WINAPI SetCurrentDeviceMediaTypeHook(IMFCaptureSource* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType** ppMediaType);
		static HRESULT WINAPI PreviewAddStreamHook(IMFCaptureSink* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType, IMFAttributes* pAttributes, DWORD* pdwSinkStreamIndex);
		static HRESULT WINAPI PhotoAddStreamHook(IMFCaptureSink* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType, IMFAttributes* pAttributes, DWORD* pdwSinkStreamIndex);
		static HRESULT WINAPI RecordAddStreamHook(IMFCaptureSink* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType, IMFAttributes* pAttributes, DWORD* pdwSinkStreamIndex);
		
		static decltype(&RemoveAllEffectsHook) GetAvailableDeviceMediaTypeOriginal;
		static decltype(&GetAvailableDeviceMediaTypeHook) SetCurrentDeviceMediaTypeOriginal;
		static decltype(&PreviewAddStreamHook) PreviewAddStreamOriginal;
		static decltype(&PhotoAddStreamHook) PhotoAddStreamOriginal;
		static decltype(&RecordAddStreamHook) RecordAddStreamOriginal;

#if ENABLE_DUI_HOOK
		static constexpr auto MAX_DUI_VERSION = 40;
		static bool IsDirectUIHooked;

		static HRESULT WINAPI InitProcessPrivHook(uint32_t dwExpectedVersion, HINSTANCE hModule, bool fRegisterControls, bool fEnableUIAutomationProvider, bool fInitCommctl);
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

		static inline DWORD RedirectStreamIndex(DWORD dwSourceStreamIndex)
		{
			switch (dwSourceStreamIndex)
			{
				case 0xFFFFFFFB:
					return MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_PHOTO;
				case 0xFFFFFFFC:
					return MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD;
				case 0xFFFFFFFD:
					//return MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_AUDIO;
					return 1;
				default:
					return dwSourceStreamIndex;
			}
		}

		static inline DWORD RedirectUnfriendlyStreamIndex(DWORD dwSourceStreamIndex)
		{
			switch (dwSourceStreamIndex)
			{
			case 0xFFFFFFFB:
			case 0xFFFFFFFC:
				return 0;
			case 0xFFFFFFFD:
				return 1;
			default:
				return dwSourceStreamIndex;
			}
		}

	public:
		static void Install();
	};
}