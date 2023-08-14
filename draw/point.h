#pragma once


#include <tau/vector2d.h>
#include <pex/interface.h>
#include <wxpex/point.h>


namespace draw
{


using PointGroup = tau::Point2dGroup<int>;
using Point = typename PointGroup::Plain;
using PointControl = typename PointGroup::Control;

using PointGroupMaker = pex::MakeGroup<PointGroup>;

} // end namespace draw
