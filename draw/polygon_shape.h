#pragma once


#include <pex/group.h>
#include "draw/polygon.h"
#include "draw/look.h"
#include "draw/shapes.h"


namespace draw
{


template<typename T>
struct PolygonShapeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::polygon, "polygon"),
        fields::Field(&T::look, "look"));
};


template<template<typename> typename T>
class PolygonShapeTemplate
{
public:
    T<PolygonGroupMaker> polygon;
    T<LookGroupMaker> look;

    static constexpr auto fields =
        PolygonShapeFields<PolygonShapeTemplate>::fields;

    static constexpr auto fieldsTypeName = "PolygonShape";
};


void DrawPolygon(
    wxpex::GraphicsContext &context,
    const PolygonPoints &points);


class PolygonShape:
    public Shape,
    public PolygonShapeTemplate<pex::Identity>
{
public:
    PolygonShape() = default;
    PolygonShape(const Polygon &polygon_, const Look &look_);
    void Draw(wxpex::GraphicsContext &context) override;
};


using PolygonShapeGroup = pex::Group
<
    PolygonShapeFields,
    PolygonShapeTemplate,
    PolygonShape
>;

using PolygonShapeModel = typename PolygonShapeGroup::Model;
using PolygonShapeControl = typename PolygonShapeGroup::Control;



} // end namespace draw
