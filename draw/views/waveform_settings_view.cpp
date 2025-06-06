#include "waveform_settings_view.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/check_box.h>
#include <wxpex/color_picker.h>
#include <wxpex/layout_items.h>


#ifdef __WXMSW__
static const long borderStyle = wxBORDER_NONE;
#else
static const long borderStyle = wxBORDER_SIMPLE;
#endif


namespace draw
{


WaveformColorView::WaveformColorView(
    wxWindow *parent,
    WaveformColorControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Waveform Color", borderStyle)
{
    auto panel = this->GetPanel();

    auto color = new wxpex::HsvPicker(
        panel,
        "Color",
        control.color);

    auto highlight = new wxpex::HsvPicker(
        panel,
        "Highlight",
        control.highlightColor);

    auto low = wxpex::LabeledWidget(
        panel,
        "Low",
        new wxpex::ValueSlider(
            panel,
            control.range.low,
            control.range.low.value));

    auto high = wxpex::LabeledWidget(
        panel,
        "High",
        new wxpex::ValueSlider(
            panel,
            control.range.high,
            control.range.high.value));

    auto count = wxpex::LabeledWidget(
        panel,
        "Count",
        new wxpex::ValueSlider(
            panel,
            control.count,
            control.count.value));

    auto sliderSizer = wxpex::LayoutLabeled(
        layoutOptions,
        low,
        high,
        count);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        color,
        highlight,
        sliderSizer);

    this->ConfigureSizer(std::move(sizer));
}


WaveformSettingsView::WaveformSettingsView(
    wxWindow *parent,
    WaveformControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Waveform", wxBORDER_SIMPLE)
{
    auto panel = this->GetPanel();

    auto enable =
        new wxpex::CheckBox(
            panel,
            "Enable",
            control.enable);

    auto levelCount = wxpex::LabeledWidget(
        panel,
        "Level count",
        new wxpex::ValueSlider(
            panel,
            control.levelCount,
            control.levelCount.value));

    auto columnCount = wxpex::LabeledWidget(
        panel,
        "Column count",
        new wxpex::ValueSlider(
            panel,
            control.columnCount,
            control.columnCount.value));

    auto verticalScale = wxpex::LabeledWidget(
        panel,
        "Vertical Scale",
        new wxpex::ValueSlider(
            panel,
            control.verticalScale,
            control.verticalScale.value));

    auto color = new WaveformColorView(
        panel,
        control.color,
        layoutOptions);

    auto sliderSizer = LayoutLabeled(
        layoutOptions,
        levelCount,
        columnCount,
        verticalScale);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        enable,
        sliderSizer,
        color);

    this->ConfigureSizer(std::move(sizer));
}


} // end namespace draw
