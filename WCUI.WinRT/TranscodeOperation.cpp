#include "pch.h"
#include "TranscodeOperation.h"

namespace winrt::WCUI::WinRT::implementation
{
    winrt::WCUI::WinRT::TranscodeOperation TranscodeOperation::CreateTranscode(hstring const& source, hstring const& destination, winrt::Windows::Foundation::IInspectable const& profile)
    {
        throw hresult_not_implemented();
    }
    winrt::WCUI::WinRT::TranscodeOperation TranscodeOperation::CreateTranscode(winrt::Windows::Foundation::IInspectable const& source, winrt::Windows::Foundation::IInspectable const& destination, winrt::Windows::Foundation::IInspectable const& profile)
    {
        throw hresult_not_implemented();
    }
    winrt::WCUI::WinRT::TranscodeOperation TranscodeOperation::CreateTranscode(winrt::Windows::Storage::Streams::IRandomAccessStream const& source, winrt::Windows::Storage::Streams::IRandomAccessStream const& destination, winrt::Windows::Foundation::IInspectable const& profile)
    {
        throw hresult_not_implemented();
    }
}
