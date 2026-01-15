#pragma once


#include <pex/endpoint.h>
#include <tau/size.h>
#include <tau/eigen.h>


namespace draw
{

using SizeType = int;
using SizeGroup = tau::SizeGroup<SizeType>;
using Size = typename SizeGroup::Plain;

using SizeModel = typename SizeGroup::Model;
using SizeControl = typename SizeGroup::DefaultControl;


template<typename Observer>
using SizeEndpoint = pex::EndpointGroup<Observer, SizeControl>;


template<typename Derived>
Size GetMatrixSize(const Eigen::DenseBase<Derived> &matrix)
{
    auto rows = matrix.rows();
    auto columns = matrix.cols();

    assert(rows <= std::numeric_limits<SizeType>::max());
    assert(rows >= std::numeric_limits<SizeType>::min());
    assert(columns <= std::numeric_limits<SizeType>::max());
    assert(columns >= std::numeric_limits<SizeType>::min());

    return Size(static_cast<SizeType>(columns), static_cast<SizeType>(rows));
}


} // end namespace draw
