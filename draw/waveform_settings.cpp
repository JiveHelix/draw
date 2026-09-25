#include "draw/waveform_settings.h"


template struct pex::Group
    <
        draw::WaveformColorSchema,
        pex::PlainT<draw::WaveformColor>
    >;


template struct pex::Group
    <
        draw::WaveformSchema,
        pex::PlainT<draw::WaveformSettings>
    >;
