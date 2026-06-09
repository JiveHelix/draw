#include "draw/segments_shape.h"
#include "draw/draw_spline.h"
#include <cmath>


namespace draw
{


std::vector<CurveStyle> CurveStyleChoices::GetChoices()
{
    return {
        CurveStyle::line,
        CurveStyle::smoothSpline,
        CurveStyle::tangentSpline,
        CurveStyle::gcdcSpline};
}


std::string CurveStyleConverter::ToString(CurveStyle curveStyle)
{
    switch (curveStyle)
    {
        case (CurveStyle::line):
            return "line";

        case (CurveStyle::smoothSpline):
            return "smoothSpline";

        case (CurveStyle::tangentSpline):
            return "tangentSpline";

        case (CurveStyle::gcdcSpline):
            return "gcdcSpline";

        default:
            throw std::logic_error("Unknown CurveStyle");
    }
}


std::ostream & operator<<(std::ostream &output, CurveStyle value)
{
    return output << CurveStyleConverter::ToString(value);
}



SegmentsShape::SegmentsShape(
    const SegmentsSettings &settings,
    const PointsDouble &points)
    :
    segmentsSettings_(settings),
    points_(points)
{
    if (points.size() < 2)
    {
        throw std::logic_error("Segments requires at least 2 points");
    }
}


void SegmentsShape::Draw(DrawContext &context)
{
    if (this->points_.empty())
    {
        return;
    }

    context.ConfigureLook(this->segmentsSettings_.look);
    auto path = context->CreatePath();

    if (this->segmentsSettings_.curveStyle == CurveStyle::smoothSpline)
    {
        DrawSpline(path, this->points_);
        context->DrawPath(path);
    }
    else if (this->segmentsSettings_.curveStyle == CurveStyle::tangentSpline)
    {
        auto look = this->segmentsSettings_.look;

        using Point = tau::Point2d<double>;
        auto pointSpan = std::span<Point>(this->points_);
        auto derivatives = GetDerivatives(pointSpan);
        auto derivativeSpan = std::span<Point>(derivatives);

        auto hueStep = 360.0 / static_cast<double>(pointSpan.size());

        for (size_t i = 0; i + 1 < pointSpan.size(); ++i)
        {
            DrawTangentSpline(
                path,
                pointSpan.subspan(i, 2),
                derivativeSpan.subspan(i, 2));

            context->DrawPath(path);

            look.stroke.color.hue =
                std::fmod(look.stroke.color.hue + hueStep, 360);

            context.ConfigureColors(look);
            path = context->CreatePath();
        }
    }
    else if (this->segmentsSettings_.curveStyle == CurveStyle::gcdcSpline)
    {
        context.GetSplineTool().DrawSpline(this->points_);
    }
    else
    {
        // Just draw lines between points.
        auto point = std::begin(this->points_);
        auto end = std::end(this->points_);

        path.MoveToPoint(point->x, point->y);

        while (++point != end)
        {
            path.AddLineToPoint(point->x, point->y);
        }

        context->DrawPath(path);
    }

    if (this->segmentsSettings_.drawPoints)
    {
        auto pointsPath = context->CreatePath();

        for (const auto &point: this->points_)
        {
            pointsPath.AddCircle(point.x, point.y, 2);
        }

        context->DrawPath(pointsPath);
    }
}


} // end namespace draw
