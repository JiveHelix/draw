#pragma once


#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>

#include "draw/waveform_settings.h"


namespace draw
{


class WaveformColorView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    WaveformColorView(
        wxWindow *parent,
        WaveformColorControl control,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


class WaveformSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    WaveformSettingsView(
        wxWindow *parent,
        WaveformControl control,
        const LayoutOptions &layoutOptions = LayoutOptions{});
};


} // end namespace draw
