#pragma once

#include <pex/endpoint.h>
#include "draw/oddeven.h"
#include "draw/drag.h"
#include "draw/views/pixel_view_settings.h"
#include "draw/node_settings.h"
#include "draw/shapes.h"


namespace draw
{


template<typename T, typename = void>
struct HandlesControlClick_: std::false_type {};

template<typename T>
struct HandlesControlClick_
<
    T,
    std::enable_if_t<T::handlesControlClick>
>: std::true_type {};


template<typename T>
inline constexpr bool HandlesControlClick = HandlesControlClick_<T>::value;


template<typename T, typename = void>
struct HandlesAltClick_: std::false_type {};

template<typename T>
struct HandlesAltClick_
<
    T,
    std::enable_if_t<T::handlesAltClick>
>: std::true_type {};


template<typename T>
inline constexpr bool HandlesAltClick = HandlesAltClick_<T>::value;


std::optional<PointsIterator> FindPoint(
    const tau::Point2d<int> &click,
    const PointsDouble &points);


double DragAngleDifference(double first, double second);


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
        this->control_->SetValueBase(this->startingShape_);
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
        this->control_->SetValueBase(*this->MakeShape_(position));
    }

protected:
    virtual std::shared_ptr<Shape> MakeShape_(
        const tau::Point2d<int> &end) const = 0;

protected:
    std::shared_ptr<ShapeControl> control_;
    DerivedShape startingShape_;
};


template<typename List>
NodeSettingsControl & GetNode(List &list, size_t index)
{
    auto &shapeControl = list.at(index);
    return shapeControl.GetControlBase()->GetNode();
}


template<typename ListControl, typename CreateShape>
class DragCreateShape: public Drag
{
public:
    DragCreateShape(
        const tau::Point2d<int> &start,
        const ListControl &shapeList)
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
            this->Select_(*newIndex);
        }
    }

    void Select_(size_t index)
    {
        auto wasSelected = this->shapeList_.selected.Get();

        if (wasSelected)
        {
            GetNode(this->shapeList_, *wasSelected).isSelected.Set(false);
        }

        this->shapeList_.selected.Set(index);
        GetNode(this->shapeList_, index).isSelected.Set(true);
    }

protected:
    ListControl shapeList_;
    tau::Point2d<int> position_;
};


template<typename ListControl, typename CreateShape>
class DragReplaceShape: public Drag
{
public:
    DragReplaceShape(
        const tau::Point2d<int> &start,
        const ListControl &shapeList)
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
    ListControl shapeList_;
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
    const DerivedShape &shape,
    const tau::Point2d<int> &click,
    const wxpex::Modifier &modifier,
    CursorControl cursor)
{
    auto points = shape.GetPoints();
    auto foundPoint = FindPoint(click, points);

    if (foundPoint)
    {
        if (modifier.IsAlt())
        {
            if constexpr (HandlesAltClick<DerivedShape>)
            {
                if (shape.ProcessAltClick(*shapeControl, *foundPoint, points))
                {
                    return {};
                }
            }
        }

        auto pointIndex =
            static_cast<size_t>(
                std::distance(points.cbegin(), *foundPoint));

        if (modifier.IsControl())
        {
            // DragEditPoint
            return std::make_unique<RotatePoint>(
                pointIndex,
                click,
                shapeControl,
                shape,
                points);
        }
        else
        {
            // DragEditPoint
            return std::make_unique<DragPoint>(
                pointIndex,
                click,
                shapeControl,
                shape,
                points);
        }
    }

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

    cursor.Set(wxpex::Cursor::closedHand);

    // DragShape
    return std::make_unique<DragShape>(
        click,
        shape.shape.center,
        shapeControl,
        shape);
}


template
<
    typename ListControl,
    typename Create
>
class ShapeBrain
{
public:
    static constexpr auto observerName = "ShapeBrain";

    using ItemControl = typename ListControl::ItemControl;

    ShapeBrain(
        ListControl shapeList,
        PixelViewControl pixelViewControl)
        :
        pixelViewControl_(pixelViewControl),
        mouseDownEndpoint_(
            this,
            pixelViewControl.mouseDown,
            &ShapeBrain::OnMouseDown_),
        logicalPositionEndpoint_(
            this,
            pixelViewControl.logicalPosition,
            &ShapeBrain::OnLogicalPosition_),
        modifierEndpoint_(
            this,
            pixelViewControl.modifier,
            &ShapeBrain::OnModifier_),
        drag_(),
        shapeList_(shapeList),
        listCount_(this, this->shapeList_.count, &ShapeBrain::OnCount_),
        countWillChange_(
            this,
            this->shapeList_.countWillChange,
            &ShapeBrain::OnCountWillChange_),
        selectConnections_()
    {
        this->OnCount_(this->shapeList_.count.Get());
    }

    ~ShapeBrain()
    {
        this->ClearSelectConnections_();
    }

    void OnModifier_(const wxpex::Modifier &)
    {
        this->UpdateCursor_();
    }


