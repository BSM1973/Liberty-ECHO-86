#include "SynthCore.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
 echo86::Synth s;s.prepare(48000);
 for(int n=60;n<76;++n)s.noteOn(n,0.8f);
 int active=0;for(auto &v:s.voices)active+=v.active;assert(active==16);
 float peak=0;
 for(int i=0;i<48000;++i){float x=s.sample(4000,.15f,.01f,.25f,.6f,.4f,5);assert(std::isfinite(x));peak=std::max(peak,std::abs(x));}
 for(int n=60;n<76;++n)s.noteOff(n);
 for(int i=0;i<96000;++i)assert(std::isfinite(s.sample(4000,.15f,.01f,.25f,.6f,.4f,5)));
 s.panic();for(auto &v:s.voices)assert(!v.active);
 std::cout<<"PASS sixteen voices, note stealing, release, finite output; peak "<<peak<<"\n";
}
