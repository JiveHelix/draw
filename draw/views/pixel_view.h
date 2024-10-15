#pragma once


#include "draw/views/pixel_view_settings.h"
#include "draw/views/canvas_view.h"
#include "draw/views/pixel_canvas.h"


namespace draw
{


class PixelView: public CanvasView<PixelCanvas>
{
public:
    static constexpr auto observerName = "PixelView";

    PixelView(
        wxWindow *parent,
        PixelViewControl control,
        CanvasViewOptions options = CanvasViewOptions{});
};


using PixelFrame = CanvasFrame<PixelView, PixelViewControl>;


} // end namespace draw
