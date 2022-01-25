// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "Profiles_Appearance.h"
#include "Profiles_Appearance.g.cpp"
#include "Profiles.h"
#include "PreviewConnection.h"
#include "EnumEntry.h"

#include <LibraryResources.h>
#include "..\WinRTUtils\inc\Utils.h"

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Navigation;

namespace winrt::Microsoft::Terminal::Settings::Editor::implementation
{
    Profiles_Appearance::Profiles_Appearance() :
        _previewControl{ Control::TermControl(Model::TerminalSettings{}, make<PreviewConnection>()) }
    {
        InitializeComponent();
        INITIALIZE_BINDABLE_ENUM_SETTING(ScrollState, ScrollbarState, winrt::Microsoft::Terminal::Control::ScrollbarState, L"Profile_ScrollbarVisibility", L"Content");

        _previewControl.IsEnabled(false);
        _previewControl.AllowFocusWhenDisabled(false);
        ControlPreview().Child(_previewControl);
    }

    void Profiles_Appearance::OnNavigatedTo(const NavigationEventArgs& e)
    {
        _State = e.Parameter().as<Editor::ProfilePageNavigationState>();

        // generate the font list, if we don't have one
        if (!_State.Profile().CompleteFontList() || !_State.Profile().MonospaceFontList())
        {
            ProfileViewModel::UpdateFontList();
        }

        // Subscribe to some changes in the view model
        // These changes should force us to update our own set of "Current<Setting>" members,
        // and propagate those changes to the UI
        _ViewModelChangedRevoker = _State.Profile().PropertyChanged(winrt::auto_revoke, [=](auto&&, const PropertyChangedEventArgs& args) {
            const auto settingName{ args.PropertyName() };
            if (settingName == L"ScrollState")
            {
                _PropertyChangedHandlers(*this, PropertyChangedEventArgs{ L"CurrentScrollState" });
            }
            _previewControl.Settings(_State.Profile().TermSettings());
            _previewControl.UpdateSettings();
        });

        // The Appearances object handles updating the values in the settings UI, but
        // we still need to listen to the changes here just to update the preview control
        _AppearanceViewModelChangedRevoker = _State.Profile().DefaultAppearance().PropertyChanged(winrt::auto_revoke, [=](auto&&, const PropertyChangedEventArgs& /*args*/) {
            _previewControl.Settings(_State.Profile().TermSettings());
            _previewControl.UpdateSettings();
        });

        _previewControl.Settings(_State.Profile().TermSettings());
        // There is a possibility that the control has not fully initialized yet,
        // so wait for it to initialize before updating the settings (so we know
        // that the renderer is set up)
        _previewControl.Initialized([&](auto&& /*s*/, auto&& /*e*/) {
            _previewControl.UpdateSettings();
        });
    }

    void Profiles_Appearance::OnNavigatedFrom(const NavigationEventArgs& /*e*/)
    {
        _ViewModelChangedRevoker.revoke();
        _AppearanceViewModelChangedRevoker.revoke();
    }

    void Profiles_Appearance::CreateUnfocusedAppearance_Click(IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        _State.CreateUnfocusedAppearance();
    }

    void Profiles_Appearance::DeleteUnfocusedAppearance_Click(IInspectable const& /*sender*/, RoutedEventArgs const& /*e*/)
    {
        _State.DeleteUnfocusedAppearance();
    }
}
