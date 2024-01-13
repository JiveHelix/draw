#pragma once


#include <pex/group.h>
#include "draw/ellipse.h"
#include "draw/look.h"
#include "draw/shapes.h"
#include "draw/polygon_shape.h"


namespace draw
{


template<typename T>
struct EllipseShapeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::ellipse, "ellipse"),
        fields::Field(&T::look, "look"));
};


template<template<typename> typename T>
class EllipseShapeTemplate
{
public:
    T<draw::EllipseGroup> ellipse;
    T<draw::LookGroup> look;

    static constexpr auto fields =
        EllipseShapeFields<EllipseShapeTemplate>::fields;

    static constexpr auto fieldsTypeName = "EllipseShape";
};


class EllipseShape:
    public DrawnShape,
    public EllipseShapeTemplate<pex::Identity>
{
public:
    EllipseShape() = default;

    EllipseShape(const Ellipse &ellipse_, const Look &look_)
        :
        EllipseShapeTemplate<pex::Identity>({ellipse_, look_})
    {

    }

    void Draw(wxpex::GraphicsContext &context) override
    {
        wxpex::MaintainTransform maintainTransform(context);
        ConfigureLook(context, this->look);
        auto center = this->ellipse.center;
        auto transform = context->GetTransform();
        transform.Translate(center.x, center.y);
        context->SetTransform(transform);
        context->Rotate(tau::ToRadians(this->ellipse.rotation));
        double ellipseMajor = this->ellipse.scale * this->ellipse.major;
        double ellipseMinor = this->ellipse.scale * this->ellipse.minor;

        context->DrawEllipse(
            -ellipseMajor / 2.0,
            -ellipseMinor / 2.0,
            ellipseMajor,
            ellipseMinor);
    }
};


using EllipseShapeGroup = pex::Group
<
    EllipseShapeFields,
    EllipseShapeTemplate,
    pex::PlainT<EllipseShape>
>;

using EllipseShapeModel = typename EllipseShapeGroup::Model;
using EllipseShapeControl = typename EllipseShapeGroup::Control;


} // end namespace draw



extern template struct pex::Group
    <
        draw::EllipseShapeFields,
        draw::EllipseShapeTemplate,
        pex::PlainT<draw::EllipseShape>
    >;
