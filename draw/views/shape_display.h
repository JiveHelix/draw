#pragma once


#include <pex/ordered_list.h>
#include "draw/views/look_view.h"


namespace draw
{


template<typename T>
struct ShapeDisplayFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::shapeExpand, "shapeExpand"),
        fields::Field(&T::lookExpand, "lookExpand"));
};


template<template<typename> typename T>
struct ShapeDisplayTemplate
{
    T<bool> shapeExpand;
    T<LookDisplayGroup> lookExpand;

    static constexpr auto fields =
        ShapeDisplayFields<ShapeDisplayTemplate>::fields;

    static constexpr auto fieldsTypeName = "ShapeDisplay";
};


using ShapeDisplayGroup = pex::Group<ShapeDisplayFields, ShapeDisplayTemplate>;
using ShapeDisplayControl = typename ShapeDisplayGroup::DefaultControl;
using ShapeDisplay = typename ShapeDisplayGroup::Plain;
using ShapeExpandControl = decltype(ShapeDisplayControl::shapeExpand);

using ShapeDisplayListMaker =
    pex::OrderedListGroup<pex::List<ShapeDisplayGroup, 0>>;

using ShapeDisplayListControl = pex::ControlSelector<ShapeDisplayListMaker>;


} // end namespace draw
