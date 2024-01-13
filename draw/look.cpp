#include "draw/look.h"


namespace draw
{


void ConfigureColors(wxpex::GraphicsContext &context, const Look &look)
{
    if (look.strokeEnable)
    {
        context->SetPen(
            context->CreatePen(
                wxPen(
                    wxpex::ToWxColour(look.strokeColor),
                    look.strokeWeight)));
    }
    else
    {
        context->SetPen(context->CreatePen(*wxTRANSPARENT_PEN));
    }

    if (look.fillEnable)
    {
        context->SetBrush(
            context->CreateBrush(
                wxBrush(
                    wxpex::ToWxColour(look.fillColor))));
    }
    else
    {
        context->SetBrush(context->CreateBrush(*wxTRANSPARENT_BRUSH));
    }
}


void ConfigureColors(
    wxpex::GraphicsContext &context,
    const Look &look,
    double value)
{
    if (look.strokeEnable)
    {
        auto strokeColor = look.strokeColor;
        strokeColor.value = value;

        context->SetPen(
            context->CreatePen(
                wxPen(
                    wxpex::ToWxColour(strokeColor),
                    look.strokeWeight)));
    }
    else
    {
        context->SetPen(context->CreatePen(*wxTRANSPARENT_PEN));
    }

    if (look.fillEnable)
    {
        auto fillColor = look.fillColor;
        fillColor.value = value;

        context->SetBrush(
            context->CreateBrush(
                wxBrush(
                    wxpex::ToWxColour(fillColor))));
    }
    else
    {
        context->SetBrush(context->CreateBrush(*wxTRANSPARENT_BRUSH));
    }
}


void ConfigureLook(wxpex::GraphicsContext &context, const Look &look)
{
    if (look.antialias)
    {
        context->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    }
    else
    {
        context->SetAntialiasMode(wxANTIALIAS_NONE);
    }

    ConfigureColors(context, look);
}


} // end namespace draw



template struct pex::Group
    <
        draw::LookFields,
        draw::LookTemplate,
        pex::PlainU<draw::Look_>
    >;
