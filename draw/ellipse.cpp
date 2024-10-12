#include "draw/ellipse.h"


namespace draw
{


Ellipse::Ellipse()
    :
    EllipseTemplate<pex::Identity>{
        {{400.0, 400.0}},
        200.0,
        300.0,
        45.0,
        1.0}
{

}


bool Ellipse::Contains(const tau::Point2d<double> &point) const
{
    return this->Contains(point, 0.0);
}


bool Ellipse::Contains(const tau::Point2d<double> &point, double margin) const
{
    auto relative = point - this->center;

    // Subtract off the rotation of the ellipse to get the angle relative
    // to the major axis.
    auto rotated = relative.ToVector().Rotate(-this->rotation);

    auto parameter =
        std::atan2(this->major * rotated.y, this->minor * rotated.x);

    auto sine = std::sin(parameter);
    auto cosine = std::cos(parameter);

    auto ellipseExtent =
        tau::Point2d<double>(
            cosine * this->major / 2.0,
            sine * this->minor / 2.0) * this->scale;

    return rotated.Magnitude() <= (ellipseExtent.Magnitude() + margin);
}


PointsDouble Ellipse::GetPoints() const
{
    auto majorAxis = tau::Vector2d<double>(1.0, 0.0).Rotate(this->rotation);
    auto minorAxis = tau::Vector2d<double>(0.0, 1.0).Rotate(this->rotation);

    auto halfMajor = (0.5 * this->major * this->scale * majorAxis).ToPoint();
    auto halfMinor = (0.5 * this->minor * this->scale * minorAxis).ToPoint();

    PointsDouble points;
    points.reserve(4);

    points.push_back(this->center + halfMajor);
    points.push_back(this->center - halfMajor);
    points.push_back(this->center + halfMinor);
    points.push_back(this->center - halfMinor);

    return points;
}


void Ellipse::EditPoint(
    const tau::Point2d<double> &point,
    size_t pointIndex)
{
    auto axis = point - this->center;
    auto axisMagnitude = 2.0 * axis.Magnitude() / this->scale;

    switch (pointIndex)
    {
        case 0:
        case 1:
            // major axis
            this->major = axisMagnitude;
            break;

        case 2:
        case 3:
            // minor axis
            this->minor = axisMagnitude;
            break;

        default:
            throw std::logic_error("Out of range point index");
    }
}


void Ellipse::Draw(DrawContext &context)
{
    wxpex::MaintainTransform maintainTransform(context);

    auto center = this->center;

    // TODO: using arrow operator to access underlying wxGraphicsContext is
    // confusing.
    auto transform = context->GetTransform();
    transform.Translate(center.x, center.y);

    context->SetTransform(transform);
    context->Rotate(tau::ToRadians(this->rotation));
    double ellipseMajor = this->scale * this->major;
    double ellipseMinor = this->scale * this->minor;

    // wx draws the ellipse contained within this rectangle.
    context->DrawEllipse(
        -ellipseMajor / 2.0,
        -ellipseMinor / 2.0,
        ellipseMajor,
        ellipseMinor);
}


} // end namespace draw


template struct pex::Group
<
    draw::EllipseFields,
    draw::EllipseTemplate,
    pex::PlainT<draw::Ellipse>
>;
