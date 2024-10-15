#pragma once


#include "draw/views/pixel_view_settings.h"
#include "draw/views/canvas_view.h"
#include "draw/views/bitmap_canvas.h"


namespace draw
{


class BitmapView: public CanvasView<BitmapCanvas>
{
public:
    static constexpr auto observerName = "BitmapView";

    BitmapView(
        wxWindow *parent,
        CanvasControl control,
        CanvasViewOptions options = CanvasViewOptions{});

    wxBitmap * GetBitmap();
};


using BitmapFrame = CanvasFrame<BitmapView, CanvasControl>;


} // end namespace draw
