#pragma once


#include <wxpex/field.h>
#include <tau/vector2d.h>



namespace draw
{


template<typename Value>
class PointView: public wxControl
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    using Control = typename tau::Point2dGroup<Value>::DefaultControl;
    using ValueControl = decltype(Control::x);
    using Converter = wxpex::PrecisionConverter<ValueControl, 1>;
    using ValueField = wxpex::Field<ValueControl, Converter>;

    PointView(wxWindow *parent, Control control)
        :
        wxControl(parent, wxID_ANY)
    {
        using namespace wxpex;

        auto xLabel = new wxStaticText(this, wxID_ANY, "x:");
        auto yLabel = new wxStaticText(this, wxID_ANY, "y:");
        auto x = new ValueField(this, control.x);
        auto y = new ValueField(this, control.y);

        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
        sizer->Add(xLabel, 0, wxRIGHT, 3);
        sizer->Add(x, 1, wxRIGHT, 6);
        sizer->Add(yLabel, 0, wxRIGHT, 3);
        sizer->Add(y, 1);

        this->SetSizerAndFit(sizer.release());
    }
};


} // end namespace draw
