#include "SynthCore.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
 echo86::Synth s;s.prepare(48000);
 for(int n=48;n<64;++n)s.noteOn(n,.8f);
 for(int i=0;i<48000;++i){
  auto x=s.sample(1800,.4f,.01f,.2f,.6f,.4f,8,.35f,.65f,.7f,.02f,.3f,.2f,1.f);
  assert(std::isfinite(x));assert(std::abs(x)<=1.f);
 }
 int active=0;for(auto &v:s.voices)active+=v.active;assert(active==16);
 s.allNotesOff();
 for(int i=0;i<96000;++i)assert(std::isfinite(s.sample(1800,.4f,.01f,.2f,.6f,.4f,8,.35f,.65f,.7f,.02f,.3f,.2f,1.f)));
 std::cout<<"PASS: 16 voices, filter envelope, experimental sync, finite audio\\n";
}
