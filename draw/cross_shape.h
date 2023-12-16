#pragma once


#include <pex/group.h>
#include "draw/cross.h"
#include "draw/look.h"
#include "draw/shapes.h"


namespace draw
{


template<typename T>
struct CrossShapeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::cross, "cross"),
        fields::Field(&T::look, "look"));
};


template<template<typename> typename T>
class CrossShapeTemplate
{
public:
    T<CrossGroup> cross;
    T<LookGroup> look;

    static constexpr auto fields =
        CrossShapeFields<CrossShapeTemplate>::fields;

    static constexpr auto fieldsTypeName = "CrossShape";
};


void DrawCross(
    wxpex::GraphicsContext &context,
    const Cross &cross);


class CrossShape:
    public Shape,
    public CrossShapeTemplate<pex::Identity>
{
public:
    CrossShape() = default;
    CrossShape(const Cross &cross_, const Look &look_);
    void Draw(wxpex::GraphicsContext &context) override;
};


using CrossShapeGroup = pex::Group
<
    CrossShapeFields,
    CrossShapeTemplate,
    CrossShape
>;

using CrossShapeModel = typename CrossShapeGroup::Model;
using CrossShapeControl = typename CrossShapeGroup::Control;



} // end namespace draw
