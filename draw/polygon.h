#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/range.h>
#include <tau/vector2d.h>
#include "draw/oddeven.h"
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


using PolygonPoints = std::vector<tau::Point2d<double>>;


using CenterGroupMaker = pex::MakeGroup<tau::Point2dGroup<double>>;

using RotationRange =
    pex::MakeRange<double, pex::Limit<-180>, pex::Limit<180>>;


struct CenteredPoints
{
    tau::Point2d<double> center;
    PolygonPoints points;

    CenteredPoints(const PolygonPoints &points_);
};


template<template<typename> typename T>
struct PolygonTemplate
{
    T<CenterGroupMaker> center;
    T<ScaleRange> scale;
    T<RotationRange> rotation;
    T<PolygonPoints> points;

    static constexpr auto fields = PolygonFields<PolygonTemplate>::fields;
};


struct Polygon: public PolygonTemplate<pex::Identity>
{
    Polygon()
        :
        PolygonTemplate<pex::Identity>{
            {{0.0, 0.0}},
            1.0,
            0.0,
            {}}
    {

    }

    static Polygon Default()
    {
        return {};
    }

    Polygon(const CenteredPoints &centeredPoints);
    Polygon(const PolygonPoints &points_);
    PolygonPoints GetPoints() const;
    PolygonLines GetLines() const;
    bool Contains(const tau::Point2d<double> &point);
};



using PolygonGroup = pex::Group
<
    PolygonFields,
    PolygonTemplate,
    Polygon
>;

using PolygonControl = typename PolygonGroup::Control;

using PolygonGroupMaker = pex::MakeGroup<PolygonGroup>;

DECLARE_OUTPUT_STREAM_OPERATOR(Polygon)
DECLARE_EQUALITY_OPERATORS(Polygon)



} // end namespace draw



extern template struct pex::Group
    <
        draw::PolygonFields,
        draw::PolygonTemplate,
        draw::Polygon
    >;
