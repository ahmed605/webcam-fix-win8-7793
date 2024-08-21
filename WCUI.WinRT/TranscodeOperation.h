#pragma once
#include "WCUI.WinRT.TranscodeOperation.g.h"

namespace winrt::WCUI::WinRT::implementation
{
    struct TranscodeOperation : TranscodeOperationT<TranscodeOperation>
    {
        TranscodeOperation() = default;

        static winrt::WCUI::WinRT::TranscodeOperation CreateTranscode(hstring const& source, hstring const& destination, winrt::Windows::Foundation::IInspectable const& profile);
        static winrt::WCUI::WinRT::TranscodeOperation CreateTranscode(winrt::Windows::Foundation::IInspectable const& source, winrt::Windows::Foundation::IInspectable const& destination, winrt::Windows::Foundation::IInspectable const& profile);
        static winrt::WCUI::WinRT::TranscodeOperation CreateTranscode(winrt::Windows::Storage::Streams::IRandomAccessStream const& source, winrt::Windows::Storage::Streams::IRandomAccessStream const& destination, winrt::Windows::Foundation::IInspectable const& profile);
    };
}
namespace winrt::WCUI::WinRT::factory_implementation
{
    struct TranscodeOperation : TranscodeOperationT<TranscodeOperation, implementation::TranscodeOperation>
    {
    };
}
