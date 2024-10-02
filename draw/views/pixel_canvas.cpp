#include "draw/views/pixel_canvas.h"

#include <wxpex/ignores.h>

WXSHIM_PUSH_IGNORES
#include <wx/dcbuffer.h>
WXSHIM_POP_IGNORES

#include <wxpex/button.h>
#include <wxpex/check_box.h>
#include <wxpex/layout_top_level.h>

namespace draw
{


PixelCanvas::PixelCanvas(
    wxWindow *parent,
    PixelViewControl controls)
    :
    Scrolled(parent, wxID_ANY),
    ignoreViewPosition_(false),
    skipUpdateViewPosition_(),

    imageSizeEndpoint_(
        this,
        controls.viewSettings.imageSize,
        &PixelCanvas::OnImageSize_),

    virtualSizeEndpoint_(
        this,
        controls.viewSettings.virtualSize,
        &PixelCanvas::OnVirtualSize_),

    viewPositionEndpoint_(
        this,
        controls.viewSettings.viewPosition,
        &PixelCanvas::OnViewPosition_),

    scaleEndpoint_(
        this,
        controls.viewSettings.scale,
        &PixelCanvas::OnScale_),

    cursorEndpoint_(this, controls.cursor),
    imageCenter_(controls.viewSettings.imageCenterPixel),
    control_(controls),

    image_(
        this->imageSizeEndpoint_.Get().width,
        this->imageSizeEndpoint_.Get().height),

