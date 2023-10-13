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


class PixelView: public wxPanel
{
    static constexpr int margin = 5;
    static constexpr int gridSpacing = 3;

public:
    static constexpr auto observerName = "PixelView";

    PixelView(
        wxWindow *parent,
        PixelViewControl control);

    wxSize DoGetBestSize() const override;

private:
    Size GetWindowSize_(const Size &canvasSize) const;

private:
    wxWindow *horizontalZoom_;
    wxWindow *verticalZoom_;
    wxBoxSizer *controlsSizer_;
    PixelCanvas *canvas_;
};


class PixelFrame: public wxFrame
{
public:
    PixelFrame(PixelViewControl control, const std::string &title);
};


} // end namespace draw
