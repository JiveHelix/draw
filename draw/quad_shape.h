#pragma once


#include <pex/group.h>
#include "draw/quad.h"
#include "draw/look.h"
#include "draw/shapes.h"


namespace draw
{


template<typename T>
struct QuadShapeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::quad, "quad"),
        fields::Field(&T::look, "look"));
};


template<template<typename> typename T>
class QuadShapeTemplate
{
public:
    T<QuadGroupMaker> quad;
    T<LookGroupMaker> look;

    static constexpr auto fields =
        QuadShapeFields<QuadShapeTemplate>::fields;

    static constexpr auto fieldsTypeName = "QuadShape";
};


class QuadShape:
    public Shape,
    public QuadShapeTemplate<pex::Identity>
{
public:
    QuadShape() = default;
    QuadShape(const Quad &quad_, const Look &look_);
    void Draw(wxpex::GraphicsContext &context) override;
};


using QuadShapeGroup = pex::Group
<
    QuadShapeFields,
    QuadShapeTemplate,
    QuadShape
>;

using QuadShapeModel = typename QuadShapeGroup::Model;
using QuadShapeControl = typename QuadShapeGroup::Control;


DECLARE_OUTPUT_STREAM_OPERATOR(QuadShape)


} // end namespace draw
