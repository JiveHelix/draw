#pragma once


#include <pex/group.h>
#include <wxpex/modifier.h>

#include "draw/quad.h"
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
class QuadShapeTemplate
{
public:
    // id is read-only to a control
    T<pex::ReadOnly<size_t>> id;
    T<QuadGroupMaker> shape;
    T<LookGroup> look;
    T<NodeSettingsGroup> node;

    static constexpr auto fields =
        ShapeFields<QuadShapeTemplate>::fields;

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
    ShapeFields,
    QuadShapeTemplate,
    QuadShape
>;


struct QuadShapeModel: public QuadShapeGroup::Model
{
public:
    QuadShapeModel();

    void Set(const QuadShape &other);

private:
    detail::PolyShapeId polyShapeId_;
};


struct QuadShapeControl
    :
    public QuadShapeGroup::Control
{
    using QuadShapeGroup::Control::Control;

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


using QuadShapeGroupMaker = pex::MakeGroup
<
    QuadShapeGroup,
    QuadShapeModel,
    QuadShapeControl
>;


DECLARE_OUTPUT_STREAM_OPERATOR(QuadShape)


using QuadListMaker = pex::MakeList<QuadShapeGroupMaker, 1>;
using QuadListControl = pex::ControlSelector<QuadListMaker>;


} // end namespace draw
