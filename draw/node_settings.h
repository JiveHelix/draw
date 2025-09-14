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


struct NodeSettingsCustom
{
    template<typename Base>
    struct Plain: public Base
    {
        static constexpr auto darkGreen = tau::Hsv<double>{{136.0, 0.57, 0.36}};

        Plain()
            :
            Base{
                {},
                false,
                darkGreen}
        {

        }
    };
};


using NodeSettingsGroup =
    pex::Group
    <
        NodeSettingsFields,
        NodeSettingsTemplate,
        NodeSettingsCustom
    >;

using NodeSettingsModel = typename NodeSettingsGroup::Model;
using NodeSettingsControl = typename NodeSettingsGroup::DefaultControl;
using NodeSettings = typename NodeSettingsGroup::Plain;
using NodeToggleSelectSignal = decltype(NodeSettingsControl::toggleSelect);


DECLARE_EQUALITY_OPERATORS(NodeSettings)
DECLARE_OUTPUT_STREAM_OPERATOR(NodeSettings)


using OptionalNodeSettings = std::optional<NodeSettingsControl>;

template<typename Observer>
using NodeToggleSelectEndpoint =
    pex::Endpoint<Observer, NodeToggleSelectSignal>;


template<typename T>
concept IsNode =
    std::is_same_v<std::remove_reference_t<T>, NodeSettingsModel>
    || std::is_same_v<std::remove_reference_t<T>, NodeSettingsControl>;


template<typename T>
concept HasVirtualGetNode = requires(T t)
{
    { t.GetVirtual()->GetNode() } -> IsNode;
};


template<typename T>
concept HasNodeMember = requires (T t)
{
    { t.node } -> IsNode;
};


template<typename T>
concept HasNode =
    HasNodeMember<T> || HasVirtualGetNode<T>;


template<HasNode Item>
auto & GetNode(Item &item)
{
    if constexpr (HasNodeMember<Item>)
    {
        return item.node;
    }
    else
    {
        return item.GetVirtual()->GetNode();
    }
}


} // end namespace draw
