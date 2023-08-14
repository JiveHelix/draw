#include "waveform_settings_view.h"
#include "draw/views/defaults.h"

#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/check_box.h>
#include <wxpex/color_picker.h>


namespace draw
{


WaveformColorView::WaveformColorView(
    wxWindow *parent,
    WaveformColorControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Waveform Color")
{
    auto pane = this->GetBorderPane(borderStyle);
    
    auto color = new wxpex::HsvPicker(
        pane,
        "Color",
        control.color);

    auto highlight = new wxpex::HsvPicker(
        pane,
        "Highlight",
        control.highlightColor);

    auto low = wxpex::LabeledWidget(
        pane,
        "Low",
        new wxpex::ValueSlider(
            pane,
            control.range.low,
            control.range.low.value));

    auto high = wxpex::LabeledWidget(
        pane,
        "High",
        new wxpex::ValueSlider(
            pane,
            control.range.high,
            control.range.high.value));

    auto count = wxpex::LabeledWidget(
        pane,
        "Count",
        new wxpex::ValueSlider(
            pane,
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
        sliderSizer.release());

    this->ConfigureBorderPane(5, std::move(sizer));
}


WaveformSettingsView::WaveformSettingsView(
    wxWindow *parent,
    WaveformControl control,
    const LayoutOptions &layoutOptions)
    :
    wxpex::Collapsible(parent, "Waveform")
{
    auto pane = this->GetBorderPane(borderStyle);

    auto enable =
        new wxpex::CheckBox(
            pane,
            "Enable",
            control.enable);

    auto levelCount = wxpex::LabeledWidget(
        pane,
        "Level count",
        new wxpex::ValueSlider(
            pane,
            control.levelCount,
            control.levelCount.value));

    auto columnCount = wxpex::LabeledWidget(
        pane,
        "Column count",
        new wxpex::ValueSlider(
            pane,
            control.columnCount,
            control.columnCount.value));

    auto verticalScale = wxpex::LabeledWidget(
        pane,
        "Vertical Scale",
        new wxpex::ValueSlider(
            pane,
            control.verticalScale,
            control.verticalScale.value));

    auto color = new WaveformColorView(
        pane,
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
        sliderSizer.release(),
        color);

    this->ConfigureBorderPane(5, std::move(sizer));
}


} // end namespace draw
