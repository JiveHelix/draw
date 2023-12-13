#pragma once


#include <vector>
#include <Eigen/Dense>
#include <tau/line2d.h>
#include <tau/vector2d.h>
#include "draw/oddeven.h"


namespace draw
{


using PolygonPoint = typename Points::value_type;


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
    PolygonLines(const Points &points);

    Points GetPoints() const;

    std::optional<size_t> Find(
        const tau::Point2d<double> &point,
        double margin);
};


} // end namespace draw
