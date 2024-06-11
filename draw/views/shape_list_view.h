#pragma once


#include <mutex>
#include <pex/signal.h>
#include <pex/endpoint.h>
#include <wxpex/border_sizer.h>
#include <wxpex/async.h>
#include "draw/depth_order.h"
#include "draw/shape_list.h"
#include "draw/views/shape_view.h"


namespace draw
{


template<typename T, typename = void>
struct HasGetVirtual_: std::false_type {};

template<typename T>
struct HasGetVirtual_
<
    T,
    std::enable_if_t
    <
        std::is_pointer_v
        <
            decltype(std::declval<T>().GetVirtual())
        >
    >
>: std::true_type {};


template<typename T>
inline constexpr bool HasGetVirtual = HasGetVirtual_<T>::value;


template<typename Control>
class ShapeAdaptor
{
public:
    ShapeAdaptor(Control &control)
        :
        control_(&control)
    {

    }

    ssize_t GetId() const
    {
        if constexpr (HasGetVirtual<Control>)
        {
            return this->control_->GetVirtual()->GetId();
        }
        else
        {
            return this-control_->GetId();
        }
    }

    std::string GetName() const
    {
        if constexpr (HasGetVirtual<Control>)
        {
            return this->control_->GetVirtual()->GetName();
        }
        else
        {
            return this-control_->GetName();
        }
    }

    DepthOrderControl & GetDepthOrder()
    {
        if constexpr (HasGetVirtual<Control>)
        {
            return this->control_->GetVirtual()->GetDepthOrder();
        }
        else
        {
            return this-control_->GetDepthOrder();
        }
    }

    NodeSettingsControl & GetNode()
    {
        if constexpr (HasGetVirtual<Control>)
        {
            return this->control_->GetVirtual()->GetNode();
        }
        else
        {
            return this-control_->GetNode();
        }
    }

    wxWindow * CreateShapeView(wxWindow *parent) const
    {
        if constexpr (HasGetVirtual<Control>)
        {
            return this->control_->GetVirtual()->CreateShapeView(parent);
        }
        else
        {
            return this-control_->CreateShapeView(parent);
        }
    }

    wxWindow * CreateLookView(
        wxWindow *parent,
        LookDisplayControl displayControl) const
    {
        if constexpr (HasGetVirtual<Control>)
        {
            return this->control_->GetVirtual()
                ->CreateLookView(parent, displayControl);
        }
        else
        {
            return this-control_->CreateLookView(parent, displayControl);
        }
    }

private:
    Control *control_;
};


template<typename ListMaker>
class ShapeListView: public wxPanel
{
public:
    static constexpr auto observerName = "ShapeListView";

    ShapeListView(
        wxWindow *parent,
        ShapeListControl<ListMaker> control)
        :
        wxPanel(parent, wxID_ANY),
        mutex_(),
        control_(control),
        viewsByShapeId_(),
        sizer_(),

        countWillChangeEndpoint_(
            this,
            control.shapes.countWillChange,
            &ShapeListView::OnCountWillChange_),

        countEndpoint_(
            this,
            control.shapes.count,
            &ShapeListView::OnCount_),

        onReorder_(
            this,
            control.reorder,
            &ShapeListView::OnReorder_),

        destroyInterface_(std::bind(&ShapeListView::DestroyInterface_, this)),
        createInterface_(std::bind(&ShapeListView::CreateInterface_, this))
    {
        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        this->sizer_ = sizer.get();
        this->CreateViews_();
        this->SetSizerAndFit(sizer.release());
    }

    void OnReorder_()
    {
        std::lock_guard lock(this->mutex_);

        for (auto it: this->viewsByShapeId_)
        {
            this->sizer_->Detach(it.second);
        }

        size_t count = this->control_.shapes.count.Get();

        for (size_t i = count; i > 0; --i)
        {
            auto &it = this->control_.shapes[i - 1];
            auto id = ShapeAdaptor(it).GetId();
            this->sizer_->Add(this->viewsByShapeId_.at(id));
        }

        this->Layout();
    }

    void DestroyInterface_()
    {
        std::lock_guard lock(this->mutex_);

        if (this->IsBeingDeleted())
        {
            return;
        }

        for (auto it: this->viewsByShapeId_)
        {
            this->sizer_->Detach(it.second);
        }

        this->viewsByShapeId_.clear();

        this->DestroyChildren();
    }

    void CreateViews_()
    {
        std::lock_guard lock(this->mutex_);

        size_t count = this->control_.shapes.count.Get();
        assert(count == this->control_.shapesDisplay.count.Get());

        for (size_t i = count; i > 0; --i)
        {
            size_t index = i - 1;

            auto &it = this->control_.shapes[index];
            auto adaptor = ShapeAdaptor(it);
            auto id = adaptor.GetId();

            auto view =
                new ShapeView(
                    this,
                    adaptor,
                    this->control_.shapesDisplay[index]);

            this->viewsByShapeId_[id] = view;
            this->sizer_->Add(view);
        }
    }

    void CreateInterface_()
    {
        this->CreateViews_();
        // this->GetParent()->Layout();
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
    using Control = ShapeListControl<ListMaker>;
    using ShapesControl = decltype(Control::shapes);

    std::mutex mutex_;
    Control control_;
    std::map<ssize_t, wxWindow *> viewsByShapeId_;
    wxBoxSizer *sizer_;

    using CountWillChangeEndpoint =
        pex::Endpoint<ShapeListView, typename ShapesControl::CountWillChange>;

    using CountEndpoint =
        pex::Endpoint<ShapeListView, typename ShapesControl::Count>;

    using ReorderEndpoint =
       pex::Endpoint<ShapeListView, decltype(Control::reorder)>;

    CountWillChangeEndpoint countWillChangeEndpoint_;
    CountEndpoint countEndpoint_;

    ReorderEndpoint onReorder_;

    wxpex::CallAfter destroyInterface_;
    wxpex::CallAfter createInterface_;
};


} // end namespace draw
