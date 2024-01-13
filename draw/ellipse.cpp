#include "draw/ellipse.h"


namespace draw
{


Ellipse Ellipse::Default()
{
    return {{
        {{400.0, 400.0}},
        200.0,
        300.0,
        45.0,
        1.0}};
}


bool Ellipse::Contains(const tau::Point2d<double> &point) const
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

    return rotated.Magnitude() <= ellipseExtent.Magnitude();
}


} // end namespace draw


template struct pex::Group
<
    draw::EllipseFields,
    draw::EllipseTemplate,
    pex::PlainT<draw::Ellipse>
>;
