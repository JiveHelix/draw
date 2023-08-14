#pragma once


#include <wxpex/wxshim.h>
#include <wxpex/point.h>
#include <wxpex/size.h>


namespace draw
{


template<typename Line>
void DrawLines(
    wxDC &dc,
    const std::vector<Line> &lines)
{
    auto size = wxpex::ToSize<double>(dc.GetSize());
    tau::Scale<double> scale;
    dc.GetUserScale(&scale.horizontal, &scale.vertical);
    size *= scale;

    auto region = tau::Region<double>{{tau::Point2d<double>(0, 0), size}};

    std::cout << "size: " << size << std::endl;
    std::cout << "scale: " << scale << std::endl;
    std::cout << "region: " << region << std::endl;

    for (auto &line: lines)
    {
        auto endPoints = line.Intersect(region);

        if (!endPoints)
        {
            continue;
        }

        dc.DrawLine(
            wxpex::ToWxPoint(endPoints->first),
            wxpex::ToWxPoint(endPoints->second));
    }
}


} // end namespace draw
