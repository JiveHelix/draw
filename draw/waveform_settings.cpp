#include "draw/waveform_settings.h"


template struct pex::Group
    <
        draw::WaveformColorFields,
        draw::WaveformColorTemplate,
        draw::WaveformColor
    >;


template struct pex::Group
    <
        draw::WaveformFields,
        draw::WaveformTemplate,
        draw::WaveformSettings
    >;
