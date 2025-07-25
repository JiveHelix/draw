#include "draw/regular_polygon.h"
#include "draw/shape_editor.h"
#include <limits>


namespace draw
{


double GetRadius(size_t sides, double sideLength)
{
    auto halfAngle = 
        0.5 * tau::Angles<double>::tau / static_cast<double>(sides);

    auto sine = std::sin(halfAngle);

    assert(sine > 0.0);

    auto halfSide = sideLength / 2.0;

    return halfSide / sine;
}


double GetSideLength(size_t sides, double radius)
{
    auto halfAngle = 
        0.5 * tau::Angles<double>::tau / static_cast<double>(sides);

    auto halfSide = std::sin(halfAngle) * radius;

    return halfSide * 2.0;
}


PointsDouble RegularPolygon::GetPoints() const
{
    return this->GetPoints_(this->GetRadius(), 1.0);
}


PolygonLines RegularPolygon::GetLines() const
{
    return {this->GetPoints()};
}


bool RegularPolygon::Contains(const tau::Point2d<double> &point) const
{
    return oddeven::Contains(
        this->GetPoints_(this->GetRadius(), 1.0),
        point);
}


bool RegularPolygon::Contains(
    const tau::Point2d<double> &point,
    double margin) const
{
    return oddeven::Contains(
        this->GetPoints_(this->GetRadius(), this->GetMarginScale(margin)),
        point);
}


double RegularPolygon::GetRadius() const
{
    return this->radius;
}


std::optional<double> RegularPolygon::HasRadius() const
{
    if (this->radius >= std::numeric_limits<double>::min())
    {
        return radius;
    }

    return {};
}


double RegularPolygon::GetMidpointRadius() const
{
    auto halfAngle = 
        0.5 * tau::Angles<double>::tau / static_cast<double>(this->sides);

    auto tangent = std::tan(halfAngle);
    assert(tangent > 0.0);

    auto halfSide = GetSideLength(this->sides, this->radius) / 2.0;

    return halfSide / tangent;
}


double RegularPolygon::GetMarginScale(double margin) const
{
    auto radius = this->HasRadius();

    if (!radius)
    {
        return 0.0;
    }

    // radius + margin = result * radius
    // result = (radius + margin) / radius
    
    return (*radius + margin) / *radius;
}


RegularPolygon RegularPolygon::Rotate_deg(double relative_deg)
{
    auto newAngle =
        Modulo(this->rotation_deg + relative_deg + 180.0, 360.0) - 180.0;

    return RegularPolygon(
        this->center,
        this->radius,
        this->sides,
        newAngle);
}


void RegularPolygon::SetRadius(double radius_)
{
    this->radius = radius_;
}


void RegularPolygon::SetMidpointRadius(double radius)
{
    auto sideLength = GetSideLength(this->sides, radius);
    this->radius = ::draw::GetRadius(this->sides, sideLength);
}


PointsDouble RegularPolygon::GetPoints_(double radius, double scale) const
{
    radius *= scale;

    auto angleStep = 
        tau::Angles<double>::tau / static_cast<double>(this->sides);

    auto halfAngle = 0.5 * angleStep;

    auto rotation_rad = tau::ToRadians(this->rotation_deg);

    auto startingAngle = rotation_rad - halfAngle;
    
    PointsDouble result;
    result.reserve(this->sides);

    tau::Line2d<double> radial(
        this->center,
        tau::Vector2d<double>(1.0, 0.0));

    for (size_t i = 0; i < this->sides; ++i)
    {
        result.push_back(
            radial.GetRotated_rad(
                startingAngle + (angleStep * static_cast<double>(i)))
                .GetEndPoint(radius));
    }

    return result;
}


} // end namespace draw


template struct pex::Group
    <
        draw::RegularPolygonFields,
        draw::RegularPolygonTemplate,
        draw::RegularPolygonCustom
    >;
