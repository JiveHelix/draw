#include <draw/views/keep_center.h>



namespace draw
{


std::map<KeepCenter, std::string_view> partialsStringsById{
    {KeepCenter::none, "none"},
    {KeepCenter::horizontal, "horizontal"},
    {KeepCenter::vertical, "vertical"},
    {KeepCenter::both, "both"}};


std::unordered_map<std::string_view, KeepCenter> GetKeepCenterByString()
{
    std::unordered_map<std::string_view, KeepCenter> result;

    for (auto [key, value]: partialsStringsById)
    {
        result[value] = key;
    }

    return result;
}


std::string ToString(KeepCenter partials)
{
    return std::string(partialsStringsById.at(partials));
}


KeepCenter ToValue(fields::Tag<KeepCenter>, std::string_view asString)
{
    static const auto partialsByString = GetKeepCenterByString();

    return partialsByString.at(asString);
}


std::string KeepCenterConverter::ToString(KeepCenter partials)
{
    return draw::ToString(partials);
}


KeepCenter KeepCenterConverter::ToValue(const std::string &asString)
{
    return ::draw::ToValue(fields::Tag<KeepCenter>{}, asString);
}


std::vector<KeepCenter> KeepCenterChoices::GetChoices()
{
    return {
        KeepCenter::none,
        KeepCenter::horizontal,
        KeepCenter::vertical,
        KeepCenter::both};
}


std::ostream & operator<<(std::ostream &outputStream, KeepCenter partials)
{
    return outputStream << draw::ToString(partials);
}


bool HasVertical(KeepCenter keepCenter)
{
    return (
        static_cast<uint8_t>(keepCenter)
        & static_cast<uint8_t>(KeepCenter::vertical)); 
}


bool HasHorizontal(KeepCenter keepCenter)
{
    return (
        static_cast<uint8_t>(keepCenter)
        & static_cast<uint8_t>(KeepCenter::horizontal)); 
}


} // end namespace draw
