#pragma once


#include <pex/endpoint.h>
#include <pex/list_observer.h>
#include "draw/selection_brain.h"
#include "draw/oddeven.h"
#include "draw/drag.h"
#include "draw/views/canvas_settings.h"
#include "draw/shapes.h"
#include "draw/shape_list.h"
#include "draw/polygon_lines.h"


namespace draw
{


std::optional<PointsIterator> FindPoint(
    const tau::Point2d<int> &click,
    const PointsDouble &points);


double DragAngleDifference(double first, double second);


// TODO: Document what Modulo does.
template<typename T>
T Modulo(T a, T b)
{
    if constexpr (std::is_floating_point_v<T>)
    {
        return std::fmod(std::fmod(a, b) + b, b);
    }
    else
    {
        return (a % b + b) % b;
    }
}



template<typename DerivedShape>
class DragShape: public Drag
{
public:
    DragShape(
        size_t index,
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        std::shared_ptr<ShapeControl> control,
        const DerivedShape &startingShape)
        :
        Drag(index, start, offset),
        control_(control),
        startingShape_(startingShape)
    {
        if (this->index_ > 0)
        {
            if (index >= startingShape.GetPoints().size())
            {
                throw std::out_of_range(
                    "Selected indexed exceeds point count.");
            }
        }
    }

    DragShape(
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        std::shared_ptr<ShapeControl> control,
        const DerivedShape &startingShape)
        :
        Drag(start, offset),
        control_(control),
        startingShape_(startingShape)
    {

    }

    void ReportLogicalPosition(const tau::Point2d<int> &position) override
    {
        this->startingShape_.shape.center = this->GetPosition(position);
        this->control_->SetValue(this->startingShape_);
    }

protected:
    std::shared_ptr<ShapeControl> control_;
    DerivedShape startingShape_;
};


template<typename DerivedShape>
class DragEditShape: public Drag
{
public:
    DragEditShape(
        size_t index,
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        std::shared_ptr<ShapeControl> control,
        const DerivedShape &startingShape)
        :
        Drag(index, start, offset),
        control_(control),
        startingShape_(startingShape)
    {
        if (this->index_ > 0)
        {
            if (index >= startingShape.GetPoints().size())
            {
                throw std::out_of_range(
                    "Selected indexed exceeds point count.");
            }
        }
    }

    DragEditShape(
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        std::shared_ptr<ShapeControl> control,
        const DerivedShape &startingShape)
        :
        Drag(start, offset),
        control_(control),
        startingShape_(startingShape)
    {

    }

    void ReportLogicalPosition(const tau::Point2d<int> &position) override
    {
        this->control_->SetValue(*this->MakeShape_(position));
    }

protected:
    virtual std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<int> &end) const = 0;

protected:
    std::shared_ptr<ShapeControl> control_;
    DerivedShape startingShape_;
};


template<typename CreateShape>
class DragCreateShape: public Drag
{
public:
    DragCreateShape(
        const tau::Point2d<int> &start,
        const ShapesControl &shapeList)
        :
        Drag(start, start),
        shapeList_(shapeList),
        position_(start)
    {

    }

    virtual ~DragCreateShape()
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
    void CreateShape_()
    {
        auto shape = CreateShape{}(*this, this->position_);

        if (!shape)
        {
            return;
        }

        auto newIndex = this->shapeList_.Append(*shape);

        if (newIndex)
        {
            this->shapeList_.MoveToTop(*newIndex);
            this->Select_(*newIndex);
        }
    }

    void Select_(size_t unordered)
    {
        auto wasSelected = this->shapeList_.selected.Get();

        if (wasSelected)
        {
            ::draw::GetNode(this->shapeList_, *wasSelected)
                .isSelected.Set(false);
        }

        this->shapeList_.selected.Set(unordered);
        ::draw::GetNode(this->shapeList_, unordered).isSelected.Set(true);
    }

protected:
    ShapesControl shapeList_;
    tau::Point2d<int> position_;
};


