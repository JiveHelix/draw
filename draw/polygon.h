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


template<typename T>
struct PolygonFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::center, "center"),
        fields::Field(&T::scale, "scale"),
        fields::Field(&T::rotation, "rotation"),
        fields::Field(&T::points, "points"));
};


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
struct PolygonTemplate
{
    T<CenterGroup> center;
    T<ScaleRange> scale;
    T<RotationRange> rotation;
    T<pex::List<tau::Point2dGroup<double>, 4>> points;

    static constexpr auto fields = PolygonFields<PolygonTemplate>::fields;
    static constexpr auto fieldsTypeName = "Polygon";
};


struct Polygon: public PolygonTemplate<pex::Identity>
{
    using Point = tau::Point2d<double>;

    Polygon()
        :
        PolygonTemplate<pex::Identity>{
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


using PolygonGroup = pex::Group
<
    PolygonFields,
    PolygonTemplate,
    pex::PlainT<Polygon>
>;

using PolygonControl = typename PolygonGroup::Control;


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
        draw::PolygonFields,
        draw::PolygonTemplate,
        pex::PlainT<draw::Polygon>
    >;
