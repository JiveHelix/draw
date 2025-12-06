#pragma once


#include <concepts>
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
    const tau::Point2d<double> &click,
    const PointsDouble &points);


double DragAngleDifference(double first, double second);


static constexpr double pointEpsilon = 0.01;


inline bool IsSamePoint(
    const tau::Point2d<double> &left,
    const tau::Point2d<double> &right)
{
    return std::abs(left.x - right.x) < pointEpsilon
        && std::abs(left.y - right.y) < pointEpsilon;
}


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
    virtual ~DragShape() {}

    DragShape(
        size_t index,
        const tau::Point2d<double> &start,
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

        PEX_NAME("DragShape");
        PEX_MEMBER(startingShape_);
    }

    DragShape(
        const tau::Point2d<double> &start,
        const tau::Point2d<double> &offset,
        std::shared_ptr<ShapeControl> control,
        const DerivedShape &startingShape)
        :
        Drag(start, offset),
        control_(control),
        startingShape_(startingShape)
    {
        PEX_NAME("DragShape");
        PEX_MEMBER(startingShape_);
    }

    void ReportLogicalPosition(const tau::Point2d<double> &position) override
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
    virtual ~DragEditShape() {}

    DragEditShape(
        size_t index,
        const tau::Point2d<double> &start,
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

        PEX_NAME("DragEditShape");
        PEX_MEMBER_ADDRESS(this->control_.get(), "control_");
        PEX_MEMBER(startingShape_);
    }

    DragEditShape(
        const tau::Point2d<double> &start,
        const tau::Point2d<double> &offset,
        std::shared_ptr<ShapeControl> control,
        const DerivedShape &startingShape)
        :
        Drag(start, offset),
        control_(control),
        startingShape_(startingShape)
    {
        PEX_NAME("DragEditShape");
        PEX_MEMBER_ADDRESS(this->control_.get(), "control_");
        PEX_MEMBER(startingShape_);
    }

    void ReportLogicalPosition(const tau::Point2d<double> &position) override
    {
        this->control_->SetValue(*this->MakeShape_(position));
    }

protected:
    virtual std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<double> &end) const = 0;

protected:
    std::shared_ptr<ShapeControl> control_;
    DerivedShape startingShape_;
};


template<typename CreateShape>
class DragCreateShape: public Drag
{
public:
    DragCreateShape(
        const tau::Point2d<double> &start,
        const ShapesControl &shapeList)
        :
        Drag(start, start),
        shapeList_(shapeList),
        position_(start)
    {
        PEX_NAME("DragCreateShape");
        PEX_MEMBER(shapeList_);
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

    void ReportLogicalPosition(const tau::Point2d<double> &position) override
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

        this->shapeList_.Prepend(*shape);
    }

protected:
    ShapesControl shapeList_;
    tau::Point2d<double> position_;
};


// Drag in empty space to create a new shape that replaces all shapes in the
// list.
template<typename CreateShape>
class DragReplaceShape: public Drag
{
public:
    DragReplaceShape(
        const tau::Point2d<double> &start,
        const ShapesControl &shapeList)
        :
        Drag(start, start),
        shapeList_(shapeList),
        position_(start)
    {
        PEX_NAME("DragReplaceShape");
        PEX_MEMBER(shapeList_);
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

    void ReportLogicalPosition(const tau::Point2d<double> &position) override
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
    tau::Point2d<double> position_;
};



template<typename DerivedShape>
class DragEditPoint: public DragEditShape<DerivedShape>
{
public:
    using Base = DragEditShape<DerivedShape>;

    virtual ~DragEditPoint() {}

    DragEditPoint(
        size_t index,
        const tau::Point2d<double> &start,
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
    const tau::Point2d<double> &click,
    const wxpex::Modifier &modifier,
    const CursorControl &cursor)
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
        auto lineIndex = lines.Find(click, 10.0);

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


template<typename T>
concept IsRightClickMenu = requires (T t)
{
    { t.GetMenu(std::declval<bool>()) } -> std::convertible_to<wxMenu &>;
    { t.IsActionId(std::declval<wxWindowID>()) } -> std::convertible_to<bool>;
    { t.ReportId(std::declval<wxWindowID>()) };
};


template<typename T>
concept HasRightClickMenu = IsRightClickMenu<typename T::RightClickMenu>;


template<typename T, typename = void>
struct RightClickMenu {};

template<typename T>
struct RightClickMenu
<
    T,
    std::enable_if_t<HasRightClickMenu<T>>
>
    :
    public T::RightClickMenu
{

};


template<typename List>
auto GetValueBase(List &list, size_t unordered)
{
    return pex::GetUnordered(list, unordered).Get().GetValueBase();
}


template<typename Create>
class ShapeEditor: public MouseSelectionBrain<ShapesControl>
{
public:
    static constexpr auto observerName = "ShapeEditor";

    static_assert(pex::HasIndices<ShapesControl>);

    ShapeEditor(
        const ShapesControl &shapeList,
        const CanvasControl &canvasControl)
        :
        MouseSelectionBrain(shapeList),
        isEnabled_(true),
        isProcessingAction_(false),

        canvasControl_(canvasControl),

        mouseDownEndpoint_(
            PEX_THIS("ShapeEditor"),
            canvasControl.mouseDown,
            &ShapeEditor::OnMouseDown_),

        rightMouseDownEndpoint_(
            this,
            canvasControl.rightMouseDown),

        logicalPositionEndpoint_(
            this,
            canvasControl.logicalPosition,
            &ShapeEditor::OnLogicalPosition_),

        modifierEndpoint_(
            this,
            canvasControl.modifier,
            &ShapeEditor::OnModifier_),

        keyCodeEndpoint_(
            this,
            canvasControl.keyCode,
            &ShapeEditor::OnKeyCode_),

