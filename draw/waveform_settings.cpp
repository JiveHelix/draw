#include "draw/waveform_settings.h"


template struct pex::Group
    <
        draw::WaveformColorTemplate,
        pex::PlainT<draw::WaveformColor>
    >;


template struct pex::Group
    <
        draw::WaveformTemplate,
        pex::PlainT<draw::WaveformSettings>
    >;
