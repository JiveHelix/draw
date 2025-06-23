#include "draw/shape_editor.h"


namespace draw
{


std::optional<PointsIterator> FindPoint(
    const tau::Point2d<int> &click,
    const PointsDouble &points)
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


double AdjustRotation(
    double startingRotation,
    const tau::Point2d<double> &center,
    const tau::Point2d<double> &referencePoint,
    const tau::Point2d<double> &endPoint)
{
    auto beginAngle = (referencePoint - center).GetAngle();
    auto endAngle = (endPoint - center).GetAngle();

    auto difference = endAngle - beginAngle;
    startingRotation += difference;

    return Modulo(startingRotation + 180.0, 360.0) - 180.0;
}


} // end namespace draw
