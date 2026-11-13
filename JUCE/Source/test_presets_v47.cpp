#include "FactoryPresets.h"
#include "SynthCore.h"
#include "StereoEffects.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <chrono>
int main(){
 static_assert(echo86::factoryPresets.size()==100);
 static_assert(echo86::parameterIds.size()==24);
 for(const auto& preset:echo86::factoryPresets){
  for(float v:preset.values)assert(std::isfinite(v));
  assert(preset.name&&preset.name[0]);
 }
 echo86::Synth synth;echo86::StereoEffects fx;
 synth.prepare(48000);fx.prepare(48000);
 for(int n=48;n<64;++n)synth.noteOn(n,.75f);
 echo86::StereoEffects::Parameters params;
 params.chorusMix=.4f;params.delayMix=.35f;params.reverbMix=.45f;
 auto t=std::chrono::steady_clock::now();
 double sum=0;
 for(int i=0;i<48000*5;++i){
  float x=synth.sample(2400,.35f,.01f,.25f,.65f,.4f,6,.5f,.4f,.5f,.03f,.4f,.3f,.15f);
  float l=x,r=x;fx.process(l,r,params);
  assert(std::isfinite(l)&&std::isfinite(r));
  sum+=l*l+r*r;
 }
 auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();
 assert(sum>0);
 std::cout<<"PASS 100 factory presets, 16 voices, 5s audio at 48kHz; offline "<<ms<<" ms; energy "<<sum<<"\n";
}
