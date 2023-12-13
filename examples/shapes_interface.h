#pragma once


#include <pex/signal.h>
#include <pex/endpoint.h>
#include <wxpex/border_sizer.h>
#include <wxpex/async.h>


template<typename View, typename ShapesControl>
class ShapesInterface: public wxPanel
{
public:
    static constexpr auto observerName = "ShapesInterface";

    ShapesInterface(
        wxWindow *parent,
        ShapesControl shapesControl,
        pex::control::Signal<> reorder)
        :
        wxPanel(parent, wxID_ANY),
        shapesControl_(shapesControl),
        viewsByShapeId_(),
        sizer_(),
        onReorder_(
            this,
            reorder,
            &ShapesInterface::OnReorder_)
    {
        wxpex::LayoutOptions layoutOptions{};
        layoutOptions.labelFlags = wxALIGN_RIGHT;

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        this->sizer_ = sizer.get();

        for (auto &it: this->shapesControl_)
        {
            auto id = it.id.Get();

            auto view =
                new View(
                    this,
                    fmt::format("Shape {}", id),
                    it,
                    layoutOptions);

            this->viewsByShapeId_[id] = view;
            sizer->Add(view);
        }

        auto topSizer = wxpex::BorderSizer(std::move(sizer), 5);
        this->SetSizerAndFit(topSizer.release());
    }

    void OnReorder_()
    {
        for (auto it: this->viewsByShapeId_)
        {
            this->sizer_->Detach(it.second);
        }

        for (auto &it: this->shapesControl_)
        {
            auto id = it.id.Get();
            this->sizer_->Add(this->viewsByShapeId_.at(id));
        }

        this->Layout();
    }

private:
    ShapesControl shapesControl_;
    std::map<size_t, wxWindow *> viewsByShapeId_;
    wxBoxSizer *sizer_;
    pex::Endpoint<ShapesInterface, pex::control::Signal<>> onReorder_;
};


template<typename View, typename ShapesControl>
class DemoInterface: public wxPanel
{
public:
    static constexpr auto observerName = "DemoInterface";

    DemoInterface(
        wxWindow *parent,
        ShapesControl control)
        :
        wxPanel(parent, wxID_ANY),
        control_(control),
        sizer_(),
        shapesInterface_(this->MakeInterface()),

        countWillChangeEndpoint_(
            this,
            control.shapes.countWillChange,
            &DemoInterface::OnCountWillChange_),

        countEndpoint_(
            this,
            control.shapes.count,
            &DemoInterface::OnCount_),

        destroyInterface_(std::bind(&DemoInterface::DestroyInterface_, this)),
        createInterface_(std::bind(&DemoInterface::CreateInterface_, this))
    {
        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(this->shapesInterface_);

        // Lifetime of sizer will be managed by topSizer, which is managed by
        // the wxPanel
        this->sizer_ = sizer.get();

        // BorderSizer expects a unique_ptr to sizer, and calls release.
        auto topSizer = wxpex::BorderSizer(std::move(sizer), 5);
        this->SetSizerAndFit(topSizer.release());
    }

    wxWindow * MakeInterface()
    {
        return new ShapesInterface<View, decltype(this->control_.shapes)>(
            this,
            this->control_.shapes,
            this->control_.reorder);
    }

private:
    void DestroyInterface_()
    {
        if (this->shapesInterface_ && !this->shapesInterface_->IsBeingDeleted())
        {
            this->sizer_->Detach(this->shapesInterface_);

            if (!this->shapesInterface_->Destroy())
            {
                std::cerr << "Unable to destroy previous polygon interface"
                    << std::endl;
            }

            this->shapesInterface_ = nullptr;
        }
    }

    void CreateInterface_()
    {
        this->shapesInterface_ = this->MakeInterface();
        this->sizer_->Add(this->shapesInterface_);
        this->Layout();
    }

    void OnCountWillChange_()
    {
        this->destroyInterface_();
    }

    void OnCount_(size_t)
    {
        this->createInterface_();
    }

private:
    ShapesControl control_;
    wxSizer * sizer_;
    wxWindow * shapesInterface_;

    using CountWillChangeEndpoint =
        pex::Endpoint<DemoInterface, decltype(control_.shapes.countWillChange)>;

    using CountEndpoint =
        pex::Endpoint<DemoInterface, decltype(control_.shapes.count)>;

    CountWillChangeEndpoint countWillChangeEndpoint_;
    CountEndpoint countEndpoint_;
    wxpex::CallAfter destroyInterface_;
    wxpex::CallAfter createInterface_;
};


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::shapes, "shapes"),
        fields::Field(&T::reorder, "reorder"));
};


template<typename ShapeListMaker>
struct DemoTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<ShapeListMaker> shapes;
        T<pex::MakeSignal> reorder;
    };
};


template<typename ShapeListMaker>
using DemoGroup = pex::Group
    <
        DemoFields,
        DemoTemplate<ShapeListMaker>::template Template
    >;
