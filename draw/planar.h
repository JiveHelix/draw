#pragma once


#include <tau/planar.h>


namespace draw
{


template<typename Pixel>
using PlanarRgb =
    tau::Planar<3, Pixel, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

template<typename Pixel>
using PlanarHsv =
    tau::Planar<3, Pixel, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;


} // end namespace draw
