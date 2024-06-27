#pragma once


#include "draw/views/list_view.h"
#include "draw/depth_order.h"
#include "draw/shape_list.h"
#include "draw/views/shape_view.h"


namespace draw
{


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
class ShapeListView: public ListView<OrderedShapesControl<ListMaker>>
{
public:
    static constexpr auto observerName = "ShapeListView";

    using Base = ListView<OrderedShapesControl<ListMaker>>;
    using ItemControl = typename Base::ItemControl;
    using Control = ShapeListControl<ListMaker>;

    ShapeListView(
        wxWindow *parent,
        Control control)
        :
        Base(
            parent,
            control.shapes,
            control.reorder),
        control_(control)
    {
        this->Initialize_();
    }

    wxWindow * CreateView_(ItemControl &itemControl, size_t index) override
    {
        return new ShapeView(
            this,
            ShapeAdaptor(itemControl),
            this->control_.shapesDisplay[index]);
    }

private:
    Control control_;
};


} // end namespace draw
