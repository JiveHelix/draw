#pragma once


#include <pex/group.h>
#include "draw/polygon.h"
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/oddeven.h"


namespace draw
{


template<typename T>
struct PolygonShapeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::id, "id"),
        fields::Field(&T::shape, "shape"),
        fields::Field(&T::look, "look"));
};


template<template<typename> typename T>
class PolygonShapeTemplate
{
public:
    // id is read-only to a control
    T<pex::Filtered<size_t, pex::NoFilter, pex::GetTag>> id;
    T<PolygonGroupMaker> shape;
    T<LookGroupMaker> look;

    static constexpr auto fields =
        PolygonShapeFields<PolygonShapeTemplate>::fields;

    static constexpr auto fieldsTypeName = "PolygonShape";
};


void DrawPolygon(
    wxpex::GraphicsContext &context,
    const Points &points);


class PolygonShape:
    public Shape,
    public PolygonShapeTemplate<pex::Identity>
{
public:
    PolygonShape() = default;
    PolygonShape(size_t id_, const Polygon &polygon_, const Look &look_);
    PolygonShape(const Polygon &polygon_, const Look &look_);

    void Draw(wxpex::GraphicsContext &context) override;
};


using PolygonShapeGroup = pex::Group
<
    PolygonShapeFields,
    PolygonShapeTemplate,
    PolygonShape
>;


struct PolygonShapeModel: public PolygonShapeGroup::Model
{
public:
    PolygonShapeModel();

    void Set(const PolygonShape &other);
};


using PolygonShapeControl = typename PolygonShapeGroup::Control;

using PolygonShapeGroupMaker =
    pex::MakeGroup<PolygonShapeGroup, PolygonShapeModel>;


using PolygonListMaker = pex::MakeList<PolygonShapeGroupMaker, 1>;
using PolygonListControl = pex::ControlSelector<PolygonListMaker>;


} // end namespace draw
