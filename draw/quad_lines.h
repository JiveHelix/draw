#pragma once


#include <vector>
#include <Eigen/Dense>
#include <tau/line2d.h>
#include <tau/vector2d.h>


namespace draw
{


template<typename T>
struct AffineFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::x, "x"),
        fields::Field(&T::y, "y"));
};


using PerspectiveRange =
    pex::MakeRange<double, pex::Limit<-90>, pex::Limit<90>>;

template<template<typename> typename T>
struct PerspectiveTemplate
{
    T<PerspectiveRange> x;
    T<PerspectiveRange> y;

    static constexpr auto fields = AffineFields<PerspectiveTemplate>::fields;
    static constexpr auto fieldsTypeName = "Perspective";
};


using PerspectiveGroup = pex::Group
<
    AffineFields,
    PerspectiveTemplate
>;

using Perspective = typename PerspectiveGroup::Plain;

using PerspectiveModel = typename PerspectiveGroup::Model;

using PerspectiveControl = typename PerspectiveGroup::Control;

using PerspectiveGroupMaker = pex::MakeGroup<PerspectiveGroup>;

DECLARE_OUTPUT_STREAM_OPERATOR(Perspective)


using QuadPoint = tau::Point2d<double>;
using QuadPoints = std::vector<QuadPoint>;
using QuadMatrix = Eigen::Matrix<double, 3, 4>;


QuadMatrix PointsToMatrix(
    const QuadPoint &topLeft,
    const QuadPoint &topRight,
    const QuadPoint &bottomRight,
    const QuadPoint &bottomLeft);


QuadMatrix PointsToMatrix(const QuadPoints &points);


QuadPoints MatrixToPoints(const QuadMatrix &pointsMatrix);


struct QuadLines
{
    using Line = tau::Line2d<double>;

    Line top;
    Line right;
    Line bottom;
    Line left;

    QuadLines() = default;

    const Line & operator[](size_t index) const;

    static QuadPoint GetMidpoint(
        const QuadPoint &first,
        const QuadPoint &second);

    QuadLines(
        const QuadPoint &topLeft,
        const QuadPoint &topRight,
        const QuadPoint &bottomRight,
        const QuadPoint &bottomLeft);

    QuadLines(double halfWidth, double halfHeight);
    QuadLines(const tau::Size<double> &size);
    QuadLines(const QuadPoints &points);
    QuadLines(const QuadMatrix &pointsMatrix);

    QuadMatrix GetMatrix() const;
    QuadPoints GetPoints() const;
    Perspective GetPerspective() const;
    QuadLines ApplyPerspective(const Perspective &perspective) const;
    QuadLines UndoPerspective();
    tau::Size<double> GetSize() const;
};


} // end namespace draw



extern template struct pex::Group
<
    draw::AffineFields,
    draw::PerspectiveTemplate
>;
