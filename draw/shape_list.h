#pragma once


#include <fields/fields.h>
#include "draw/depth_order.h"


namespace draw
{


template<typename T>
struct ShapeListFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::shapes, "shapes"),
        fields::Field(&T::shapesDisplay, "shapesDisplay"),
        fields::Field(&T::reorder, "reorder"));
};


template<typename ShapeListMaker>
using OrderedShapesMaker = OrderedListGroup<ShapeListMaker, true>;


template<typename ShapeListMaker>
struct ShapeListTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<OrderedShapesMaker<ShapeListMaker>> shapes;
        T<ShapeDisplayListMaker> shapesDisplay;
        T<pex::MakeSignal> reorder;
    };
};


template<typename ShapeListMaker>
using OrderedShapesControl =
    pex::ControlSelector<OrderedShapesMaker<ShapeListMaker>>;


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
            pex::Endpoint<Model, OrderedIndicesControl>;

        CountEndpoint countEndpoint_;
        OrderedIndicesEndpoint orderedIndicesEndpoint_;
    };
};


template<typename ShapeListMaker>
using ShapeListGroup = pex::Group
    <
        ShapeListFields,
        ShapeListTemplate<ShapeListMaker>::template Template,
        ShapeListCustom
    >;


template<typename ShapeListMaker>
using ShapeListControl = typename ShapeListGroup<ShapeListMaker>::Control;


} // end namespace draw
