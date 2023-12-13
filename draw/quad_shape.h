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
        fields::Field(&T::id, "id"),
        fields::Field(&T::shape, "shape"),
        fields::Field(&T::look, "look"));
};


template<template<typename> typename T>
class QuadShapeTemplate
{
public:
    // id is read-only to a control
    T<pex::Filtered<size_t, pex::NoFilter, pex::GetTag>> id;
    T<QuadGroupMaker> shape;
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
    QuadShape(size_t id_, const Quad &quad_, const Look &look_);

    void Draw(wxpex::GraphicsContext &context) override;
};


using QuadShapeGroup = pex::Group
<
    QuadShapeFields,
    QuadShapeTemplate,
    QuadShape
>;


struct QuadShapeModel: public QuadShapeGroup::Model
{
public:
    QuadShapeModel();

    void Set(const QuadShape &other);
};


using QuadShapeControl = typename QuadShapeGroup::Control;
using QuadShapeGroupMaker = pex::MakeGroup<QuadShapeGroup, QuadShapeModel>;


DECLARE_OUTPUT_STREAM_OPERATOR(QuadShape)


using QuadListMaker = pex::MakeList<QuadShapeGroupMaker, 1>;
using QuadListControl = pex::ControlSelector<QuadListMaker>;


} // end namespace draw
