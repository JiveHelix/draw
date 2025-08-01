#pragma once


#include <draw/ellipse_shape.h>
#include <draw/polygon_shape.h>
#include <draw/regular_polygon_shape.h>
#include <draw/quad_shape.h>
#include <draw/cross_shape.h>
#include <draw/shape_editor.h>
#include <draw/shape_list.h>


namespace draw
{


enum class SelectedShape
{
    ellipse,
    polygon,
    regularPolygon,
    quad,
    cross,
    none
};


struct SelectedShapeConverter
{
    static std::string ToString(SelectedShape selectedShape);
};


struct SelectedShapeChoices
{
    using Type = SelectedShape;
    static std::vector<SelectedShape> GetChoices();

    using Converter = SelectedShapeConverter;

    static std::string ToString(SelectedShape selectedShape)
    {
        return Converter::ToString(selectedShape);
    }
};




std::ostream & operator<<(std::ostream &, SelectedShape);


enum class Action
{
    moveToTop,
    moveUp,
    moveDown,
    moveToBottom,
    remove,
    cancel
};


struct ActionConverter
{
    static std::string ToString(Action);
};


struct Actions
{
    using Type = Action;
    static std::vector<Action> GetChoices();

    using Converter = ActionConverter;

    static std::string ToString(Action action)
    {
        return Converter::ToString(action);
    }

    static bool ProcessAction(Action action, ListedShape &shapeControl)
    {
        switch (action)
        {
            case (Action::moveToTop):
                pex::GetOrder(shapeControl).moveToTop.Trigger();
                break;

            case (Action::moveUp):
                pex::GetOrder(shapeControl).moveUp.Trigger();
                break;

            case (Action::moveDown):
                pex::GetOrder(shapeControl).moveDown.Trigger();
                break;

            case (Action::moveToBottom):
                pex::GetOrder(shapeControl).moveToBottom.Trigger();
                break;

            case (Action::remove):
                return true;
                break;

            case (Action::cancel):
                break;

            default:
                throw std::logic_error("Not an action");
        }

        return false;
    }
};


std::ostream & operator<<(std::ostream &, Action);


struct CrossActions
{
    using Type = Action;
    static std::vector<Action> GetChoices();

    using Converter = ActionConverter;

    static std::string ToString(Action action)
    {
        return Converter::ToString(action);
    }

    static bool ProcessAction(Action action, ListedShape &)
    {
        switch (action)
        {
            case (Action::remove):
                return true;
                break;

            case (Action::cancel):
                break;

            default:
                throw std::logic_error("Not an action");
        }

        return false;
    }
};


template<typename ShapeChoices, typename Actions>
class SelectedMenu
{
public:
    using SelectionType = typename ShapeChoices::Type;
    using ActionType = typename Actions::Type;

    SelectedMenu()
        :
        shapesById_{},
        actionsById_{},
        creationMenu_{},
        actionMenu_{},
        selectedId_{}
    {
        auto choices = ShapeChoices::GetChoices();

        for (auto &choice: choices)
        {
            auto menuId = wxWindow::NewControlId();
            this->shapesById_[menuId] = choice;

            this->creationMenu_.AppendCheckItem(
                menuId,
                ShapeChoices::ToString(choice));
        }

        auto &menuItems = this->creationMenu_.GetMenuItems();
        auto first = menuItems.front();

        first->Check(true);
        this->selectedId_ = first->GetId();

        auto actions = Actions::GetChoices();

        for (auto &action: actions)
        {
            auto menuId = wxWindow::NewControlId();
            this->actionsById_[menuId] = action;

            this->actionMenu_.Append(
                menuId,
                Actions::ToString(action));
        }
    }

    SelectionType GetSelectedShape() const
    {
        return this->shapesById_.at(this->selectedId_);
    }

    wxMenu & GetMenu(bool objectSelected)
    {
        if (objectSelected)
        {
            return this->actionMenu_;
        }

        return this->creationMenu_;
    }

    using ShapeControl = pex::poly::Control<ShapeSupers>;

    // Returns true if the shape should be deleted
    bool ProcessAction(ActionType action, ListedShape &shapeControl)
    {
        return Actions::ProcessAction(action, shapeControl);
    }

    bool IsActionId(wxWindowID windowId) const
    {
        return this->actionsById_.count(windowId);
    }

