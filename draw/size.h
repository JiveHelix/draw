#pragma once


#include <pex/endpoint.h>
#include <tau/size.h>


namespace draw
{

using SizeType = int;
using SizeGroup = tau::SizeGroup<SizeType>;
using Size = typename SizeGroup::Plain;

using SizeModel = typename SizeGroup::Model;
using SizeControl = typename SizeGroup::Control;


template<typename Observer>
using SizeEndpoint = pex::EndpointGroup<Observer, SizeControl>;


} // end namespace draw
