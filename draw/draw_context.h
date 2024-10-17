#pragma once


#include <wxpex/graphics.h>
#include <draw/look.h>


namespace draw
{


class SplineTool
{
public:
    SplineTool(wxGCDC *gcdc)
        :
        gcdc_(gcdc),
        pointList_(std::make_unique<wxPointList>()),
        points_()
    {

    }

    template<typename T>
    void DrawSpline(const std::vector<tau::Point2d<T>> &points)
    {
        this->pointList_->Clear();
        this->points_.clear();
        this->points_.reserve(points.size());

        for (auto &point: points)
        {
            this->points_.push_back(wxpex::ToWxPoint(point));
            this->pointList_->Append(&this->points_.back());
        }

        this->gcdc_->DrawSpline(this->pointList_.get());
    }

private:
    wxGCDC *gcdc_;
    std::unique_ptr<wxPointList> pointList_;
    std::vector<wxPoint> points_;
};



class DrawContext: public wxpex::GraphicsContext
{
public:
    template<typename T>
    DrawContext(T &&dc)
        :
        wxpex::GraphicsContext(std::forward<T>(dc)),
        look_{}
    {

    }

    DrawContext(const DrawContext &) = delete;
    DrawContext & operator=(const DrawContext &) = delete;

    void ConfigureLook(const Look &look);

    void ConfigureColors(const Look &);

    void ConfigureColors(const Look &, double value);

    const Look & GetLook() const
    {
        return this->look_;
    }

    SplineTool & GetSplineTool()
    {
        if (!this->splineTool_)
        {
            this->splineTool_.emplace(this->GetGCDC());
        }

        return *this->splineTool_;
    }

private:
    Look look_;
    std::optional<SplineTool> splineTool_;
};



} // end namespace draw
