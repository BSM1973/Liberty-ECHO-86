#include "SynthCore.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

static std::vector<float> render(int waveform,float level,float drive) {
 echo86::Synth s;s.prepare(48000);
 echo86::PatchExtras p;
 p.wave1=waveform;p.wave2=4;
 p.level1=level;p.level2=0.f;p.noiseLevel=0.f;
 p.filterDrive=drive;
 s.noteOn(60,.9f);
 std::vector<float> out;
 out.reserve(8192);
 for(int i=0;i<8192;++i){
  float x=s.sample(7200.f,.13f,.01f,.2f,.7f,.4f,0.f,.49f,
                   0.f,.3f,.02f,.25f,.4f,0.f,p);
  assert(std::isfinite(x));
  out.push_back(x);
 }
 return out;
}
int main(){
 // Hardware waves actually change the signal; oscillator levels actually mute it.
 auto saw=render(0,1.f,.1f),sine=render(4,1.f,.1f),mute=render(0,0.f,.1f);
 double diff=0,signal=0,muted=0;
 for(size_t i=0;i<saw.size();++i){
  diff+=std::abs((double)saw[i]-sine[i]);
  signal+=std::abs((double)saw[i]);
  muted+=std::abs((double)mute[i]);
 }
 assert(signal>0.1&&diff>0.1&&muted<.00001);
 // Monophonic low-note priority and fallback must match the vintage UI.
 echo86::Synth mono;
 mono.prepare(48000);mono.monoMode=true;
 mono.noteOn(60,.7f);mono.noteOn(48,.8f);mono.noteOn(72,.9f);
 assert(mono.voices[0].note==48);
 mono.noteOff(48);assert(mono.voices[0].note==60);
 mono.noteOff(60);assert(mono.voices[0].note==72);
 mono.noteOff(72);assert(!mono.voices[0].held);
 for(int i=0;i<8192;++i)
  assert(std::isfinite(mono.sample(3000,.3f,.02f,.2f,.6f,.4f,0)));
 // Modulation and voice parameters are bounded; no nonfinite audio.
 echo86::PatchExtras ext;
 ext.wave1=5;ext.wave2=2;ext.pwm1=1;ext.pwm2=.8;
 ext.lfoPitch=45;ext.lfoValue=.75;ext.crossMod=.9;
 ext.filterDrive=1;ext.noiseLevel=.7;ext.vintageAge=1;ext.voiceSpread=1;
 echo86::Synth s;s.prepare(48000);s.noteOn(80,.9f);
 for(int i=0;i<10000;++i)
  assert(std::isfinite(s.sample(12000,.8f,.002f,.02f,.7f,.2f,48,.92f,.4f,.9f,
                                  .02f,.03f,.7f,1.f,ext)));
 // Cross Mod and FM AUDIO must have separate DSP destinations.
 const auto fmRender=[](float cross,float filterFM){
   echo86::Synth engine;
   engine.prepare(48000);
   echo86::PatchExtras p;
   p.wave1=0;p.wave2=4;
   p.level1=.83f;p.level2=.83f;
   p.crossMod=cross;p.filterAudioFM=filterFM;
   engine.noteOn(60,.95f);
   std::vector<float> audio;
   audio.reserve(16384);
   for(int i=0;i<16384;++i){
     const float v=engine.sample(2500.f,.2f,.01f,.2f,.8f,.4f,0.f,
                                 .46f,.0f,.4f,.02f,.32f,.5f,0.f,p);
     assert(std::isfinite(v));
     audio.push_back(v);
   }
   return audio;
 };
 const auto original=fmRender(0.f,0.f);
 const auto crossSound=fmRender(.8f,0.f);
 const auto fmSound=fmRender(0.f,.8f);
 double crossDifference=0,fmDifference=0;
 for(size_t i=0;i<original.size();++i){
   crossDifference+=std::abs((double)original[i]-crossSound[i]);
   fmDifference+=std::abs((double)original[i]-fmSound[i]);
 }
 assert(crossDifference>.0005);
 assert(fmDifference>.0005);
 std::cout<<"PASS native UI DSP: waves, mixer, mono priority, modulation, independent audio FM, finite output\n";
}
