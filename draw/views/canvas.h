#pragma once


#include <jive/scope_flag.h>
#include <pex/value.h>
#include <pex/endpoint.h>

#include <wxpex/ignores.h>
#include <wxpex/cursor.h>
#include <wxpex/layout_top_level.h>

WXSHIM_PUSH_IGNORES
#include <wx/scrolwin.h>

#ifdef __WXMSW__
#include <wx/dcbuffer.h>
#endif

WXSHIM_POP_IGNORES


#include "draw/point.h"
#include "draw/size.h"
#include "draw/view.h"
#include "draw/scale.h"
#include "draw/views/canvas_settings.h"


namespace draw
{


using Scrolled = wxScrolled<wxPanel>;


class Canvas: public Scrolled
{
public:
    static constexpr auto observerName = "Canvas";
    static constexpr int pixelsPerScrollUnit = 10;

    Canvas(
        wxWindow *parent,
        CanvasControl control)
        :
        Scrolled(parent, wxID_ANY),
        ignoreViewPosition_(false),
        skipUpdateViewPosition_(),

        virtualSizeEndpoint_(
            this,
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
        imageCenter_(control.viewSettings.imageCenterPixel),
        control_(control)
    {

#ifdef __WXMSW__
        this->SetBackgroundStyle(wxBG_STYLE_PAINT);
#else
        this->SetBackgroundStyle(wxBG_STYLE_SYSTEM);
#endif

        this->Bind(wxEVT_SIZE, &Canvas::OnSize_, this);

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

    Size GetVirtualSize() const
    {
        return this->virtualSize_;
    }

protected:
    void ScrollWindow(int dx, int dy, const wxRect *rect)
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

    void OnVirtualSize_(const Size &virtualSize)
    {
        this->virtualSize_ = virtualSize;
        auto viewPosition = this->viewPositionEndpoint_.Get();

        auto unitCount = this->virtualSize_ / Canvas::pixelsPerScrollUnit;

        auto positionInUnits = viewPosition / pixelsPerScrollUnit;

        jive::ScopedCountFlag skipUpdate(this->skipUpdateViewPosition_);

        this->SetScrollbars(
            Canvas::pixelsPerScrollUnit,
            Canvas::pixelsPerScrollUnit,
            unitCount.width,
            unitCount.height,
            positionInUnits.x,
            positionInUnits.y);

        wxpex::LayoutTopLevel(this);
    }

    void OnSize_(wxSizeEvent &event)
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

    void OnMouseMotion_(wxMouseEvent &event)
    {
        event.Skip();

        this->control_.mousePosition.Set(
            wxpex::ToPoint<double>(event.GetPosition()));
    }

    void OnLeftDown_(wxMouseEvent &event)
    {
        event.Skip();
        auto defer = pex::MakeDefer(this->control_);
        defer.mousePosition.Set(wxpex::ToPoint<double>(event.GetPosition()));
        defer.mouseDown.Set(true);
    }

    void OnLeftUp_(wxMouseEvent &event)
    {
        event.Skip();
        this->control_.mouseDown.Set(false);
    }

    void OnRightDown_(wxMouseEvent &event)
    {
        event.Skip();
        auto defer = pex::MakeDefer(this->control_);
        defer.mousePosition.Set(wxpex::ToPoint<double>(event.GetPosition()));
        defer.rightMouseDown.Set(true);
    }

    void OnRightUp_(wxMouseEvent &event)
    {
        event.Skip();
        this->control_.rightMouseDown.Set(false);
    }

    void OnKeyDown_(wxKeyEvent &event)
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

    void OnKeyUp_(wxKeyEvent &event)
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

    void OnEventMenu_(wxCommandEvent &event)
    {
        this->control_.menuId.Set(event.GetId());
    }

    void OnScale_(const Scale &)
    {
        this->Refresh(false);
        this->Update();
    }

    void OnCursor_([[maybe_unused]] wxpex::Cursor cursor)
    {
        // I haven't configured the resoure file necessary for loading cursors
        // in windows.
#ifndef __WXMSW__
        this->SetCursor(wxCursor(static_cast<wxStockCursor>(cursor)));
#endif
    }

    void OnViewPosition_(const IntPoint &viewPosition)
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
    tau::Point2d<int> CorrectCenterPixel_(draw::View<int> &view) const
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

protected:
    bool ignoreViewPosition_;
    jive::CountFlag<uint8_t> skipUpdateViewPosition_;

    using SizeEndpoint = pex::Endpoint<Canvas, SizeControl>;
    using PositionEndpoint = pex::Endpoint<Canvas, IntPointControl>;
    using ScaleEndpoint = pex::Endpoint<Canvas, ScaleControl>;

    static_assert(pex::IsGroupNode<IntPointControl>);

    SizeEndpoint virtualSizeEndpoint_;
    PositionEndpoint viewPositionEndpoint_;
    ScaleEndpoint scaleEndpoint_;

    using CursorEndpoint =
        typename pex::Endpoint
        <
            Canvas,
            decltype(CanvasControl::cursor)
        >;

    CursorEndpoint cursorEndpoint_;

    using ImageCenterControl = typename tau::Point2dGroup<double>::Control;
    ImageCenterControl imageCenter_;

    CanvasControl control_;

    Size virtualSize_;
};


} // end namespace draw
