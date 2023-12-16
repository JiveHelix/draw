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


} // end namespace draw
