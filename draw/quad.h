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

using ShearGroupMaker = pex::MakeGroup<ShearGroup>;

DECLARE_OUTPUT_STREAM_OPERATOR(Shear)


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
    T<CenterGroupMaker> center;
    T<SizeGroupMaker> size;
    T<ScaleRange> scale;
    T<RotationRange> rotation;
    T<ShearGroupMaker> shear;
    T<PerspectiveGroupMaker> perspective;
    T<pex::MakeSignal> reset;

    static constexpr auto fields = QuadFields<QuadTemplate>::fields;
};


struct Quad: public QuadTemplate<pex::Identity>
{
    static Quad Default()
    {
        return {{
            {{960, 540}},
            {{300, 200}},
            1.0,
            0.0,
            {{0, 0}},
            {{0, 0}},
            {}}};
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


using QuadGroup = pex::Group
<
    QuadFields,
    QuadTemplate,
    Quad
>;


struct QuadModel: public QuadGroup::Model
{
public:
    QuadModel()
        :
        QuadGroup::Model(),
        resetEndpoint_(this, this->reset, &QuadModel::OnReset_)
    {

    }

private:
    void OnReset_()
    {
        this->Set(Quad::Default());
    }

private:
    pex::Endpoint<QuadModel, decltype(QuadGroup::Control::reset)> resetEndpoint_;
};



using QuadControl = typename QuadGroup::Control;
using QuadGroupMaker = pex::MakeGroup<QuadGroup, QuadModel>;


DECLARE_OUTPUT_STREAM_OPERATOR(Quad)


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
    draw::Quad
>;
