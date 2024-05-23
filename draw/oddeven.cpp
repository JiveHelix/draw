#include "draw/oddeven.h"


namespace draw
{


namespace oddeven
{


bool SegmentIntersects(
    PointDouble point,
    const PointDouble &start,
    const PointDouble &end)
{
    static const auto epsilon = 1.0e-3;
    static const auto infinity = std::numeric_limits<double>::infinity();

    auto Close = [](double first, double second) -> bool
    {
        return std::abs(first - second) < epsilon;
    };

    // end must be above start
    if (start.y > end.y)
    {
        return SegmentIntersects(point, end, start);
    }

    if (Close(point.y, start.y) || Close(point.y, end.y))
    {
        // The vertex falls on the line. Move the line up slighly.
        point.y += epsilon;
    }

    if (point.y < start.y || point.y > end.y)
    {
        // The horizontal line starting at point does not intersect
        // with the line segment.
        return false;
    }

    if (point.x >= std::max(start.x, end.x))
    {
        // The line begins to the right of the segment.
        return false;
    }

    if (point.x < std::min(start.x, end.x))
    {
        // The line begins to the left of the segment, so must intersect.
        return true;
    }

    double segmentSlope;

    if (!Close(start.x, end.x))
    {
        // We can compute the slope of the segment.
        segmentSlope = (end.y - start.y) / (end.x - start.x);
    }
    else
    {
        segmentSlope = infinity;
    }

    double startToPointSlope;

    if (!Close(point.x, start.x))
    {
        startToPointSlope = (point.y - start.y) / (point.x - start.x);
    }
    else
    {
        startToPointSlope = infinity;
    }

    return (startToPointSlope >= segmentSlope);
}


bool Contains(
    const PointsDouble &points,
    const PointDouble &point)
{
    if (points.size() < 3)
    {
        return false;
    }

    // Apply the odd-even rule. If a line drawn from point intersects the
    // polygon an even number of times, it lies on the outside.
    size_t count = 0;

    for (size_t index = 0; index < points.size() - 1; ++index)
    {
        count += static_cast<size_t>(
            SegmentIntersects(point, points[index], points[index + 1]));
    }

    count += SegmentIntersects(point, points.front(), points.back());

    return (count % 2) != 0;
}


} // end namespace oddeven


} // end namespace draw
