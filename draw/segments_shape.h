#pragma once


#include <pex/group.h>
#include <tau/vector2d.h>
#include "draw/points.h"
#include "draw/look.h"
#include "draw/shapes.h"


namespace draw
{


template<typename T>
struct SegmentsSettingsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::isSpline, "isSpline"),
        fields::Field(&T::look, "look"));
};


template<template<typename> typename T>
struct SegmentsSettingsTemplate
{
    T<bool> isSpline;
    T<LookGroup> look;

    static constexpr auto fields =
        SegmentsSettingsFields<SegmentsSettingsTemplate>::fields;

    static constexpr auto fieldsTypeName = "SegmentsSettings";
};


using SegmentsSettingsGroup = pex::Group
<
    SegmentsSettingsFields,
    SegmentsSettingsTemplate
>;

using SegmentsSettings = typename SegmentsSettingsGroup::Plain;
using SegmentsSettingsModel = typename SegmentsSettingsGroup::Model;
using SegmentsSettingsControl = typename SegmentsSettingsGroup::DefaultControl;


DECLARE_EQUALITY_OPERATORS(SegmentsSettings)


class SegmentsShape
    :
    public DrawnShape
{
public:
    SegmentsShape() = default;

    SegmentsShape(
        const SegmentsSettings &settings,
        const PointsDouble &points);

    void Draw(DrawContext &context) override;

private:
    SegmentsSettings segmentsSettings_;
    PointsDouble points_;
};


} // end namespace draw
