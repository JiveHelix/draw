#include <wxpex/wxshim_app.h>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <tau/eigen.h>
#include <tau/angles.h>
#include <wxpex/app.h>
#include <wxpex/border_sizer.h>
#include <wxpex/list_view.h>
#include <wxpex/check_box.h>
#include <wxpex/button.h>
#include <wxpex/layout_items.h>


#include <draw/pixels.h>
#include <draw/segments_shape.h>
#include <draw/views/look_view.h>

#include <draw/views/pixel_view_settings.h>
#include <draw/views/pixel_view.h>
#include <draw/shapes.h>

#include "common/observer.h"
#include "common/about_window.h"
#include "common/brain.h"


using FrequencyRange =
    pex::MakeRange<double, pex::Limit<0, 1, 32>, pex::Limit<32>>;

constexpr size_t initialFunctionCount = 4;


template<typename T>
struct SettingsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::functionCount, "functionCount"),
        fields::Field(&T::pointCount, "pointCount"),
        fields::Field(&T::startFrequency, "startFrequency"),
        fields::Field(&T::endFrequency, "endFrequency"),
        fields::Field(&T::amplitude, "amplitude"),
        fields::Field(&T::isLogarithmic, "isLogarithmic"),
        fields::Field(&T::resetPhase, "resetPhase"),
        fields::Field(&T::resetLook, "resetLook"));
};


template<template<typename> typename T>
struct SettingsTemplate
{
    T<pex::MakeRange<size_t, pex::Limit<2>, pex::Limit<32>>> functionCount;
    T<pex::MakeRange<size_t, pex::Limit<2>, pex::Limit<2048>>> pointCount;
    T<FrequencyRange> startFrequency;
    T<FrequencyRange> endFrequency;
    T<pex::MakeRange<double, pex::Limit<0>, pex::Limit<1000>>> amplitude;
    T<bool> isLogarithmic;
    T<pex::MakeSignal> resetPhase;
    T<pex::MakeSignal> resetLook;

    static constexpr auto fields = SettingsFields<SettingsTemplate>::fields;
    static constexpr auto fieldsTypeName = "Settings";
};


struct Settings: public SettingsTemplate<pex::Identity>
{
    Settings()
        :
        SettingsTemplate<pex::Identity>{
            initialFunctionCount,
            256,
            1.0,
            4.0,
            200.0,
            true,
            {},
            {}}
    {

    }
};

using SettingsGroup =
    pex::Group<SettingsFields, SettingsTemplate, pex::PlainT<Settings>>;

using SettingsModel = typename SettingsGroup::Model;
using SettingsControl = typename SettingsGroup::DefaultControl;


template<typename T>
struct TrigFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::amplitude, "amplitude"),
        fields::Field(&T::frequency, "frequency"),
        fields::Field(&T::phase, "phase"),
        fields::Field(&T::look, "look"));
};


template<template<typename> typename T>
struct TrigTemplate
{
    T<pex::MakeRange<double, pex::Limit<0>, pex::Limit<1000>>> amplitude;
    T<FrequencyRange> frequency;
    T<pex::MakeRange<double, pex::Limit<-180>, pex::Limit<180>>> phase;
    T<draw::LookGroup> look;

    static constexpr auto fields = TrigFields<TrigTemplate>::fields;
    static constexpr auto fieldsTypeName = "TrigSettings";
};


struct TrigSettings: public TrigTemplate<pex::Identity>
{
    TrigSettings()
        :
        TrigTemplate<pex::Identity>{
            400.0,
            1.0,
            0.0,
            {}}
    {

    }
};


using TrigGroup =
    pex::Group<TrigFields, TrigTemplate, pex::PlainT<TrigSettings>>;

using TrigModel = typename TrigGroup::Model;
using TrigControl = typename TrigGroup::DefaultControl;


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::settings, "settings"),
        fields::Field(&T::functions, "functions"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<SettingsGroup> settings;
    T<pex::List<TrigGroup, initialFunctionCount>> functions;

    static constexpr auto fields = DemoFields<DemoTemplate>::fields;
    static constexpr auto fieldsTypeName = "Demo";
};


using FunctionCountControl = decltype(SettingsControl::functionCount);
using FrequencyControl = decltype(SettingsControl::startFrequency);
using AmplitudeControl = decltype(SettingsControl::amplitude);
using BooleanControl = decltype(SettingsControl::isLogarithmic);
using SignalControl = pex::control::DefaultSignal;


