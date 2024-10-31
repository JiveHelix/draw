#include <pex/endpoint.h>
#include <wxpex/scrolled.h>
#include <wxpex/border_sizer.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/app.h>
#include <draw/size.h>
#include <draw/bitmap.h>
#include <draw/regular_polygon.h>
#include <draw/color_map.h>

#include <draw/views/bitmap_view.h>
#include <draw/views/pixel_view.h>
#include <draw/views/size_view.h>
#include <draw/views/regular_polygon_view.h>
#include <draw/views/color_map_settings_view.h>
#include <draw/draw_segments.h>

#include "common/brain.h"


using MonoValue = int32_t;



template<typename T>
struct DrawingFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::regularPolygon, "regularPolygon"),
        fields::Field(&T::look, "look"),
        fields::Field(&T::colorMapSettings, "colorMapSettings"));
};


template<template<typename> typename T>
struct DrawingTemplate
{
    T<draw::RegularPolygonGroup> regularPolygon;
    T<draw::LookGroup> look;
    T<draw::ColorMapSettingsGroup<MonoValue>> colorMapSettings;

    static constexpr auto fields = DrawingFields<DrawingTemplate>::fields;
};


using DrawingGroup = pex::Group<DrawingFields, DrawingTemplate>;
using DrawingModel = typename DrawingGroup::Model;
using DrawingControl = typename DrawingGroup::Control;
using DrawingSettings = typename DrawingGroup::Plain;



template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::drawing, "drawing"),
        fields::Field(&T::requestDrawing, "requestDrawing"),
        fields::Field(&T::renderCanvas, "renderCanvas"),
        fields::Field(&T::pixelView, "pixelView"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<DrawingGroup> drawing;
    T<pex::MakeSignal> requestDrawing;
    T<draw::CanvasGroup> renderCanvas;
    T<draw::PixelViewGroup> pixelView;

    static constexpr auto fields = DemoFields<DemoTemplate>::fields;
};


struct DemoCustom
{
    template<typename Base>
    struct Model: public Base
    {
        Model()
            :
            Base(),
            sizeEndpoint_(
                this,
                this->renderCanvas.viewSettings.imageSize,
                &Model::OnSize_),
            imageSize_()
        {
            this->OnSize_(this->renderCanvas.viewSettings.imageSize.Get());
        }

    private:
        void OnSize_(const draw::Size &size_)
        {
            if (size_ == this->imageSize_)
            {
                return;
            }

            this->pixelView.canvas.viewSettings.imageSize.Set(size_);

            tau::Point2d<int> sizeAsPoint(size_.width, size_.height);

            this->drawing.regularPolygon.center.Set(
                sizeAsPoint.template Cast<double>() / 2.0);

            this->imageSize_ = size_;
        }

    private:
        using SizeEndpoint = pex::Endpoint<Model, draw::SizeControl>;

        SizeEndpoint sizeEndpoint_;
        draw::Size imageSize_;
    };

    template<typename Base>
    struct Plain: public Base
    {
        Plain()
            :
            Base{}
        {
            auto defaultImageSize = draw::Size(512, 512);
            this->renderCanvas.viewSettings.imageSize = defaultImageSize;
            this->pixelView.canvas.viewSettings.imageSize = defaultImageSize;
        }
    };
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate, DemoCustom>;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::Control;
using DemoSettings = typename DemoGroup::Plain;


class DemoControlView: public wxpex::Scrolled
{
public:
    DemoControlView(wxWindow *parent, DemoControl control)
        :
        wxpex::Scrolled(parent)
    {
        auto size = wxpex::LabeledWidget(
            this,
            "Image Size",
            new draw::SizeView<int>(
                this,
                control.renderCanvas.viewSettings.imageSize));

        auto regularPolygon =
            new draw::RegularPolygonView(
                this,
                control.drawing.regularPolygon,
                wxpex::LayoutOptions{});

        auto look = new draw::LookView(
            this,
            "Look",
            control.drawing.look,
            wxpex::LayoutOptions{});


        auto colorMap = new draw::ColorMapSettingsView<MonoValue>(
            this,
            control.drawing.colorMapSettings,
            {});

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            size.Layout().release(),
            regularPolygon,
            look,
            colorMap);

        this->ConfigureSizer(
            wxpex::verticalScrolled,
            wxpex::BorderSizer(std::move(sizer), 5));
    }
};


