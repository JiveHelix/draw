#pragma once


#include "draw/views/canvas.h"

#include "draw/pixels.h"
#include "draw/views/pixel_view_settings.h"


namespace draw
{


class PixelCanvas: public Canvas
{
public:
    static constexpr auto observerName = "PixelCanvas";

    PixelCanvas(
        wxWindow *parent,
        PixelViewControl control);

private:
    void OnImageSize_(const Size &imageSize);

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

        auto scale = this->scaleEndpoint_.Get();

        auto viewRegion = tau::Region<int>{{
            this->viewPositionEndpoint_.Get(),
            this->control_.viewSettings.viewSize.Get()}};

        auto imageSize = this->imageSizeEndpoint_.Get();

        auto view =
            draw::View<int>(viewRegion, imageSize, scale);

        bool hasShapes = this->HasShapes_();

        if (!view.HasArea() && !hasShapes)
        {
            return false;
        }

        tau::Point2d<int> correction(0, 0);

        if (view.HasArea())
        {
            correction = this->CorrectCenterPixel_(view);

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

        auto gc = DrawContext(context);

        auto viewPosition =
            (this->viewPositionEndpoint_.Get() + correction)
                .template Cast<double>();

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
    using SizeEndpoint = pex::Endpoint<PixelCanvas, SizeControl>;
    SizeEndpoint imageSizeEndpoint_;

    wxImage image_;

    pex::Endpoint<PixelCanvas, PixelsControl> pixelsEndpoint_;
    std::shared_ptr<Pixels> pixelData_;

    AsyncShapesEndpoint<PixelCanvas> shapesEndpoint_;
    std::map<ssize_t, Shapes> shapesById_;
};


} // end namespace draw