        menuIdEndpoint_(
            this,
            canvasControl.menuId),

        drag_(),
        rightClickMenu_{}
    {
        if constexpr (HasRightClickMenu<Create>)
        {
            this->rightMouseDownEndpoint_.Connect(
                &ShapeEditor::OnRightMouseDown_);

            this->menuIdEndpoint_.Connect(&ShapeEditor::OnMenuId_);
        }
    }

    ~ShapeEditor()
    {

    }

    void SetIsEnabled(bool isEnabled)
    {
        // Do not leave drag state set when toggling the enable switch.
        this->drag_.reset();
        this->isEnabled_ = isEnabled;
    }

    void OnModifier_(const wxpex::Modifier &)
    {
        if (!this->isEnabled_)
        {
            return;
        }

        this->UpdateCursor_();
    }

    void OnKeyCode_(int keyCode)
    {
        if (!this->isEnabled_)
        {
            return;
        }

        if (keyCode == WXK_BACK || keyCode == WXK_DELETE)
        {
            this->DeleteSelected();
        }
    }

    void OnLogicalPosition_(const tau::Point2d<double> &position)
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
        auto wasSelected = this->FindSelected();

        auto found = this->FindClicked(click);

        if (found)
        {
            // The user clicked on a shape.

            if (!GetNode(found->item).isSelected.Get())
            {
                // The user clicked on the selected shape.
                GetNode(found->item).toggleSelect.Trigger();
            }

            // else
            // The user has left-clicked on an already selected shape.
            // Leave it selected.

            this->drag_ = found->item.Get().GetValueBase()->ProcessMouseDown(
                found->item.GetVirtual()->Copy(),
                click,
                this->canvasControl_.modifier.Get(),
                this->canvasControl_.cursor);

            return;
        }

        // The user clicked in empty space.

        if (wasSelected)
        {
            auto selected = wasSelected->item;
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
            GetNode(wasSelected->item).toggleSelect.Trigger();
        }

        if constexpr (!std::is_same_v<DoNotCreate, Create>)
        {
            // There is no selection to edit.
            // Begin creating a new shape.
            // DragCreate
            if constexpr (HasRightClickMenu<Create>)
            {
                this->drag_ =
                    std::make_unique<Create>(
                        this->rightClickMenu_,
                        click,
                        this->GetListControl());
            }
            else
            {
                this->drag_ =
                    std::make_unique<Create>(click, this->GetListControl());
            }
        }
    }

    void OnRightMouseDown_(bool isDown)
    {
        if (!this->isEnabled_)
        {
            return;
        }

        if (!isDown)
        {
            // Mouse has been released.
            this->UpdateCursor_();

            return;
        }

        if constexpr (HasRightClickMenu<Create>)
        {
            auto click = this->canvasControl_.logicalPosition.Get();
            auto found = this->FindClicked(click);

            bool foundHasValue = found.has_value();

            if (foundHasValue)
            {
                if (!GetNode(found->item).isSelected.Get())
                {
                    // Toggle to the new selection.
                    GetNode(found->item).toggleSelect.Trigger();
                    auto checkFound = this->FindSelected();
                    assert(checkFound->unordered == found->unordered);
                }
                // else
                // The user has right-clicked on an already selected shape.
                // Leave it selected.
            }

            found.reset();

            auto window = this->canvasControl_.window.Get();

            assert(window);

            window->PopupMenu(
                &this->rightClickMenu_.GetMenu(foundHasValue));
        }
    }

    void OnMenuId_(wxWindowID windowId)
    {
        if constexpr (HasRightClickMenu<Create>)
        {
            auto action = this->rightClickMenu_.GetAction(windowId);

            if (action)
            {
                jive::ScopeFlag processingAction(this->isProcessingAction_);

                auto found = this->FindSelected();

                if (!found)
                {
                    return;
                }

                if (this->rightClickMenu_.ProcessAction(*action, found->item))
                {
                    // This action is a deletion.
                    found.reset();
                    this->DeleteSelected();
                }
            }
            else
            {
                this->rightClickMenu_.ReportId(windowId);
            }
        }
    }

    void UpdateCursor_()
    {
        if (this->isProcessingAction_)
        {
            return;
        }

        if (this->IsDragging_())
        {
            return;
        }

        auto modifier = this->canvasControl_.modifier.Get();
        auto click = this->canvasControl_.logicalPosition.Get();
        auto found = this->FindClicked(click);

        if (!found)
        {
            // The cursor is not contained by any of the shapes.
            auto shapeSelection = this->FindSelected();

            if (modifier.IsControl() && shapeSelection)
            {
                auto shape = shapeSelection->item.Get().GetValueBase();

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
        auto value = found->item.Get();
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
                PolygonLines(points).Find(click, 10.0);

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


    bool isEnabled_;
    bool isProcessingAction_;

    CanvasControl canvasControl_;

    pex::Endpoint<ShapeEditor, decltype(CanvasControl::mouseDown)>
        mouseDownEndpoint_;

    pex::Endpoint<ShapeEditor, decltype(CanvasControl::rightMouseDown)>
        rightMouseDownEndpoint_;

    pex::Endpoint<ShapeEditor, PointControl> logicalPositionEndpoint_;

    pex::Endpoint<ShapeEditor, decltype(CanvasControl::modifier)>
        modifierEndpoint_;

    pex::Endpoint<ShapeEditor, decltype(CanvasControl::keyCode)>
        keyCodeEndpoint_;

    pex::Endpoint<ShapeEditor, decltype(CanvasControl::menuId)>
        menuIdEndpoint_;

    std::unique_ptr<Drag> drag_;

    RightClickMenu<Create> rightClickMenu_;
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
