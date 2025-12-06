#pragma once


#include "draw/views/canvas.h"
#include "draw/views/canvas_settings.h"


namespace draw
{


class BitmapCanvas: public Canvas
{
public:
    static constexpr auto observerName = "BitmapCanvas";

    BitmapCanvas(
        wxWindow *parent,
        const CanvasControl &control);

    wxBitmap & GetBitmap();
    const wxBitmap & GetBitmap() const;
    void SetBitmap(const wxBitmap &bitmap);

private:
    void OnImageSize_(const Size &imageSize);

    void OnPaint_(wxPaintEvent &);

    template<typename Context>
    void Draw_(Context &&context)
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

        if (!view.HasArea())
        {
            return;
        }

        if (view.HasArea())
        {
            auto source = wxMemoryDC(this->bitmap_);

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
    }

private:
    using SizeEndpoint = pex::Endpoint<BitmapCanvas, SizeControl>;
    SizeEndpoint imageSizeEndpoint_;

    wxBitmap bitmap_;
};


} // end namespace draw
