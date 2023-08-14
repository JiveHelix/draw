#pragma once


#include <pex/endpoint.h>
#include <tau/vector2d.h>
#include "draw/polygon.h"
#include "draw/drag.h"
#include "draw/views/pixel_view_settings.h"


namespace draw
{


std::optional<size_t> GetPoint(
    const tau::Point2d<int> &click,
    const std::vector<tau::Point2d<double>> &points);


std::optional<size_t> GetLine(
    const tau::Point2d<int> &click,
    const std::vector<tau::Point2d<double>> &points);


class DragPolygon: public Drag
{
public:
    DragPolygon(
        size_t index,
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        const Polygon &polygon);

protected:
    Polygon polygon_;
};


double DragAngleDifference(double first, double second);


class DragPolygonPoint: public DragPolygon
{
public:
    DragPolygonPoint(
        size_t index,
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        const Polygon &polygon);

    Polygon GetPolygon(const tau::Point2d<int> &end) const;

private:
    std::vector<tau::Point2d<double>> points_;
};


class DragRotatePolygonPoint: public DragPolygon
{
public:
    DragRotatePolygonPoint(
        size_t index,
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        const Polygon &polygon);

    Polygon GetPolygon(const tau::Point2d<int> &end) const;

private:
    std::vector<tau::Point2d<double>> points_;
};


class DragPolygonLine: public DragPolygon
{
public:
    DragPolygonLine(
        size_t index,
        const tau::Point2d<int> &start,
        const Polygon &polygon);

    Polygon GetPolygon(const tau::Point2d<int> &end) const;

private:
    PolygonLines lines_;
};


class PolygonBrain
{
public:
    PolygonBrain(
        PolygonControl polygonControl,
        PixelViewControl pixelViewControl);

private:
    void OnMouseDown_(bool isDown);
    void OnModifier_(const wxpex::Modifier &);
    void OnLogicalPosition_(const tau::Point2d<int> &position);
    bool IsDragging_() const;
    void UpdateCursor_();

    PolygonControl polygonControl_;
    PixelViewControl pixelViewControl_;

    pex::Endpoint<PolygonBrain, decltype(PixelViewControl::mouseDown)>
        mouseDownEndpoint_;

    pex::Endpoint<PolygonBrain, PointControl> logicalPositionEndpoint_;

    pex::Endpoint<PolygonBrain, decltype(PixelViewControl::modifier)>
        modifierEndpoint_;

    std::optional<Drag> dragCenter_;
    std::optional<DragPolygonPoint> dragPoint_;
    std::optional<DragRotatePolygonPoint> dragRotatePoint_;
    std::optional<DragPolygonLine> dragLine_;
    std::optional<Drag> dragCreate_;
};

} // end namespace draw
