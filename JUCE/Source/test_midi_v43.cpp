#include "SynthCore.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){
 echo86::Synth s;s.prepare(48000);s.noteOn(60,1);s.sustain(true);s.noteOff(60);
 assert(s.voices[0].active&&!s.voices[0].held&&s.voices[0].sustained);
 for(int i=0;i<4800;++i)assert(std::isfinite(s.sample(2000,.2,.01,.2,.5,.1,5)));
 assert(s.voices[0].active);s.sustain(false);assert(s.voices[0].stage==echo86::Voice::Release);
 for(int i=0;i<48000;++i)assert(std::isfinite(s.sample(2000,.2,.01,.2,.5,.1,5)));
 assert(!s.voices[0].active);
 for(int n=60;n<76;++n)s.noteOn(n,1);s.noteOff(60);s.noteOn(80,1);
 bool found80=false,found60=false;for(auto &v:s.voices){found80|=v.note==80;found60|=v.note==60;}assert(found80&&!found60);
 s.allNotesOff();for(auto &v:s.voices)assert(!v.held);
 s.panic();for(auto &v:s.voices)assert(!v.active);
 std::cout<<"PASS MIDI sustain, pedal release, voice priority, all-notes-off, panic\n";
}
