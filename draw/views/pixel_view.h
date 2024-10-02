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


struct PixelViewOptions
{
    bool useDualZoom;
    bool displayControls;

    PixelViewOptions()
        :
        useDualZoom(false),
        displayControls(true)
    {

    }

    PixelViewOptions & SetUseDualZoom(bool value)
    {
        this->useDualZoom = value;

        return *this;
    }

    PixelViewOptions & SetDisplayControls(bool value)
    {
        this->displayControls = value;

        return *this;
    }
};


class PixelView: public wxPanel
{
    static constexpr int margin = 3;
    static constexpr int gridSpacing = 3;

public:
    static constexpr auto observerName = "PixelView";

    PixelView(
        wxWindow *parent,
        PixelViewControl control,
        PixelViewOptions options = PixelViewOptions{});

    wxSize DoGetBestSize() const override;

private:
    Size GetWindowSize_(const Size &canvasSize) const;

private:
    PixelViewOptions options_;
    wxWindow *horizontalZoom_;
    wxWindow *verticalZoom_;
    wxBoxSizer *controlsSizer_;
    PixelCanvas *canvas_;
};


class PixelFrame: public wxFrame
{
public:
    PixelFrame(
        PixelViewControl control,
        const std::string &title,
        PixelViewOptions options = PixelViewOptions{});
};


} // end namespace draw
