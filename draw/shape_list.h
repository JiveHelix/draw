#pragma once


#include <fields/fields.h>
#include <pex/ordered_list.h>
#include "draw/shapes.h"


namespace draw
{


using ShapeListMaker = pex::List<pex::MakePoly<ShapeSupers>>;

static_assert(pex::IsList<ShapeListMaker>);
static_assert(pex::ListHasOrder<ShapeListMaker>);

template<typename T>
struct ShapeListFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::shapes, "shapes"),
        fields::Field(&T::shapesDisplay, "shapesDisplay"));
};


using OrderedShapes = pex::OrderedListGroup<ShapeListMaker>;


template<template<typename> typename T>
struct ShapeListTemplate
{
    T<OrderedShapes> shapes;
    T<ShapeDisplayListMaker> shapesDisplay;

    static constexpr auto fields = ShapeListFields<ShapeListTemplate>::fields;
};


using OrderedShapesControl = pex::ControlSelector<OrderedShapes>;


struct ShapeListCustom
{
    template<typename Base>
    class Model: public Base
    {
    public:
        Model()
            :
            Base(),
            countEndpoint_(this, this->shapes.count, &Model::OnShapesCount_),

            indicesEndpoint_(
                this,
                this->shapes.indices,
                &Model::OnShapesIndices_)
        {
            this->OnShapesCount_(this->shapes.count.Get());
        }

    private:
        void OnShapesCount_(size_t value)
        {
            this->shapesDisplay.count.Set(value);
        }

        void OnShapesIndices_(const std::vector<size_t> &indices)
        {
            this->shapesDisplay.indices.Set(indices);
        }

    private:
        using CountEndpoint = pex::Endpoint<Model, pex::model::ListCount>;

        using IndicesEndpoint =
            pex::Endpoint
            <
                Model,
                decltype(OrderedShapesControl::indices)
            >;

        CountEndpoint countEndpoint_;
        IndicesEndpoint indicesEndpoint_;
    };
};


using ShapeListGroup =
    pex::Group<ShapeListFields, ShapeListTemplate, ShapeListCustom>;

using ShapeListModel = typename ShapeListGroup::Model;
using ShapeListControl = typename ShapeListGroup::Control;
using ShapesControl = decltype(ShapeListControl::shapes);

template<typename Observer>
using ShapesEndpoint = pex::Endpoint<Observer, ShapesControl>;


using TestShapesDisplay = decltype(ShapeListControl::shapesDisplay);
static_assert(std::is_same_v<TestShapesDisplay, ShapeDisplayListControl>);

using TestListItem = typename TestShapesDisplay::ListItem;

static_assert(std::is_same_v<TestListItem, ShapeDisplayControl>);


static_assert(pex::ListHasVirtualGetOrder<OrderedShapes>);
static_assert(decltype(ShapeListModel::shapes)::hasOrder);


} // end namespace draw
