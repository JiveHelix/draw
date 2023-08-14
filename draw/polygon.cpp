#include "draw/polygon.h"


namespace draw
{


CenteredPoints::CenteredPoints(const PolygonPoints &points_)
    :
    center(0, 0),
    points(points_)
{
    if (points_.empty())
    {
        return;
    }

    // Recompute the points from their common center.
    if (points_.size() == 1)
    {
        this->center = this->points.front();
        this->points.front().x = 0;
        this->points.front().y = 0;
        return;
    }

    if (points_.size() == 2)
    {
        this->center.x = (points_.front().x - points_.back().x) / 2;
        this->center.y = (points_.front().y - points_.back().y) / 2;

        for (auto &point: this->points)
        {
            point -= this->center;
        }

        return;
    }

    using Eigen::Index;

    Eigen::VectorX<double> weight(Index(points_.size()));
    Eigen::RowVectorX<double> x(Index(points_.size()));
    Eigen::RowVectorX<double> y(Index(points_.size()));

    Index i = 0;

    for (; i < Index(points_.size() - 1); ++i)
    {
        const auto &start = points_[size_t(i)];
        const auto &end = points_[size_t(i + 1)];

        x(i) = (start.x + end.x) / 2;
        y(i) = (start.y + end.y) / 2;
        weight(i) = start.Distance(end);
    }

    // Average the first and last points.
    const auto &start = points_.back();
    const auto &end = points_.front();
    x(i) = (start.x + end.x) / 2;
    y(i) = (start.y + end.y) / 2;
    weight(i) = start.Distance(end);

    double sumOfWeights = weight.sum();

    if (sumOfWeights < 1e-3)
    {
        this->center = points_.front();
    }
    else
    {
        this->center = tau::Point2d<double>(
            (x * weight)(0) / sumOfWeights,
            (y * weight)(0) / sumOfWeights);
    }

    // Recompute the points from their common center.
    for (auto &point: this->points)
    {
        point -= this->center;
    }
}


Polygon::Polygon(const CenteredPoints &centeredPoints)
{
    this->center = centeredPoints.center;
    this->scale = 1.0;
    this->rotation = 0.0;
    this->points = centeredPoints.points;
}


Polygon::Polygon(const PolygonPoints &points_)
    :
    Polygon(CenteredPoints(points_))
{

}


PolygonPoints Polygon::GetPoints() const
{
    PolygonPoints result = this->points;

    for (auto &point: result)
    {
        point = point.ToVector().Rotate(this->rotation);
        point *= this->scale;
        point += this->center;
    }

    return result;
}


PolygonLines Polygon::GetLines() const
{
    return {this->GetPoints()};
}


bool Polygon::Contains(const tau::Point2d<double> &point)
{
    if (this->points.size() < 3)
    {
        return false;
    }

    return oddeven::Contains(this->GetPoints(), point);
}


} // end namespace draw


template struct pex::Group
    <
        draw::PolygonFields,
        draw::PolygonTemplate,
        draw::Polygon
    >;
