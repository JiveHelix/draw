#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/range.h>
#include <wxpex/color_picker.h>


namespace draw
{


template<typename T>
struct LookFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::strokeEnable, "strokeEnable"),
        fields::Field(&T::strokeWeight, "strokeWeight"),
        fields::Field(&T::strokeColor, "strokeColor"),
        fields::Field(&T::fillEnable, "fillEnable"),
        fields::Field(&T::fillColor, "fillColor"),
        fields::Field(&T::antialias, "antialias"));
};


template<template<typename> typename T>
struct LookTemplate
{
    using WeightRange = pex::MakeRange<int, pex::Limit<1>, pex::Limit<100>>;

    T<bool> strokeEnable;
    T<WeightRange> strokeWeight;
    T<pex::MakeGroup<wxpex::HsvGroup>> strokeColor;
    T<bool> fillEnable;
    T<pex::MakeGroup<wxpex::HsvGroup>> fillColor;
    T<bool> antialias;

    static constexpr auto fields = LookFields<LookTemplate>::fields;
    static constexpr auto fieldsTypeName = "Look";
};


struct Look: public LookTemplate<pex::Identity>
{
    static Look Default()
    {
        return {{
            true,
            1,
            {{0.0, 0.0, 1.0}},
            false,
            {{0.0, 0.0, 0.5}},
            true}};
    }
};


DECLARE_EQUALITY_OPERATORS(Look)


using LookGroup = pex::Group<LookFields, LookTemplate, Look>;

using LookModel = typename LookGroup::Model;

using LookControl = typename LookGroup::Control;


using LookGroupMaker = pex::MakeGroup<LookGroup>;


void ConfigureLook(wxpex::GraphicsContext &, const Look &);


} // end namespace draw


extern template struct pex::Group
    <
        draw::LookFields,
        draw::LookTemplate,
        draw::Look
    >;
