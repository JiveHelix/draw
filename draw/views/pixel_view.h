#pragma once


#include <mutex>
#include <pex/value.h>

#include <wxpex/ignores.h>

WXSHIM_PUSH_IGNORES
#include <wx/scrolwin.h>
WXSHIM_POP_IGNORES

#include "draw/views/pixel_canvas.h"
#include "draw/views/pixel_view_settings.h"


namespace draw
{


class PixelView: public wxFrame
{
    static constexpr int margin = 5;
    static constexpr int gridSpacing = 3;

public:
    static constexpr auto observerName = "PixelView";

    PixelView(
        wxWindow *parent,
        PixelViewControl controls,
        const std::string &title);

    wxSize DoGetBestSize() const override;

private:
    void OnImageSize_(const Size &imageSize);

    Size GetWindowSize_(const Size &canvasSize) const;

private:
    using SizeEndpoint = pex::Endpoint<PixelView, SizeControl>;
    SizeEndpoint imageSizeEndpoint_;
    wxWindow *horizontalZoom_;
    wxWindow *verticalZoom_;
    wxBoxSizer *controlsSizer_;
    PixelCanvas *canvas_;
};


} // end namespace draw
