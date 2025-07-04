#pragma once


#include <tau/size.h>


namespace draw
{


class Drag
{
public:
    virtual ~Drag() {}

    Drag(
        const tau::Point2d<double> &start,
        const tau::Point2d<double> &offset)
        :
        index_(0),
        start_(start),
        offset_(offset)
    {

    }

    Drag(
        size_t index,
        const tau::Point2d<double> &start,
        const tau::Point2d<double> &offset)
        :
        index_(index),
        start_(start),
        offset_(offset)
    {

    }

    virtual void ReportLogicalPosition(
        const tau::Point2d<double> &position) = 0;

    // Get the position of the dragged element.
    // The start_ is usually near the dragged element.
    // The offset_ the position of the dragged shape feature.
    tau::Point2d<double> GetPosition(const tau::Point2d<double> &end) const
    {
        tau::Point2d<double> delta = end - this->start_;
        return this->offset_ + delta;
    }

    tau::Point2d<double> GetDragCenter(const tau::Point2d<double> &end) const
    {
        return (this->start_ + end) / 2.0;
    }

    tau::Size<double> GetSize(const tau::Point2d<double> &end) const
    {
        return (end - this->start_);
    }

    double GetMagnitude(const tau::Point2d<double> &end) const
    {
        return (end - this->start_).ToVector().Magnitude();
    }

    double GetAngle(const tau::Point2d<double> &end) const
    {
        return (end - this->start_).GetAngle();
    }

    size_t GetIndex() const
    {
        return this->index_;
    }

    void SetOffset(const tau::Point2d<double> &offset)
    {
        this->offset_ = offset;
    }

    tau::Point2d<double> GetStart() const
    {
        return this->start_;
    }

    tau::Point2d<double> GetOffset() const
    {
        return this->offset_;
    }

protected:
    size_t index_;
    tau::Point2d<double> start_;
    tau::Point2d<double> offset_;
};


} // end namespace draw
