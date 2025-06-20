#pragma once


#include <draw/ellipse_shape.h>
#include <draw/polygon_shape.h>
#include <draw/quad_shape.h>
#include <draw/cross_shape.h>
#include <draw/shape_edit.h>


namespace draw
{


enum class SelectedShape
{
    ellipse,
    polygon,
    quad,
    cross,
    none
};


struct SelectedShapeChoices
{
    using Type = SelectedShape;
    static std::vector<SelectedShape> GetChoices();
};


struct SelectedShapeConverter
{
    static std::string ToString(SelectedShape selectedShape);
};


std::ostream & operator<<(std::ostream &, SelectedShape);


template<typename Choices>
class SelectedMenu
{
public:
    using SelectionType = typename Choices::Type;

    SelectedMenu()
        :
        shapesById_{},
        creationMenu_{},
        selectedId_{},
        deletionId_{wxWindow::NewControlId()},
        deletionMenu_{}
    {
        auto choices = Choices::GetChoices();

        for (auto &choice: choices)
        {
            auto menuId = wxWindow::NewControlId();
            this->shapesById_[menuId] = choice;

            this->creationMenu_.AppendCheckItem(
                menuId,
                SelectedShapeConverter::ToString(choice));
        }

        auto &menuItems = this->creationMenu_.GetMenuItems();
        auto first = menuItems.front();

        first->Check(true);
        this->selectedId_ = first->GetId();

        this->deletionMenu_.Append(
            this->deletionId_,
            "delete");
    }

    SelectionType GetSelectedShape() const
    {
        return this->shapesById_.at(this->selectedId_);
    }

    wxMenu & GetMenu(bool objectSelected)
    {
        if (objectSelected)
        {
            return this->deletionMenu_;
        }

        return this->creationMenu_;
    }

    bool IsDeletion(wxWindowID id) const
    {
        return id == this->deletionId_;
    }

    void ReportId(wxWindowID id)
    {
        if (id == this->deletionId_ || id == wxID_NONE)
        {
            return;
        }

        auto menuItem = this->creationMenu_.FindItem(this->selectedId_);

        if (menuItem == NULL)
        {
            throw std::logic_error("Selected menu item not found");
        }

        menuItem->Check(false);

        menuItem = this->creationMenu_.FindItem(id);

        if (menuItem == NULL)
        {
            throw std::logic_error("Selected menu item not found");
        }

        this->selectedId_ = id;
        menuItem->Check(true);
    }

private:
    std::map<wxWindowID, SelectionType> shapesById_;
    wxMenu creationMenu_;
    wxWindowID selectedId_;
    wxWindowID deletionId_;
    wxMenu deletionMenu_;
};


using SelectedShapesMenu = SelectedMenu<SelectedShapeChoices>;

static_assert(IsRightClickMenu<SelectedShapesMenu>);


struct SelectedCrossChoices
{
    using Type = SelectedShape;
    static std::vector<SelectedShape> GetChoices()
    {
        return {SelectedShape::cross};
    }
};


using SelectedCrossMenu = SelectedMenu<SelectedCrossChoices>;





template<typename RightClickMenu_>
class DragCreateSelected: public Drag
{
public:
    using RightClickMenu = RightClickMenu_;

    DragCreateSelected(
        const RightClickMenu &rightClickMenu,
        const tau::Point2d<int> &start,
        const ShapesControl &shapeList)
        :
        Drag(start, start),
        selectedShape_(rightClickMenu.GetSelectedShape()),
        shapeList_(shapeList),
        position_(start)
    {
        REGISTER_PEX_NAME(this, "DragCreateSelected");
        REGISTER_PEX_NAME_WITH_PARENT(&this->shapeList_, this, "shapeList_");
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

    void ReportLogicalPosition(const tau::Point2d<int> &position) override
    {
        this->position_ = position;
    }

protected:
    std::optional<ShapeValue> Create_(
        const tau::Point2d<int> position)
    {
        switch (this->selectedShape_)
        {
            case (SelectedShape::ellipse):
                return CreateEllipse{}(*this, position);

            case (SelectedShape::polygon):
                return CreatePolygon{}(*this, position);

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

        this->shapeList_.Prepend(*shape);
    }

protected:
    SelectedShape selectedShape_;
    ShapesControl shapeList_;
    tau::Point2d<int> position_;
};


using DragCreateSelectedShape = DragCreateSelected<SelectedShapesMenu>;


static_assert(HasRightClickMenu<DragCreateSelectedShape>);


using ShapeCreatorBrain = draw::ShapeBrain<DragCreateSelectedShape>;


using DragCreateSelectedCross = DragCreateSelected<SelectedCrossMenu>;


static_assert(HasRightClickMenu<DragCreateSelectedCross>);


using CrossCreatorBrain = draw::ShapeBrain<DragCreateSelectedCross>;


} // end namespace draw


extern template class draw::SelectedMenu<draw::SelectedShapeChoices>;
extern template class draw::DragCreateSelected<draw::SelectedShapesMenu>;

extern template class draw::SelectedMenu<draw::SelectedCrossChoices>;
extern template class draw::DragCreateSelected<draw::SelectedCrossMenu>;
