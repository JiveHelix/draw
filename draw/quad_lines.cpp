#include "draw/quad_lines.h"


namespace draw
{


QuadMatrix PointsToMatrix(
    const QuadPoint &topLeft,
    const QuadPoint &topRight,
    const QuadPoint &bottomRight,
    const QuadPoint &bottomLeft)
{
    QuadMatrix result;

    result(0, 0) = topLeft.x;
    result(1, 0) = topLeft.y;

    result(0, 1) = topRight.x;
    result(1, 1) = topRight.y;

    result(0, 2) = bottomRight.x;
    result(1, 2) = bottomRight.y;

    result(0, 3) = bottomLeft.x;
    result(1, 3) = bottomLeft.y;

    // Homogeneous points have w = 1.
    result.array().row(2) = 1.0;

    return result;
}


QuadMatrix PointsToMatrix(const QuadPoints &points)
{
    return PointsToMatrix(
        points.at(0),
        points.at(1),
        points.at(2),
        points.at(3));
}


QuadPoints MatrixToPoints(const QuadMatrix &pointsMatrix)
{
    QuadPoints result(4);
    result[0] = QuadPoint(pointsMatrix(0, 0), pointsMatrix(1, 0));
    result[1] = QuadPoint(pointsMatrix(0, 1), pointsMatrix(1, 1));
    result[2] = QuadPoint(pointsMatrix(0, 2), pointsMatrix(1, 2));
    result[3] = QuadPoint(pointsMatrix(0, 3), pointsMatrix(1, 3));

    return result;
}


const QuadLines::Line & QuadLines::operator[](size_t index) const
{
    switch (index)
    {
        case 0:
            return this->top;

        case 1:
            return this->right;

        case 2:
            return this->bottom;

        case 3:
            return this->left;

        default:
            throw std::out_of_range("QuadLines only has 4 items");
    }
}


QuadPoint QuadLines::GetMidpoint(
    const QuadPoint &first,
    const QuadPoint &second)
{
    return (first + second) / 2.0;
}


QuadLines::QuadLines(
    const QuadPoint &topLeft,
    const QuadPoint &topRight,
    const QuadPoint &bottomRight,
    const QuadPoint &bottomLeft)
    :
    top(topLeft, topRight),
    right(topRight, bottomRight),
    bottom(bottomRight, bottomLeft),
    left(bottomLeft, topLeft)
{
    this->top.point = GetMidpoint(topLeft, topRight);
    this->bottom.point = GetMidpoint(bottomLeft, bottomRight);
    this->right.point = GetMidpoint(topRight, bottomRight);
    this->left.point = GetMidpoint(topLeft, bottomLeft);
}


QuadLines::QuadLines(double halfWidth, double halfHeight)
    :
    QuadLines(
        QuadPoint(-halfWidth, -halfHeight),
        QuadPoint(halfWidth, -halfHeight),
        QuadPoint(halfWidth, halfHeight),
        QuadPoint(-halfWidth, halfHeight))
{

}


QuadLines::QuadLines(const tau::Size<double> &size)
    :
    QuadLines(size.width / 2.0, size.height / 2.0)
{

}


QuadLines::QuadLines(const QuadPoints &points)
    :
    QuadLines(points.at(0), points.at(1), points.at(2), points.at(3))
{

}


QuadLines::QuadLines(const QuadMatrix &pointsMatrix)
    :
    QuadLines(MatrixToPoints(pointsMatrix))
{

}


QuadMatrix QuadLines::GetMatrix() const
{
    QuadMatrix result;

    auto topLeft = this->top.Intersect(this->left);
    auto topRight = this->top.Intersect(this->right);
    auto bottomRight = this->bottom.Intersect(this->right);
    auto bottomLeft = this->bottom.Intersect(this->left);

    result(0, 0) = topLeft.x;
    result(1, 0) = topLeft.y;

    result(0, 1) = topRight.x;
    result(1, 1) = topRight.y;

    result(0, 2) = bottomRight.x;
    result(1, 2) = bottomRight.y;

    result(0, 3) = bottomLeft.x;
    result(1, 3) = bottomLeft.y;

    // Homogeneous points have w = 1.
    result.array().row(2) = 1.0;

    return result;
}


QuadLines QuadLines::ApplyPerspective(const Perspective &perspective) const
{
    QuadLines result{};

    result.top = this->top.GetRotated(perspective.x / 2.0);
    result.bottom = this->bottom.GetRotated(-perspective.x / 2.0);

    result.left = this->left.GetRotated(perspective.y / 2.0);
    result.right = this->right.GetRotated(-perspective.y / 2.0);

    return result;
}


Perspective QuadLines::GetPerspective() const
{
    return {{
        this->top.GetAngleDegrees() - this->bottom.GetAngleDegrees(),
        this->left.GetAngleDegrees() - this->right.GetAngleDegrees()}};
}


QuadLines QuadLines::UndoPerspective()
{
    Perspective perspective = this->GetPerspective();
    double halfX = perspective.x / 2.0;
    double halfY = perspective.y / 2.0;

    QuadLines result;

    result.top = this->top.GetRotated(-halfX);
    result.bottom = this->bottom.GetRotated(halfX);

    result.left = this->left.GetRotated(-halfY);
    result.right = this->bottom.GetRotated(halfY);

    return result;
}


tau::Size<double> QuadLines::GetSize() const
{
    auto topLeft = this->top.Intersect(this->left);
    auto bottomRight = this->bottom.Intersect(this->right);

    return tau::Size<double>(
        bottomRight.x - topLeft.x,
        bottomRight.y - topLeft.y);
}


} // end namespace draw


template struct pex::Group
<
    draw::AffineFields,
    draw::PerspectiveTemplate
>;
