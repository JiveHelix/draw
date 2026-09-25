#pragma once


#include <pex/group.h>
#include <tau/line2d.h>
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/edge.h"


namespace draw
{


template<template<typename> typename T>
struct EdgeSettingsSchema
{
    T<LookGroup> look;

    static constexpr auto fieldsTypeName = "EdgeShape";
};


using EdgeSettingsGroup = pex::Group<EdgeSettingsSchema>;

using EdgeSettings = typename EdgeSettingsGroup::Plain;
using EdgeShapeModel = typename EdgeSettingsGroup::Model;
using EdgeShapeControl = typename EdgeSettingsGroup::DefaultControl;


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
    draw::EdgeSettingsSchema,
    pex::PlainT<draw::EdgeSettings>
>;
