#pragma once


#include <pex/group.h>
#include <tau/vector2d.h>
#include <wxpex/combo_box.h>
#include <draw/points.h>
#include <draw/look.h>
#include <draw/shapes.h>


namespace draw
{


enum class CurveStyle: int
{
    line = 0,
    tangentSpline,
    gcdcSpline
};


struct CurveStyleChoices
{
    using Type = CurveStyle;
    static std::vector<CurveStyle> GetChoices();
};


using CurveStyleSelect = pex::MakeSelect<CurveStyleChoices>;
using CurveStyleModel = pex::ModelTailor<CurveStyleSelect>;
using CurveStyleControl = pex::ControlTailor<CurveStyleSelect>;

struct CurveStyleConverter
{
    static std::string ToString(CurveStyle curveStyle);
};


std::ostream & operator<<(std::ostream &, CurveStyle);


using CurveStyleComboBox =
    wxpex::ComboBox<CurveStyleControl, CurveStyleConverter>;


template<template<typename> typename T>
struct SegmentsSettingsSchema
{
    T<CurveStyleSelect> curveStyle;
    T<bool> drawPoints;
    T<LookGroup> look;

    static constexpr auto fieldsTypeName = "SegmentsSettings";
};


using SegmentsSettingsGroup = pex::Group<SegmentsSettingsSchema>;

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
