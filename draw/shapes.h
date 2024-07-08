#pragma once


#include <nlohmann/json.hpp>
#include <pex/endpoint.h>
#include <pex/poly.h>
#include <memory>
#include <vector>
#include <pex/ordered_list.h>
#include <wxpex/graphics.h>
#include <wxpex/async.h>
#include <wxpex/modifier.h>
#include <wxpex/cursor.h>
#include <wxpex/labeled_widget.h>
#include "draw/detail/unique_id.h"
#include "draw/detail/poly_shape_id.h"
#include "draw/drag.h"
#include "draw/oddeven.h"
#include "draw/node_settings.h"
#include "draw/views/look_view.h"
#include "draw/views/shape_view.h"


namespace draw
{


using CursorControl = pex::control::Value<pex::model::Value<wxpex::Cursor>>;


class DrawnShape
{
public:
    virtual ~DrawnShape()
    {

    }

    virtual void Draw(wxpex::GraphicsContext &) = 0;
};


class ShapeControlUserBase
{
public:
    virtual ~ShapeControlUserBase() {}

    virtual ssize_t GetId() const = 0;
    virtual std::string GetName() const = 0;
    virtual NodeSettingsControl & GetNode() = 0;
    virtual pex::OrderControl & GetOrder() = 0;

    virtual wxWindow * CreateShapeView(wxWindow *parent) const = 0;

    virtual wxWindow * CreateLookView(
        wxWindow *parent,
        LookDisplayControl displayControl) const = 0;
};


class ShapeModelUserBase
{
public:
    virtual ~ShapeModelUserBase() {}

    virtual pex::OrderControl GetOrder() = 0;
};


class Shape
    :
    public DrawnShape,
    public pex::poly::PolyBase<nlohmann::json, Shape>
{
public:
    static constexpr auto polyTypeName = "Shape";

    using ControlBase =
        pex::poly::ControlBase<Shape, ShapeControlUserBase>;

    virtual bool HandlesAltClick() const { return false; }
    virtual bool HandlesControlClick() const { return false; }
    virtual bool HandlesRotate() const { return false; }
    virtual bool HandlesEditPoint() const { return false; }
    virtual bool HandlesEditLine() const { return false; }

    virtual ssize_t GetId() const = 0;
    virtual PointsDouble GetPoints() const = 0;

    virtual bool Contains(
        const tau::Point2d<int> &point,
        double margin) const = 0;

    virtual std::string GetName() const = 0;

    virtual std::unique_ptr<Drag> ProcessMouseDown(
        std::shared_ptr<ControlBase> shapeControl,
        const tau::Point2d<int> &click,
        const wxpex::Modifier &modifier,
        CursorControl cursor) = 0;

    virtual bool ProcessControlClick(
        ControlBase &,
        const tau::Point2d<int> &)
    {
        return false;
    }

    virtual bool ProcessAltClick(
        ControlBase &,
        PointsIterator,
        PointsDouble &)
    {
        return false;
    }

    virtual std::shared_ptr<Shape> Copy() const = 0;
};


struct ShapeSupers
{
    using ValueBase = Shape;
    using ModelUserBase = ShapeModelUserBase;
    using ControlUserBase = ShapeControlUserBase;
};


template<typename T>
struct ShapeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::id, "id"),
        fields::Field(&T::order, "order"),
        fields::Field(&T::shape, "shape"),
        fields::Field(&T::look, "look"),
        fields::Field(&T::node, "node"));
};


template<typename ShapeGroup, typename View>
struct ShapeCommon
{
    using Supers = ShapeSupers;

    template<template<typename> typename T>
    struct Template
    {
        // id is read-only to a control
        T<pex::ReadOnly<ssize_t>> id;
        T<pex::OrderGroup> order;
        T<ShapeGroup> shape;
        T<LookGroup> look;
        T<NodeSettingsGroup> node;

        static constexpr auto fields = ShapeFields<Template>::fields;

        static constexpr auto fieldsTypeName =
            ShapeGroup::template Template<pex::Identity>::fieldsTypeName;
    };

    using ControlMembers = pex::MakeControlMembers<Template>;

    template<typename Base>
    class Model: public Base
    {
    public:
        using Base::Base;

        Model()
            :
            Base(),
            polyShapeId_()
        {
            pex::SetOverride(this->id, this->polyShapeId_.Get());
        }

        pex::OrderControl GetOrder() override
        {
            return this->order;
        }

    private:
        detail::PolyShapeId polyShapeId_;
    };

    template<typename Base>
    class Control: public Base
    {
    public:
        using Base::Base;

        ssize_t GetId() const override
        {
            return this->id.Get();
        }

        std::string GetName() const override
        {
            return this->Get().GetName();
        }

        NodeSettingsControl & GetNode() override
        {
            return this->node;
        }

        pex::OrderControl & GetOrder() override
        {
            return this->order;
        }

        wxWindow * CreateShapeView(wxWindow *parent) const override
        {
            return new View(
                parent,
                this->shape,
                wxpex::LayoutOptions{});
        }

        wxWindow * CreateLookView(
            wxWindow *parent,
            LookDisplayControl displayControl) const override
        {
            return new LookView(
                parent,
                this->look,
                displayControl,
                wxpex::LayoutOptions{});
        }
    };
};


// Common shape overrides.
template<typename Base, typename Derived>
class ShapeImpl: public Base
{
public:
    using Base::Base;
    using PlainShape = decltype(Base::shape);
    static constexpr auto fieldsTypeName = PlainShape::fieldsTypeName;

    static Derived Default()
    {
        return {{
            0,
            {},
            PlainShape::Default(),
            Look::Default(),
            NodeSettings::Default()}};
    }

    ssize_t GetId() const override
    {
        return this->id;
    }

    PointsDouble GetPoints() const override
    {
        return this->shape.GetPoints();
    }

    bool Contains(
        const tau::Point2d<int> &point,
        double margin) const override
    {
        return this->shape.Contains(point, margin);
    }

    std::shared_ptr<Shape> Copy() const override
    {
        return std::make_shared<Derived>(*this);
    }
};


using ShapeValue = pex::poly::Value<Shape>;
using ShapeControl = typename Shape::ControlBase;


static_assert(pex::poly::detail::IsCompatibleBase<Shape>);


class ShapesId: public UniqueId<ssize_t>
{
public:
    ShapesId();
};


struct Shapes
{
public:
    static constexpr bool isShape = true;

    using ShapeVector = std::vector<std::shared_ptr<DrawnShape>>;

    Shapes();

    Shapes(ssize_t id);

    const ShapeVector & GetShapes() const;

    ssize_t GetId() const;

    // Shapes with the same id_ compare equal.
    bool operator==(const Shapes &other) const;

    template<typename Derived, typename ...Args>
    void EmplaceBack(Args && ...args)
    {
        if (this->id_ < 0)
        {
            throw std::logic_error("Shapes must have a valid ShapesId");
        }

        this->shapes_.push_back(
            std::make_shared<Derived>(std::forward<Args>(args)...));
    }

    void Append(const Shape &shape)
    {
        this->shapes_.push_back(shape.Copy());
    }

    static Shapes MakeResetter();

    bool IsResetter() const;

private:
    ssize_t id_;
    std::vector<std::shared_ptr<DrawnShape>> shapes_;
};


using AsyncShapes = wxpex::MakeAsync<Shapes>;
using AsyncShapesControl = typename AsyncShapes::Control;

template<typename Observer>
using AsyncShapesEndpoint = pex::Endpoint<Observer, AsyncShapesControl>;


} // end namespace draw
