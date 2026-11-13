#include "SynthCore.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
 echo86::Synth s;s.prepare(48000);
 static_assert(echo86::Synth::maxVoices==16,"16 voices required");
 for(int n=48;n<64;n++)s.noteOn(n,.7f);
 int active=0;for(auto &v:s.voices)active+=v.active;
 assert(active==16);
 s.noteOn(80,.8f);
 bool found=false, oldestGone=true;
 for(auto &v:s.voices){if(v.note==80)found=true;if(v.note==48)oldestGone=false;}
 assert(found&&oldestGone);
 s.sustain(true);
 s.noteOff(49);
 bool sustained=false;
 for(auto &v:s.voices)if(v.note==49)sustained=v.sustained;
 assert(sustained);
 s.sustain(false);
 for(auto &v:s.voices)if(v.note==49)assert(v.stage==echo86::Voice::Release);
 float peak=0;
 for(int i=0;i<96000;i++){auto x=s.sample(5000,.3f,.01f,.2f,.6f,.5f,6,.45f,.5f);assert(std::isfinite(x));peak=std::max(peak,std::abs(x));}
 s.allNotesOff();
 for(int i=0;i<96000;i++)assert(std::isfinite(s.sample(5000,.3f,.01f,.2f,.6f,.5f,6,.45f,.5f)));
 s.panic();for(auto &v:s.voices)assert(!v.active);
 std::cout<<"PASS: 16 voices, stealing, sustain, release, finite audio; peak "<<peak<<"\n";
}
