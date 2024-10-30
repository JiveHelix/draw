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

    bitmap_(16, 16)
{
    this->OnImageSize_(control.viewSettings.imageSize.Get());

    this->Bind(wxEVT_PAINT, &BitmapCanvas::OnPaint_, this);
}


wxBitmap & BitmapCanvas::GetBitmap()
{
    return this->bitmap_;
}


const wxBitmap & BitmapCanvas::GetBitmap() const
{
    return this->bitmap_;
}


void BitmapCanvas::SetBitmap(const wxBitmap &bitmap)
{
    this->bitmap_ = bitmap;
}


void BitmapCanvas::OnImageSize_(const Size &imageSize)
{
    if (imageSize.width == 0 || imageSize.height == 0)
    {
        return;
    }

    if (wxpex::ToSize<int>(this->bitmap_.GetSize()) != imageSize)
    {
        this->bitmap_ = wxBitmap(imageSize.width, imageSize.height);
    }
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
