#include "draw/font_look.h"


namespace draw
{


void ConfigureFontLook(
    wxpex::GraphicsContext &context,
    const FontLook &fontLook)
{
    wxColour color;

    if (!fontLook.enable)
    {
        color = wxTransparentColour;
        return;
    }
    else
    {
        color = wxpex::ToWxColour(fontLook.color);
    }

    context->SetFont(
        wxFont(
            wxFontInfo(fontLook.pointSize)
            .Family(wxFONTFAMILY_MODERN)
            .AntiAliased(fontLook.antialias)),
        color);
}


} // end namespace draw



template struct pex::Group
    <
        draw::FontLookFields,
        draw::FontLookTemplate,
        pex::PlainT<draw::FontLook>
    >;
