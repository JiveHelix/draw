#pragma once


#include "draw/points.h"


namespace draw
{


namespace oddeven
{


bool SegmentIntersects(
    tau::Point2d<double> point,
    const PointDouble &start,
    const PointDouble &end);


bool Contains(const PointsDouble &points, const PointDouble &point);


} // end namespace oddeven


} // end namespace draw
