#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <tau/eigen.h>
#include <tau/angles.h>
#include <wxpex/app.h>
#include <wxpex/border_sizer.h>
#include <wxpex/collapsible.h>
#include <wxpex/scrolled.h>

#include <draw/pixels.h>
#include <draw/segments_shape.h>
#include <draw/views/look_view.h>

#include <draw/views/pixel_view_settings.h>
#include <draw/views/pixel_view.h>
#include <draw/shapes.h>

#include "common/observer.h"
#include "common/about_window.h"
#include "common/brain.h"


template<typename T>
struct TrigFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::amplitude, "amplitude"),
        fields::Field(&T::frequency, "frequency"),
        fields::Field(&T::phase, "phase"),
        fields::Field(&T::points, "points"),
        fields::Field(&T::look, "look"));
};


template<template<typename> typename T>
struct TrigTemplate
{
    T<pex::MakeRange<double, pex::Limit<0>, pex::Limit<1000>>> amplitude;
    T<pex::MakeRange<double, pex::Limit<-10>, pex::Limit<10>>> frequency;
    T<pex::MakeRange<double, pex::Limit<-180>, pex::Limit<180>>> phase;
    T<pex::MakeRange<size_t, pex::Limit<2>, pex::Limit<1920>>> points;
    T<draw::LookGroup> look;
};


struct TrigSettings: public TrigTemplate<pex::Identity>
{
    static TrigSettings Default()
    {
        return {{400.0, 1.0, 0.0, 100, draw::Look::Default()}};
    }
};


using TrigGroup =
    pex::Group<TrigFields, TrigTemplate, pex::PlainT<TrigSettings>>;

using TrigModel = typename TrigGroup::Model;
using TrigControl = typename TrigGroup::Control;


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::sine, "sine"),
        fields::Field(&T::cosine, "cosine"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<TrigGroup> sine;
    T<TrigGroup> cosine;
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::Control;


class TrigSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    TrigSettingsView(
        wxWindow *parent,
        const std::string &name,
        TrigControl control,
        const LayoutOptions &layoutOptions)
        :
        wxpex::Collapsible(parent, name)
    {
        using namespace wxpex;

        auto panel = this->GetPanel();

        auto amplitude = wxpex::LabeledWidget(
            panel,
            "amplitude",
            new wxpex::ValueSlider(
                panel,
                control.amplitude,
                control.amplitude.value));

        auto frequency = wxpex::LabeledWidget(
            panel,
            "frequency",
            new wxpex::ValueSlider(
                panel,
                control.frequency,
                control.frequency.value));

        auto phase = wxpex::LabeledWidget(
            panel,
            "phase",
            new wxpex::ValueSlider(
                panel,
                control.phase,
                control.phase.value));

        auto points = wxpex::LabeledWidget(
            panel,
            "points",
            new wxpex::ValueSlider(
                panel,
                control.points,
                control.points.value));

        auto look =
            new draw::LookView(
                panel,
                "Look",
                control.look,
                layoutOptions);

        auto labeledSizer = wxpex::LayoutLabeled(
            layoutOptions,
            amplitude,
            frequency,
            phase,
            points);

        auto sizer = wxpex::LayoutItems(
            wxpex::ItemOptions(wxpex::verticalItems).SetProportion(1),
            labeledSizer.release(),
            look);

        this->ConfigureTopSizer(std::move(sizer));
    }
};


class DemoControls: public wxpex::Scrolled
{
public:
    DemoControls(
        wxWindow *parent,
        DemoControl control)
        :
        wxpex::Scrolled(parent)
    {
        wxpex::LayoutOptions layoutOptions{};
        layoutOptions.labelFlags = wxALIGN_RIGHT;

        auto sineView =
            new TrigSettingsView(
                this,
                "Sine",
                control.sine,
                layoutOptions);

        auto cosineView =
            new TrigSettingsView(
                this,
                "Cosine",
                control.cosine,
                layoutOptions);

        sineView->Expand();
        cosineView->Expand();

        auto sizer = wxpex::LayoutItems(
            wxpex::ItemOptions(wxpex::verticalItems).SetProportion(1),
            sineView,
            cosineView);

        auto topSizer = wxpex::BorderSizer(std::move(sizer), 5);
        this->ConfigureTopSizer(wxpex::verticalScrolled, std::move(topSizer));
    }
};


