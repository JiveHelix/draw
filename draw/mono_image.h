#pragma once


#include <tau/eigen.h>


namespace draw
{


template<typename T>
using MonoImage = Eigen::Matrix
<
    T,
    Eigen::Dynamic,
    Eigen::Dynamic,
    Eigen::RowMajor
>;


} // end namespace draw
