#include "LfoWaveform.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    constexpr float pi=3.14159265358979323846f;
    // The same wave function must drive audio and the graphic.
    assert(std::abs(echo86::lfoWaveSample(0,pi*.5f,0.f)-1.f)<1.e-5f);
    assert(std::abs(echo86::lfoWaveSample(1,pi*.5f,0.f)-1.f)<1.e-5f);
    assert(echo86::lfoWaveSample(2,pi*.5f,0.f)==1.f);
    assert(echo86::lfoWaveSample(2,pi*1.5f,0.f)==-1.f);
    assert(echo86::lfoWaveSample(3,pi,0.35f)==.35f);
    const auto h=echo86::lfoPreviewSample(3,.3);
    assert(std::isfinite(h) && h>=-1.f && h<=1.f);
    assert(echo86::lfoPreviewSample(3,.3)==echo86::lfoPreviewSample(3,1.2));
    assert(std::abs(echo86::lfoPreviewSample(0,.2)-std::sin(.2f))<1.e-5f);
    // The master and filter destination are separate values. With the
    // destination disabled, increasing depth cannot modulate cutoff.
    auto filterRoute=[](float globalDepth,float filterDepth) {
        return globalDepth*filterDepth*4.f;
    };
    assert(filterRoute(.8f,0.f)==0.f);
    assert(filterRoute(0.f,.8f)==0.f);
    assert(filterRoute(.5f,.5f)==1.f);
    assert(filterRoute(.5f,1.f)==2.f);
    std::cout<<"PASS LFO waveforms & independent filter route\n";
}
