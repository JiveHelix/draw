#pragma once

#include <pex/endpoint.h>
#include "draw/oddeven.h"
#include "draw/drag.h"
#include "draw/views/pixel_view_settings.h"


namespace draw
{


using PointsIterator = typename Points::const_iterator;


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
        this->shapeList_.selected.Set(newIndex);
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


class ShapeBrainBase
{
public:
    static constexpr auto observerName = "ShapeBrainBase";

    ShapeBrainBase(PixelViewControl pixelViewControl);

    virtual ~ShapeBrainBase() {}

protected:
    virtual void OnMouseDown_(bool isDown) = 0;
    virtual void UpdateCursor_() = 0;

protected:
    void OnModifier_(const wxpex::Modifier &);
    void OnLogicalPosition_(const tau::Point2d<int> &position);
    bool IsDragging_() const;

    PixelViewControl pixelViewControl_;

    pex::Endpoint<ShapeBrainBase, decltype(PixelViewControl::mouseDown)>
        mouseDownEndpoint_;

    pex::Endpoint<ShapeBrainBase, PointControl> logicalPositionEndpoint_;

    pex::Endpoint<ShapeBrainBase, decltype(PixelViewControl::modifier)>
        modifierEndpoint_;

    std::unique_ptr<Drag> drag_;
};


template
<
    typename ListControl,
    typename Create,
    typename RotatePoint,
    typename DragPoint,
    typename DragLine,
    typename DragShape
>
class ShapeBrain: public ShapeBrainBase
{
public:
    using ItemControl = typename ListControl::ItemControl;

    ShapeBrain(
        ListControl shapeList,
        PixelViewControl pixelViewControl)
        :
        ShapeBrainBase(pixelViewControl),
        shapeList_(shapeList)
    {

    }

    virtual ~ShapeBrain() {}

protected:

    virtual bool HandleControlModifier_(
        [[maybe_unused]] const tau::Point2d<int> click,
        [[maybe_unused]] ItemControl control)
    {
        return false;
    }

    virtual bool HandleAltModifier_(
        [[maybe_unused]] PointsIterator iterator,
        [[maybe_unused]] ItemControl control,
        [[maybe_unused]] Points &points)
    {
        return false;
    }

    void OnMouseDown_(bool isDown) override
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
            if (wasSelected
                && this->pixelViewControl_.modifier.Get().IsControl())
            {
                // There was a selected shape.
                // Keep it selected and handle a control modifier.
                auto control = this->shapeList_[*wasSelected];

                if (this->HandleControlModifier_(click, control))
                {
                    return;
                }
            }

            // The user clicked away from a shape without holding control.
            this->shapeList_.selected.Set({});

            // There is no selection to edit.
            // Begin a drag to create a new shape.
            this->drag_ =
                std::make_unique<Create>(click, this->shapeList_);

            return;
        }

        // The user clicked on a shape.
        this->shapeList_.selected.Set(found->first);
        auto selectedControl = found->second;
        auto shape = selectedControl.shape.Get();
        auto points = shape.GetPoints();
        auto foundPoint = FindPoint(click, points);

        if (foundPoint)
        {
            if (this->pixelViewControl_.modifier.Get().IsAlt())
            {
                if (this->HandleAltModifier_(
                        *foundPoint,
                        selectedControl,
                        points))
                {
                    return;
                }
            }

            auto pointIndex =
                static_cast<size_t>(
                    std::distance(points.cbegin(), *foundPoint));

            if (this->pixelViewControl_.modifier.Get().IsControl())
            {
                this->drag_ =
                    std::make_unique<RotatePoint>(
                        pointIndex,
                        click,
                        selectedControl.shape,
                        points);
            }
            else
            {
                this->drag_ =
                    std::make_unique<DragPoint>(
                        pointIndex,
                        click,
                        selectedControl.shape,
                        points);
            }

            return;
        }

        auto lines = PolygonLines(points);
        auto lineIndex = lines.Find(click.template Convert<double>(), 10.0);

        if (lineIndex)
        {
            this->drag_ =
                std::make_unique<DragLine>(
                    *lineIndex,
                    click,
                    selectedControl.shape,
                    lines);

            return;
        }

        this->drag_ = std::make_unique<DragShape>(
            click,
            shape.center,
            selectedControl.shape);

        this->pixelViewControl_.cursor.Set(
            wxpex::Cursor::closedHand);
    }

    void UpdateCursor_() override
    {
        if (this->IsDragging_())
        {
            return;
        }

        auto modifier = this->pixelViewControl_.modifier.Get();

        auto click = this->pixelViewControl_.logicalPosition.Get();
        auto found = this->FindSelected_(click.template Convert<double>());

        if (!found)
        {
            // The cursor is not contained by any of the polygons.
            if (modifier.IsControl())
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
            else if (modifier.IsAlt())
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
            auto & shapeControl = this->shapeList_[count];
            auto shape = shapeControl.shape.Get();

            if (shape.Contains(position, 10.0))
            {
                this->shapeList_.selected.Set(count);
                return std::make_pair(count, shapeControl);
            }
        }

        return {};
    }

private:
    ListControl shapeList_;
};



} // end namespace draw