    std::optional<ActionType> GetAction(wxWindowID windowId) const
    {
        if (this->IsActionId(windowId))
        {
            return this->actionsById_.at(windowId);
        }

        return {};
    }

    void ReportId(wxWindowID windowId)
    {
        if (this->IsActionId(windowId) || windowId == wxID_NONE)
        {
            return;
        }

        auto menuItem = this->creationMenu_.FindItem(this->selectedId_);

        if (menuItem == NULL)
        {
            throw std::logic_error("Selected menu item not found");
        }

        menuItem->Check(false);

        menuItem = this->creationMenu_.FindItem(windowId);

        if (menuItem == NULL)
        {
            throw std::logic_error("Selected menu item not found");
        }

        this->selectedId_ = windowId;
        menuItem->Check(true);
    }

private:
    std::map<wxWindowID, SelectionType> shapesById_;
    std::map<wxWindowID, ActionType> actionsById_;
    wxMenu creationMenu_;
    wxMenu actionMenu_;
    wxWindowID selectedId_;
};


using SelectedShapesMenu = SelectedMenu<SelectedShapeChoices, Actions>;

static_assert(IsRightClickMenu<SelectedShapesMenu>);


struct SelectedCrossChoices
{
    using Type = SelectedShape;

    static std::vector<SelectedShape> GetChoices()
    {
        return {SelectedShape::cross};
    }

    static std::string ToString(SelectedShape selectedShape)
    {
        return SelectedShapeConverter::ToString(selectedShape);
    }
};


using SelectedCrossMenu = SelectedMenu<SelectedCrossChoices, CrossActions>;



template<typename RightClickMenu_>
class DragCreateSelected: public Drag
{
public:
    using RightClickMenu = RightClickMenu_;

    DragCreateSelected(
        const RightClickMenu &rightClickMenu,
        const tau::Point2d<double> &start,
        const ShapesControl &shapeList)
        :
        Drag(start, start),
        selectedShape_(rightClickMenu.GetSelectedShape()),
        shapeList_(shapeList),
        position_(start)
    {
        PEX_NAME("DragCreateSelected");
        PEX_MEMBER(shapeList_);
    }

    virtual ~DragCreateSelected()
    {
        try
        {
            this->CreateShape_();
        }
        catch (std::exception &e)
        {
            std::cerr << "Error creating shape: " << e.what() << std::endl;
        }
    }

    void ReportLogicalPosition(const tau::Point2d<double> &position) override
    {
        this->position_ = position;
    }

protected:
    std::optional<ShapeValue> Create_(
        const tau::Point2d<double> position)
    {
        switch (this->selectedShape_)
        {
            case (SelectedShape::ellipse):
                return CreateEllipse{}(*this, position);

            case (SelectedShape::polygon):
                return CreatePolygon{}(*this, position);

            case (SelectedShape::regularPolygon):
                return CreateRegularPolygon{}(*this, position);

            case (SelectedShape::quad):
                return CreateQuad{}(*this, position);

            case (SelectedShape::cross):
                return CreateCross{}(*this, position);

            case (SelectedShape::none):
                return std::nullopt;

            default:
                std::cerr << "Unsupported shape" << std::endl;

                return std::nullopt;
        }
    }

    void CreateShape_()
    {
        auto shape = this->Create_(this->position_);

        if (!shape)
        {
            return;
        }

        // this->shapeList_.Prepend(*shape);
        this->shapeList_.Append(*shape);
    }

protected:
    SelectedShape selectedShape_;
    ShapesControl shapeList_;
    tau::Point2d<double> position_;
};


using DragCreateSelectedShape = DragCreateSelected<SelectedShapesMenu>;


static_assert(HasRightClickMenu<DragCreateSelectedShape>);


using ShapeCreatorBrain = draw::ShapeEditor<DragCreateSelectedShape>;


using DragCreateSelectedCross = DragCreateSelected<SelectedCrossMenu>;


static_assert(HasRightClickMenu<DragCreateSelectedCross>);


using CrossCreatorBrain = draw::ShapeEditor<DragCreateSelectedCross>;


} // end namespace draw


extern template class draw::SelectedMenu
<
    draw::SelectedShapeChoices,
    draw::Actions
>;

extern template class draw::DragCreateSelected<draw::SelectedShapesMenu>;

extern template class draw::SelectedMenu
<
    draw::SelectedCrossChoices,
    draw::CrossActions
>;

extern template class draw::DragCreateSelected<draw::SelectedCrossMenu>;
