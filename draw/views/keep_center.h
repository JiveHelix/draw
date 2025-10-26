#pragma once


#include <fields/fields.h>


namespace draw
{


enum class KeepCenter: uint8_t
{
    none = 0x0,
    horizontal = 0x1,
    vertical = 0x2,
    both = 0x3
};

std::string ToString(KeepCenter);

KeepCenter ToValue(fields::Tag<KeepCenter>, std::string_view asString);


struct KeepCenterConverter
{
    static std::string ToString(KeepCenter);

    static KeepCenter ToValue(const std::string &asString);
};


struct KeepCenterChoices
{
    using Type = KeepCenter;
    static std::vector<KeepCenter> GetChoices();
    using Converter = KeepCenterConverter;
};


std::ostream & operator<<(std::ostream &, KeepCenter);


bool HasVertical(KeepCenter keepCenter);
bool HasHorizontal(KeepCenter keepCenter);


} // end namespace
