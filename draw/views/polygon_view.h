#pragma once


#include <pex/endpoint.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/field.h>
#include <wxpex/knob.h>
#include <wxpex/converter.h>
#include <wxpex/static_box.h>

#include <wxpex/ignores.h>

WXSHIM_PUSH_IGNORES
#include <wx/listctrl.h>
WXSHIM_POP_IGNORES

#include "draw/views/point_view.h"
#include "draw/polygon.h"


namespace draw
{


class PointsView: public wxListView
{
public:
    using Control = typename pex::ControlSelector<PolygonPoints>;

    using Converter = pex::Converter<double, wxpex::ViewTraits<10, 0, 1>>;

    PointsView(wxWindow *parent, Control control);

    void OnPoints_(const PolygonPoints &points);

private:
    pex::Endpoint<PointsView, Control> endpoint_;
};


class PolygonView: public wxpex::StaticBox
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    PolygonView(
        wxWindow *parent,
        const std::string &name,
        PolygonControl control,
        const LayoutOptions &layoutOptions);
};


} // end namespace draw
