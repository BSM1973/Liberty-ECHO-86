#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace echo86 {
constexpr double pi=3.14159265358979323846;
inline float clamp(float x,float a,float b){return std::max(a,std::min(b,x));}
inline float polyBlep(float t,float dt){
 if(dt<=0.f)return 0.f;
 if(t<dt){float x=t/dt;return x+x-x*x-1.f;}
 if(t>1.f-dt){float x=(t-1.f)/dt;return x*x+x+x+1.f;}
 return 0.f;
}
struct Voice{
 bool active=false, held=false, sustained=false;
 int note=-1; uint64_t age=0;
 double phase1=0,phase2=0,phaseSub=0;
 float env=0, filterEnv=0, velocity=0,lp1=0,lp2=0,lp3=0,lp4=0;
 enum Stage{Attack,Decay1,Decay2,Sustain,Release,Off} stage=Off;
 void on(int n,float v,uint64_t a){note=n;velocity=v;age=a;active=held=true;sustained=false;stage=Attack;env=0;filterEnv=0;phase1=phase2=phaseSub=0;lp1=lp2=lp3=lp4=0;}
 void off(bool pedal=false){held=false;sustained=pedal;if(active&&!pedal)stage=Release;}
 void pedalUp(){if(active&&sustained){sustained=false;stage=Release;}}
 float render(double sr,float cutoff,float resonance,float attack,float decay,float sustain,float release,float detune,float pulseWidth=.5f,float waveMix=0.f,float filterAmount=0.f,float filterAttack=.02f,float filterDecay=.35f,float filterSustain=.35f,float sync=0.f){
  if(!active)return 0;
  auto step=[&](float target,float sec){float c=1.f-std::exp(-1.f/std::max(1.f,sec*(float)sr));env+=(target-env)*c;return std::abs(target-env)<.001f;};
  switch(stage){
   case Attack:if(step(1.f,attack*.23f))stage=Decay1;break;
   case Decay1:if(step(.72f,decay*.23f))stage=Decay2;break;
   case Decay2:if(step(sustain,decay*.38f))stage=Sustain;break;
   case Sustain:env=sustain;break;
   case Release:if(step(0.f,release*.23f)){env=0;active=false;stage=Off;}break;
   case Off:return 0;
  }
  // Dedicated filter envelope with attack/decay/sustain and release.
  const float fTarget=stage==Release?0.f:(stage==Attack?1.f:clamp(filterSustain,0.f,1.f));
  const float fTime=stage==Attack?filterAttack:(stage==Release?release:filterDecay);
  filterEnv+=(fTarget-filterEnv)*(1.f-std::exp(-1.f/std::max(1.f,fTime*.25f*(float)sr)));
  double hz=440.*std::pow(2.,(note-69)/12.);
  float dt=(float)(hz/sr),dt2=(float)(hz*std::pow(2.,detune/1200.f)/sr);
  float t=(float)phase1,t2=(float)phase2;
  float saw1=2.f*t-1.f-polyBlep(t,dt);
  float saw2=2.f*t2-1.f-polyBlep(t2,dt2);
  pulseWidth=clamp(pulseWidth,.08f,.92f);waveMix=clamp(waveMix,0.f,1.f);
  auto pulse=[&](float ph,float step){float p=ph<pulseWidth?1.f:-1.f;return p+polyBlep(ph,step)-polyBlep(std::fmod(ph+1.f-pulseWidth,1.f),step);};
  float osc1=saw1*(1-waveMix)+pulse(t,dt)*waveMix;
  float osc2=saw2*(1-waveMix)+pulse(t2,dt2)*waveMix;
  const double oldPhase1=phase1;
  phase1+=dt;phase2+=dt2;
  if(phase1>=1.0){
   phase1-=std::floor(phase1);
   // Experimental oscillator hard-sync. Crossfade limits the discontinuity
   // but this is not yet a band-limited BLEP sync implementation.
   phase2=phase2*(1.0-clamp(sync,0.f,1.f));
  }
  phase2-=std::floor(phase2);
  float input=(osc1*.58f+osc2*.42f)*velocity*env*.20f;
  // Stable 4-pole lowpass approximation; NOT a circuit model of CEM3396.
  float effectiveCutoff=clamp(cutoff*std::pow(2.f,filterEnv*filterAmount*5.f),40.f,(float)sr*.45f);
  float g=clamp(1.f-std::exp(-2.f*(float)pi*effectiveCutoff/(float)sr),.0001f,.98f);
  float feedback=clamp(resonance,0.f,.92f)*3.5f;
  float x=std::tanh(input-feedback*lp4);
  lp1+=g*(x-lp1);lp2+=g*(lp1-lp2);lp3+=g*(lp2-lp3);lp4+=g*(lp3-lp4);
  return lp4;
 }
};
struct Synth{
 static constexpr int maxVoices=16;
 std::array<Voice,maxVoices> voices{};
 uint64_t age=0;double sampleRate=48000;bool sustainPedal=false;
 void prepare(double sr){sampleRate=sr;panic();}
 void panic(){for(auto &v:voices)v=Voice{};sustainPedal=false;}
 void allNotesOff(){for(auto &v:voices)if(v.active)v.off(false);sustainPedal=false;}
 void sustain(bool down){if(sustainPedal==down)return;sustainPedal=down;if(!down)for(auto &v:voices)v.pedalUp();}
 void noteOn(int note,float velocity){
  Voice* choice=nullptr;
  for(auto &v:voices)if(!v.active){choice=&v;break;}
  if(!choice)for(auto &v:voices)if(!v.held&&!v.sustained&&(!choice||v.age<choice->age))choice=&v;
  if(!choice)for(auto &v:voices)if(!v.held&&(!choice||v.age<choice->age))choice=&v;
  if(!choice)for(auto &v:voices)if(!choice||v.age<choice->age)choice=&v;
  if(choice)choice->on(note,clamp(velocity,0,1),++age);
 }
 void noteOff(int note){for(auto &v:voices)if(v.active&&v.note==note&&v.held)v.off(sustainPedal);}
 float sample(float cutoff,float resonance,float attack,float decay,float sustain,float release,float detune,float pulseWidth=.5f,float waveMix=0.f,float filterAmount=0.f,float filterAttack=.02f,float filterDecay=.35f,float filterSustain=.35f,float sync=0.f){
  float x=0;for(auto &v:voices)x+=v.render(sampleRate,cutoff,resonance,attack,decay,sustain,release,detune,pulseWidth,waveMix,filterAmount,filterAttack,filterDecay,filterSustain,sync);
  // Gain adjusted to keep dense 16-note chords below hard saturation.
  return std::tanh(x*.52f);
 }
};
}
