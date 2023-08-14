#pragma once


#include <tau/vector2d.h>
#include "draw/quad.h"
#include "draw/views/pixel_view_settings.h"
#include "draw/drag.h"


namespace draw
{


struct Bounds2d
{
    pex::Bounds<double> x;
    pex::Bounds<double> y;

    template<typename Control>
    static Bounds2d Make(Control control)
    {
        return {control.x.GetBounds(), control.y.GetBounds()};
    }
};


struct QuadBounds
{
    Bounds2d shear;
    Bounds2d perspective;
    pex::Bounds<double> rotation;

    QuadBounds(QuadControl control);
};


class DragQuad: public Drag
{
public:
    DragQuad(
        size_t index,
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        const Quad &quad,
        const QuadBounds &quadBounds);

protected:
    Quad quad_;
    QuadBounds quadBounds_;
};


class DragQuadPoint: public DragQuad
{
public:
    DragQuadPoint(
        size_t index,
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        const Quad &quad,
        const QuadBounds &quadBounds);

    Quad GetQuad(const tau::Point2d<int> &end) const;

private:
    QuadPoints points_;
};


class DragRotatePoint: public DragQuad
{
public:
    DragRotatePoint(
        size_t index,
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset,
        const Quad &quad,
        const QuadBounds &quadBounds);

    Quad GetQuad(const tau::Point2d<int> &end) const;

private:
    QuadPoints points_;
};


class DragQuadLine: public DragQuad
{
public:
    DragQuadLine(
        size_t index,
        const tau::Point2d<int> &start,
        const Quad &quad,
        const QuadBounds &quadBounds);

    Quad GetQuad(const tau::Point2d<int> &end) const;

private:
    QuadLines lines_;
};


class QuadBrain
{
public:
    QuadBrain(
        QuadControl quadControl,
        PixelViewControl pixelViewControl);

private:
    void OnMouseDown_(bool isDown);
    void OnModifier_(const wxpex::Modifier &);
    void OnLogicalPosition_(const tau::Point2d<int> &position);
    bool IsDragging_() const;
    void UpdateCursor_();

    QuadControl quadControl_;
    PixelViewControl pixelViewControl_;

    pex::Endpoint<QuadBrain, decltype(PixelViewControl::mouseDown)>
        mouseDownEndpoint_;

    pex::Endpoint<QuadBrain, PointControl> logicalPositionEndpoint_;

    pex::Endpoint<QuadBrain, decltype(PixelViewControl::modifier)>
        modifierEndpoint_;


    std::optional<Drag> dragCenter_;
    std::optional<DragQuadPoint> dragPoint_;
    std::optional<DragRotatePoint> dragRotatePoint_;
    std::optional<DragQuadLine> dragLine_;
    std::optional<Drag> dragCreate_;
};

} // end namespace draw
