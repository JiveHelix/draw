#pragma once


#include <pex/group.h>
#include <wxpex/async.h>
#include <wxpex/modifier.h>
#include <wxpex/cursor.h>
#include "draw/pixels.h"
#include "draw/shapes.h"
#include "draw/views/canvas_settings.h"
#include "draw/pixels.h"


namespace draw
{


template<typename T>
struct PixelViewFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::canvas, "canvas"),
        fields::Field(&T::pixels, "pixels"),
        fields::Field(&T::shapes, "shapes"));
};


template<template<typename> typename T>
struct PixelViewTemplate
{
    using AsyncPixels = wxpex::MakeAsync<std::shared_ptr<draw::Pixels>>;
    using AsyncShapes = wxpex::MakeAsync<Shapes>;

    T<CanvasGroup> canvas;
    T<AsyncPixels> pixels;
    T<AsyncShapes> shapes;

    static constexpr auto fields = PixelViewFields<PixelViewTemplate>::fields;
    static constexpr auto fieldsTypeName = "PixelView";
};


struct PixelViewTemplates
{
    template<typename Base>
    struct Model: public Base
    {
        using Base::Base;

        Model()
            :
            Base{}
        {
            REGISTER_PEX_NAME(this, "PixelViewModel");
        }
    };

    template<typename Base>
    struct Control: public Base
    {
        using AsyncPixelsControl =
            typename Base::AsyncPixels::Control;

        using AsyncShapesControl =
            typename Base::AsyncShapes::Control;

        AsyncPixelsControl asyncPixels;
        AsyncShapesControl asyncShapes;

        using Base::Base;

        Control(typename Base::Upstream &upstream)
            :
            Base(upstream),
            asyncPixels(upstream.pixels.GetWorkerControl()),
            asyncShapes(upstream.shapes.GetWorkerControl())
        {
            REGISTER_PEX_NAME(this, "PixelViewControl");
            REGISTER_PEX_PARENT(asyncPixels);
            REGISTER_PEX_PARENT(asyncShapes);
        }
    };
};


using PixelViewGroup =
    pex::Group<PixelViewFields, PixelViewTemplate, PixelViewTemplates>;

using PixelViewSettings = typename PixelViewGroup::Plain;
using PixelViewModel = typename PixelViewGroup::Model;
using PixelViewControl = typename PixelViewGroup::Control;

using AsyncShapesControl = typename PixelViewControl::AsyncShapesControl;


} // end namespace draw


extern template struct pex::Group
    <
        draw::PixelViewFields,
        draw::PixelViewTemplate,
        draw::PixelViewTemplates
    >;