    bitmap_(),
    pixelsEndpoint_(this, controls.pixels, &PixelCanvas::OnPixels_),
    pixelData_(),
    shapesEndpoint_(this, controls.shapes, &PixelCanvas::OnShapes_),
    shapesById_()
{

#ifdef __WXMSW__
    this->SetBackgroundStyle(wxBG_STYLE_PAINT);
#else
    this->SetBackgroundStyle(wxBG_STYLE_SYSTEM);
#endif

    this->Bind(wxEVT_SIZE, &PixelCanvas::OnSize_, this);

    this->Bind(
        wxEVT_MOTION,
        &PixelCanvas::OnMouseMotion_,
        this);

    this->Bind(
        wxEVT_LEFT_DOWN,
        &PixelCanvas::OnLeftDown_,
        this);

    this->Bind(
        wxEVT_LEFT_DCLICK,
        &PixelCanvas::OnLeftDown_,
        this);

    this->Bind(
        wxEVT_LEFT_UP,
        &PixelCanvas::OnLeftUp_,
        this);

    this->Bind(
        wxEVT_RIGHT_DOWN,
        &PixelCanvas::OnRightDown_,
        this);

    this->Bind(
        wxEVT_RIGHT_UP,
        &PixelCanvas::OnRightUp_,
        this);

    this->Bind(
        wxEVT_KEY_DOWN,
        &PixelCanvas::OnKeyDown_,
        this);

    this->Bind(
        wxEVT_KEY_UP,
        &PixelCanvas::OnKeyUp_,
        this);

    this->cursorEndpoint_.Connect(&PixelCanvas::OnCursor_);

    // Configure the scrollable area
    this->OnVirtualSize_(this->virtualSizeEndpoint_.Get());

    this->Bind(wxEVT_PAINT, &PixelCanvas::OnPaint_, this);
}



void PixelCanvas::ScrollWindow(int dx, int dy, const wxRect *rect)
{
    auto position = this->viewPositionEndpoint_.Get();

    if (!this->skipUpdateViewPosition_)
    {
        // This is not an internal repositioning.
        // Apply constraints.

        auto maximumPosition = GetMaximumViewPosition(
            this->control_.viewSettings.viewSize.Get(),
            this->control_.viewSettings.virtualSize.Get());

        if (position.x - dx < 0)
        {
            dx = position.x;
        }
        else if (position.x - dx > maximumPosition.x)
        {
            dx = position.x - maximumPosition.x;
        }

        if (position.y - dy < 0)
        {
            dy = position.y;
        }
        else if (position.y - dy > maximumPosition.y)
        {
            dy = position.y - maximumPosition.y;
        }
    }

    this->Scrolled::ScrollWindow(dx, dy, rect);

    if (this->skipUpdateViewPosition_)
    {
        return;
    }

    jive::ScopeFlag ignoreViewPosition(this->ignoreViewPosition_);
    auto delta = tau::Point2d<int>(-dx, -dy);
    this->viewPositionEndpoint_.Set(position + delta);
}

Size PixelCanvas::GetVirtualSize() const
{
    return this->virtualSize_;
}

void PixelCanvas::OnImageSize_(const Size &imageSize)
{
    this->image_ = wxImage(imageSize.width, imageSize.height, true);
}


void PixelCanvas::OnSize_(wxSizeEvent &event)
{
    event.Skip();

    auto clientSize = wxpex::ToSize<int>(this->GetClientSize());
    auto windowSize = wxpex::ToSize<int>(this->GetSize());
    auto position = wxpex::ToPoint<int>(this->GetScreenPosition());

    this->control_.viewSettings.viewSize.Set(clientSize);
    this->control_.viewSettings.windowSize.Set(windowSize);
    this->control_.viewSettings.screenPosition.Set(position);
}

void PixelCanvas::OnMouseMotion_(wxMouseEvent &event)
{
    event.Skip();

    this->control_.mousePosition.Set(
        wxpex::ToPoint<int>(event.GetPosition()));
}

void PixelCanvas::OnLeftDown_(wxMouseEvent &event)
{
    event.Skip();
    auto mousePosition = pex::MakeDefer(this->control_.mousePosition);
    auto mouseDown = pex::MakeDefer(this->control_.mouseDown);
    mousePosition.Set(wxpex::ToPoint<int>(event.GetPosition()));
    mouseDown.Set(true);
}

void PixelCanvas::OnLeftUp_(wxMouseEvent &event)
{
    event.Skip();
    this->control_.mouseDown.Set(false);
}

void PixelCanvas::OnRightDown_(wxMouseEvent &event)
{
    event.Skip();
    auto mousePosition = pex::MakeDefer(this->control_.mousePosition);
    auto rightMouseDown = pex::MakeDefer(this->control_.rightMouseDown);

    mousePosition.Set(
        wxpex::ToPoint<int>(event.GetPosition()));

    rightMouseDown.Set(true);
}

void PixelCanvas::OnRightUp_(wxMouseEvent &event)
{
    event.Skip();
    this->control_.rightMouseDown.Set(false);
}

void PixelCanvas::OnKeyDown_(wxKeyEvent &event)
{
    event.Skip();
    auto keyCode = event.GetKeyCode();
    auto modifier = this->control_.modifier.Get();

    switch (keyCode)
    {
        case WXK_ALT:
            modifier.Add(wxMOD_ALT);
            break;

        case WXK_CONTROL:
            modifier.Add(wxMOD_CONTROL);
            break;

        case WXK_SHIFT:
            modifier.Add(wxMOD_SHIFT);
            break;

#ifdef __APPLE__
        case WXK_RAW_CONTROL:
            modifier.Add(wxMOD_RAW_CONTROL);
            break;
#endif

        default:
            break;
    };

    this->control_.modifier.Set(modifier);
}

void PixelCanvas::OnKeyUp_(wxKeyEvent &event)
{
    event.Skip();
    auto keyCode = event.GetKeyCode();
    auto modifier = this->control_.modifier.Get();

    switch (keyCode)
    {
        case WXK_ALT:
            modifier.Remove(wxMOD_ALT);
            break;

        case WXK_CONTROL:
            modifier.Remove(wxMOD_CONTROL);
            break;

        case WXK_SHIFT:
            modifier.Remove(wxMOD_SHIFT);
            break;

#ifdef __APPLE__
        case WXK_RAW_CONTROL:
            modifier.Remove(wxMOD_RAW_CONTROL);
            break;
#endif

        default:
            break;
    };

    this->control_.modifier.Set(modifier);
}

void PixelCanvas::OnScale_(const Scale &scale)
{
    this->Refresh(false);
    this->Update();
}

void PixelCanvas::OnCursor_([[maybe_unused]] wxpex::Cursor cursor)
{
    // I haven't configured the resoure file necessary for loading cursors in
    // windows.
#ifndef __WXMSW__
    this->SetCursor(wxCursor(static_cast<wxStockCursor>(cursor)));
#endif
}

void PixelCanvas::OnViewPosition_(const Point &viewPosition)
{
    // The view position has been set from the model.
    if (this->ignoreViewPosition_)
    {
        // Ignoring because this viewPosition originated with this class
        // and is now echoed from the model.
        return;
    }

    // Scroll to the correct position, but do not allow ScrollWindow to
    // publish a redundant viewPosition.
    jive::ScopedCountFlag skipUpdate(this->skipUpdateViewPosition_);
    auto newViewStart = viewPosition / pixelsPerScrollUnit;
    this->Scroll(wxpex::ToWxPoint(newViewStart));
}


void PixelCanvas::OnVirtualSize_(const Size &virtualSize)
{
    this->virtualSize_ = virtualSize;
    auto viewPosition = this->viewPositionEndpoint_.Get();

    auto unitCount = this->virtualSize_ / PixelCanvas::pixelsPerScrollUnit;

    auto positionInUnits = viewPosition / pixelsPerScrollUnit;

    jive::ScopedCountFlag skipUpdate(this->skipUpdateViewPosition_);

    this->SetScrollbars(
        PixelCanvas::pixelsPerScrollUnit,
        PixelCanvas::pixelsPerScrollUnit,
        unitCount.width,
        unitCount.height,
        positionInUnits.x,
        positionInUnits.y);

    wxpex::LayoutTopLevel(this);
}


void PixelCanvas::OnPixels_(const std::shared_ptr<Pixels> &pixels)
{
    if (!pixels)
    {
#ifdef DEBUG_REQUIRE_PIXELS
        throw std::logic_error("pixels must not be NULL");
#else
        return;
#endif
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


/**
 ** The center pixel appears to jump around as we zoom in or out.
 ** This is caused by rounding error of the source region.
 **
 ** Shift the blit target coordinates to maintain the center pixel.
 **/
tau::Point2d<int> PixelCanvas::CorrectCenterPixel_(draw::View<int> &view) const
{
    auto sourceAsDouble = view.source.template Cast<double>();

    auto apparentCenter =
        sourceAsDouble.topLeft + (sourceAsDouble.size / 2.0);

    auto actualCenter = this->imageCenter_.Get();

    auto error = apparentCenter - actualCenter;

    if (error.Magnitude() <= 1.0)
    {
        auto correction =
            (error * view.scale).template Cast<int, tau::Round>();

        view.target.topLeft += correction;

        return correction;
    }

    return {};
}


} // end namespace draw
