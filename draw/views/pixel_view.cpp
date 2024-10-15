#include "draw/views/pixel_view.h"


namespace draw
{


PixelView::PixelView(
    wxWindow *parent,
    PixelViewControl control,
    CanvasViewOptions options)
    :
    CanvasView<PixelCanvas>(parent, control.canvas, control, options)
{

}


} // end namespace draw
