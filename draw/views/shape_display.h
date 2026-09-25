#pragma once


#include <pex/ordered_list.h>
#include "draw/views/look_view.h"


namespace draw
{


template<template<typename> typename T>
struct ShapeDisplaySchema
{
    T<bool> shapeExpand;
    T<LookDisplayGroup> lookExpand;

    static constexpr auto fieldsTypeName = "ShapeDisplay";
};


using ShapeDisplayGroup = pex::Group<ShapeDisplaySchema>;
using ShapeDisplayControl = typename ShapeDisplayGroup::DefaultControl;
using ShapeDisplay = typename ShapeDisplayGroup::Plain;
using ShapeExpandControl = decltype(ShapeDisplayControl::shapeExpand);

using ShapeDisplayListMaker =
    pex::OrderedListGroup<pex::List<ShapeDisplayGroup, 0>>;

using ShapeDisplayListControl = pex::ControlTailor<ShapeDisplayListMaker>;


} // end namespace draw
