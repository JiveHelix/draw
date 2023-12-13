#include "draw/polygon_lines.h"


namespace draw
{


PolygonPoint PolygonLines::GetMidpoint(
    const PolygonPoint &first,
    const PolygonPoint &second)
{
    return (first + second) / 2.0;
}


PolygonLines::PolygonLines(
    const PolygonPoint &topLeft,
    const PolygonPoint &topRight,
    const PolygonPoint &bottomRight,
    const PolygonPoint &bottomLeft)
    :
    PolygonLines(Points({topLeft, topRight, bottomRight, bottomLeft}))
{

}


PolygonLines::PolygonLines(double halfWidth, double halfHeight)
    :
    PolygonLines(
        PolygonPoint(-halfWidth, -halfHeight),
        PolygonPoint(halfWidth, -halfHeight),
        PolygonPoint(halfWidth, halfHeight),
        PolygonPoint(-halfWidth, halfHeight))
{

}


PolygonLines::PolygonLines(const tau::Size<double> &size)
    :
    PolygonLines(size.width / 2.0, size.height / 2.0)
{

}


PolygonLines::PolygonLines(const Points &points)
    :
    lines(points.size())
{
    if (points.size() < 3)
    {
        this->lines.clear();
        return;
    }

    for (size_t index = 0; index < points.size() - 1; ++index)
    {
        auto &first = points[index];
        auto &second = points[index + 1];
        this->lines[index] = PolygonLines::Line(first, second);
        this->lines[index].point = GetMidpoint(first, second);
    }

    // Create the last line
    auto &first = points.back();
    auto &second = points.front();
    this->lines.back() = PolygonLines::Line(first, second);
    this->lines.back().point = GetMidpoint(first, second);
}


Points PolygonLines::GetPoints() const
{
    if (this->lines.size() < 3)
    {
        return {};
    }

    Points points(this->lines.size());

    // First point is intersection of last line with first line.
    points[0] = this->lines.back().Intersect(this->lines.front());

    for (size_t index = 1; index < this->lines.size(); ++index)
    {
        points[index] =
            this->lines[index - 1].Intersect(this->lines[index]);
    }

    return points;
}


std::optional<size_t> PolygonLines::Find(
    const tau::Point2d<double> &point,
    double margin)
{
    for (size_t index = 0; index < this->lines.size(); ++index)
    {
        auto &line = this->lines[index];

        if (line.DistanceToPoint(point) < margin)
        {
            return index;
        }
    }

    return {};
}


} // end namespace draw
