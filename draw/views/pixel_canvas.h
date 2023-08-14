#pragma once

#include <mutex>
#include <wx/scrolwin.h>
#include <pex/value.h>
#include <pex/endpoint.h>
#include <tau/view.h>

#include "draw/scale.h"
#include "draw/pixels.h"
#include "draw/views/pixel_view_settings.h"


namespace draw
{


using Scrolled = wxScrolled<wxPanel>;

class PixelCanvas: public Scrolled
{
public:
    static constexpr auto observerName = "PixelCanvas";
    static constexpr int pixelsPerScrollUnit = 10;

    PixelCanvas(
        wxWindow *parent,
        PixelViewControl controls);

    Size GetVirtualSize() const;

protected:
    void ScrollWindow(int dx, int dy, const wxRect *rect) override;

private:
    void OnImageSize_(const Size &imageSize);

    void OnSize_(wxSizeEvent &event);

    void OnMouseMotion_(wxMouseEvent &event);

    void OnLeftDown_(wxMouseEvent &event);

    void OnLeftUp_(wxMouseEvent &event);

    void OnRightDown_(wxMouseEvent &event);

    void OnRightUp_(wxMouseEvent &event);

    void OnKeyDown_(wxKeyEvent &event);

    void OnKeyUp_(wxKeyEvent &event);

    void OnScale_(const Scale &scale);

    void OnCursor_(wxpex::Cursor cursor);

    void OnViewPosition_(const Point &viewPosition);

    void SizeVirtualPanel_(const Scale &scale);

    void OnPixels_(const std::shared_ptr<Pixels> &pixels);

    void OnShapes_(const Shapes &shapes);

    bool HasShapes_() const;

    void OnPaint_(wxPaintEvent &);

    template<typename Context>
    bool Draw_(Context &&context)
    {

#ifdef __WXMSW__
        context.SetBrush(wxBrush(*wxBLACK));
        context.Clear();
#endif

        auto scale = this->scaleEndpoint_.control.Get();

        auto viewRegion = tau::Region<int>{{
            this->viewPositionEndpoint_.control.Get(),
            this->control_.viewSettings.viewSize.Get()}};

        auto view = tau::View<int>(
            viewRegion,
            this->imageSizeEndpoint_.control.Get(),
            scale);

        bool hasShapes = this->HasShapes_();

        if (!view.HasArea() && !hasShapes)
        {
            return false;
        }

        if (view.HasArea())
        {
            auto bitmap = wxBitmap(this->image_);
            auto source = wxMemoryDC(bitmap);

            context.StretchBlit(
                view.target.topLeft.x,
                view.target.topLeft.y,
                view.target.size.width,
                view.target.size.height,
                &source,
                view.source.topLeft.x,
                view.source.topLeft.y,
                view.source.size.width,
                view.source.size.height);
        }

        if (!hasShapes)
        {
            return true;
        }

        auto gc = wxpex::GraphicsContext(context);

        auto viewPosition =
            this->viewPositionEndpoint_.control.Get()
                .template Convert<double>();

        gc->Translate(-viewPosition.x, -viewPosition.y);
        gc->Scale(scale.horizontal, scale.vertical);

        for (auto &it: this->shapesById_)
        {
            for (auto &shape: it.second.GetShapes())
            {
                shape->Draw(gc);
            }
        }

        return true;
    }

private:
    bool ignoreViewPosition_;
    bool skipUpdateViewPosition_;

    using SizeEndpoint = pex::EndpointControl<PixelCanvas, SizeControl>;
    using PositionEndpoint = pex::EndpointControl<PixelCanvas, PointControl>;
    using ScaleEndpoint = pex::EndpointControl<PixelCanvas, ScaleControl>;

    SizeEndpoint imageSizeEndpoint_;
    PositionEndpoint viewPositionEndpoint_;

    using CursorEndpoint =
        typename pex::Endpoint
        <
            PixelCanvas,
            decltype(PixelViewControl::cursor)
        >;

    ScaleEndpoint scaleEndpoint_;
    CursorEndpoint cursorEndpoint_;

    PixelViewControl control_;
    wxImage image_;

    Size virtualSize_;

    wxBitmap bitmap_;
    pex::Endpoint<PixelCanvas, PixelsControl> pixelsEndpoint_;
    std::shared_ptr<Pixels> pixelData_;

    pex::Endpoint<PixelCanvas, ShapesControl> shapesEndpoint_;
    std::map<ssize_t, Shapes> shapesById_;
};


} // end namespace draw