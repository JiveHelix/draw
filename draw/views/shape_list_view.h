#pragma once


#include <pex/ordered_list.h>
#include <wxpex/list_view.h>
#include "draw/shape_list.h"
#include "draw/views/shape_view.h"


namespace draw
{


template<typename Control>
class ShapeAdaptor
{
public:
    static_assert(pex::HasOrder<Control>);

    ShapeAdaptor(Control &control)
        :
        control_(&control)
    {

    }

    ssize_t GetId() const
    {
        if constexpr (pex::HasGetVirtual<Control>)
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
        if constexpr (pex::HasGetVirtual<Control>)
        {
            return this->control_->GetVirtual()->GetName();
        }
        else
        {
            return this-control_->GetName();
        }
    }

    pex::OrderControl & GetOrder()
    {
        if constexpr (pex::HasVirtualGetOrder<Control>)
        {
            return this->control_->GetVirtual()->GetOrder();
        }
        else
        {
            return this->control_->order;
        }
    }

    NodeSettingsControl & GetNode()
    {
        if constexpr (pex::HasGetVirtual<Control>)
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
        if constexpr (pex::HasGetVirtual<Control>)
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
        if constexpr (pex::HasGetVirtual<Control>)
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


class ShapeListView: public wxpex::ListView<OrderedShapesControl>
{
public:
    static constexpr auto observerName = "ShapeListView";

    using Base = wxpex::ListView<OrderedShapesControl>;
    using ListItem = typename Base::ListItem;
    using Control = ShapeListControl;

    ShapeListView(
        wxWindow *parent,
        Control control)
        :
        Base(
            parent,
            control.shapes,
            control.shapesDisplay.reorder),
        control_(control)
    {
        this->Initialize_();
    }

    wxWindow * CreateView_(ListItem &listItem, size_t index) override
    {
        return new ShapeView(
            this,
            ShapeAdaptor(listItem),
            this->control_.shapesDisplay.at(index));
    }

private:
    Control control_;
};


} // end namespace draw
