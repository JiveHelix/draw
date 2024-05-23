#pragma once


#include <wxpex/graphics.h>
#include "draw/oddeven.h"


namespace draw
{


void DrawSegments(
    wxpex::GraphicsContext &context,
    const PointsDouble &points);


} // end namespace draw
