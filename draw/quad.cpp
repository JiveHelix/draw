#include "draw/quad.h"
#include <tau/rotation.h>


namespace draw
{


Quad::Affine Quad::MakeTransform() const
{
    Affine scale_ = Affine::Identity();
    scale_(0, 0) = this->scale;
    scale_(1, 1) = this->scale;

    Affine rotate = tau::MakeAxial<2, double>(this->rotation);
    Affine shear_ = Affine::Identity();
    shear_(1, 0) = this->shear.y;
    shear_(0, 1) = this->shear.x;

    Affine translate = Affine::Identity();
    translate(0, 2) = this->center.x;
    translate(1, 2) = this->center.y;

    return translate * scale_ * rotate * shear_;
}


QuadMatrix Quad::GetPerspectiveMatrix() const
{
    auto quadLines = QuadLines(this->size);
    auto perspectiveLines = quadLines.ApplyPerspective(this->perspective);
    return perspectiveLines.GetMatrix();
}

QuadPoints Quad::GetPerspectivePoints() const
{
    return MatrixToPoints(this->GetPerspectiveMatrix());
}

QuadPoints Quad::GetPoints() const
{
    Affine transform = this->MakeTransform();

    return MatrixToPoints(transform * this->GetPerspectiveMatrix());
}


QuadPoints Quad::GetPoints_(double scale_) const
{
    auto scaledQuad = *this;
    scaledQuad.scale = scale_;

    return scaledQuad.GetPoints();
}


double Quad::GetSideLength(size_t index) const
{
    QuadPoints points = this->GetPoints();

    switch (index)
    {
        case 0:
            return points[0].Distance(points[1]);

        case 1:
            return points[1].Distance(points[2]);

        case 2:
            return points[2].Distance(points[3]);

        case 3:
            return points[3].Distance(points[0]);

        default:
            throw std::logic_error("Only 4 sides");
    }
}

QuadLines Quad::GetLines() const
{
    return QuadLines(this->GetPoints());
}

void Quad::SetPoints(const QuadPoints &quadPoints)
{
    Affine inverseTransform = this->MakeTransform().inverse();
    auto pointsMatrix = PointsToMatrix(quadPoints);
    QuadMatrix untransformed = inverseTransform * pointsMatrix;
    auto quadLines = QuadLines(untransformed);
    this->perspective = quadLines.GetPerspective();
    auto unperspective = quadLines.UndoPerspective();
    this->size = unperspective.GetSize();
}


bool Quad::Contains(const tau::Point2d<double> &point) const
{
    return oddeven::Contains(this->GetPoints(), point);
}


bool Quad::Contains(const tau::Point2d<double> &point, double margin) const
{
    return oddeven::Contains(
        this->GetPoints_(this->GetMarginScale(margin)),
        point);
}


double Quad::GetMarginScale(double margin) const
{
    return this->scale + (margin / this->size.Magnitude());
}


double Quad::GetArea() const
{
    auto points = this->GetPoints();
    auto bisecting = tau::Line2d<double>(points[0], points[2]);
    auto bisectingLength = (points[2] - points[0]).Magnitude();
    auto upperHeight = bisecting.DistanceToPoint(points[1]);
    auto lowerHeight = bisecting.DistanceToPoint(points[3]);

    return (upperHeight + lowerHeight) * bisectingLength / 2.0;
}


} // end namespace draw


template struct pex::Group
<
    draw::AffineFields,
    draw::ShearTemplate
>;


template struct pex::Group
<
    draw::QuadFields,
    draw::QuadTemplate,
    draw::Quad
>;
