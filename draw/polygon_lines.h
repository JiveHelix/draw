#pragma once


#include <vector>
#include <Eigen/Dense>
#include <tau/line2d.h>
#include <tau/vector2d.h>


namespace draw
{


using PolygonPoint = tau::Point2d<double>;
using PolygonPoints = std::vector<PolygonPoint>;


struct PolygonLines
{
    using Line = tau::Line2d<double>;

    std::vector<Line> lines;

    PolygonLines() = default;

    const Line & operator[](size_t index) const
    {
        return this->lines[index];
    }

    const Line & at(size_t index) const
    {
        return this->lines.at(index);
    }

    Line & operator[](size_t index)
    {
        return this->lines[index];
    }

    Line & at(size_t index)
    {
        return this->lines.at(index);
    }

    static PolygonPoint GetMidpoint(
        const PolygonPoint &first,
        const PolygonPoint &second);

    PolygonLines(
        const PolygonPoint &topLeft,
        const PolygonPoint &topRight,
        const PolygonPoint &bottomRight,
        const PolygonPoint &bottomLeft);

    PolygonLines(double halfWidth, double halfHeight);
    PolygonLines(const tau::Size<double> &size);
    PolygonLines(const PolygonPoints &points);

    PolygonPoints GetPoints() const;
};


} // end namespace draw
