#include "draw/shape_edit.h"


namespace draw
{


std::optional<PointsIterator> FindPoint(
    const tau::Point2d<int> &click,
    const Points &points)
{
    auto found = std::find_if(
        std::begin(points),
        std::end(points),
        [&click](const auto &p) -> bool
        {
            return click.Distance(p) < 10.0;
        });

    if (found != std::end(points))
    {
        return found;
    }

    return {};
}


double DragAngleDifference(double first, double second)
{
    if (first < -90 || second < -90)
    {
        first = std::fmod(first + 360, 360);
        second = std::fmod(second + 360, 360);
    }

    return first - second;
}



ShapeBrainBase::ShapeBrainBase(
    PixelViewControl pixelViewControl)
    :
    pixelViewControl_(pixelViewControl),
    mouseDownEndpoint_(
        this,
        pixelViewControl.mouseDown,
        &ShapeBrainBase::OnMouseDown_),
    logicalPositionEndpoint_(
        this,
        pixelViewControl.logicalPosition,
        &ShapeBrainBase::OnLogicalPosition_),
    modifierEndpoint_(
        this,
        pixelViewControl.modifier,
        &ShapeBrainBase::OnModifier_),
    drag_()
{

}


void ShapeBrainBase::OnModifier_(const wxpex::Modifier &)
{
    this->UpdateCursor_();
}


void ShapeBrainBase::OnLogicalPosition_(const tau::Point2d<int> &position)
{
    this->UpdateCursor_();

    if (this->drag_)
    {
        this->drag_->ReportLogicalPosition(position);
    }
}


bool ShapeBrainBase::IsDragging_() const
{
    return !!this->drag_;
}


} // end namespace draw
