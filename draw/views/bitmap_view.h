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
        const CanvasControl &control,
        const CanvasViewOptions &options = CanvasViewOptions{});

    wxBitmap & GetBitmap();
    const wxBitmap & GetBitmap() const;
    void SetBitmap(const wxBitmap &bitmap);
};


using BitmapFrame = CanvasFrame<BitmapView, CanvasControl>;


} // end namespace draw
