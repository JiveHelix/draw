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
struct ShearSchema
{
    T<ShearRange> x;
    T<ShearRange> y;

    static constexpr auto fieldsTypeName = "Shear";
};


using ShearGroup = pex::Group<ShearSchema>;

using Shear = typename ShearGroup::Plain;
using ShearModel = typename ShearGroup::Model;
using ShearControl = typename ShearGroup::DefaultControl;

DECLARE_OUTPUT_STREAM_OPERATOR(Shear)
DECLARE_EQUALITY_OPERATORS(Shear)


using RotationRange =
    pex::MakeRange<double, pex::Limit<-180>, pex::Limit<180>>;


template<template<typename> typename T>
struct QuadSchema
{
    T<CenterGroup> center;
    T<SizeGroup> size;
    T<ScaleRange> scale;
    T<RotationRange> rotation;
    T<ShearGroup> shear;
    T<PerspectiveGroup> perspective;
    T<pex::MakeSignal> reset;

    static constexpr auto fieldsTypeName = "Quad";
};


struct QuadGroupFinisher
{
    struct Plain: public QuadSchema<pex::Identity>
    {
        using Point = typename CenterGroup::Plain;
        using Size = typename SizeGroup::Plain;

        Plain()
            :
            QuadSchema<pex::Identity>{
                Point(960, 540),
                Size(300, 200),
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

    template<typename Base>
    struct Model: public Base
    {
    public:
        Model()
            :
            Base(),
            resetEndpoint_(this, this->reset, &Model::OnReset_)
        {

        }

    private:
        void OnReset_()
        {
            this->Set(Plain{});
        }

    private:
        using ResetControl = ::pex::control::DefaultSignal;
        pex::Endpoint<Model, ResetControl> resetEndpoint_;
    };
};


using QuadGroup = pex::Group
<
    QuadSchema,
    QuadGroupFinisher
>;

using Quad = typename QuadGroup::Plain;
using QuadModel = typename QuadGroup::Model;
using QuadControl = typename QuadGroup::DefaultControl;


DECLARE_OUTPUT_STREAM_OPERATOR(Quad)
DECLARE_EQUALITY_OPERATORS(Quad)


} // end namespace draw



extern template struct pex::Group<draw::ShearSchema>;


extern template struct pex::Group
<
    draw::QuadSchema,
    draw::QuadGroupFinisher
>;
