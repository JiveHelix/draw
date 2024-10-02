#pragma once

#include <set>
#include <cassert>
#include <stdexcept>


namespace draw
{


namespace detail
{


template<typename T>
T CreateUniqueId(std::set<T> &idStore)
{
    T result = 0;

    if (idStore.empty())
    {
        idStore.insert(result);
        return result;
    }

    auto first = idStore.begin();
    auto second = first;
    ++second;

    while (second != idStore.end())
    {
        if ((*second - *first) > 1)
        {
            // There is a gap in IDs.
            // Fill it.
            result = *first + 1;
            idStore.insert(result);

            return result;
        }

        ++first;
        ++second;
    }

    // We got to the end of the set without finding any holes.
    assert(first != idStore.end());

    // std::set lacks a back() function.
    assert(*first == *idStore.rbegin());

    result = *first + 1;
    idStore.insert(result);

    return result;
}


template<typename T>
void ReleaseUniqueId(std::set<T> &idStore, T id)
{
    if (1 != idStore.erase(id))
    {
        throw std::logic_error("Unique ID does not exist.");
    }
}


} // end namespace detail


template<typename T>
class UniqueId
{
public:
    static_assert(std::is_signed_v<T>, "Designed for signed values");
    static_assert(std::is_integral_v<T>, "Designed for integral values");

protected:
    UniqueId(std::set<T> &idStore)
        :
        idStore_(&idStore),
        id_(detail::CreateUniqueId(idStore))
    {

    }

public:
    ~UniqueId()
    {
        if (this->id_ >= 0)
        {
            detail::ReleaseUniqueId(*this->idStore_, this->id_);
        }
    }

    UniqueId(const UniqueId &) = delete;
    UniqueId & operator=(const UniqueId &) = delete;

    UniqueId(UniqueId &&other)
        :
        idStore_(other.idStore_),
        id_(other.id_)
    {
        other.id_ = -1;
    }

    UniqueId & operator=(UniqueId &&other)
    {
        if (this->id_ >= 0)
        {
            detail::ReleaseUniqueId(*this->idStore_, this->id_);
        }

        this->id_ = other.id_;
        other.id_ = -1;

        return *this;
    }

    T Get() const
    {
        return this->id_;
    }

private:
    std::set<T> *idStore_;
    T id_;
};


} // end namespace draw
