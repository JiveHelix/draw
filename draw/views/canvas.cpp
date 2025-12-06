#include <draw/views/canvas.h>


namespace draw
{


Canvas::Canvas(
    wxWindow *parent,
    const CanvasControl &control)
    :
    Scrolled(parent, wxID_ANY),
    ignoreViewPosition_(false),
    skipUpdateViewPosition_(),

    virtualSizeEndpoint_(
        PEX_THIS("Canvas"),
        control.viewSettings.virtualSize,
        &Canvas::OnVirtualSize_),

    viewPositionEndpoint_(
        this,
        control.viewSettings.viewPosition,
        &Canvas::OnViewPosition_),

    scaleEndpoint_(
        this,
        control.viewSettings.scale,
        &Canvas::OnScale_),

    cursorEndpoint_(this, control.cursor),
    imagePivot_(control.viewSettings.imagePivot),
    control_(control)
{

#ifdef __WXMSW__
    this->SetBackgroundStyle(wxBG_STYLE_PAINT);
#else
    this->SetBackgroundStyle(wxBG_STYLE_SYSTEM);
#endif

    this->Bind(wxEVT_SIZE, &Canvas::OnSize_, this);
    this->Bind(wxEVT_MOVE, &Canvas::OnMove_, this);

    this->Bind(
        wxEVT_MOTION,
        &Canvas::OnMouseMotion_,
        this);

    this->Bind(
        wxEVT_LEFT_DOWN,
        &Canvas::OnLeftDown_,
        this);

    this->Bind(
        wxEVT_LEFT_DCLICK,
        &Canvas::OnLeftDown_,
        this);

    this->Bind(
        wxEVT_LEFT_UP,
        &Canvas::OnLeftUp_,
        this);

    this->Bind(
        wxEVT_RIGHT_DOWN,
        &Canvas::OnRightDown_,
        this);

    this->Bind(
        wxEVT_RIGHT_UP,
        &Canvas::OnRightUp_,
        this);

    this->Bind(
        wxEVT_KEY_DOWN,
        &Canvas::OnKeyDown_,
        this);

    this->Bind(
        wxEVT_KEY_UP,
        &Canvas::OnKeyUp_,
        this);

    this->Bind(
        wxEVT_MENU,
        &Canvas::OnEventMenu_,
        this);

    this->cursorEndpoint_.Connect(&Canvas::OnCursor_);

    this->control_.window.Set(this);

    // Configure the scrollable area
    this->OnVirtualSize_(this->virtualSizeEndpoint_.Get());
}


Size Canvas::GetVirtualSize() const
{
    return this->virtualSize_;
}


void Canvas::ScrollWindow(int dx, int dy, const wxRect *rect)
{
    auto position = this->viewPositionEndpoint_.Get();

    if (!this->skipUpdateViewPosition_)
    {
        // This is not an internal repositioning.
        // Apply constraints.
        auto viewSize = this->control_.viewSettings.viewSize.Get();

        auto maximumPosition = GetMaximumViewPosition(
            viewSize,
            this->control_.viewSettings.virtualSize.Get());

        auto minimumPosition = -1 * viewSize.ToPoint();

        if (position.x - dx < minimumPosition.x)
        {
            // Clamp dx to maintain minimum position
            dx = position.x - minimumPosition.x;
        }
        else if (position.x - dx > maximumPosition.x)
        {
            // Clamp dx to maintain maximum position
            dx = position.x - maximumPosition.x;
        }

        if (position.y - dy < minimumPosition.y)
        {
            // Clamp dy to maintain minimum position
            dy = position.y - minimumPosition.y;
        }
        else if (position.y - dy > maximumPosition.y)
        {
            // Clamp dy to maintain maximum position
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


void Canvas::OnVirtualSize_(const Size &virtualSize)
{
    this->virtualSize_ = virtualSize;
    auto viewPosition = this->viewPositionEndpoint_.Get();

    auto unitCount = this->virtualSize_ / Canvas::pixelsPerScrollUnit;

    auto positionInUnits = viewPosition / pixelsPerScrollUnit;

    jive::ScopedCountFlag skipUpdate(this->skipUpdateViewPosition_);

    auto clientSize = wxpex::ToSize<int>(this->GetClientSize());
    auto effectiveVirtualSize = unitCount * Canvas::pixelsPerScrollUnit;

#if 0
    if (
        effectiveVirtualSize.height > clientSize.height)
    {
        std::cout
            << "OnVirtualSize_"
            << "\n  pixelsPerScrollUnit: " << Canvas::pixelsPerScrollUnit
            << "\n  unitCount: " << unitCount
            << "\n  positionInUnits: " << positionInUnits
            << "\n  clientSize: " << clientSize
            << "\n  effectiveVirtualSize: " << effectiveVirtualSize
            << std::endl;
    }
#endif

    this->SetScrollbars(
        Canvas::pixelsPerScrollUnit,
        Canvas::pixelsPerScrollUnit,
        unitCount.width,
        unitCount.height,
        positionInUnits.x,
        positionInUnits.y);

    wxpex::LayoutTopLevel(this);
}


void Canvas::OnSize_(wxSizeEvent &event)
{
    event.Skip();

    auto clientSize = wxpex::ToSize<int>(this->GetClientSize());
    auto windowSize = wxpex::ToSize<int>(this->GetSize());
    auto position = wxpex::ToPoint<int>(this->GetScreenPosition());

    auto viewSettings = pex::MakeDefer(this->control_.viewSettings);

    viewSettings.viewSize.Set(clientSize);
    viewSettings.windowSize.Set(windowSize);
    viewSettings.screenPosition.Set(position);
}


void Canvas::OnMove_(wxMoveEvent &event)
{
    event.Skip();

    auto position = wxpex::ToPoint<int>(this->GetScreenPosition());
    this->control_.viewSettings.screenPosition.Set(position);
}


void Canvas::OnMouseMotion_(wxMouseEvent &event)
{
    event.Skip();

    this->control_.mousePosition.Set(
        wxpex::ToPoint<double>(event.GetPosition()));
}


void Canvas::OnLeftDown_(wxMouseEvent &event)
{
    event.Skip();
    auto defer = pex::MakeDefer(this->control_);
    defer.mousePosition.Set(wxpex::ToPoint<double>(event.GetPosition()));
    defer.mouseDown.Set(true);
}


void Canvas::OnLeftUp_(wxMouseEvent &event)
{
    event.Skip();
    this->control_.mouseDown.Set(false);
}


void Canvas::OnRightDown_(wxMouseEvent &event)
{
    event.Skip();
    auto defer = pex::MakeDefer(this->control_);
    defer.mousePosition.Set(wxpex::ToPoint<double>(event.GetPosition()));
    defer.rightMouseDown.Set(true);
}


void Canvas::OnRightUp_(wxMouseEvent &event)
{
    event.Skip();
    this->control_.rightMouseDown.Set(false);
}


void Canvas::OnKeyDown_(wxKeyEvent &event)
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

    auto defer = pex::MakeDefer(this->control_);
    defer.modifier.Set(modifier);
    defer.keyCode.Set(keyCode);
}


void Canvas::OnKeyUp_(wxKeyEvent &event)
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


void Canvas::OnEventMenu_(wxCommandEvent &event)
{
    this->control_.menuId.Set(event.GetId());
}


void Canvas::OnScale_(const Scale &)
{
    this->Refresh(false);
    this->Update();
}


void Canvas::OnCursor_([[maybe_unused]] wxpex::Cursor cursor)
{
    // I haven't configured the resoure file necessary for loading cursors
    // in windows.
#ifndef __WXMSW__
    this->SetCursor(wxCursor(static_cast<wxStockCursor>(cursor)));
#endif
}


void Canvas::OnViewPosition_(const IntPoint &viewPosition)
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


/**
 ** The center pixel appears to jump around as we zoom in or out.
 ** This is caused by rounding error of the source region.
 **
 ** Shift the blit target coordinates to maintain the center pixel.
 **/
tau::Point2d<int> Canvas::CorrectCenterPixel_(draw::View<int> &view) const
{
    auto sourceAsDouble = view.source.template Cast<double>();

    auto apparentCenter =
        sourceAsDouble.topLeft + (sourceAsDouble.size / 2.0);

    auto imagePivot = this->imagePivot_.Get();

    auto error = apparentCenter - imagePivot;

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
