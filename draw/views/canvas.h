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
        const CanvasControl &control);

    Size GetVirtualSize() const;

protected:
    void ScrollWindow(int dx, int dy, const wxRect *rect);

    void OnVirtualSize_(const Size &virtualSize);

    void OnSize_(wxSizeEvent &event);

    void OnMove_(wxMoveEvent &event);

    void OnMouseMotion_(wxMouseEvent &event);

    void OnLeftDown_(wxMouseEvent &event);

    void OnLeftUp_(wxMouseEvent &event);

    void OnRightDown_(wxMouseEvent &event);

    void OnRightUp_(wxMouseEvent &event);

    void OnKeyDown_(wxKeyEvent &event);

    void OnKeyUp_(wxKeyEvent &event);

    void OnEventMenu_(wxCommandEvent &event);

    void OnScale_(const Scale &);

    void OnCursor_([[maybe_unused]] wxpex::Cursor cursor);

    void OnViewPosition_(const IntPoint &viewPosition);

    /**
     ** The center pixel appears to jump around as we zoom in or out.
     ** This is caused by rounding error of the source region.
     **
     ** Shift the blit target coordinates to maintain the center pixel.
     **/
    tau::Point2d<int> CorrectCenterPixel_(draw::View<int> &view) const;

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

    using ImagePivotControl =
        typename tau::Point2dGroup<double>::DefaultControl;

    ImagePivotControl imagePivot_;

    CanvasControl control_;

    Size virtualSize_;
};


} // end namespace draw
