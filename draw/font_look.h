#pragma once


#include <fields/fields.h>
#include <pex/group.h>
#include <wxpex/color_picker.h>
#include "draw/draw_context.h"


namespace draw
{


template<template<typename> typename T>
struct FontLookSchema
{
    using WeightRange = pex::MakeRange<int, pex::Limit<1>, pex::Limit<100>>;

    T<bool> enable;
    T<double> pointSize;
    T<wxpex::HsvGroup> color;
    T<bool> antialias;

    static constexpr auto fieldsTypeName = "FontLook";
};


struct FontLook: public FontLookSchema<pex::Identity>
{
    FontLook()
        :
        FontLookSchema<pex::Identity>{
            true,
            18.0,
            {{0.0, 0.0, 1.0}},
            true}
    {

    }
};


DECLARE_EQUALITY_OPERATORS(FontLook)
DECLARE_OUTPUT_STREAM_OPERATOR(FontLook)


using FontLookGroup =
    pex::Group<FontLookSchema, pex::PlainT<FontLook>>;

using FontLookModel = typename FontLookGroup::Model;
using FontLookControl = typename FontLookGroup::DefaultControl;


void ConfigureFontLook(
    DrawContext &context,
    const FontLook &fontLook);


} // end namespace draw



extern template struct pex::Group
    <
        draw::FontLookSchema,
        pex::PlainT<draw::FontLook>
    >;
