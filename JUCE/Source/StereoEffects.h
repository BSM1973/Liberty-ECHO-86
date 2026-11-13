#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
namespace echo86 {
class StereoEffects {
 static constexpr float pi=3.14159265358979323846f;
 struct DelayLine{
  std::vector<float> samples;size_t head=0;
  void prepare(size_t n){samples.assign(n,0.f);head=0;}
  void clear(){std::fill(samples.begin(),samples.end(),0.f);head=0;}
  float tap(float delaySamples)const{
   if(samples.empty())return 0;
   float len=(float)samples.size();
   float pos=(float)head-std::clamp(delaySamples,1.f,len-2.f);
   while(pos<0)pos+=len;
   size_t a=(size_t)pos;size_t b=(a+1)%samples.size();
   float f=pos-(float)a;return samples[a]*(1-f)+samples[b]*f;
  }
  void push(float x){samples[head]=x;head=(head+1)%samples.size();}
 };
 std::array<DelayLine,2> chorus,delay,reverbA,reverbB;
 double sr=48000,phase=0;
public:
 void prepare(double sampleRate){
  sr=std::max(8000.,sampleRate);phase=0;
  for(int c=0;c<2;c++){
   chorus[c].prepare((size_t)(sr*.06)+8);
   delay[c].prepare((size_t)(sr*2.)+8);
   reverbA[c].prepare((size_t)(sr*.15)+8);
   reverbB[c].prepare((size_t)(sr*.21)+8);
  }
 }
 void reset(){
  phase=0;
  for(auto* bank:{&chorus,&delay,&reverbA,&reverbB})for(auto& line:*bank)line.clear();
 }
 struct Parameters {
  float chorusMix=0,chorusRate=.35f,chorusDepth=.5f,chorusWidth=.85f;
  float delayMix=0,delayTime=.35f,delayFeedback=.3f;
  float reverbMix=0,reverbSize=.6f;
 };
 void process(float& left,float& right,const Parameters& p){
  float dry[2]={left,right},wet[2]={};
  phase+=2.0*pi*std::clamp(p.chorusRate,.05f,8.f)/sr;
  if(phase>=2*pi)phase-=2*pi;
  for(int c=0;c<2;c++){
   float x=dry[c];
   float mod=std::sin((float)phase+(c?pi*std::clamp(p.chorusWidth,0.f,1.f):0.f));
   float ch=chorus[c].tap((.018f+.009f*std::clamp(p.chorusDepth,0.f,1.f)*mod)*(float)sr);
   chorus[c].push(x);
   x=x*(1-std::clamp(p.chorusMix,0.f,1.f)*.5f)+ch*std::clamp(p.chorusMix,0.f,1.f)*.5f;
   float dt=std::clamp(p.delayTime,.05f,1.5f)*(float)sr;
   float echo=delay[c].tap(dt+(c?.011f*(float)sr:0.f));
   delay[c].push(std::tanh(x+echo*std::clamp(p.delayFeedback,0.f,.85f)));
   x=x*(1-std::clamp(p.delayMix,0.f,1.f)*.55f)+echo*std::clamp(p.delayMix,0.f,1.f)*.55f;
   // Two cross-coupled short feedback delays: lightweight ambience, not convolution.
   float a=reverbA[c].tap((.041f+.036f*p.reverbSize+(c?.003f:0.f))*(float)sr);
   float b=reverbB[c].tap((.067f+.057f*p.reverbSize+(c?.005f:0.f))*(float)sr);
   reverbA[c].push(std::tanh(x*.25f+b*.67f));
   reverbB[c].push(std::tanh(x*.25f+a*.63f));
   wet[c]=x*(1-std::clamp(p.reverbMix,0.f,1.f)*.6f)+(a+b)*.38f*std::clamp(p.reverbMix,0.f,1.f);
  }
  left=std::tanh(wet[0]);right=std::tanh(wet[1]);
 }
};
}
