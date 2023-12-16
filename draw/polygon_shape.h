#pragma once


#include <pex/group.h>
#include <wxpex/modifier.h>

#include "draw/polygon.h"
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/oddeven.h"
#include "draw/drag.h"
#include "draw/views/pixel_view_settings.h"
#include "draw/detail/poly_shape_id.h"
#include "draw/node_settings.h"


namespace draw
{


template<template<typename> typename T>
class PolygonShapeTemplate
{
public:
    // id is read-only to a control
    T<pex::ReadOnly<size_t>> id;
    T<PolygonGroup> shape;
    T<LookGroup> look;
    T<NodeSettingsGroup> node;

    static constexpr auto fields =
        ShapeFields<PolygonShapeTemplate>::fields;

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
    ShapeFields,
    PolygonShapeTemplate,
    PolygonShape
>;


struct PolygonShapeModel: public PolygonShapeGroup::Model
{
public:
    PolygonShapeModel();

    void Set(const PolygonShape &other);

private:
    detail::PolyShapeId polyShapeId_;
};


struct PolygonShapeControl
    :
    public PolygonShapeGroup::Control
{
    static constexpr bool handlesControlClick = true;
    static constexpr bool handlesAltClick = true;

    using PolygonShapeGroup::Control::Control;

    bool ProcessControlClick(const tau::Point2d<int> &click);

    bool ProcessAltClick(
        PointsIterator foundPoint,
        Points &points);

    std::unique_ptr<Drag> ProcessMouseDown(
        const tau::Point2d<int> &click,
        const wxpex::Modifier &modifier,
        CursorControl cursor);

    NodeSettingsControl & GetNode()
    {
        return this->node;
    }

    wxWindow * CreateShapeView(wxWindow *parent) const;
    wxWindow * CreateLookView(wxWindow *parent) const;
    std::string GetName() const;
};


using PolygonShapeGroupMaker = pex::MakeGroup
<
    PolygonShapeGroup,
    PolygonShapeModel,
    PolygonShapeControl
>;


using PolygonListMaker = pex::MakeList<PolygonShapeGroupMaker, 1>;
using PolygonListControl = pex::ControlSelector<PolygonListMaker>;


} // end namespace draw