    void OnLogicalPosition_(const tau::Point2d<int> &position)
    {
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
        if (!isDown)
        {
            // Mouse has been released.
            this->drag_.reset();
            this->UpdateCursor_();

            return;
        }

        auto click = this->pixelViewControl_.logicalPosition.Get();
        auto wasSelected = this->shapeList_.selected.Get();

        auto found = this->FindSelected_(click.template Cast<double>());

        if (!found)
        {
            if (wasSelected)
            {
                auto selected = this->shapeList_[*wasSelected];
                auto shape = selected.Get();

                if (shape.GetValueBase()->HandlesControlClick())
                {
                    if (this->pixelViewControl_.modifier.Get().IsControl())
                    {
                        // There was a selected shape.
                        // Keep it selected and handle a control modifier.
                        shape.GetValueBase()->ProcessControlClick(
                            *selected.GetControlBase(),
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

            // There is no selection to edit.
            // Begin creating a new shape.
            // DragCreate
            this->drag_ =
                std::make_unique<Create>(click, this->shapeList_);

            return;
        }

        // The user clicked on a shape.
        this->Select(found->first);

        this->drag_ = found->second.Get().GetValueBase()->ProcessMouseDown(
            found->second.GetControlBase()->Copy(),
            click,
            this->pixelViewControl_.modifier.Get(),
            this->pixelViewControl_.cursor);
    }

    void UpdateCursor_()
    {
        if (this->IsDragging_())
        {
            return;
        }

        auto modifier = this->pixelViewControl_.modifier.Get();
        auto click = this->pixelViewControl_.logicalPosition.Get();
        auto found = this->FindSelected_(click.template Cast<double>());

#if 0
        std::cout << std::boolalpha;
        std::cout << "found: " << !!found << std::endl;
        std::cout << "modifer.IsControl(): " << modifier.IsControl() << std::endl;

        std::cout << "this->shapeList_.selected.Get(): " << !!this->shapeList_.selected.Get() << std::endl;
#endif

        if (!found)
        {
            // The cursor is not contained by any of the shapes.
            if (modifier.IsControl() && this->shapeList_.selected.Get())
            {
                this->pixelViewControl_.cursor.Set(wxpex::Cursor::pencil);

                return;
            }

            if (this->pixelViewControl_.cursor.Get() != wxpex::Cursor::arrow)
            {
                this->pixelViewControl_.cursor.Set(wxpex::Cursor::arrow);
            }

            return;
        }

        // There is a shape under the cursor.
        auto value = found->second.Get();
        auto shape = value.GetValueBase();
        auto points = shape->GetPoints();
        auto foundPoint = FindPoint(click, points);

        if (foundPoint)
        {
            // Hovering over a point, change cursor
            if (modifier.IsControl())
            {
                // TODO: Create a rotate cursor
                this->pixelViewControl_.cursor.Set(wxpex::Cursor::pointRight);
            }
            else if (modifier.IsAlt() && HandlesAltClick<ItemControl>)
            {
                this->pixelViewControl_.cursor.Set(wxpex::Cursor::bullseye);
            }
            else
            {
                this->pixelViewControl_.cursor.Set(wxpex::Cursor::cross);
            }

            return;
        }

        auto lineIndex =
            PolygonLines(points).Find(click.template Cast<double>(), 10.0);

        if (lineIndex)
        {
            // Hovering over a line, change cursor
            this->pixelViewControl_.cursor.Set(wxpex::Cursor::sizing);
            return;
        }

        // The mouse is over a shape, but not over a line or a point.
        this->pixelViewControl_.cursor.Set(wxpex::Cursor::openHand);
    }

    std::optional<std::pair<size_t, ItemControl>> FindSelected_(
        const tau::Point2d<int> &position)
    {
        auto count = this->shapeList_.count.Get();

        while (count-- > 0)
        {
            auto &shapeControl = this->shapeList_[count];
            auto value = shapeControl.Get();
            auto shape = value.GetValueBase();

            if (shape->Contains(position, 10.0))
            {
                return std::make_pair(count, shapeControl);
            }
        }

        return {};
    }

private:
    void ClearSelectConnections_()
    {
        for (auto &connection: this->selectConnections_)
        {
            connection.Disconnect(this);
        }

        this->selectConnections_.clear();
    }

    void OnCountWillChange_()
    {
        this->ClearSelectConnections_();
    }

    void OnCount_(size_t count_)
    {
        this->selectConnections_.reserve(count_);

        for (size_t i = 0; i < count_; ++i)
        {
            this->selectConnections_.emplace_back(
                this,
                GetNode(this->shapeList_, i).select,
                std::bind(&ShapeBrain::OnSelect_, i, std::placeholders::_1));
        }
    }

    static void OnSelect_(size_t index, void *context)
    {
        auto self = static_cast<ShapeBrain *>(context);
        auto selected = self->shapeList_.selected.Get();

        if (selected && (*selected == index))
        {
            self->Deselect(index);
            return;
        }

        self->Select(index);
    }

    void Select(size_t index)
    {
        auto wasSelected = this->shapeList_.selected.Get();

        if (wasSelected && (*wasSelected != index))
        {
            GetNode(this->shapeList_, *wasSelected).isSelected.Set(false);
        }

        this->shapeList_.selected.Set(index);
        GetNode(this->shapeList_, index).isSelected.Set(true);
    }

    void Deselect(size_t index)
    {
        this->shapeList_.selected.Set({});
        GetNode(this->shapeList_, index).isSelected.Set(false);
    }

    PixelViewControl pixelViewControl_;

    pex::Endpoint<ShapeBrain, decltype(PixelViewControl::mouseDown)>
        mouseDownEndpoint_;

    pex::Endpoint<ShapeBrain, PointControl> logicalPositionEndpoint_;

    pex::Endpoint<ShapeBrain, decltype(PixelViewControl::modifier)>
        modifierEndpoint_;

    using ListCountEndpoint =
        pex::Endpoint<ShapeBrain, pex::control::ListCount>;

    using CountWillChange =
        pex::Endpoint<ShapeBrain, pex::control::Signal<pex::GetTag>>;

    std::unique_ptr<Drag> drag_;

    ListControl shapeList_;
    ListCountEndpoint listCount_;
    CountWillChange countWillChange_;

    std::vector<NodeSelectSignal> selectConnections_;
};



} // end namespace draw
