#include "draw/views/view_settings.h"


template struct pex::Group
    <
        draw::ViewFields,
        draw::ViewTemplate,
        draw::ViewGroupTemplates_
    >;


namespace draw
{


IntPoint GetMaximumViewPosition(const Size &viewSize, const Size &virtualSize)
{
    IntPoint maximumPosition = (virtualSize - viewSize).ToPoint2d();
    maximumPosition.x = std::max(0, maximumPosition.x);
    maximumPosition.y = std::max(0, maximumPosition.y);

    return maximumPosition;
}


} // end namespace draw
