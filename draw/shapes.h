#pragma once


#include <nlohmann/json.hpp>
#include <pex/endpoint.h>
#include <pex/poly.h>
#include <memory>
#include <vector>
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


template<typename Base>
class ShapeControlUserBase: public Base
{
public:
    using Base::Base;

    virtual ~ShapeControlUserBase() {}

    virtual ssize_t GetId() const = 0;
    virtual std::string GetName() const = 0;
    virtual NodeSettingsControl & GetNode() = 0;

    virtual wxWindow * CreateShapeView(wxWindow *parent) const = 0;

    virtual wxWindow * CreateLookView(
        wxWindow *parent,
        LookDisplayControl displayControl) const = 0;
};


struct ShapeTemplates
{
    template<typename Base>
    using ControlUserBase = ShapeControlUserBase<Base>;
};


template<typename View>
struct ShapeCustom
{
    template<typename Base>
    using ControlUserBase = ShapeControlUserBase<Base>;

    template<typename GroupBase>
    class Model: public GroupBase
    {
    public:
        using GroupBase::GroupBase;

        Model()
            :
            GroupBase(),
            polyShapeId_()
        {
            pex::SetOverride(this->id, this->polyShapeId_.Get());
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


class Shape
    :
    public DrawnShape,
    public pex::poly::PolyBase<nlohmann::json, Shape>
{
public:
    static constexpr auto polyTypeName = "Shape";

    using ControlUserBase =
        pex::poly::detail::MakeControlUserBase<ShapeTemplates, Shape>;

    virtual bool HandlesAltClick() const { return false; }
    virtual bool HandlesControlClick() const { return false; }

    virtual ssize_t GetId() const = 0;
    virtual PointsDouble GetPoints() const = 0;

    virtual bool Contains(
        const tau::Point2d<int> &point,
        double margin) const = 0;

    virtual std::string GetName() const = 0;

    virtual std::unique_ptr<Drag> ProcessMouseDown(
        std::shared_ptr<ControlUserBase> shapeControl,
        const tau::Point2d<int> &click,
        const wxpex::Modifier &modifier,
        CursorControl cursor) = 0;

    virtual bool ProcessControlClick(
        ControlUserBase &,
        const tau::Point2d<int> &)
    {
        return false;
    }

    virtual bool ProcessAltClick(
        ControlUserBase &,
        PointsIterator,
        PointsDouble &)
    {
        return false;
    }

    virtual std::shared_ptr<Shape> Copy() const = 0;

    template<typename DerivedControl>
    static DerivedControl * GetDerivedControl(ControlUserBase &control)
    {
        auto result = dynamic_cast<DerivedControl *>(&control);

        if (!result)
        {
            throw std::logic_error("DerivedControl must be a match");
        }

        return result;
    }
};


using ShapeControl = typename Shape::ControlUserBase;


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
using ShapesControl = typename AsyncShapes::Control;

template<typename Observer>
using ShapesEndpoint = pex::Endpoint<Observer, ShapesControl>;


template<typename T>
struct ShapeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::id, "id"),
        fields::Field(&T::shape, "shape"),
        fields::Field(&T::look, "look"),
        fields::Field(&T::node, "node"));
};


} // end namespace draw