struct DemoCustom
{
    template<typename Base>
    class Model: public Base
    {
    public:
        Model()
            :
            Base{},
            ignoreFunctionEndpoints_{},

            functionCountEndpoint_(
                this,
                this->settings.functionCount,
                &Model::OnFunctionCount_),

            startFrequencyEndpoint_(
                this,
                this->settings.startFrequency,
                &Model::OnStartingFrequency_),

            endFrequencyEndpoint_(
                this,
                this->settings.endFrequency,
                &Model::OnEndingFrequency_),

            amplitudeEndpoint_(
                this,
                this->settings.amplitude,
                &Model::OnAmplitude_),

            isLogarithmicEndpoint_(
                this,
                this->settings.isLogarithmic,
                &Model::OnIsLogarithmic_),

            resetPhaseEndpoint_(
                this,
                this->settings.resetPhase,
                &Model::OnResetPhase_),

            resetLookEndpoint_(
                this,
                this->settings.resetLook,
                &Model::OnResetLook_),

            functionFrequencyEndpoints_{}
        {
            this->CreateFunctions_();
            this->OnResetLook_();
        }

        static double GetFundamental(double frequency)
        {
            auto positiveFrequency = std::abs(frequency);

            // The FrequencyRange should not allow an input of zero.
            assert(positiveFrequency > 0.0);

            double octave = std::floor(std::log2(positiveFrequency));

            return positiveFrequency / std::exp2(octave);
        }

        static double GetHue(double frequency)
        {
            // Rounding-errors can disagree with mathematical theory.
            // It shouldn't be possible to obtain a result less than 1.0, yet
            // it is possible.
            double fundamental = std::max(1.0, GetFundamental(frequency));
            return 360.0 * (fundamental - 1.0);
        }

    private:
        void CreateLinearFunctions_()
        {
            auto theseSettings = this->settings.Get();

            double frequencyRange =
                theseSettings.endFrequency - theseSettings.startFrequency;

            double frequencyStep =
                frequencyRange
                / static_cast<double>(theseSettings.functionCount - 1);

            double nextFrequency = theseSettings.startFrequency;

            jive::ScopeFlag ignore(this->ignoreFunctionEndpoints_);

            for (size_t i = 0; i < theseSettings.functionCount; ++i)
            {
                auto &functionModel = this->functions[i];
                functionModel.amplitude.Set(theseSettings.amplitude);
                functionModel.frequency.Set(nextFrequency);
                nextFrequency += frequencyStep;
                functionModel.phase.Set(0);
            }
        }

        void CreateLogarithmicFunctions_()
        {
            auto theseSettings = this->settings.Get();

            assert(theseSettings.functionCount > 1);

            double frequencyRange =
                theseSettings.endFrequency / theseSettings.startFrequency;

            double stepFactor = std::pow(
                frequencyRange,
                1.0 / static_cast<double>(theseSettings.functionCount - 1));

            double nextFrequency = theseSettings.startFrequency;

            jive::ScopeFlag ignore(this->ignoreFunctionEndpoints_);

            auto defer = pex::MakeDefer(this->functions);

            for (size_t i = 0; i < theseSettings.functionCount; ++i)
            {
                auto &functionModel = defer[i];
                functionModel.amplitude.Set(theseSettings.amplitude);
                functionModel.frequency.Set(nextFrequency);
                nextFrequency *= stepFactor;
                functionModel.phase.Set(0);
            }
        }

        void CreateFunctions_()
        {
            if (this->settings.isLogarithmic.Get())
            {
                this->CreateLogarithmicFunctions_();
            }
            else
            {
                this->CreateLinearFunctions_();
            }
        }

        void OnFunctionFrequency_(double frequency, size_t index)
        {
            if (this->ignoreFunctionEndpoints_)
            {
                return;
            }

            this->functions[index].look.stroke.color.hue.Set(GetHue(frequency));
        }

        void OnFunctionCount_(size_t functionCount)
        {
            bool countIncreased = functionCount > this->functions.count.Get();

            this->functionFrequencyEndpoints_.clear();
            this->functions.count.Set(functionCount);

            this->CreateFunctions_();

            if (countIncreased)
            {
                this->OnResetLook_();
            }

            for (size_t i = 0; i < functionCount; ++i)
            {
                this->functionFrequencyEndpoints_.emplace_back(
                    this,
                    this->functions[i].frequency,
                    &Model::OnFunctionFrequency_,
                    i);
            }
        }

