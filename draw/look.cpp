#include "draw/look.h"


namespace draw
{


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


} // end namespace draw



template struct pex::Group
    <
        draw::LookFields,
        draw::LookTemplate,
        draw::Look
    >;
