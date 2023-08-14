#pragma once


#include <tau/size.h>


namespace draw
{


using SizeType = int;
using SizeGroup = tau::SizeGroup<SizeType>;
using Size = typename SizeGroup::Plain;

using SizeModel = typename SizeGroup::Model;
using SizeControl = typename SizeGroup::Control;

using SizeGroupMaker = pex::MakeGroup<SizeGroup>;


} // end namespace draw
