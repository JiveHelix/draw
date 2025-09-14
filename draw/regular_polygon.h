#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/range.h>
#include <pex/endpoint.h>
#include <tau/vector2d.h>
#include "draw/points.h"
#include "draw/polygon_lines.h"


namespace draw
{


template<typename T>
struct RegularPolygonFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::center, "center"),
        fields::Field(&T::radius, "radius"),
        fields::Field(&T::sides, "sides"),
        fields::Field(&T::rotation_deg, "rotation (deg)"));
};


using CenterGroup = tau::Point2dGroup<double>;

using RotationRange =
    pex::MakeRange<double, pex::Limit<-180>, pex::Limit<180>>;

using SidesRange =
    pex::MakeRange<size_t, pex::Limit<3>, pex::Limit<36>>;

using RadiusRange =
    pex::MakeRange<double, pex::Limit<0>, pex::Limit<1000>>;


template<template<typename> typename T>
struct RegularPolygonTemplate
{
    T<CenterGroup> center;
    T<RadiusRange> radius;
    T<SidesRange> sides;
    T<RotationRange> rotation_deg;

    static constexpr auto fields =
        RegularPolygonFields<RegularPolygonTemplate>::fields;

    static constexpr auto fieldsTypeName = "RegularPolygon";
};


struct RegularPolygon: public RegularPolygonTemplate<pex::Identity>
{
    RegularPolygon()
        :
        RegularPolygonTemplate<pex::Identity>{
            tau::Point2d<double>(0.0, 0.0),
            100.0,
            3,
            0.0}
    {

    }

    RegularPolygon(
        const tau::Point2d<double> &center_,
        double radius_,
        size_t sideCount_,
        double rotation_deg_)
        :
        RegularPolygonTemplate<pex::Identity>{
            center_,
            radius_,
            sideCount_,
            rotation_deg_}
    {

    }

    PointsDouble GetPoints() const;
    PolygonLines GetLines() const;
    bool Contains(const tau::Point2d<double> &point) const;
    bool Contains(const tau::Point2d<double> &point, double margin) const;
    double GetRadius() const;
    double GetMarginScale(double margin) const;
    std::optional<double> HasRadius() const;

    RegularPolygon Rotate_deg(double relative_deg);
    void SetRadius(double radius);

    std::optional<double> HasMidpointRadius() const;
    double GetMidpointRadius() const;
    void SetMidpointRadius(double radius);

private:
    PointsDouble GetPoints_(double radius, double scale) const;
};


using SideLengthModel = pex::ModelSelector<double>;
using SideLengthControl = pex::ControlSelector<double>;

double GetRadius(size_t sides, double sideLength);

double GetSideLength(size_t sides, double radius);

struct RegularPolygonCustom
{
    using Plain = RegularPolygon;

    template<typename Base>
    struct Model: public Base
    {
        Model()
            :
            Base(),
            sideLength(),
            ignore_(false),

            sideLengthEndpoint_(
                PEX_THIS("RegularPolygonModel"),
                PEX_MEMBER_PASS(sideLength),
                &Model::OnSideLength_),

            radiusEndpoint_(this, this->radius, &Model::OnRadius_),
            sidesEndpoint_(this, this->sides, &Model::OnSides_)
        {
            jive::ScopeFlag ignore(this->ignore_);

            this->sideLength.Set(
                GetSideLength(this->sides.Get(), this->radius.Get()));
        }

        void OnSides_(size_t sides_)
        {
            if (this->ignore_)
            {
                return;
            }

            jive::ScopeFlag ignore(this->ignore_);
            this->sideLength.Set(GetSideLength(sides_, this->radius.Get()));
        }

        void OnRadius_(double radius_)
        {
            if (this->ignore_)
            {
                return;
            }

            jive::ScopeFlag ignore(this->ignore_);
            this->sideLength.Set(GetSideLength(this->sides.Get(), radius_));
        }

        void OnSideLength_(double sideLength_)
        {
            if (this->ignore_)
            {
                return;
            }

            jive::ScopeFlag ignore(this->ignore_);
            this->radius.Set(GetRadius(this->sides.Get(), sideLength_));
        }

        SideLengthModel sideLength;

        using SideLengthEndpoint = pex::Endpoint<Model, SideLengthControl>;
        using RadiusEndpoint = pex::Endpoint<Model, decltype(Model::radius)>;
        using SidesEndpoint = pex::Endpoint<Model, decltype(Model::sides)>;

        bool ignore_;
        SideLengthEndpoint sideLengthEndpoint_;
        RadiusEndpoint radiusEndpoint_;
        SidesEndpoint sidesEndpoint_;
    };

    template<typename Base>
    struct Control: public Base
    {
        using Base::Base;

        Control()
            :
            Base(),
            sideLength()
        {

        }

        Control(typename Base::Upstream &upstream)
            :
            Base(upstream),
            sideLength(upstream.sideLength)
        {

        }

        Control(const Control &other)
            :
            Base(other),
            sideLength(other.sideLength)
        {

        }

        Control & operator=(const Control &other)
        {
            this->Base::operator=(other);
            this->sideLength = other.sideLength;

            return *this;
        }

        SideLengthControl sideLength;
    };
};


using RegularPolygonGroup = pex::Group
<
    RegularPolygonFields,
    RegularPolygonTemplate,
    RegularPolygonCustom
>;

using RegularPolygonControl = typename RegularPolygonGroup::DefaultControl;


DECLARE_OUTPUT_STREAM_OPERATOR(RegularPolygon)
DECLARE_EQUALITY_OPERATORS(RegularPolygon)



} // end namespace draw



extern template struct pex::Group
    <
        draw::RegularPolygonFields,
        draw::RegularPolygonTemplate,
        draw::RegularPolygonCustom
    >;
