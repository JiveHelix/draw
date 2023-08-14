#pragma once


#include <pex/endpoint.h>
#include <memory>
#include <vector>
#include <wxpex/graphics.h>
#include <wxpex/async.h>


namespace draw
{


class Shape
{
public:
    virtual ~Shape()
    {

    }

    virtual void Draw(wxpex::GraphicsContext &) = 0;
};


size_t CreateShapesId();


void ReleaseShapesId();


class ShapesId
{
public:
    ShapesId();

    ~ShapesId();

    ShapesId(const ShapesId &) = delete;
    ShapesId & operator=(const ShapesId &) = delete;

    ShapesId(ShapesId &&other);
    ShapesId & operator=(ShapesId &&other);

    ssize_t Get() const;

private:
    ssize_t id_;
};


struct Shapes
{
public:
    static constexpr bool isShape = true;

    using ShapeVector = std::vector<std::shared_ptr<Shape>>;

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

    static Shapes MakeResetter();

    bool IsResetter() const;

private:
    ssize_t id_;
    std::vector<std::shared_ptr<Shape>> shapes_;
};


using AsyncShapes = wxpex::MakeAsync<Shapes>;
using ShapesControl = typename AsyncShapes::Control;

template<typename Observer>
using ShapesEndpoint = pex::Endpoint<Observer, ShapesControl>;


} // end namespace draw
