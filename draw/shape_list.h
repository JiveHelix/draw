#pragma once


#include <fields/fields.h>
#include <pex/ordered_list.h>
#include "draw/shapes.h"


namespace draw
{


using ShapeListMaker = pex::MakePolyList<ShapeSupers>;

static_assert(pex::IsMakePolyList<ShapeListMaker>);
static_assert(pex::HasOrder<ShapeListMaker>);

template<typename T>
struct ShapeListFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::shapes, "shapes"),
        fields::Field(&T::shapesDisplay, "shapesDisplay"),
        fields::Field(&T::reorder, "reorder"));
};


using OrderedShapesMaker = pex::OrderedListGroup<ShapeListMaker>;


template<template<typename> typename T>
struct ShapeListTemplate
{
    T<OrderedShapesMaker> shapes;
    T<ShapeDisplayListMaker> shapesDisplay;
    T<pex::MakeSignal> reorder;

    static constexpr auto fields = ShapeListFields<ShapeListTemplate>::fields;
};


using OrderedShapesControl = pex::ControlSelector<OrderedShapesMaker>;


struct ShapeListCustom
{
    template<typename GroupBase>
    class Model: public GroupBase
    {
    public:
        Model()
            :
            GroupBase(),
            countEndpoint_(this, this->shapes.count, &Model::OnShapesCount_),

            orderedIndicesEndpoint_(
                this,
                this->shapes.indices,
                &Model::OnIndices_)
        {
            this->OnShapesCount_(this->shapes.count.Get());
        }

    private:
        void OnShapesCount_(size_t value)
        {
            this->shapesDisplay.count.Set(value);
        }

        void OnIndices_(const std::vector<size_t> &orderedIndices)
        {
            this->shapesDisplay.indices.Set(orderedIndices);
            this->reorder.Trigger();
        }

    private:
        using CountEndpoint = pex::Endpoint<Model, pex::model::ListCount>;

        using OrderedIndicesEndpoint =
            pex::Endpoint<Model, pex::OrderedIndicesControl>;

        CountEndpoint countEndpoint_;
        OrderedIndicesEndpoint orderedIndicesEndpoint_;
    };
};


using ShapeListGroup =
    pex::Group<ShapeListFields, ShapeListTemplate, ShapeListCustom>;

using ShapeListModel = typename ShapeListGroup::Model;
using ShapeListControl = typename ShapeListGroup::Control;
using ShapesControl = decltype(ShapeListControl::shapes);

template<typename Observer>
using ShapesEndpoint = pex::Endpoint<Observer, ShapesControl>;


static_assert(decltype(ShapeListModel::shapes)::hasOrder);


} // end namespace draw
