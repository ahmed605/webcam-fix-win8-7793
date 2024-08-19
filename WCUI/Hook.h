#pragma once

#include <Windows.h>
#include <Objbase.h>
#include <detours/detours.h>
#include <mfapi.h>
#include <mfcaptureengine.h>

namespace WCUI
{
	class Hook
	{
	private:
		static decltype(&CoCreateInstance) CoCreateInstanceOriginal;
		static HRESULT CoCreateInstanceHook(_In_ REFCLSID rclsid, _In_opt_ LPUNKNOWN pUnkOuter, _In_ DWORD dwClsContext, _In_ REFIID riid, _COM_Outptr_ _At_(*ppv, _Post_readable_size_(_Inexpressible_(varies))) LPVOID  FAR* ppv);
		
		static HRESULT RemoveAllEffectsHook(IMFCaptureSource* thisPtr, DWORD dwSourceStreamIndex, DWORD dwMediaTypeIndex, IMFMediaType** ppMediaType);
		static HRESULT PreviewAddStreamHook(IMFCaptureSink* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType, IMFAttributes* pAttributes, DWORD* pdwSinkStreamIndex);
		static HRESULT PhotoAddStreamHook(IMFCaptureSink* thisPtr, DWORD dwSourceStreamIndex, IMFMediaType* pMediaType, IMFAttributes* pAttributes, DWORD* pdwSinkStreamIndex);
		
		static decltype(&RemoveAllEffectsHook) RemoveAllEffectsOriginal;
		static decltype(&PreviewAddStreamHook) PreviewAddStreamOriginal;
		static decltype(&PhotoAddStreamHook) PhotoAddStreamOriginal;

		static LSTATUS RegQueryValueExHook
		(
			_In_ HKEY hKey,
			_In_opt_ LPCWSTR lpValueName,
			_Reserved_ LPDWORD lpReserved,
			_Out_opt_ LPDWORD lpType,
			_Out_writes_bytes_to_opt_(*lpcbData, *lpcbData) __out_data_source(REGISTRY) LPBYTE lpData,
			_When_(lpData == NULL, _Out_opt_) _When_(lpData != NULL, _Inout_opt_) LPDWORD lpcbData
		);
		static bool IsHooked;

		static HRESULT InstallInternal(IMFCaptureEngine* engine);

	public:
		static void Install();
	};
}