        void OnStartingFrequency_(double)
        {
            this->CreateFunctions_();
            this->OnResetLook_();
        }

        void OnEndingFrequency_(double)
        {
            this->CreateFunctions_();
            this->OnResetLook_();
        }

        void OnAmplitude_(double)
        {
            this->CreateFunctions_();
        }

        void OnIsLogarithmic_(bool)
        {
            this->CreateFunctions_();
            this->OnResetLook_();
        }

        void OnResetPhase_()
        {
            auto count = this->functions.count.Get();

            for (size_t i = 0; i < count; ++i)
            {
                this->functions[i].phase.Set(0.0);
            }
        }

        void OnResetLook_()
        {
            auto count = this->functions.count.Get();

            for (size_t i = 0; i < count; ++i)
            {
                auto &functionModel = this->functions[i];
                auto &look = functionModel.look;

                auto thisLook = draw::Look{};
                thisLook.stroke.color.saturation = 1.0;

                thisLook.stroke.color.hue =
                    GetHue(functionModel.frequency.Get());

                look.Set(thisLook);
            }
        }

        using FunctionCountEndpoint =
            pex::Endpoint<Model, FunctionCountControl>;

        using FrequencyEndpoint =
            pex::Endpoint<Model, FrequencyControl>;

        using AmplitudeEndpoint =
            pex::Endpoint<Model, AmplitudeControl>;

        using BooleanEndpoint =
            pex::Endpoint<Model, BooleanControl>;

        using SignalEndpoint =
            pex::Endpoint<Model, SignalControl>;

        using FunctionFrequencyEndpoint =
            pex::BoundEndpoint
            <
                FrequencyControl,
                decltype(&Model::OnFunctionFrequency_)
            >;

        bool ignoreFunctionEndpoints_;
        FunctionCountEndpoint functionCountEndpoint_;
        FrequencyEndpoint startFrequencyEndpoint_;
        FrequencyEndpoint endFrequencyEndpoint_;
        AmplitudeEndpoint amplitudeEndpoint_;
        BooleanEndpoint isLogarithmicEndpoint_;
        SignalEndpoint resetPhaseEndpoint_;
        SignalEndpoint resetLookEndpoint_;

        std::vector<FunctionFrequencyEndpoint> functionFrequencyEndpoints_;
    };
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate, DemoCustom>;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::DefaultControl;

using FunctionsControl = decltype(DemoControl::functions);
using FunctionAddedControl = typename FunctionsControl::MemberAdded;
using FunctionWillRemoveControl = typename FunctionsControl::MemberWillRemove;


class SettingsView: public wxPanel
{
public:
    SettingsView(wxWindow *parent, const SettingsControl &control)
        :
        wxPanel(parent, wxID_ANY)
    {
        auto functionCount = wxpex::LabeledWidget(
            this,
            "functions",
            new wxpex::FieldSlider(
                this,
                control.functionCount,
                control.functionCount.value));

        auto pointCount = wxpex::LabeledWidget(
            this,
            "points",
            new wxpex::FieldSlider(
                this,
                control.pointCount,
                control.pointCount.value));

        auto startFrequency = wxpex::LabeledWidget(
            this,
            "start frequency",
            new wxpex::FieldSlider(
                this,
                control.startFrequency,
                control.startFrequency.value));

        auto endFrequency = wxpex::LabeledWidget(
            this,
            "end frequency",
            new wxpex::FieldSlider(
                this,
                control.endFrequency,
                control.endFrequency.value));

        auto amplitude = wxpex::LabeledWidget(
            this,
            "amplitude",
            new wxpex::FieldSlider(
                this,
                control.amplitude,
                control.amplitude.value));

        auto isLogarithmic = wxpex::LabeledWidget(
            this,
            "logarithmic",
            new wxpex::CheckBox(
                this,
                "",
                control.isLogarithmic));

        auto resetPhase = new wxpex::Button(
            this,
            "reset phase",
            control.resetPhase);

        auto resetLook = new wxpex::Button(
            this,
            "reset look",
            control.resetLook);

        auto sizer = wxpex::LayoutLabeled(
            wxpex::LayoutOptions{},
            functionCount,
            pointCount,
            startFrequency,
            endFrequency,
            amplitude,
            isLogarithmic);

        auto topSizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            sizer.release(),
            wxpex::LayoutItems(
                wxpex::horizontalItems,
                resetPhase,
                resetLook).release());

