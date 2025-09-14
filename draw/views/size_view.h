#pragma once


#include <wxpex/field.h>
#include <tau/vector2d.h>



namespace draw
{


template<typename Value, int precision = 1>
class SizeView: public wxControl
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    using Control = typename tau::SizeGroup<Value>::DefaultControl;
    using ValueControl = decltype(Control::width);
    using Converter = wxpex::PrecisionConverter<ValueControl, precision>;
    using ValueField = wxpex::Field<ValueControl, Converter>;

    SizeView(wxWindow *parent, Control control)
        :
        wxControl(parent, wxID_ANY)
    {
        using namespace wxpex;

        auto widthLabel = new wxStaticText(this, wxID_ANY, "width:");
        auto heightLabel = new wxStaticText(this, wxID_ANY, "height:");
        auto width = new ValueField(this, control.width);
        auto height = new ValueField(this, control.height);

        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
        sizer->Add(widthLabel, 0, wxRIGHT, 3);
        sizer->Add(width, 1, wxRIGHT, 6);
        sizer->Add(heightLabel, 0, wxRIGHT, 3);
        sizer->Add(height, 1);

        this->SetSizerAndFit(sizer.release());
    }
};


} // end namespace draw
