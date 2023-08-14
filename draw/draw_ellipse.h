#pragma once


#include <wxpex/color.h>
#include <wxpex/graphics.h>


namespace draw
{


template<typename Device>
void DrawEllipse(
    Device &&device,
    const Ellipse<double> &ellipse,
    const Look &look)
{
    auto gc = wxpex::GraphicsContext(std::forward<Device>(device));

    if (look.antialias)
    {
        gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    }
    else
    {
        gc->SetAntialiasMode(wxANTIALIAS_NONE);
    }

    if (look.strokeEnable)
    {
        gc->SetPen(
            gc->CreatePen(
                wxPen(
                    wxpex::ToWxColour(look.strokeColor),
                    look.strokeWeight)));
    }
    else
    {
        gc->SetPen(gc->CreatePen(*wxTRANSPARENT_PEN));
    }

    if (look.fillEnable)
    {
        gc->SetBrush(
            gc->CreateBrush(wxBrush(wxpex::ToWxColour(look.fillColor))));
    }
    else
    {
        gc->SetBrush(gc->CreateBrush(*wxTRANSPARENT_BRUSH));
    }

    gc->Translate(ellipse.center.x, ellipse.center.y);
    gc->Rotate(tau::ToRadians(ellipse.rotation));

    double ellipseMajor = ellipse.scale * ellipse.major;
    double ellipseMinor = ellipse.scale * ellipse.minor;

    gc->DrawEllipse(
        -ellipseMajor / 2.0,
        -ellipseMinor / 2.0,
        ellipseMajor,
        ellipseMinor);
}


} // end namespace draw
