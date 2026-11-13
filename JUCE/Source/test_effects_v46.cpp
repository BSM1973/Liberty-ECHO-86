#include "StereoEffects.h"
#include "SynthCore.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
 echo86::StereoEffects fx;fx.prepare(48000);
 echo86::StereoEffects::Parameters p;p.chorusMix=.65f;p.chorusRate=1.2f;p.chorusDepth=.8f;
 p.delayMix=.45f;p.delayTime=.27f;p.delayFeedback=.4f;p.reverbMix=.6f;p.reverbSize=.7f;
 echo86::Synth synth;synth.prepare(48000);
 for(int n=48;n<64;++n)synth.noteOn(n,.7f);
 double energy=0;
 for(int i=0;i<96000;++i){
  float x=synth.sample(2000,.3f,.01f,.2f,.5f,.4f,7,.45f,.3f,.5f,.02f,.4f,.3f,.2f);
  float l=x,r=x;fx.process(l,r,p);
  assert(std::isfinite(l)&&std::isfinite(r));assert(std::abs(l)<=1.00001f&&std::abs(r)<=1.00001f);
  energy+=l*l+r*r;
 }
 assert(energy>1.);
 fx.reset();
 std::cout<<"PASS 16-voice stereo chorus/delay/reverb: finite output, nonzero energy\n";
}
