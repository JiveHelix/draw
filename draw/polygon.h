#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/range.h>
#include <tau/vector2d.h>
#include "draw/points.h"
#include "draw/polygon_lines.h"
#include "draw/scale.h"


namespace draw
{


using CenterGroup = tau::Point2dGroup<double>;

using RotationRange =
    pex::MakeRange<double, pex::Limit<-180>, pex::Limit<180>>;


struct CenteredPoints
{
    tau::Point2d<double> center;
    PointsDouble points;

    CenteredPoints(const PointsDouble &points_);

    CenteredPoints(
        const tau::Point2d<double> &pointsCenter,
        const tau::Point2d<double> &transformCenter,
        double width,
        double height);
};


template<template<typename> typename T>
struct PolygonSchema
{
    T<CenterGroup> center;
    T<ScaleRange> scale;
    T<RotationRange> rotation;
    T<pex::List<tau::Point2dGroup<double>, 4>> points;

    static constexpr auto fieldsTypeName = "Polygon";
};


struct Polygon: public PolygonSchema<pex::Identity>
{
    using Point = tau::Point2d<double>;

    Polygon()
        :
        PolygonSchema<pex::Identity>{
            Point(0.0, 0.0),
            1.0,
            0.0,
            PointsDouble{
                Point(-100.0, -100.0),
                Point(100.0, -100.0),
                Point(100.0, 100.0),
                Point(-100.0, 100.0)}}
    {

    }

    Polygon(const CenteredPoints &centeredPoints);
    Polygon(const PointsDouble &points_);
    PointsDouble GetPoints() const;
    PolygonLines GetLines() const;
    bool Contains(const tau::Point2d<double> &point) const;
    bool Contains(const tau::Point2d<double> &point, double margin) const;
    double GetRadius() const;
    double GetMarginScale(double margin) const;

private:
    PointsDouble GetPoints_(double scale_) const;
};


struct PolygonFinisher
{
    using Plain = Polygon;
};

using PolygonGroup = pex::Group
<
    PolygonSchema,
    PolygonFinisher
>;

using PolygonControl = typename PolygonGroup::DefaultControl;


inline
std::ostream & operator<<(std::ostream &output, const Polygon &polygon)
{
    output << fields::DescribeCompact(polygon) << " points: ";

    auto points = polygon.GetPoints();
    for (auto &point: points)
    {
        output << point << ", ";
    }

    return output;
}

DECLARE_EQUALITY_OPERATORS(Polygon)



} // end namespace draw



extern template struct pex::Group
    <
        draw::PolygonSchema,
        draw::PolygonFinisher
    >;
