#pragma once


namespace draw
{


class Drag
{
public:
    Drag(
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset)
        :
        index_(0),
        start_(start),
        offset_(offset)
    {

    }

    Drag(
        size_t index,
        const tau::Point2d<int> &start,
        const tau::Point2d<double> &offset)
        :
        index_(index),
        start_(start),
        offset_(offset)
    {

    }

    tau::Point2d<double> GetPosition(const tau::Point2d<int> &end) const
    {
        tau::Point2d<int> delta = end - this->start_;
        return this->offset_ + delta.template Convert<double>();
    }

    tau::Point2d<double> GetDragCenter(const tau::Point2d<int> &end) const
    {
        return (this->start_ + end).template Convert<double>() / 2.0;
    }

    tau::Size<double> GetSize(const tau::Point2d<int> &end) const
    {
        return (end - this->start_).template Convert<double>();
    }

    double GetMagnitude(const tau::Point2d<int> &end) const
    {
        return (end - this->start_).ToVector().Magnitude();
    }

    double GetAngle(const tau::Point2d<int> &end) const
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

protected:
    size_t index_;
    tau::Point2d<int> start_;
    tau::Point2d<double> offset_;
};


} // end namespace draw
