#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/range.h>
#include <pex/endpoint.h>
#include <tau/vector2d.h>
#include <tau/line2d.h>
#include "draw/scale.h"
#include "draw/polygon.h"
#include "draw/size.h"
#include "draw/quad_lines.h"


namespace draw
{


using ShearRange =
    pex::MakeRange<double, pex::Limit<-3>, pex::Limit<3>>;


template<template<typename> typename T>
struct ShearTemplate
{
    T<ShearRange> x;
    T<ShearRange> y;

    static constexpr auto fields = AffineFields<ShearTemplate>::fields;
    static constexpr auto fieldsTypeName = "Shear";
};


using ShearGroup = pex::Group
<
    AffineFields,
    ShearTemplate
>;

using Shear = typename ShearGroup::Plain;
using ShearModel = typename ShearGroup::Model;
using ShearControl = typename ShearGroup::Control;

DECLARE_OUTPUT_STREAM_OPERATOR(Shear)
DECLARE_EQUALITY_OPERATORS(Shear)


template<typename T>
struct QuadFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::center, "center"),
        fields::Field(&T::size, "size"),
        fields::Field(&T::scale, "scale"),
        fields::Field(&T::rotation, "rotation"),
        fields::Field(&T::shear, "shear"),
        fields::Field(&T::perspective, "perspective"),
        fields::Field(&T::reset, "reset"));
};


using RotationRange =
    pex::MakeRange<double, pex::Limit<-180>, pex::Limit<180>>;


template<template<typename> typename T>
struct QuadTemplate
{
    T<CenterGroup> center;
    T<SizeGroup> size;
    T<ScaleRange> scale;
    T<RotationRange> rotation;
    T<ShearGroup> shear;
    T<PerspectiveGroup> perspective;
    T<pex::MakeSignal> reset;

    static constexpr auto fields = QuadFields<QuadTemplate>::fields;
    static constexpr auto fieldsTypeName = "Quad";
};


struct QuadGroupTemplates_
{
    struct Plain: public QuadTemplate<pex::Identity>
    {
        Plain()
            :
            QuadTemplate<pex::Identity>{
                {{960, 540}},
                {{300, 200}},
                1.0,
                {},
                {},
                {},
                {}}
        {

        }

        using Affine = Eigen::Matrix<double, 3, 3>;

        Affine MakeTransform() const;
        QuadMatrix GetPerspectiveMatrix() const;
        QuadPoints GetPerspectivePoints() const;
        QuadPoints GetPoints() const;
        double GetSideLength(size_t index) const;
        QuadLines GetLines() const;
        void SetPoints(const QuadPoints &quadPoints);
        bool Contains(const tau::Point2d<double> &point) const;
        bool Contains(const tau::Point2d<double> &point, double margin) const;
        double GetArea() const;

        double GetMarginScale(double margin) const;

    private:
        QuadPoints GetPoints_(double scale_) const;
    };

    template<typename GroupBase>
    struct Model: public GroupBase
    {
    public:
        Model()
            :
            GroupBase(),
            resetEndpoint_(this, this->reset, &Model::OnReset_)
        {

        }

    private:
        void OnReset_()
        {
            this->Set(Plain{});
        }

    private:
        using ResetControl = decltype(GroupBase::ControlType::reset);
        pex::Endpoint<Model, ResetControl> resetEndpoint_;
    };

};

using QuadGroup = pex::Group
<
    QuadFields,
    QuadTemplate,
    QuadGroupTemplates_
>;

using Quad = typename QuadGroup::Plain;
using QuadModel = typename QuadGroup::Model;
using QuadControl = typename QuadGroup::Control;


DECLARE_OUTPUT_STREAM_OPERATOR(Quad)
DECLARE_EQUALITY_OPERATORS(Quad)


} // end namespace draw



extern template struct pex::Group
<
    draw::AffineFields,
    draw::ShearTemplate
>;


extern template struct pex::Group
<
    draw::QuadFields,
    draw::QuadTemplate,
    draw::QuadGroupTemplates_
>;
