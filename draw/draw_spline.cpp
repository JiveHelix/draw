#include "draw/draw_spline.h"


namespace draw
{


using Point = tau::Point2d<double>;


std::vector<Point> GetDerivatives(
    std::span<const Point> points,
    const Point &firstDerivative,
    const Point &lastDerivative)
{
    if (points.size() < 2)
    {
        return {};
    }

    std::vector<Point> result(points.size());

    result.front() = firstDerivative;
    result.back() = lastDerivative;

    static constexpr double oneSixth = 1.0 / 6.0;

    // For all intervening points, the tangent is average of the points that
    // come before and after.
    for (std::size_t i = 1; i + 1 < points.size(); ++i)
    {
        result[i] = oneSixth * (points[i + 1] - points[i - 1]);
    }

    return result;
}


std::vector<Point> GetDerivatives(std::span<const Point> points)
{
    if (points.size() < 2)
    {
        return {};
    }

    std::vector<Point> result(points.size());

    // The first and last points define the tangent end points.
    result.front() = points[1] - points[0];
    result.back() = points.back() - points[points.size() - 2];

    static constexpr double oneSixth = 1.0 / 6.0;

    // For all intervening points, the tangent is average of the points that
    // come before and after.
    for (std::size_t i = 1; i + 1 < points.size(); ++i)
    {
        result[i] = oneSixth * (points[i + 1] - points[i - 1]);
    }

    return result;
}


void DrawTangentSpline(
    wxGraphicsPath &path,
    std::span<const Point> points,
    std::span<const Point> derivatives)
{
    for (std::size_t i = 0; i + 1 < points.size(); ++i)
    {
        Point start = points[i];
        Point end = points[i + 1];

        // Control points lie on the tangent lines of the incoming and outgoing
        // curves.
        Point startControl = start + derivatives[i];
        Point endControl = end - derivatives[i + 1];

        path.MoveToPoint(start.x, start.y);

        path.AddCurveToPoint(
            startControl.x,
            startControl.y,
            endControl.x,
            endControl.y,
            end.x,
            end.y);
    }
}


void DrawTangentSpline(
    wxGraphicsPath &path,
    const std::vector<Point> &points)
{
    auto derivatives = GetDerivatives(points);
    DrawTangentSpline(path, points, derivatives);
}


void DrawTangentSpline(
    wxGraphicsPath &path,
    std::span<const Point> points,
    const Point &firstDerivative,
    const Point &lastDerivative)
{
    auto derivatives = GetDerivatives(points, firstDerivative, lastDerivative);
    DrawTangentSpline(path, points, derivatives);
}


} // end namespace draw
