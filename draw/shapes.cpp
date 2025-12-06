#include "draw/shapes.h"
#include <cstdint>
#include <set>


namespace draw
{


static std::set<int64_t> shapesIds;

static constexpr int64_t resetId = -2;


ShapesId::ShapesId()
    :
    UniqueId<int64_t>(shapesIds)
{

}


Shapes::Shapes()
    :
    id_(-1)
{

}


Shapes::Shapes(int64_t id)
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


int64_t Shapes::GetId() const
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
