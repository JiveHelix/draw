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
    template<typename GroupBase>
    struct Control: public GroupBase
    {
        using AsyncPixelsControl =
            typename GroupBase::AsyncPixels::Control;

        using AsyncShapesControl =
            typename GroupBase::AsyncShapes::Control;

        AsyncPixelsControl asyncPixels;
        AsyncShapesControl asyncShapes;

        Control() = default;

        Control(typename GroupBase::Upstream &upstream)
            :
            GroupBase(upstream),
            asyncPixels(upstream.pixels.GetWorkerControl()),
            asyncShapes(upstream.shapes.GetWorkerControl())
        {

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
