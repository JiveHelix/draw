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


wxBitmap & BitmapView::GetBitmap()
{
    return this->GetCanvas()->GetBitmap();
}


const wxBitmap & BitmapView::GetBitmap() const
{
    return this->GetCanvas()->GetBitmap();
}


void BitmapView::SetBitmap(const wxBitmap &bitmap)
{
    return this->GetCanvas()->SetBitmap(bitmap);
}


} // end namespace draw
