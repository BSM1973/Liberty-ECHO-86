#include "FactoryPresets.h"
#include "FactoryPresetExtras.h"
#include "SynthCore.h"
#include <array>
#include <cassert>
#include <cmath>
#include <set>
#include <string>
#include <iostream>
int main(){
 static_assert(echo86::factoryPresets.size()==100);
 static_assert(echo86::parameterIds.size()==24);
 static_assert(echo86::factoryExtendedPresets.size()==100);
 static_assert(echo86::factoryExtendedIds.size()==43);
 std::set<std::string> names,patches;
 std::array<std::set<float>,24> coreVariation;
 std::array<std::set<float>,43> extraVariation;
 for(size_t n=0;n<100;++n){
  const auto& f=echo86::factoryPresets[n];
  const auto& ex=echo86::factoryExtendedPresets[n];
  assert(names.insert(f.name).second);
  assert(std::string(f.name)==std::string(ex.name));
  std::string fingerprint;
  for(size_t i=0;i<24;++i){
   assert(std::isfinite(f.values[i]));
   coreVariation[i].insert(f.values[i]);
   fingerprint+=std::to_string(f.values[i])+",";
  }
  for(size_t i=0;i<43;++i){
   assert(std::isfinite(ex.values[i]));
   extraVariation[i].insert(ex.values[i]);
   fingerprint+=std::to_string(ex.values[i])+",";
  }
  assert(patches.insert(fingerprint).second);
 }
 for(const auto& v:coreVariation)assert(v.size()>1);
 for(const auto& v:extraVariation)assert(v.size()>1);
 // All 100 patches must generate stable, finite signal in the shared DSP core.
 for(size_t n=0;n<100;++n){
  const auto& b=echo86::factoryPresets[n].values;
  const auto& e=echo86::factoryExtendedPresets[n].values;
  const auto value=[&](const char* id)->float{
   for(size_t i=0;i<24;++i)if(std::string(echo86::parameterIds[i])==id)return b[i];
   for(size_t i=0;i<43;++i)if(std::string(echo86::factoryExtendedIds[i])==id)return e[i];
   assert(false);return 0.f;
  };
  echo86::PatchExtras patch;
  patch.wave1=(int)value("osc1Wave");patch.wave2=(int)value("osc2Wave");
  patch.octave1=value("osc1Oct");patch.octave2=value("osc2Oct");
  patch.semi1=value("osc1Semi");patch.semi2=value("osc2Semi");
  patch.fine1=value("osc1Fine");patch.fine2=value("osc2Fine");
  patch.level1=value("osc1Level");patch.level2=value("osc2Level");
  patch.noiseLevel=value("noiseLevel");patch.noiseColour=value("noiseColour");
  patch.keyTrack=value("keyTrack");patch.velocityFilter=value("velocityToFilter");
  patch.filterDrive=value("filterDrive");patch.ampBreak=value("ampBreak");
  patch.ampDecay2=value("ampDecay2");patch.filterBreak=value("filterBreak");
  patch.filterDecay2=value("filterDecay2");patch.filterRelease=value("filterRelease");
  patch.crossMod=value("crossMod");patch.filterAudioFM=value("filterAudioFM");
  patch.vintageAge=value("vintageAge");patch.voiceSpread=value("voiceSpread");
  patch.transpose=value("transpose");patch.masterTune=value("masterTune");
  patch.velocityResponse=value("velocityResponse");
  echo86::Synth synth;synth.prepare(48000);synth.noteOn(60,.8f);
  double energy=0;
  for(int i=0;i<8192;++i){
   float sample=synth.sample(value("cutoff"),value("resonance"),value("attack"),
    value("decay"),value("sustain"),value("release"),value("detune"),
    value("pulseWidth"),value("waveMix"),value("filterEnvAmount"),
    value("filterAttack"),value("filterDecay"),value("filterSustain"),
    value("hardSync"),patch);
   assert(std::isfinite(sample));energy+=(double)sample*sample;
  }
  assert(energy>1.e-25);
 }
 std::cout<<"PASS: 100 unique musical presets, full 67-parameter diversity, 100 DSP renders\n";
}
