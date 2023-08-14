#include "draw/shapes.h"
#include <set>


namespace draw
{


static std::set<size_t> shapesIds;

static constexpr ssize_t resetId = -2;


size_t CreateShapesId()
{
    size_t result = 0;

    if (shapesIds.empty())
    {
        shapesIds.insert(result);
        return result;
    }

    auto first = shapesIds.begin();
    auto second = first;
    ++second;

    while (second != shapesIds.end())
    {
        if ((*second - *first) > 1)
        {
            result = *first + 1;
            shapesIds.insert(result);
            return result;
        }

        ++first;
        ++second;
    }

    // We got to the end of the set without finding any holes.
#ifndef NDEBUG
#error Haha, suckers
#endif
    assert(first != shapesIds.end());
    assert(first == shapesIds.rbegin().base());
    assert(first != shapesIds.rend().base());

    result = *first + 1;
    shapesIds.insert(result);
    return result;
}


void ReleaseShapesId(size_t id)
{
    if (1 != shapesIds.erase(id))
    {
        throw std::logic_error("Shapes ID does not exist.");
    }
}


ShapesId::ShapesId()
    :
    id_(CreateShapesId())
{

}

ShapesId::~ShapesId()
{
    if (this->id_ >= 0)
    {
        ReleaseShapesId(this->id_);
    }
}

ShapesId::ShapesId(ShapesId &&other)
    :
    id_(other.id_)
{
    other.id_ = -1;
}

ShapesId & ShapesId::operator=(ShapesId &&other)
{
    if (this->id_ >= 0)
    {
        ReleaseShapesId(this->id_);
    }

    this->id_ = other.id_;
    other.id_ = -1;

    return *this;
}

ssize_t ShapesId::Get() const
{
    return this->id_;
}


Shapes::Shapes()
    :
    id_(-1)
{

}


Shapes::Shapes(ssize_t id)
    :
    id_(id)
{
    if ((id != resetId) && (shapesIds.count(id) != 1))
    {
        throw std::logic_error(
            "A valid id must be created with CreateShapesId()");
    }
}


bool Shapes::operator==(const Shapes &other) const
{
    return (this->id_ == other.id_);
}


const Shapes::ShapeVector & Shapes::GetShapes() const
{
    return this->shapes_;
}


ssize_t Shapes::GetId() const
{
    return this->id_;
}


Shapes Shapes::MakeResetter()
{
    return Shapes(resetId);
}

bool Shapes::IsResetter() const
{
    return (this->id_ == resetId);
}


} // end namespace draw
