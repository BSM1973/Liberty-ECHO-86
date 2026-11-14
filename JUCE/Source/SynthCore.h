#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace echo86 {
constexpr double pi=3.14159265358979323846;
inline float clamp(float x,float a,float b){return std::max(a,std::min(b,x));}
inline float polyBlep(float t,float dt) {
 if(dt<=0.f)return 0.f;
 if(t<dt){float x=t/dt;return x+x-x*x-1.f;}
 if(t>1.f-dt){float x=(t-1.f)/dt;return x*x+x+x+1.f;}
 return 0.f;
}
// Additional controls are real DSP parameters, not screen-only values.
struct PatchExtras {
 int wave1=0,wave2=0;
 float octave1=0,semi1=0,fine1=0,octave2=0,semi2=0,fine2=0;
 float level1=.78f,level2=.68f,noiseLevel=0,noiseColour=.3f;
 float keyTrack=.65f,velocityFilter=.4f,filterDrive=.12f;
 float transpose=0,masterTune=0,bend=0,portamento=0,velocityResponse=.75f;
 float lfoValue=0,lfoPitch=0,pwm1=0,pwm2=0,envPitch2=0,crossMod=0,filterAudioFM=0;
 float ampBreak=.72f,ampDecay2=.32f,filterBreak=.4f,filterDecay2=.4f,filterRelease=.38f;
 float vintageAge=0,voiceSpread=0;
};
inline float oscillator(int code,float t,float dt,float pulseWidth){
 const float saw=2*t-1-polyBlep(t,dt);
 auto pulse=[&](float width){
  return (t<width?1.f:-1.f)+polyBlep(t,dt)-
           polyBlep(std::fmod(t+1.f-width,1.f),dt);
 };
 switch(code){
  case 1:return pulse(.5f);
  case 2:return pulse(pulseWidth);
  case 3:return 1.f-4.f*std::abs(t-.5f);
  case 4:return std::sin((float)(2*pi)*t);
  case 5:return .55f*saw+.45f*pulse(pulseWidth);
  default:return saw;
 }
}
struct Voice {
 bool active=false,held=false,sustained=false;
 int note=-1;uint64_t age=0;
 double phase1=0,phase2=0,phaseSub=0,pitchHz=0;
 float env=0,filterEnv=0,velocity=0,lp1=0,lp2=0,lp3=0,lp4=0,noiseLp=0;
 uint32_t randomState=1;
 enum Stage{Attack,Decay1,Decay2,Sustain,Release,Off}stage=Off;
 void on(int n,float v,uint64_t a){
  note=n;velocity=v;age=a;active=held=true;sustained=false;
  stage=Attack;env=filterEnv=0;phase1=phase2=phaseSub=0;
  lp1=lp2=lp3=lp4=noiseLp=0;randomState=0x9e3779b9u^(uint32_t)(n*719+a);
  pitchHz=0;
 }
 void off(bool pedal=false){held=false;sustained=pedal;if(active&&!pedal)stage=Release;}
 void pedalUp(){if(active&&sustained){sustained=false;stage=Release;}}
 float render(double sr,float cutoff,float resonance,float attack,float decay,
    float sustain,float release,float detune,float pulseWidth=.5f,float waveMix=0.f,
    float filterAmount=0.f,float filterAttack=.02f,float filterDecay=.35f,
    float filterSustain=.35f,float sync=0.f,const PatchExtras& ext={}) {
  if(!active)return 0;
  auto step=[&](float target,float sec){
   const float c=1.f-std::exp(-1.f/std::max(1.f,sec*(float)sr));
   env+=(target-env)*c;
   return std::abs(target-env)<.001f;
  };
  switch(stage){
   case Attack:if(step(1.f,attack*.23f))stage=Decay1;break;
   case Decay1:if(step(ext.ampBreak,decay*.23f))stage=Decay2;break;
   case Decay2:if(step(sustain,ext.ampDecay2*.38f))stage=Sustain;break;
   case Sustain:env=sustain;break;
   case Release:if(step(0.f,release*.23f)){env=0;active=false;stage=Off;}break;
   case Off:return 0;
  }
  const float fTarget=stage==Release?0.f:(stage==Attack?1.f:
                    stage==Decay1?ext.filterBreak:clamp(filterSustain,0.f,1.f));
  const float fTime=stage==Attack?filterAttack:
                    stage==Release?ext.filterRelease:
                    stage==Decay1?filterDecay:ext.filterDecay2;
  filterEnv+=(fTarget-filterEnv)*(1.f-std::exp(-1.f/std::max(1.f,fTime*.25f*(float)sr)));
  const double targetHz=440.*std::pow(2.,(note+ext.transpose-69)/12.);
  if(pitchHz<=0||ext.portamento<=.0005f)pitchHz=targetHz;
  else pitchHz+=(targetHz-pitchHz)*(1.-std::exp(-1./(sr*std::max(.001f,ext.portamento))));
  const double common=(ext.masterTune+ext.bend+ext.lfoPitch*ext.lfoValue)/1200.;
  const double ageDrift=(1+ext.vintageAge*.0025*std::sin(phase1*6.283));
  const double hz1=pitchHz*std::pow(2.,ext.octave1+ext.semi1/12.+ext.fine1/1200.+common)*ageDrift;
  const double hz2=pitchHz*std::pow(2.,ext.octave2+ext.semi2/12.+
                        (ext.fine2+detune+ext.envPitch2*filterEnv*100.f)/1200.+common+
                        ext.voiceSpread*((int)(this->age%7)-3)/4800.)/ageDrift;
  const float dt=clamp((float)(hz1/sr),.0000001f,.45f);
  const float dt2=clamp((float)(hz2/sr),.0000001f,.45f);
  const float t=(float)phase1,t2=(float)phase2;
  const auto pw1=clamp(pulseWidth+ext.lfoValue*ext.pwm1*.3f,.08f,.92f);
  const auto pw2=clamp(pulseWidth+ext.lfoValue*ext.pwm2*.3f,.08f,.92f);
  const float saw1=2*t-1-polyBlep(t,dt),saw2=2*t2-1-polyBlep(t2,dt2);
  const float osc1=oscillator(ext.wave1,t,dt,pw1)*(1-waveMix)+saw1*waveMix;
  const float osc2=oscillator(ext.wave2,t2,dt2,pw2)*(1-waveMix)+saw2*waveMix;
  phase1+=dt;phase2+=dt2*(1+ext.crossMod*osc1*.28f);
  if(phase1>=1.0){phase1-=std::floor(phase1);phase2*=1.0-clamp(sync,0.f,1.f);}
  phase2-=std::floor(phase2);
  randomState^=randomState<<13;randomState^=randomState>>17;randomState^=randomState<<5;
  const float noise=((float)((randomState>>8)&65535)/32768.f-1.f);
  noiseLp+=clamp(.02f+ext.noiseColour*.92f,0.f,1.f)*(noise-noiseLp);
  const float v=std::pow(clamp(velocity,.001f,1.f),
                clamp(1.f+(ext.velocityResponse-.75f)*1.2f,.4f,1.8f));
  const float in=(osc1*ext.level1*.58f+osc2*ext.level2*.42f+
                   noiseLp*ext.noiseLevel*.25f)*v*env*.25f;
  const float noteTrack=(note-60)*(ext.keyTrack-.65f)/12.f;
  // Audio-rate filter FM is driven by oscillator 2, independently of
  // cross-modulation (oscillator 1 -> oscillator 2 frequency).
  // At depth zero, legacy presets produce identical audio.
  const float effectiveCutoff=clamp(cutoff*std::pow(2.f,filterEnv*filterAmount*5.f+
             noteTrack+(velocity-.5f)*ext.velocityFilter*2.f+
             osc2*ext.filterAudioFM*3.f),40.f,(float)sr*.45f);
  const float g=clamp(1.f-std::exp(-2.f*(float)pi*effectiveCutoff/(float)sr),.0001f,.98f);
  const float feedback=clamp(resonance,0.f,.92f)*3.5f;
  const float drive=1.f+ext.filterDrive*4.f;
  const float x=std::tanh(in*drive-feedback*lp4);
  lp1+=g*(x-lp1);lp2+=g*(lp1-lp2);lp3+=g*(lp2-lp3);lp4+=g*(lp3-lp4);
  return lp4;
 }
};
struct Synth{
 static constexpr int maxVoices=16;
 std::array<Voice,maxVoices> voices{};
 uint64_t age=0;double sampleRate=48000;bool sustainPedal=false;
 bool monoMode=false;float glideTime=0;
 std::array<bool,128> monoHeld{};
 std::array<float,128> monoVel{};
 void prepare(double sr){sampleRate=sr;panic();}
 void panic(){for(auto& v:voices)v=Voice{};sustainPedal=false;monoHeld.fill(false);}
 void allNotesOff(){for(auto& v:voices)if(v.active)v.off(false);sustainPedal=false;monoHeld.fill(false);}
 void sustain(bool down){
  if(sustainPedal==down)return;
  sustainPedal=down;
  if(!down)for(auto& v:voices)v.pedalUp();
 }
 void noteOn(int n,float vel){
  if(monoMode){
   if(n<0||n>=128)return;
   monoHeld[(size_t)n]=true;monoVel[(size_t)n]=clamp(vel,0.f,1.f);
   int lowest=-1;
   for(int i=0;i<128;++i)if(monoHeld[(size_t)i]){lowest=i;break;}
   if(lowest<0)return;
   if(voices[0].active&&voices[0].note==lowest&&voices[0].held)return;
   const double previous=voices[0].active?voices[0].pitchHz:0;
   for(size_t i=1;i<voices.size();++i)voices[i]=Voice{};
   voices[0].on(lowest,monoVel[(size_t)lowest],++age);
   if(previous>0&&glideTime>.001f)voices[0].pitchHz=previous;
   return;
  }
  Voice* choice=nullptr;
  for(auto& v:voices)if(!v.active){choice=&v;break;}
  if(!choice)for(auto& v:voices)if(!v.held&&!v.sustained&&(!choice||v.age<choice->age))choice=&v;
  if(!choice)for(auto& v:voices)if(!v.held&&(!choice||v.age<choice->age))choice=&v;
  if(!choice)for(auto& v:voices)if(!choice||v.age<choice->age)choice=&v;
  if(choice)choice->on(n,clamp(vel,0,1),++age);
 }
 void noteOff(int n){
  if(monoMode&&n>=0&&n<128){
   monoHeld[(size_t)n]=false;
   int lowest=-1;
   for(int i=0;i<128;++i)if(monoHeld[(size_t)i]){lowest=i;break;}
   if(lowest>=0){
    if(voices[0].note!=lowest){
     const double previous=voices[0].pitchHz;
     voices[0].on(lowest,monoVel[(size_t)lowest],++age);
     if(previous>0&&glideTime>.001f)voices[0].pitchHz=previous;
    }
   }else if(voices[0].active)voices[0].off(sustainPedal);
   return;
  }
  for(auto& v:voices)if(v.active&&v.note==n&&v.held)v.off(sustainPedal);
 }
 float sample(float cutoff,float resonance,float attack,float decay,float sustain,float release,
     float detune,float pulseWidth=.5f,float waveMix=0.f,float filterAmount=0.f,
     float filterAttack=.02f,float filterDecay=.35f,float filterSustain=.35f,float sync=0.f,
     const PatchExtras& patch={}){
  float x=0;
  for(auto& v:voices)x+=v.render(sampleRate,cutoff,resonance,attack,decay,sustain,
         release,detune,pulseWidth,waveMix,filterAmount,filterAttack,filterDecay,filterSustain,sync,patch);
  return std::tanh(x*.52f);
 }
};
}
