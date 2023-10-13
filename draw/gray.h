#pragma once


#include <Eigen/Dense>


namespace draw
{


template<typename Pixel>
using Gray =
    Eigen::Matrix<Pixel, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;


} // end namespace draw
