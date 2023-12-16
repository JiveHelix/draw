#pragma once

#include <pex/endpoint.h>
#include "draw/oddeven.h"
#include "draw/drag.h"
#include "draw/views/pixel_view_settings.h"
#include "draw/node_settings.h"


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
    const Points &points);


double DragAngleDifference(double first, double second);


template<typename Control>
class DragShape: public Drag
{
public:
    DragShape(
        size_t index,
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        Control control)
        :
        Drag(index, start, offset),
        control_(control)
    {
        if (this->index_ > 0)
        {
            if (index >= control.Get().GetPoints().size())
            {
                throw std::out_of_range(
                    "Selected indexed exceeds point count.");
            }
        }
    }

    DragShape(
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        Control control)
        :
        Drag(start, offset),
        control_(control)
    {

    }

    void ReportLogicalPosition(const tau::Point2d<int> &position) override
    {
        this->control_.center.Set(this->GetPosition(position));
    }

protected:
    Control control_;
};


template<typename Control>
class DragEditShape: public Drag
{
public:
    using Shape = typename Control::Type;

    DragEditShape(
        size_t index,
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        Control control)
        :
        Drag(index, start, offset),
        control_(control),
        startingShape_(control.Get())
    {
        if (this->index_ > 0)
        {
            if (index >= control.Get().GetPoints().size())
            {
                throw std::out_of_range(
                    "Selected indexed exceeds point count.");
            }
        }
    }

    DragEditShape(
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        Control control)
        :
        Drag(start, offset),
        control_(control),
        startingShape_(control.Get())
    {

    }

    void ReportLogicalPosition(const tau::Point2d<int> &position) override
    {
        this->control_.Set(this->MakeShape_(position));
    }

protected:
    virtual Shape MakeShape_(const tau::Point2d<int> &end) const = 0;

protected:
    Control control_;
    Shape startingShape_;
};


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
            this->shapeList_.at(*wasSelected).GetNode().isSelected.Set(false);
        }

        this->shapeList_.selected.Set(index);
        this->shapeList_.at(index).GetNode().isSelected.Set(true);
    }

protected:
    ListControl shapeList_;
    tau::Point2d<int> position_;
};


template<typename Control>
class DragEditPoint: public DragEditShape<Control>
{
public:
    DragEditPoint(
        size_t index,
        const tau::Point2d<int> &start,
        Control control,
        const Points &points)
        :
        DragEditShape<Control>(index, start, points.at(index), control),
        points_(points)
    {

    }

protected:
    Points points_;
};


template
<
    typename RotatePoint,
    typename DragPoint,
    typename DragLine,
    typename DragShape,
    typename ItemControl
>
std::unique_ptr<Drag> ProcessMouseDown(
    ItemControl itemControl,
    const tau::Point2d<int> &click,
    const wxpex::Modifier &modifier,
    CursorControl cursor)
{
    auto shape = itemControl.shape.Get();
    auto points = shape.GetPoints();
    auto foundPoint = FindPoint(click, points);

    if (foundPoint)
    {
        if (modifier.IsAlt())
        {
            if constexpr (HandlesAltClick<ItemControl>)
            {
                itemControl.ProcessAltClick(*foundPoint, points);

                return {};
            }
        }

        auto pointIndex =
            static_cast<size_t>(
                std::distance(points.cbegin(), *foundPoint));

        if (modifier.IsControl())
        {
            return std::make_unique<RotatePoint>(
                pointIndex,
                click,
                itemControl.shape,
                points);
        }
        else
        {
            return std::make_unique<DragPoint>(
                pointIndex,
                click,
                itemControl.shape,
                points);
        }
    }

    auto lines = PolygonLines(points);
    auto lineIndex = lines.Find(click.template Convert<double>(), 10.0);

    if (lineIndex)
    {
        return std::make_unique<DragLine>(
            *lineIndex,
            click,
            itemControl.shape,
            lines);
    }

    cursor.Set(wxpex::Cursor::closedHand);

    return std::make_unique<DragShape>(
        click,
        shape.center,
        itemControl.shape);
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
    using Shape = typename ItemControl::Type;

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

        auto found = this->FindSelected_(click.template Convert<double>());

        if (!found)
        {
            if constexpr (HandlesControlClick<ItemControl>)
            {
                if (wasSelected)
                {
                    auto selected = this->shapeList_[*wasSelected];

                    if (this->pixelViewControl_.modifier.Get().IsControl())
                    {
                        // There was a selected shape.
                        // Keep it selected and handle a control modifier.
                        selected.ProcessControlClick(click);

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
            this->drag_ =
                std::make_unique<Create>(click, this->shapeList_);

            return;
        }

        // The user clicked on a shape.
        this->Select(found->first);

        this->drag_ = found->second.ProcessMouseDown(
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
        auto found = this->FindSelected_(click.template Convert<double>());

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
        auto shape = found->second.shape.Get();
        auto points = shape.GetPoints();
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
            PolygonLines(points).Find(click.template Convert<double>(), 10.0);

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
            auto shape = shapeControl.shape.Get();

            if (shape.Contains(position, 10.0))
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
                this->shapeList_[i].GetNode().select,
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
            this->shapeList_.at(*wasSelected).GetNode().isSelected.Set(false);
        }

        this->shapeList_.selected.Set(index);
        this->shapeList_.at(index).GetNode().isSelected.Set(true);
    }

    void Deselect(size_t index)
    {
        this->shapeList_.selected.Set({});
        this->shapeList_.at(index).GetNode().isSelected.Set(false);
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