// Drag in empty space to create a new shape that replaces all shapes in the
// list.
template<typename CreateShape>
class DragReplaceShape: public Drag
{
public:
    DragReplaceShape(
        const tau::Point2d<int> &start,
        const ShapesControl &shapeList)
        :
        Drag(start, start),
        shapeList_(shapeList),
        position_(start)
    {

    }

    virtual ~DragReplaceShape()
    {
        try
        {
            this->ReplaceShape_();
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
    void ReplaceShape_()
    {
        auto shape = CreateShape{}(*this, this->position_);

        if (!shape)
        {
            return;
        }

        this->shapeList_.count.Set(0);
        this->shapeList_.Append(*shape);
    }

protected:
    ShapesControl shapeList_;
    tau::Point2d<int> position_;
};



template<typename DerivedShape>
class DragEditPoint: public DragEditShape<DerivedShape>
{
public:
    using Base = DragEditShape<DerivedShape>;

    DragEditPoint(
        size_t index,
        const tau::Point2d<int> &start,
        std::shared_ptr<ShapeControl> control,
        const DerivedShape &startingShape,
        const PointsDouble &points)
        :
        Base(index, start, points.at(index), control, startingShape),
        points_(points)
    {

    }

protected:
    PointsDouble points_;
};


struct IgnoreMouse {};
struct DoNotCreate {};

/*
 * DragShape is required. RotatePoint, DragPoint, and DragLine can be bypassed
 * with IgnoreMouse.
 */
template
<
    typename RotatePoint,
    typename DragPoint,
    typename DragLine,
    typename DragShape,
    typename DerivedShape
>
std::unique_ptr<Drag> ProcessMouseDown(
    std::shared_ptr<ShapeControl> shapeControl,
    DerivedShape &shape,
    const tau::Point2d<int> &click,
    const wxpex::Modifier &modifier,
    CursorControl cursor)
{
    auto points = shape.GetPoints();
    auto foundPoint = FindPoint(click, points);

    if (foundPoint)
    {
        if (modifier.IsAlt() && shape.HandlesAltClick())
        {
            if (shape.ProcessAltClick(*shapeControl, *foundPoint, points))
            {
                return {};
            }
        }

        auto pointIndex =
            static_cast<size_t>(
                std::distance(points.cbegin(), *foundPoint));

        if (modifier.IsControl())
        {
            if constexpr (!std::is_same_v<IgnoreMouse, RotatePoint>)
            {
                return std::make_unique<RotatePoint>(
                    pointIndex,
                    click,
                    shapeControl,
                    shape,
                    points);
            }
        }
        else
        {
            if constexpr (!std::is_same_v<IgnoreMouse, DragPoint>)
            {
                return std::make_unique<DragPoint>(
                    pointIndex,
                    click,
                    shapeControl,
                    shape,
                    points);
            }
        }
    }

    if constexpr (!std::is_same_v<DragLine, IgnoreMouse>)
    {
        auto lines = PolygonLines(points);
        auto lineIndex = lines.Find(click.template Cast<double>(), 10.0);

        if (lineIndex)
        {
            return std::make_unique<DragLine>(
                *lineIndex,
                click,
                lines,
                shapeControl,
                shape);
        }
    }

    if constexpr (std::is_same_v<DragShape, IgnoreMouse>)
    {
        return {};
    }
    else
    {
        if (!shape.HandlesDrag())
        {
            return {};
        }

        cursor.Set(wxpex::Cursor::closedHand);

        // DragShape
        return std::make_unique<DragShape>(
            click,
            shape.shape.center,
            shapeControl,
            shape);
    }
}


using ListItem = typename ShapesControl::ListItem;


struct FoundShape
{
    size_t listIndex;
    size_t unordered;
    ListItem shape;
};


template<typename Create>
class ShapeBrain: public SelectionBrain<ShapesControl>
{
public:
    static constexpr auto observerName = "ShapeBrain";

    using ListItem = typename ShapesControl::ListItem;

    ShapeBrain(
        const std::vector<ShapesControl> &shapeLists,
        CanvasControl canvasControl)
        :
        SelectionBrain(shapeLists),
        isEnabled_(true),

        canvasControl_(canvasControl),

        mouseDownEndpoint_(
            this,
            canvasControl.mouseDown,
            &ShapeBrain::OnMouseDown_),

        logicalPositionEndpoint_(
            this,
            canvasControl.logicalPosition,
            &ShapeBrain::OnLogicalPosition_),

        modifierEndpoint_(
            this,
            canvasControl.modifier,
            &ShapeBrain::OnModifier_),


        drag_()
    {
    }

    ShapeBrain(
        ShapesControl shapeList,
        CanvasControl canvasControl)
        :
        ShapeBrain(std::vector<ShapesControl>({shapeList}), canvasControl)
    {

    }

    ~ShapeBrain()
    {

    }

    void SetIsEnabled(bool isEnabled)
    {
        // Do not leave drag state set when toggling the enable switch.
        this->drag_.reset();
        this->isEnabled_ = isEnabled;
        this->DeselectAll();
    }

    std::optional<FoundShape> GetShapeSelection() const
    {
        for (size_t i = 0; i < this->lists_.size(); ++i)
        {
            const auto &shapeList = this->lists_[i];

            if (shapeList.selected.Get())
            {
                auto unordered = *shapeList.selected.Get();

                return FoundShape
                    {
                        i,
                        unordered,
                        GetUnordered(shapeList, unordered)
                    };
            }
        }

        return {};
    }

    void OnModifier_(const wxpex::Modifier &)
    {
        if (!this->isEnabled_)
        {
            return;
        }

        this->UpdateCursor_();
    }


    void OnLogicalPosition_(const tau::Point2d<int> &position)
    {
        if (!this->isEnabled_)
        {
            return;
        }

        this->UpdateCursor_();

        if (this->drag_)
        {
            this->drag_->ReportLogicalPosition(position);
        }
    }

    bool IsDragging_() const
    {
        return !!this->drag_;
    }

protected:
    void OnMouseDown_(bool isDown)
    {
        if (!this->isEnabled_)
        {
            return;
        }

        if (!isDown)
        {
            // Mouse has been released.
            this->drag_.reset();
            this->UpdateCursor_();

            return;
        }

        auto click = this->canvasControl_.logicalPosition.Get();
        auto wasSelected = this->GetShapeSelection();

        auto found = this->FindClicked_(click.template Cast<double>());

        if (!found)
        {
            if (wasSelected)
            {
                auto selected = wasSelected->shape;
                auto shape = selected.Get();

                if (shape.GetValueBase()->HandlesControlClick())
                {
                    if (this->canvasControl_.modifier.Get().IsControl())
                    {
                        // There was a selected shape.
                        // Keep it selected and handle a control modifier.
                        // Handles
                        shape.GetValueBase()->ProcessControlClick(
                            *selected.GetVirtual(),
                            click);

                        return;
                    }
                }
            }

            // The user clicked away from a shape without holding control.

            if (wasSelected)
            {
                this->Deselect(*wasSelected);
            }

            if constexpr (!std::is_same_v<DoNotCreate, Create>)
            {
                // There is no selection to edit.
                // Begin creating a new shape.
                // DragCreate
                this->drag_ =
                    std::make_unique<Create>(click, this->lists_.at(0));
            }

            return;
        }

        // The user clicked on a shape.
        this->ToggleSelect(found->unordered, found->listIndex);

        this->drag_ = found->shape.Get().GetValueBase()->ProcessMouseDown(
            found->shape.GetVirtual()->Copy(),
            click,
            this->canvasControl_.modifier.Get(),
            this->canvasControl_.cursor);
    }

    void UpdateCursor_()
    {
        if (this->IsDragging_())
        {
            return;
        }

        auto modifier = this->canvasControl_.modifier.Get();
        auto click = this->canvasControl_.logicalPosition.Get();
        auto found = this->FindClicked_(click.template Cast<double>());

        if (!found)
        {
            // The cursor is not contained by any of the shapes.
            auto shapeSelection = this->GetShapeSelection();

            if (modifier.IsControl() && shapeSelection)
            {
                auto shape =
                    GetValueBase(
                        this->lists_.at(shapeSelection->listIndex),
                        shapeSelection->unordered);

                if (shape->HandlesControlClick())
                {
                    this->canvasControl_.cursor.Set(wxpex::Cursor::pencil);

                    return;
                }
            }

            if (this->canvasControl_.cursor.Get() != wxpex::Cursor::arrow)
            {
                this->canvasControl_.cursor.Set(wxpex::Cursor::arrow);
            }

            return;
        }

        // There is a shape under the cursor.
        auto value = found->shape.Get();
        auto shape = value.GetValueBase();
        auto points = shape->GetPoints();
        auto foundPoint = FindPoint(click, points);

        if (foundPoint)
        {
            // Hovering over a point, change cursor
            if (modifier.IsControl() && shape->HandlesRotate())
            {
                // TODO: Create a rotate cursor
                this->canvasControl_.cursor.Set(wxpex::Cursor::pointRight);
            }
            else if (modifier.IsAlt() && shape->HandlesAltClick())
            {
                this->canvasControl_.cursor.Set(wxpex::Cursor::bullseye);
            }
            else if (shape->HandlesEditPoint())
            {
                this->canvasControl_.cursor.Set(wxpex::Cursor::cross);
            }
            else
            {
                this->canvasControl_.cursor.Set(wxpex::Cursor::openHand);
            }

            return;
        }

        if (shape->HandlesEditLine())
        {
            auto lineIndex =
                PolygonLines(points).Find(click.template Cast<double>(), 10.0);

            if (lineIndex)
            {
                // Hovering over a line, change cursor
                this->canvasControl_.cursor.Set(wxpex::Cursor::sizing);
                return;
            }
        }

        if (shape->HandlesDrag())
        {
            // The mouse is over a shape, but not over a line or a point.
            this->canvasControl_.cursor.Set(wxpex::Cursor::openHand);
        }
    }

    std::optional<FoundShape> FindClicked_(
        const tau::Point2d<int> &position)
    {
        auto listCount = this->lists_.size();

        for (size_t listIndex = 0; listIndex < listCount; ++listIndex)
        {
            auto &shapeList = this->lists_[listIndex];
            auto count = shapeList.count.Get();

            for (size_t index = 0; index < count; ++index)
            {
                auto &shapeControl = shapeList[index];
                auto value = shapeControl.Get();
                auto shape = value.GetValueBase();

                if (shape->Contains(position, 10.0))
                {
                    return FoundShape{
                        listIndex,
                        shapeList.indices.at(index).Get(),
                        shapeControl};
                }
            }
        }

        return {};
    }

private:
    void Deselect(const FoundShape &foundShape)
    {
        auto &shapeList = this->lists_.at(foundShape.listIndex);
        shapeList.selected.Set({});
        ::draw::GetNode(shapeList, foundShape.unordered).isSelected.Set(false);
    }

    void DeselectAll()
    {
        for (auto &shapeList: this->lists_)
        {
            auto selectedIndex = shapeList.selected.Get();

            if (selectedIndex)
            {
                ::draw::GetNode(shapeList, *selectedIndex)
                    .isSelected.Set(false);
            }
        }
    }

    bool isEnabled_;

    CanvasControl canvasControl_;

    pex::Endpoint<ShapeBrain, decltype(CanvasControl::mouseDown)>
        mouseDownEndpoint_;

    pex::Endpoint<ShapeBrain, PointControl> logicalPositionEndpoint_;

    pex::Endpoint<ShapeBrain, decltype(CanvasControl::modifier)>
        modifierEndpoint_;

    std::unique_ptr<Drag> drag_;
};


double AdjustRotation(
    double startingRotation,
    const tau::Point2d<double> &center,
    const tau::Point2d<double> &referencePoint,
    const tau::Point2d<double> &endPoint);


template<typename DerivedShape>
void RotatePoint(
    DerivedShape &derivedShape,
    const tau::Point2d<double> &referencePoint,
    const tau::Point2d<double> &endPoint)
{
    derivedShape.shape.rotation = AdjustRotation(
        derivedShape.shape.rotation,
        derivedShape.shape.center,
        referencePoint,
        endPoint);
}


} // end namespace draw
