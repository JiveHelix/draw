#pragma once


#include <tau/eigen.h>
#include <wxpex/graphics.h>
#include <wxpex/point.h>
#include <span>
#include <vector>


namespace draw
{


void AppendTangentSpline(
    wxGraphicsPath &path,
    const std::vector<tau::Point2d<double>> &points);

void AppendTangentSpline(
    wxGraphicsPath &path,
    std::span<const tau::Point2d<double>> points,
    std::span<const tau::Point2d<double>> derivatives);

void AppendTangentSpline(
    wxGraphicsPath &path,
    std::span<const tau::Point2d<double>> points,
    const tau::Point2d<double> &firstDerivative,
    const tau::Point2d<double> &lastDerivative);

std::vector<tau::Point2d<double>> GetDerivatives(
    std::span<const tau::Point2d<double>> points);

void DrawTangentSpline(
    wxGraphicsPath &path,
    const std::vector<tau::Point2d<double>> &points);

void DrawTangentSpline(
    wxGraphicsPath &path,
    std::span<const tau::Point2d<double>> points,
    std::span<const tau::Point2d<double>> derivatives);


} // end namespace draw