        this->SetSizerAndFit(topSizer.release());
    }
};


class FunctionView: public wxpex::Collapsible
{
public:
    FunctionView(
        wxWindow *parent,
        const std::string &name,
        const TrigControl &control)
        :
        wxpex::Collapsible(parent, name)
    {
        using namespace wxpex;

        auto panel = this->GetPanel();

        auto amplitude = wxpex::LabeledWidget(
            panel,
            "amplitude",
            new wxpex::FieldSlider(
                panel,
                control.amplitude,
                control.amplitude.value));

        auto frequency = wxpex::LabeledWidget(
            panel,
            "frequency",
            new wxpex::FieldSlider(
                panel,
                control.frequency,
                control.frequency.value));

        auto phase = wxpex::LabeledWidget(
            panel,
            "phase",
            new wxpex::FieldSlider(
                panel,
                control.phase,
                control.phase.value));

        auto look =
            new draw::LookView(
                panel,
                "Look",
                control.look,
                wxpex::LayoutOptions{});

        auto labeledSizer = wxpex::LayoutLabeled(
            wxpex::LayoutOptions{},
            amplitude,
            frequency,
            phase);

        auto sizer = wxpex::LayoutItems(
            wxpex::VerticalItems().Proportion(1),
            labeledSizer,
            look);

        this->ConfigureSizer(std::move(sizer));
    }
};


class FunctionListView: public wxpex::ListView<FunctionsControl>
{
public:
    using Base = wxpex::ListView<FunctionsControl>;

    FunctionListView(wxWindow *parent, const FunctionsControl &control)
        :
        Base(parent, control)
    {
        this->Initialize_();
    }

    wxWindow * CreateView_(ListItem &listItem, size_t index) override
    {
        return new FunctionView(
            this,
            fmt::format("Function {}", index),
            listItem);
    }
};


class DemoControls: public wxPanel
{
public:
    DemoControls(
        wxWindow *parent,
        const DemoControl &control)
        :
        wxPanel(parent, wxID_ANY)
    {
        auto settingsView = new SettingsView(
            this,
            control.settings);

        auto functionList = new FunctionListView(
            this,
            control.functions);

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            settingsView,
            functionList);

        auto topSizer = wxpex::BorderSizer(std::move(sizer), 2);
        this->SetSizerAndFit(topSizer.release());
    }
};


class DemoBrain: public Brain<DemoBrain>
{
public:
    DemoBrain()
        :
        shapesId_(),
        observer_(this, UserControl(this->user_)),
        demoModel_(),
        demoControl_(this->demoModel_),

        imageSizeEndpoint_(
            this,
            this->userControl_.pixelView.canvas.viewSettings.imageSize,
            &DemoBrain::OnImageSize_),

        functionWillRemoveEndpoint_(
            this,
            this->demoModel_.functions.memberWillRemove,
            &DemoBrain::OnFunctionWillRemove_),

        functionAddedEndpoint_(
            this,
            this->demoModel_.functions.memberAdded,
            &DemoBrain::OnFunctionAdded_),

        pointCountEndpoint_(
            this,
            this->demoModel_.settings.pointCount,
            &DemoBrain::OnPointCount_),

        functionEndpoints_(),

        listChangedConnection_(
            this,
            this->demoModel_.functions,
            &DemoBrain::OnFunctionList_)

    {
        auto count = this->demoModel_.functions.count.Get();
        this->functionPoints_.resize(count);
        this->CreateFunctionEndpoints_(count);
    }

    std::string GetAppName() const
    {
        return "Segments Demo";
    }

    wxWindow * CreateControls(wxWindow *parent)
    {
        this->userControl_.pixelView.canvas.viewSettings.imageSize.Set(
            draw::Size(1920, 1080));

        return new DemoControls(parent, this->demoControl_);
    }

    void ShowAbout()
    {
        wxAboutBox(MakeAboutDialogInfo("Segments Demo"));
    }

