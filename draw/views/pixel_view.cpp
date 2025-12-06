#include "draw/views/pixel_view.h"


namespace draw
{


PixelView::PixelView(
    wxWindow *parent,
    const PixelViewControl &control,
    const CanvasViewOptions &options)
    :
    CanvasView<PixelCanvas>(parent, control.canvas, control, options)
{

}


} // end namespace draw
