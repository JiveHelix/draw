#include "draw/views/pixel_canvas.h"

#include <wxpex/ignores.h>

#ifdef __WXMSW__

WXSHIM_PUSH_IGNORES
#include <wx/dcbuffer.h>
WXSHIM_POP_IGNORES

#endif


namespace draw
{


PixelCanvas::PixelCanvas(
    wxWindow *parent,
    const PixelViewControl &control)
    :
    Canvas(parent, control.canvas),

    imageSizeEndpoint_(
        this,
        control.canvas.viewSettings.imageSize,
        &PixelCanvas::OnImageSize_),

    image_(
        this->imageSizeEndpoint_.Get().width,
        this->imageSizeEndpoint_.Get().height,
        true),

    pixelsEndpoint_(this, control.pixels, &PixelCanvas::OnPixels_),
    pixelData_(),
    shapesEndpoint_(this, control.shapes, &PixelCanvas::OnShapes_),
    shapesById_()
{
    this->Bind(wxEVT_PAINT, &PixelCanvas::OnPaint_, this);
}


void PixelCanvas::OnImageSize_(const Size &imageSize)
{
    if (!this->image_.IsOk())
    {
        this->image_ = wxImage(imageSize.width, imageSize.height, true);

        return;
    }

    auto currentImageSize =
        wxpex::ToSize<draw::SizeType>(this->image_.GetSize());

    if (currentImageSize == imageSize)
    {
        // TODO: Fix pex sending repeated notifications for values that
        // haven't changed.
        // I suspect it is in the destructor of pex::DeferGroup.
        return;
    }

    this->image_ = wxImage(imageSize.width, imageSize.height, true);
}


void PixelCanvas::OnPixels_(const std::shared_ptr<Pixels> &pixels)
{
    if (!pixels)
    {
        this->pixelData_ = pixels;

        return;
    }

    auto dataSize = pixels->size;

    if (dataSize.width == 0 || dataSize.height == 0)
    {
        throw std::logic_error("pixels must not be empty");
    }

    this->pixelData_ = pixels;

    auto imageSize = wxpex::ToSize<Pixels::Index>(this->image_.GetSize());

    if (imageSize != dataSize)
    {
        this->image_ = wxImage(dataSize.width, dataSize.height, false);
    }

    this->image_.SetData(this->pixelData_->data.data(), true);

    this->Refresh(false);
    this->Update();
}


void PixelCanvas::OnShapes_(const Shapes &shapes)
{
    if (shapes.IsResetter())
    {
        this->shapesById_.clear();
        return;
    }

    if (shapes.GetId() == -1)
    {
        std::cerr << "Warning: shapes has uninitialized id." << std::endl;
        return;
    }

    this->shapesById_[shapes.GetId()] = shapes;
    this->Refresh(false);
    this->Update();
}


bool PixelCanvas::HasShapes_() const
{
    if (this->shapesById_.empty())
    {
        return false;
    }

    for (auto &it: this->shapesById_)
    {
        if (!it.second.GetShapes().empty())
        {
            return true;
        }
    }

    return false;
}


void PixelCanvas::OnPaint_(wxPaintEvent &)
{
#ifdef __WXMSW__
    wxAutoBufferedPaintDC dc(this);
#else
    wxPaintDC dc(this);
#endif

    this->Draw_(dc);
}


} // end namespace draw
