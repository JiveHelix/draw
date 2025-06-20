#pragma once


#include <optional>
#include <fields/fields.h>
#include <pex/group.h>
#include <tau/color.h>
#include <pex/endpoint.h>


namespace draw
{


template<typename T>
struct NodeSettingsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::toggleSelect, "toggleSelect"),
        fields::Field(&T::isSelected, "isSelected"),
        fields::Field(&T::highlightColor, "highlightColor"));
};



template<template<typename> typename T>
struct NodeSettingsTemplate
{
    T<pex::MakeSignal> toggleSelect;
    T<bool> isSelected;
    T<tau::HsvGroup<double>> highlightColor;

    static constexpr auto fields =
        NodeSettingsFields<NodeSettingsTemplate>::fields;

    static constexpr auto fieldsTypeName = "NodeSettings";
};


struct NodeSettings: public NodeSettingsTemplate<pex::Identity>
{
    static constexpr auto darkGreen = tau::Hsv<double>{{136.0, 0.57, 0.36}};

    using Base = NodeSettingsTemplate<pex::Identity>;

    NodeSettings()
        :
        Base{
            {},
            false,
            darkGreen}
    {

    }
};


DECLARE_EQUALITY_OPERATORS(NodeSettings)
DECLARE_OUTPUT_STREAM_OPERATOR(NodeSettings)


using NodeSettingsGroup =
    pex::Group
    <
        NodeSettingsFields,
        NodeSettingsTemplate,
        pex::PlainT<NodeSettings>
    >;

using NodeSettingsModel = typename NodeSettingsGroup::Model;
using NodeSettingsControl = typename NodeSettingsGroup::Control;
using NodeToggleSelectSignal = decltype(NodeSettingsControl::toggleSelect);

using OptionalNodeSettings = std::optional<NodeSettingsControl>;

template<typename Observer>
using NodeToggleSelectEndpoint =
    pex::Endpoint<Observer, NodeToggleSelectSignal>;


} // end namespace draw
