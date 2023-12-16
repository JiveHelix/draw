#pragma once


#include <vector>
#include <tau/vector2d.h>


namespace draw
{


using Points = std::vector<tau::Point2d<double>>;
using PointsIterator = typename Points::const_iterator;


namespace oddeven
{


bool SegmentIntersects(
    tau::Point2d<double> point,
    const tau::Point2d<double> &start,
    const tau::Point2d<double> &end);


bool Contains(const Points &points, const tau::Point2d<double> &point);


} // end namespace oddeven


} // end namespace draw