    void Display()
    {
        auto shapes = draw::Shapes(this->shapesId_.Get());

        size_t i = 0;

        draw::SegmentsSettings settings{};
        settings.isSpline = true;

        for (auto &function: this->demoControl_.functions)
        {
            settings.look = function.look.Get();

            shapes.EmplaceBack<draw::SegmentsShape>(
                settings,
                this->functionPoints_.at(i++));
        }

        this->userControl_.pixelView.asyncShapes.Set(shapes);
    }

    void Shutdown()
    {
        Brain<DemoBrain>::Shutdown();
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
        size_t pointCount,
        const TrigSettings &settings,
        const draw::Size &imageSize)
    {
        auto tauRadians = tau::Angles<double>::tau;

        RowVector x =
            RowVector::LinSpaced(
                static_cast<int64_t>(pointCount),
                0,
                tauRadians);

        RowVector sine =
            settings.amplitude
            * Function{}(
                settings.frequency * x.array()
                + tau::ToRadians(settings.phase));

        double drawSpacing = static_cast<double>(imageSize.width)
            / static_cast<double>(pointCount - 1);

        double zeroLine = 1080.0 / 2.;

        points.clear();

        for (int64_t i = 0; i < x.size(); ++i)
        {
                points.emplace_back(
                    static_cast<double>(i) * drawSpacing,
                    zeroLine - sine(i));
        }
    }

    void OnFunction_(const TrigSettings &trigSettings, size_t index)
    {
        MakeTrigPoints_<Sin>(
            this->functionPoints_.at(index),
            this->demoModel_.settings.pointCount.Get(),
            trigSettings,
            this->userControl_.pixelView.canvas.viewSettings.imageSize.Get());
    }

    void OnFunctionList_()
    {
        this->Display();
    }

    void CreateFunctionEndpoints_(size_t functionCount)
    {
        for (size_t i = 0; i < functionCount; ++i)
        {
                this->functionEndpoints_.emplace_back(
                    this,
                    this->demoModel_.functions[i],
                    &DemoBrain::OnFunction_,
                    i);
        }
    }

    void OnImageSize_(const draw::Size &)
    {
        this->ComputeFunctions_();
        this->Display();
    }

    void ComputeFunctions_()
    {
        size_t index = 0;

        for (auto &function: this->demoControl_.functions)
        {
            this->OnFunction_(function.Get(), index++);
        }
    }

    void OnFunctionWillRemove_(const std::optional<size_t> &index)
    {
        if (!index)
        {
            return;
        }

        this->functionEndpoints_.erase(
            jive::SafeEraseIterator(this->functionEndpoints_, *index));
    }

    void OnFunctionAdded_(const std::optional<size_t> &index)
    {
        if (!index)
        {
            return;
        }
#if 0
        this->functionEndpoints_.emplace(
            jive::SafeInsertIterator(this->functionEndpoints_, *index),
            this,
            this->demoModel_.functions[*index],
            &DemoBrain::OnFunction_,
            *index);
#endif
    }

    void OnPointCount_(size_t)
    {
        this->ComputeFunctions_();
        this->Display();
    }

private:
    draw::ShapesId shapesId_;
    Observer<DemoBrain> observer_;
    DemoModel demoModel_;
    DemoControl demoControl_;

    using ImageSizeEndpoint = pex::Endpoint<DemoBrain, draw::SizeControl>;

    using FunctionWillRemoveEndpoint =
        pex::Endpoint<DemoBrain, FunctionWillRemoveControl>;

    using FunctionAddedEndpoint =
        pex::Endpoint<DemoBrain, FunctionAddedControl>;

    ImageSizeEndpoint imageSizeEndpoint_;
    FunctionWillRemoveEndpoint functionWillRemoveEndpoint_;
    FunctionAddedEndpoint functionAddedEndpoint_;

    using PointCountEndpoint =
        pex::Endpoint<DemoBrain, decltype(SettingsControl::pointCount)>;

    PointCountEndpoint pointCountEndpoint_;

    using FunctionEndpoint =
        pex::BoundEndpoint<TrigControl, decltype(&DemoBrain::OnFunction_)>;

    std::vector<FunctionEndpoint> functionEndpoints_;
    std::vector<draw::PointsDouble> functionPoints_;

    using ListChangedConnection =
        pex::detail::ListConnect<DemoBrain, decltype(DemoModel::functions)>;

    ListChangedConnection listChangedConnection_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimAPP(wxpex::App<DemoBrain>)