class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain()
        :
        sineId_(),
        cosineId_(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),
        demoControl_(this->demoModel_),

        sineEndpoint_(
            this,
            this->demoControl_.sine,
            &DemoBrain::OnSine_),

        cosineEndpoint_(
            this,
            this->demoControl_.cosine,
            &DemoBrain::OnCosine_),

        pixelViewEndpoint_(
            this,
            this->userControl_.pixelView)
    {
        this->demoControl_.sine.look.strokeEnable.Set(true);
        this->demoControl_.cosine.look.strokeEnable.Set(true);

        MakeTrigPoints_<Sin>(
            this->sinePoints_,
            this->demoControl_.sine.Get());

        MakeTrigPoints_<Cos>(
            this->cosinePoints_,
            this->demoControl_.cosine.Get());
    }

    std::string GetAppName() const
    {
        return "Segments Demo";
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        this->userControl_.pixelView.viewSettings.imageSize.Set(
            draw::Size(1920, 1080));

        return new DemoControls(parent, this->demoControl_);
    }

    void SaveSettings() const
    {
        std::cout << "TODO: Persist the processing settings." << std::endl;
    }

    void LoadSettings()
    {
        std::cout << "TODO: Restore the processing settings." << std::endl;
    }

    void ShowAbout()
    {
        wxAboutBox(MakeAboutDialogInfo("Segments Demo"));
    }

    void Display()
    {
        auto sineShapes = draw::Shapes(this->sineId_.Get());

        sineShapes.EmplaceBack<draw::SegmentsShape>(
            this->demoModel_.sine.look.Get(),
            this->sinePoints_);

        auto cosineShapes = draw::Shapes(this->cosineId_.Get());

        cosineShapes.EmplaceBack<draw::SegmentsShape>(
            this->demoModel_.cosine.look.Get(),
            this->cosinePoints_);

        this->userControl_.pixelView.asyncShapes.Set(sineShapes);
        this->userControl_.pixelView.asyncShapes.Set(cosineShapes);
    }

    void Shutdown()
    {
        Brain<DemoBrain>::Shutdown();
    }

    void LoadPng(const draw::GrayPng<PngPixel> &)
    {

    }

private:
    using RowVector = Eigen::RowVector<double, Eigen::Dynamic>;

    struct Sin
    {
        RowVector operator()(RowVector input)
        {
            return Eigen::sin(input.array());
        }
    };

    struct Cos
    {
        RowVector operator()(RowVector input)
        {
            return Eigen::cos(input.array());
        }
    };

    template<typename Function>
    static void MakeTrigPoints_(
        draw::PointsDouble &points,
        const TrigSettings &settings)
    {
        auto tauRadians = tau::Angles<double>::tau;

        RowVector x = RowVector::LinSpaced(static_cast<ssize_t>(settings.points), 0, tauRadians);

        RowVector sine =
            settings.amplitude
            * Function{}(
                settings.frequency * x.array()
                + tau::ToRadians(settings.phase));

        double drawSpacing = 1920.0
            / static_cast<double>(settings.points - 1);

        double zeroLine = 1080.0 / 2.;

        points.clear();

        for (ssize_t i = 0; i < x.size(); ++i)
        {
            points.emplace_back(i * drawSpacing, zeroLine - sine(i));
        }
    }

    void OnSine_(const TrigSettings &trigSettings)
    {
        MakeTrigPoints_<Sin>(this->sinePoints_, trigSettings);
        this->Display();
    }

    void OnCosine_(const TrigSettings &trigSettings)
    {
        MakeTrigPoints_<Cos>(this->cosinePoints_, trigSettings);
        this->Display();
    }

private:
    draw::ShapesId sineId_;
    draw::ShapesId cosineId_;
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    DemoControl demoControl_;
    pex::Endpoint<DemoBrain, TrigControl> sineEndpoint_;
    pex::Endpoint<DemoBrain, TrigControl> cosineEndpoint_;
    pex::EndpointGroup<DemoBrain, draw::PixelViewControl> pixelViewEndpoint_;
    draw::PointsDouble sinePoints_;
    draw::PointsDouble cosinePoints_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(wxpex::App<DemoBrain>)
