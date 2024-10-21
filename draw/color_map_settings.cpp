#include "draw/color_map_settings.h"


template struct pex::Group
    <
        draw::ColorMapSettingsFields,
        draw::ColorMapSettingsTemplate<int32_t>::template Template,
        draw::ColorMapSettingsCustom<int32_t>
    >;
