#include "draw/shapes.h"
#include <set>


namespace draw
{


static std::set<ssize_t> shapesIds;

static constexpr ssize_t resetId = -2;


ShapesId::ShapesId()
    :
    UniqueId<ssize_t>(shapesIds)
{

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
            "A valid id must be created with CreateUniqueId()");
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
