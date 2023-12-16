#include <fmt/core.h>
#include "draw/polygon_shape.h"
#include "draw/quad_shape.h"
#include "draw/quad_brain.h"
#include "draw/views/look_view.h"
#include "draw/views/quad_view.h"

// #define ENABLE_BLOCK_TIMER
#include <jive/block_timer.h>


namespace draw
{


QuadShape::QuadShape(const Quad &quad, const Look &look_)
    :
    QuadShapeTemplate<pex::Identity>({0, quad, look_, {}})
{

}


QuadShape::QuadShape(size_t id_, const Quad &quad, const Look &look_)
    :
    QuadShapeTemplate<pex::Identity>({id_, quad, look_, {}})
{

}


void QuadShape::Draw(wxpex::GraphicsContext &context)
{
    BLOCK_TIMER(jive::TimeValue());

    if (this->shape.size.GetArea() < 0.5)
    {
        return;
    }

    ConfigureLook(context, this->look);
    DrawPolygon(context, this->shape.GetPoints());
}


QuadShapeModel::QuadShapeModel()
    :
    QuadShapeGroup::Model(),
    polyShapeId_()
{
    this->id.Set(this->polyShapeId_.Get());
}


void QuadShapeModel::Set(const QuadShape &other)
{
    // Do not change the id.
    this->shape.Set(other.shape);
    this->look.Set(other.look);
}


std::unique_ptr<Drag> QuadShapeControl::ProcessMouseDown(
    const tau::Point2d<int> &click,
    const wxpex::Modifier &modifier,
    CursorControl cursor)
{
    return ::draw::ProcessMouseDown
        <
            DragRotateQuadPoint,
            DragQuadPoint,
            DragQuadLine,
            DragQuad
        >(*this, click, modifier, cursor);
}


std::string QuadShapeControl::GetName() const
{
    return fmt::format("Quad {}", this->id.Get());
}


wxWindow * QuadShapeControl::CreateShapeView(wxWindow *parent) const
{
    return new QuadView(
        parent,
        "Quad",
        this->shape,
        wxpex::LayoutOptions{});
}


wxWindow * QuadShapeControl::CreateLookView(wxWindow *parent) const
{
    return new LookView(parent, "Look", this->look, wxpex::LayoutOptions{});
}


} // end namespace draw
