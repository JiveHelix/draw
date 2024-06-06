#pragma once


#include <pex/signal.h>
#include <pex/endpoint.h>
#include <wxpex/border_sizer.h>
#include <wxpex/async.h>
#include <draw/views/shape_view.h>


template<typename ShapesControl>
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
            auto id = it.GetId();
            auto view = new draw::ShapeView(this, it);
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
            auto id = it.GetId();
            this->sizer_->Add(this->viewsByShapeId_.at(id));
        }

        this->Layout();
    }

private:
    ShapesControl shapesControl_;
    std::map<ssize_t, wxWindow *> viewsByShapeId_;
    wxBoxSizer *sizer_;
    pex::Endpoint<ShapesInterface, pex::control::Signal<>> onReorder_;
};


template<typename Control>
class VirtualControl
{
public:
    VirtualControl(Control control)
        :
        control_(control)
    {

    }

    ssize_t GetId() const
    {
        return this->control_.GetVirtual()->GetId();
    }

    std::string GetName() const
    {
        return this->control_.GetVirtual()->GetName();
    }

    draw::NodeSettingsControl & GetNode()
    {
        return this->control_.GetVirtual()->GetNode();
    }

    wxWindow * CreateShapeView(wxWindow *parent) const
    {
        return this->control_.GetVirtual()->CreateShapeView(parent);
    }

    wxWindow * CreateLookView(wxWindow *parent) const
    {
        return this->control_.GetVirtual()->CreateLookView(parent);
    }

private:
    Control control_;
};


template<typename Control>
using VirtualShapesVector =
    std::vector<VirtualControl<typename Control::ItemControl>>;


template<typename Control>
struct VirtualShapes: public VirtualShapesVector<Control>
{
    using CountWillChange = typename Control::CountWillChange;
    using Count = typename Control::Count;

    using Base = VirtualShapesVector<Control>;

    VirtualShapes(Control control)
        :
        Base(),
        control_(control),
        countWillChangeEndpoint_(
            this,
            this->control_.countWillChange,
            &VirtualShapes::OnCountWillChange_),
        countEndpoint_(
            this,
            this->control_.count,
            &VirtualShapes::OnCount_),
        countWillChange(this->control_.countWillChange),
        count(this->control_.count)
    {
        this->OnCount_(this->count.Get());
    }

    VirtualShapes(const VirtualShapes &other)
        :
        Base(other),
        control_(other.control_),
        countWillChangeEndpoint_(this, other.countWillChangeEndpoint_),
        countEndpoint_(this, other.countEndpoint_),
        countWillChange(other.countWillChange),
        count(other.count)
    {

    }

    VirtualShapes & operator=(const VirtualShapes &other)
    {
        this->Base::operator=(other);
        this->control_ = other.control_;

        this->countWillChangeEndpoint_ =
            CountWillChangeEndpoint(this, other.countWillChangeEndpoint_);

        this->countEndpoint_ =
            CountEndpoint(this, other.countEndpoint_);

        this->countWillChange = other.countWillChange;
        this->count = other.count;

        return *this;
    }

private:
    void OnCountWillChange_()
    {
        this->clear();
    }

    void OnCount_(size_t count_)
    {
        assert(this->empty());

        this->reserve(count_);

        for (auto &it: this->control_)
        {
            this->push_back(VirtualControl(it));
        }

        assert(count_ == this->size());
    }

    using CountWillChangeEndpoint =
        pex::Endpoint<VirtualShapes, CountWillChange>;

    using CountEndpoint =
        pex::Endpoint<VirtualShapes, Count>;

    Control control_;
    CountWillChangeEndpoint countWillChangeEndpoint_;
    CountEndpoint countEndpoint_;

public:
    CountWillChange countWillChange;
    Count count;
};


template<typename ShapesControl>
class DemoInterface: public wxPanel
{
public:
    static constexpr auto observerName = "DemoInterface";

    DemoInterface(
        wxWindow *parent,
        ShapesControl control,
        pex::control::Signal<> reorder)
        :
        wxPanel(parent, wxID_ANY),
        control_(control),
        reorder_(reorder),
        sizer_(),
        shapesInterface_(this->MakeInterface()),

        countWillChangeEndpoint_(
            this,
            control.countWillChange,
            &DemoInterface::OnCountWillChange_),

        countEndpoint_(
            this,
            control.count,
            &DemoInterface::OnCount_),

        destroyInterface_(std::bind(&DemoInterface::DestroyInterface_, this)),
        createInterface_(std::bind(&DemoInterface::CreateInterface_, this))
    {
        assert(control.count.HasModel());
        assert(control.countWillChange.HasModel());

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
        return new ShapesInterface<ShapesControl>(
            this,
            this->control_,
            this->reorder_);
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
    pex::control::Signal<> reorder_;
    wxSizer * sizer_;
    wxWindow * shapesInterface_;

    using CountWillChangeEndpoint =
        pex::Endpoint<DemoInterface, typename ShapesControl::CountWillChange>;

    using CountEndpoint =
        pex::Endpoint<DemoInterface, typename ShapesControl::Count>;

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


template<typename Control>
wxWindow * CreateDemoInterface(
    wxWindow *parent,
    Control control)
{
    return new DemoInterface(
        parent,
        VirtualShapes(control.shapes),
        control.reorder);
}
