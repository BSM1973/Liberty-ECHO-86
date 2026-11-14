#pragma once
#include <cmath>
#include <cstdint>

namespace echo86 {
inline constexpr double lfoTwoPi = 6.2831853071795864769;

// Shared LFO waveform definitions for the audio processor and JUCE preview.
inline float lfoWaveSample(int wave,float phase,float held) {
    switch(wave) {
        case 1: return 0.6366197723675814f*std::asin(std::sin(phase)); // triangle
        case 2: return std::sin(phase)>=0.f ? 1.f : -1.f;              // square
        case 3: return held;                                           // sample & hold
        default: return std::sin(phase);                               // sine
    }
}

// Visual-only deterministic held values. The DSP generates its own random
// held values each period; this preview depicts that waveform without
// consuming or changing the live audio engine random state.
inline float lfoPreviewSample(int wave,double absolutePhase) {
    if(wave!=3) return lfoWaveSample(wave,(float)std::fmod(absolutePhase,lfoTwoPi),0.f);
    const auto cycle=(int64_t)std::floor(absolutePhase/lfoTwoPi);
    uint32_t v=(uint32_t)cycle;
    v^=v>>16; v*=0x7feb352dU; v^=v>>15;
    v*=0x846ca68bU;v^=v>>16;
    return (float)(v&0xffffu)/32767.5f-1.f;
}
}
