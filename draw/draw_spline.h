#pragma once


#include <wxpex/graphics.h>
#include <wxpex/point.h>


namespace draw
{


void DrawSpline(
    wxGraphicsPath &path,
    const std::vector<tau::Point2d<double>> &points);


} // end namespace draw
