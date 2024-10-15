#include "bitmap_canvas.h"

#include <wxpex/ignores.h>

#ifdef __WXMSW__

WXSHIM_PUSH_IGNORES
#include <wx/dcbuffer.h>
WXSHIM_POP_IGNORES

#endif


namespace draw
{


BitmapCanvas::BitmapCanvas(
    wxWindow *parent,
    CanvasControl control)
    :
    Canvas(parent, control),

    imageSizeEndpoint_(
        this,
        control.viewSettings.imageSize,
        &BitmapCanvas::OnImageSize_),

    bitmap_()
{
    this->OnImageSize_(control.viewSettings.imageSize.Get());

    this->Bind(wxEVT_PAINT, &BitmapCanvas::OnPaint_, this);
}


wxBitmap * BitmapCanvas::GetBitmap()
{
    return &this->bitmap_;
}


void BitmapCanvas::OnImageSize_(const Size &imageSize)
{
    this->bitmap_ = wxBitmap(imageSize.width, imageSize.height);
}


void BitmapCanvas::OnPaint_(wxPaintEvent &)
{
#ifdef __WXMSW__
    wxAutoBufferedPaintDC dc(this);
#else
    wxPaintDC dc(this);
#endif

    this->Draw_(dc);
}


} // end namespace draw
