#pragma once


#include "draw/draw_context.h"
#include "draw/oddeven.h"


namespace draw
{


void AddSegmentsToPoint(
    wxGraphicsPath &path,
    const PointsDouble &points);


void DrawSegments(
    DrawContext &context,
    const PointsDouble &points);


} // end namespace draw
