#include "draw/waveform_settings.h"


template struct pex::Group
    <
        draw::WaveformColorFields,
        draw::WaveformColorTemplate,
        pex::PlainT<draw::WaveformColor>
    >;


template struct pex::Group
    <
        draw::WaveformFields,
        draw::WaveformTemplate,
        pex::PlainT<draw::WaveformSettings>
    >;
