#include "bitmap_view.h"


namespace draw
{


BitmapView::BitmapView(
    wxWindow *parent,
    CanvasControl control,
    CanvasViewOptions options)
    :
    CanvasView<BitmapCanvas>(parent, control, control, options)
{

}


wxBitmap * BitmapView::GetBitmap()
{
    return this->GetCanvas()->GetBitmap();
}


} // end namespace draw
