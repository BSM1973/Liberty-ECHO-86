#include <JuceHeader.h>
#include "SynthCore.h"
#include "StereoEffects.h"
#include "FactoryPresets.h"
class Echo86Processor final : public juce::AudioProcessor {
public:
 Echo86Processor():juce::AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),
 state(*this,nullptr,"ECHO86",{
 std::make_unique<juce::AudioParameterFloat>("cutoff","Cutoff",juce::NormalisableRange<float>(60,16000,1,.32f),4200),
 std::make_unique<juce::AudioParameterFloat>("resonance","Resonance",0,0.92f,.16f),
 std::make_unique<juce::AudioParameterFloat>("attack","Attack",.005f,3.f,.02f),
 std::make_unique<juce::AudioParameterFloat>("decay","Decay",.02f,4.f,.32f),
 std::make_unique<juce::AudioParameterFloat>("sustain","Sustain",0,1.f,.65f),
 std::make_unique<juce::AudioParameterFloat>("release","Release",.02f,5.f,.38f),
 std::make_unique<juce::AudioParameterFloat>("detune","DCO2 Detune",-50.f,50.f,5.f),
 std::make_unique<juce::AudioParameterFloat>("pulseWidth","Pulse Width",.08f,.92f,.5f),
 std::make_unique<juce::AudioParameterFloat>("waveMix","Pulse / Saw",0.f,1.f,0.f),
 std::make_unique<juce::AudioParameterFloat>("lfoRate","LFO Rate",.05f,16.f,2.f),
 std::make_unique<juce::AudioParameterFloat>("lfoDepth","LFO Depth",0.f,1.f,0.f),
 std::make_unique<juce::AudioParameterFloat>("filterEnvAmount","Filter Envelope",0.f,1.f,.4f),
 std::make_unique<juce::AudioParameterFloat>("filterAttack","Filter Attack",.005f,3.f,.02f),
 std::make_unique<juce::AudioParameterFloat>("filterDecay","Filter Decay",.02f,4.f,.35f),
 std::make_unique<juce::AudioParameterFloat>("filterSustain","Filter Sustain",0.f,1.f,.35f),
 std::make_unique<juce::AudioParameterFloat>("hardSync","Hard Sync",0.f,1.f,0.f),
 std::make_unique<juce::AudioParameterFloat>("chorusMix","Chorus Mix",0.f,1.f,0.f),
 std::make_unique<juce::AudioParameterFloat>("chorusRate","Chorus Rate",.05f,8.f,.35f),
 std::make_unique<juce::AudioParameterFloat>("chorusDepth","Chorus Depth",0.f,1.f,.5f),
 std::make_unique<juce::AudioParameterFloat>("delayMix","Delay Mix",0.f,1.f,0.f),
 std::make_unique<juce::AudioParameterFloat>("delayTime","Delay Time",.05f,1.5f,.35f),
 std::make_unique<juce::AudioParameterFloat>("delayFeedback","Delay Feedback",0.f,.85f,.3f),
 std::make_unique<juce::AudioParameterFloat>("reverbMix","Reverb Mix",0.f,1.f,0.f),
 std::make_unique<juce::AudioParameterFloat>("reverbSize","Reverb Size",0.f,1.f,.6f),
 std::make_unique<juce::AudioParameterFloat>("osc1Wave","OSC1 Wave",juce::NormalisableRange<float>(0f,5f,1f),0f),
 std::make_unique<juce::AudioParameterFloat>("osc2Wave","OSC2 Wave",juce::NormalisableRange<float>(0f,5f,1f),0f),
 std::make_unique<juce::AudioParameterFloat>("osc1Oct","OSC1 Octave",juce::NormalisableRange<float>(-2f,2f,1f),0f),
 std::make_unique<juce::AudioParameterFloat>("osc1Semi","OSC1 Semitone",juce::NormalisableRange<float>(-12f,12f,1f),0f),
 std::make_unique<juce::AudioParameterFloat>("osc1Fine","OSC1 Fine",juce::NormalisableRange<float>(-50f,50f,1f),0f),
 std::make_unique<juce::AudioParameterFloat>("osc2Oct","OSC2 Octave",juce::NormalisableRange<float>(-2f,2f,1f),0f),
 std::make_unique<juce::AudioParameterFloat>("osc2Semi","OSC2 Semitone",juce::NormalisableRange<float>(-12f,12f,1f),0f),
 std::make_unique<juce::AudioParameterFloat>("osc2Fine","OSC2 Fine",juce::NormalisableRange<float>(-50f,50f,1f),0f),
 std::make_unique<juce::AudioParameterFloat>("osc1Level","OSC1 Level",juce::NormalisableRange<float>(0f,1f,0.001f),0.78f),
 std::make_unique<juce::AudioParameterFloat>("osc2Level","OSC2 Level",juce::NormalisableRange<float>(0f,1f,0.001f),0.68f),
 std::make_unique<juce::AudioParameterFloat>("noiseLevel","Noise Level",juce::NormalisableRange<float>(0f,1f,0.001f),0f),
 std::make_unique<juce::AudioParameterFloat>("noiseColour","Noise Colour",juce::NormalisableRange<float>(0f,1f,0.001f),0.3f),
 std::make_unique<juce::AudioParameterFloat>("keyTrack","Filter Keytrack",juce::NormalisableRange<float>(0f,1f,0.001f),0.65f),
 std::make_unique<juce::AudioParameterFloat>("velocityToFilter","Velocity to Filter",juce::NormalisableRange<float>(0f,1f,0.001f),0.4f),
 std::make_unique<juce::AudioParameterFloat>("filterDrive","Filter Drive",juce::NormalisableRange<float>(0f,1f,0.001f),0.12f),
 std::make_unique<juce::AudioParameterFloat>("masterLevel","Master Level",juce::NormalisableRange<float>(0f,1f,0.001f),0.9f),
 std::make_unique<juce::AudioParameterFloat>("lfoWave","LFO Waveform",juce::NormalisableRange<float>(0f,3f,1f),0f),
 std::make_unique<juce::AudioParameterFloat>("lfoPitch","LFO Pitch",juce::NormalisableRange<float>(0f,100f,0.1f),0f),
 std::make_unique<juce::AudioParameterFloat>("lfoDelay","LFO Delay",juce::NormalisableRange<float>(0f,3f,0.01f),0f),
 std::make_unique<juce::AudioParameterFloat>("lfoPwm1","LFO PWM 1",juce::NormalisableRange<float>(0f,1f,0.001f),0f),
 std::make_unique<juce::AudioParameterFloat>("lfoPwm2","LFO PWM 2",juce::NormalisableRange<float>(0f,1f,0.001f),0f),
 std::make_unique<juce::AudioParameterFloat>("envPitch2","Envelope Pitch 2",juce::NormalisableRange<float>(-1f,1f,0.001f),0f),
 std::make_unique<juce::AudioParameterFloat>("crossMod","Cross Modulation",juce::NormalisableRange<float>(0f,1f,0.001f),0f),
 std::make_unique<juce::AudioParameterFloat>("chorusEnabled","Chorus Enabled",juce::NormalisableRange<float>(0f,1f,1f),1f),
 std::make_unique<juce::AudioParameterFloat>("chorusWidth","Chorus Width",juce::NormalisableRange<float>(0f,1f,0.001f),0.85f),
 std::make_unique<juce::AudioParameterFloat>("monoMode","Mono Mode",juce::NormalisableRange<float>(0f,1f,1f),0f),
 std::make_unique<juce::AudioParameterFloat>("portamento","Portamento",juce::NormalisableRange<float>(0f,1f,0.001f),0f),
 std::make_unique<juce::AudioParameterFloat>("transpose","Transpose",juce::NormalisableRange<float>(-24f,24f,1f),0f),
 std::make_unique<juce::AudioParameterFloat>("masterTune","Master Tune",juce::NormalisableRange<float>(-50f,50f,0.1f),0f),
 std::make_unique<juce::AudioParameterFloat>("velocityResponse","Velocity Response",juce::NormalisableRange<float>(0f,1f,0.001f),0.75f),
 std::make_unique<juce::AudioParameterFloat>("bendRange","Bend Range",juce::NormalisableRange<float>(0f,12f,1f),2f),
 std::make_unique<juce::AudioParameterFloat>("wheelVibrato","Wheel Vibrato",juce::NormalisableRange<float>(0f,100f,0.1f),35f),
 std::make_unique<juce::AudioParameterFloat>("wheelFilter","Wheel to Filter",juce::NormalisableRange<float>(0f,1f,0.001f),0.12f),
 std::make_unique<juce::AudioParameterFloat>("pressureFilter","Pressure to Filter",juce::NormalisableRange<float>(0f,1f,0.001f),0.08f),
 std::make_unique<juce::AudioParameterFloat>("vintageAge","Vintage Age",juce::NormalisableRange<float>(0f,1f,0.001f),0.18f),
 std::make_unique<juce::AudioParameterFloat>("voiceSpread","Voice Spread",juce::NormalisableRange<float>(0f,1f,0.001f),0.3f),
 std::make_unique<juce::AudioParameterFloat>("ampBreak","Amp Breakpoint",juce::NormalisableRange<float>(0f,1f,0.001f),0.72f),
 std::make_unique<juce::AudioParameterFloat>("ampDecay2","Amp Decay 2",juce::NormalisableRange<float>(0.02f,4f,0.001f),0.32f),
 std::make_unique<juce::AudioParameterFloat>("filterBreak","Filter Breakpoint",juce::NormalisableRange<float>(0f,1f,0.001f),0.4f),
 std::make_unique<juce::AudioParameterFloat>("filterDecay2","Filter Decay 2",juce::NormalisableRange<float>(0.02f,4f,0.001f),0.4f),
 std::make_unique<juce::AudioParameterFloat>("filterRelease","Filter Release",juce::NormalisableRange<float>(0.02f,5f,0.001f),0.38f)
 }) {}
 const juce::String getName() const override{return "ECHO 86";}
 void prepareToPlay(double sr,int)override{synth.prepare(sr);effects.prepare(sr);lfoPhase=0;lfoAge=10000.0;}
 void releaseResources()override{}
 bool isBusesLayoutSupported(const BusesLayout& l)const override{return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
 void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi)override{
  synth.monoMode=state.getRawParameterValue("monoMode")->load()>.5f;
  synth.glideTime=state.getRawParameterValue("portamento")->load();
  buffer.clear();int pos=0;
  for(const auto meta:midi){
   const int target=juce::jlimit(0,buffer.getNumSamples(),meta.samplePosition);
   render(buffer,pos,target);
   auto m=meta.getMessage();
   if(m.isNoteOn()){synth.noteOn(m.getNoteNumber(),m.getFloatVelocity());lfoAge=0;}
   else if(m.isNoteOff())synth.noteOff(m.getNoteNumber());
   else if(m.isController() && m.getControllerNumber()==64)synth.sustain(m.getControllerValue()>=64);
   else if(m.isAllSoundOff())synth.panic();
   else if(m.isAllNotesOff())synth.allNotesOff();
   else if(m.isController() && m.getControllerNumber()==1)modWheel=m.getControllerValue()/127.f;
   else if(m.isPitchWheel())pitchWheel=(m.getPitchWheelValue()-8192)/8192.f;
   else if(m.isChannelPressure())channelPressure=m.getChannelPressureValue()/127.f;
   pos=target;
  }
  render(buffer,pos,buffer.getNumSamples());
 }
 void render(juce::AudioBuffer<float>& b,int start,int end){
  if(start>=end)return;
  auto val=[&](const char* id){return state.getRawParameterValue(id)->load();};
  const float cutoff=val("cutoff"),res=val("resonance"),a=val("attack"),d=val("decay"),
   sus=val("sustain"),release=val("release"),det=val("detune"),pw=val("pulseWidth"),
   mix=val("waveMix"),rate=val("lfoRate"),depth=val("lfoDepth"),fe=val("filterEnvAmount"),
   fa=val("filterAttack"),fd=val("filterDecay"),fs=val("filterSustain"),sync=val("hardSync");
  const float master=val("masterLevel");
  synth.monoMode=val("monoMode")>.5f;
  synth.glideTime=val("portamento");
  echo86::PatchExtras ex;
  ex.wave1=(int)val("osc1Wave");ex.wave2=(int)val("osc2Wave");
  ex.octave1=val("osc1Oct");ex.semi1=val("osc1Semi");ex.fine1=val("osc1Fine");
  ex.octave2=val("osc2Oct");ex.semi2=val("osc2Semi");ex.fine2=val("osc2Fine");
  ex.level1=val("osc1Level");ex.level2=val("osc2Level");
  ex.noiseLevel=val("noiseLevel");ex.noiseColour=val("noiseColour");
  ex.keyTrack=val("keyTrack");ex.velocityFilter=val("velocityToFilter");
  ex.filterDrive=val("filterDrive");
  ex.transpose=val("transpose");ex.masterTune=val("masterTune");
  ex.velocityResponse=val("velocityResponse");
  ex.ampBreak=val("ampBreak");ex.ampDecay2=val("ampDecay2");
  ex.filterBreak=val("filterBreak");ex.filterDecay2=val("filterDecay2");
  ex.filterRelease=val("filterRelease");
  ex.vintageAge=val("vintageAge");ex.voiceSpread=val("voiceSpread");
  ex.portamento=val("portamento");
  ex.pwm1=val("lfoPwm1");ex.pwm2=val("lfoPwm2");
  ex.envPitch2=val("envPitch2");ex.crossMod=val("crossMod");
  ex.bend=pitchWheel*val("bendRange")*100.f;
  const float lfoPitch=val("lfoPitch")+modWheel*val("wheelVibrato");
  const float lfoDepth=depth+modWheel*val("wheelFilter")+
                              channelPressure*val("pressureFilter");
  const int wave=(int)val("lfoWave");
  const float delay=val("lfoDelay");
  echo86::StereoEffects::Parameters fx;
  fx.chorusMix=val("chorusEnabled")>.5f?val("chorusMix"):0.f;
  fx.chorusRate=val("chorusRate");fx.chorusDepth=val("chorusDepth");
  fx.chorusWidth=val("chorusWidth");
  fx.delayMix=val("delayMix");fx.delayTime=val("delayTime");fx.delayFeedback=val("delayFeedback");
  fx.reverbMix=val("reverbMix");fx.reverbSize=val("reverbSize");
  for(int i=start;i<end;++i){
   const float phase=(float)lfoPhase;
   float lfo=std::sin(phase);
   if(wave==1)lfo=2.f/(float)echo86::pi*std::asin(std::sin(phase));
   if(wave==2)lfo=std::sin(phase)>=0?1.f:-1.f;
   if(wave==3)lfo=lfoHeld;
   lfoPhase+=2.0*echo86::pi*rate/synth.sampleRate;
   if(lfoPhase>=2*echo86::pi){
    lfoPhase-=2*echo86::pi;
    randomState^=randomState<<13;randomState^=randomState>>17;randomState^=randomState<<5;
    lfoHeld=((float)((randomState>>8)&65535)/32768.f-1.f);
   }
   lfoAge=std::min(lfoAge+1.0/synth.sampleRate,10000.0);
   const float fade=delay<=.001f?1.f:echo86::clamp((float)((lfoAge-delay)/.05),0.f,1.f);
   ex.lfoValue=lfo*fade;
   ex.lfoPitch=lfoPitch;
   const float modCutoff=echo86::clamp(cutoff*std::pow(2.f,ex.lfoValue*lfoDepth*2.f),60.f,16000.f);
   float x=synth.sample(modCutoff,res,a,d,sus,release,det,pw,mix,fe,fa,fd,fs,sync,ex);
   float left=x,right=x;
   effects.process(left,right,fx);
   if(b.getNumChannels()>0)b.setSample(0,i,left*master);
   if(b.getNumChannels()>1)b.setSample(1,i,right*master);
  }
 }
 juce::AudioProcessorEditor* createEditor()override;
 bool hasEditor()const override{return true;}
 double getTailLengthSeconds()const override{return 5.;}
 bool acceptsMidi()const override{return true;}
 bool producesMidi()const override{return false;}
 bool isMidiEffect()const override{return false;}
 int getNumPrograms()override{return 1;}
 int getCurrentProgram()override{return 0;}
 void setCurrentProgram(int)override{}
 const juce::String getProgramName(int)override{return "Init";}
 void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock& b)override{auto xml=state.copyState().createXml();copyXmlToBinary(*xml,b);}
 void setStateInformation(const void* data,int size)override{auto xml=getXmlFromBinary(data,size);if(xml&&xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));}
private:
 echo86::Synth synth;
 echo86::StereoEffects effects;
 double lfoPhase=0,lfoAge=10000.0;
 float lfoHeld=0,modWheel=0,pitchWheel=0,channelPressure=0;
 uint32_t randomState=0x8bc18285u;
 juce::AudioProcessorValueTreeState state;
 friend class Echo86Editor;
};
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Echo86Processor();}


#include "NativeHardwareEditor.h"
juce::AudioProcessorEditor* Echo86Processor::createEditor(){return new Echo86Editor(*this);}
