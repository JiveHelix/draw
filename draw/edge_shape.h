#pragma once


#include <pex/group.h>
#include <tau/line2d.h>
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/edge.h"


namespace draw
{

template<typename T>
struct EdgeSettingsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::look, "look"));
};


template<template<typename> typename T>
struct EdgeSettingsTemplate
{
    T<LookGroup> look;

    static constexpr auto fields =
        EdgeSettingsFields<EdgeSettingsTemplate>::fields;

    static constexpr auto fieldsTypeName = "EdgeShape";
};


using EdgeSettingsGroup = pex::Group
<
    EdgeSettingsFields,
    EdgeSettingsTemplate
>;

using EdgeSettings = typename EdgeSettingsGroup::Plain;
using EdgeShapeModel = typename EdgeSettingsGroup::Model;
using EdgeShapeControl = typename EdgeSettingsGroup::Control;


DECLARE_EQUALITY_OPERATORS(EdgeSettings)


class EdgeShape
    :
    public DrawnShape
{
public:
    using Edges = std::vector<Edge>;

    EdgeShape() = default;

    EdgeShape(
        const EdgeSettings &settings,
        const Edges &edges);

    void Draw(DrawContext &context) override;

    EdgeSettings settings_;
    Edges edges_;
};


} // end namespace draw


extern template struct pex::Group
<
    draw::EdgeSettingsFields,
    draw::EdgeSettingsTemplate,
    pex::PlainT<draw::EdgeSettings>
>;
