#pragma once


#include <fields/fields.h>
#include <pex/group.h>
#include <wxpex/color_picker.h>
#include "draw/draw_context.h"


namespace draw
{


template<typename T>
struct FontLookFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::pointSize, "pointSize"),
        fields::Field(&T::color, "color"),
        fields::Field(&T::antialias, "antialias"));
};


template<template<typename> typename T>
struct FontLookTemplate
{
    using WeightRange = pex::MakeRange<int, pex::Limit<1>, pex::Limit<100>>;

    T<bool> enable;
    T<double> pointSize;
    T<wxpex::HsvGroup> color;
    T<bool> antialias;

    static constexpr auto fields = FontLookFields<FontLookTemplate>::fields;
    static constexpr auto fieldsTypeName = "FontLook";
};


struct FontLook: public FontLookTemplate<pex::Identity>
{
    FontLook()
        :
        FontLookTemplate<pex::Identity>{
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
    pex::Group<FontLookFields, FontLookTemplate, pex::PlainT<FontLook>>;

using FontLookModel = typename FontLookGroup::Model;
using FontLookControl = typename FontLookGroup::DefaultControl;


void ConfigureFontLook(
    DrawContext &context,
    const FontLook &fontLook);


} // end namespace draw



extern template struct pex::Group
    <
        draw::FontLookFields,
        draw::FontLookTemplate,
        pex::PlainT<draw::FontLook>
    >;
