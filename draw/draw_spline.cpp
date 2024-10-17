#include "draw/draw_spline.h"
#include <tau/line2d.h>


namespace draw
{


using Point = tau::Point2d<double>;


void SmoothJoin(Point &controlA, const Point &join, Point &controlB)
{
#ifndef NDEBUG
    auto pointA = controlA;
    auto pointB = controlB;
#endif

    auto lineA = tau::Line2d<double>(join, controlA);
    auto sizeA = join.Distance(controlA);
    auto lineB = tau::Line2d<double>(join, controlB);
    auto sizeB = join.Distance(controlB);

    double meetingAngle = lineA.GetAngle_rad(lineB);

    double correction;

    if (meetingAngle < 0.0)
    {
        correction = (-tau::Angles<double>::pi - meetingAngle) / 2.0;
    }
    else
    {
        correction = (tau::Angles<double>::pi - meetingAngle) / 2.0;
    }

    controlA = lineA.GetRotated_rad(-correction).GetEndPoint(sizeA);
    controlB = lineB.GetRotated_rad(correction).GetEndPoint(sizeB);
}


void DrawSpline(
    wxGraphicsPath &path,
    const std::vector<tau::Point2d<double>> &points)
{
    // Implement a spline using cubic bezier curves.
    // After setting the initial point, each new curve has 3 new points:
    // 2 control points, and an end point.
    size_t count = points.size();

    if (count < 2)
    {
        throw std::logic_error("Spline requies 2 or more points");
    }

    if (count == 2)
    {
        auto &start = points.front();
        auto &end = points.back();
        path.MoveToPoint(start.x, start.y);
        path.AddLineToPoint(end.x, end.y);

        path.AddQuadCurveToPoint(
            start.x,
            start.y,
            end.x,
            end.y);

        return;
    }

    if (count == 3)
    {
        auto &control1 = points[0];
        auto &control2 = points[1];
        auto &end = points[2];

        path.AddCurveToPoint(
            control1.x,
            control1.y,
            control2.x,
            control2.y,
            end.x,
            end.y);

        return;
    }

    size_t curveCount = count / 3u;
    size_t remainder = count % 3;
    
    assert(curveCount > 0);
    assert(count > 3);

    auto smoothedPoints = points;

    for (size_t i = 0; i < curveCount - 1; ++i)
    {
        auto &controlA = smoothedPoints[1 + (i * 3)];
        auto &join = smoothedPoints[2 + (i * 3)];
        auto &controlB = smoothedPoints[3 + (i * 3)];

        SmoothJoin(controlA, join, controlB);
    }

    if (remainder > 0)
    {
        auto lastJoinIndex = (curveCount * 3) - 1;

        SmoothJoin(
            smoothedPoints[lastJoinIndex - 1],
            smoothedPoints[lastJoinIndex],
            smoothedPoints[lastJoinIndex + 1]);
    }

    for (size_t i = 0; i < curveCount; ++i)
    {
        auto &control1 = smoothedPoints[0 + (i * 3)];
        auto &control2 = smoothedPoints[1 + (i * 3)];
        auto &end = smoothedPoints[2 + (i * 3)];

        path.AddCurveToPoint(
            control1.x,
            control1.y,
            control2.x,
            control2.y,
            end.x,
            end.y);
    }

    if (remainder == 1)
    {
        auto &point = smoothedPoints.back();
        path.AddLineToPoint(point.x, point.y);
    }
    else if (remainder == 2)
    {
        auto &control1 = smoothedPoints[smoothedPoints.size() - 2];
        auto &end = smoothedPoints.back();
        path.AddQuadCurveToPoint(control1.x, control1.y, end.x, end.y);
    }
}


} // end namespace draw