class DemoPixelViews: public wxPanel
{
public:
    DemoPixelViews(wxWindow *parent, DemoControl control)
        :
        wxPanel(parent, wxID_ANY),
        control_(control)
    {
        this->renderView_ =
            new draw::BitmapView(this, control.renderCanvas);

        this->rasterView_ =
            new draw::PixelView(this, control.pixelView);

        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
        sizer->Add(this->renderView_, 1, wxEXPAND | wxRIGHT, 10);
        sizer->Add(this->rasterView_, 1, wxEXPAND);

        this->SetSizer(sizer.release());
        this->Bind(wxEVT_PAINT, &DemoPixelViews::OnPaint_, this);
    }

    wxWindow * GetRenderView()
    {
        return this->renderView_;
    }

    draw::BitmapView * GetBitmapView()
    {
        return this->renderView_;
    }

    void OnPaint_(wxPaintEvent &)
    {
        this->control_.requestDrawing.Trigger();
    }

private:
    DemoControl control_;
    draw::BitmapView *renderView_;
    draw::PixelView *rasterView_;
};


class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain()
        :
        Brain<DemoBrain>(),
        model_(),
        control_(this->model_),
        ignore_(false),
        demoPixelViews_(nullptr),
        colorMap_(this->model_.drawing.colorMapSettings.Get()),

        drawingSettingsEndpoint_(
            this,
            this->control_.drawing),

        colorMapSettingsEndpoint_(
            this,
            this->control_.drawing.colorMapSettings,
            &DemoBrain::OnColorMapSettings_),

        requestDrawingEndpoint_(
            this,
            this->control_.requestDrawing)
    {

    }

    std::string GetAppName() const
    {
        return "Bitmap Demo";
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        return new DemoControlView(parent, this->control_);
    }

    wxWindow * CreatePixelView(wxWindow *parent)
    {
        this->demoPixelViews_ = new DemoPixelViews(parent, this->control_);

        this->drawingSettingsEndpoint_.Connect(
            &DemoBrain::OnDrawingSettings_);

        this->requestDrawingEndpoint_.Connect(&DemoBrain::Display);

        return this->demoPixelViews_;
    }

    void Display()
    {
        // Draw the polygon first.
        this->DrawPolygon_();
        this->DisplayColorMapped_();
    }

private:
    wxBitmap & GetBitmap()
    {
        assert(this->demoPixelViews_);

        return this->demoPixelViews_->GetBitmapView()->GetBitmap();
    }

    void DrawPolygon_()
    {
        auto &bitmap = this->GetBitmap();
        auto memory = wxMemoryDC(bitmap);
        memory.SetBackground(*wxBLACK_BRUSH);
        memory.Clear();
        auto context = draw::DrawContext(memory);
        context.ConfigureLook(this->model_.drawing.look.Get());

        auto points = this->model_.drawing.regularPolygon.Get().GetPoints();
        draw::DrawSegments(context, points);

        this->demoPixelViews_->GetBitmapView()->Refresh();
    }

    void DisplayColorMapped_()
    {
        auto image = draw::GetMonoImage(
            this->GetBitmap(),
            this->model_.drawing.colorMapSettings.maximum.Get());

        auto colorMapped = this->colorMap_.Filter(image);
        this->control_.pixelView.asyncPixels.Set(colorMapped.MakeShared());
    }

    void OnDrawingSettings_(const DrawingSettings &)
    {
        if (this->ignore_)
        {
            return;
        }

        jive::ScopeFlag scopeFlag(this->ignore_);
        this->Display();
    }

    void OnColorMapSettings_(
        const draw::ColorMapSettings<MonoValue> &colorMapSettings)
    {
        this->colorMap_ = draw::ColorMap<MonoValue>(colorMapSettings);
    }

private:
    DemoModel model_;
    DemoControl control_;
    bool ignore_;
    DemoPixelViews *demoPixelViews_;
    draw::ColorMap<MonoValue> colorMap_;

    using DrawingSettingsEndpoint = pex::Endpoint<DemoBrain, DrawingControl>;
    DrawingSettingsEndpoint drawingSettingsEndpoint_;

    using ColorMapSettingsEndpoint =
        pex::Endpoint<DemoBrain, draw::ColorMapSettingsControl<MonoValue>>;

    ColorMapSettingsEndpoint colorMapSettingsEndpoint_;

    using RequestDrawingEndpoint =
        pex::Endpoint<DemoBrain, pex::control::Signal<>>;

    RequestDrawingEndpoint requestDrawingEndpoint_;
};



wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
