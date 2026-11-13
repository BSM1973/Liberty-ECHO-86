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
 std::make_unique<juce::AudioParameterFloat>("reverbSize","Reverb Size",0.f,1.f,.6f)
 }) {}
 const juce::String getName() const override{return "ECHO 86";}
 void prepareToPlay(double sr,int)override{synth.prepare(sr);effects.prepare(sr);lfoPhase=0;}
 void releaseResources()override{}
 bool isBusesLayoutSupported(const BusesLayout& l)const override{return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
 void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi)override{
  buffer.clear();int pos=0;
  for(const auto meta:midi){
   const int target=juce::jlimit(0,buffer.getNumSamples(),meta.samplePosition);
   render(buffer,pos,target);
   auto m=meta.getMessage();
   if(m.isNoteOn())synth.noteOn(m.getNoteNumber(),m.getFloatVelocity());
   else if(m.isNoteOff())synth.noteOff(m.getNoteNumber());
   else if(m.isController() && m.getControllerNumber()==64)synth.sustain(m.getControllerValue()>=64);
   else if(m.isAllSoundOff())synth.panic();
   else if(m.isAllNotesOff())synth.allNotesOff();
   pos=target;
  }
  render(buffer,pos,buffer.getNumSamples());
 }
 void render(juce::AudioBuffer<float>& b,int start,int end){
  auto val=[&](const char* name){return state.getRawParameterValue(name)->load();};
  const float cutoff=val("cutoff"),res=val("resonance"),a=val("attack"),d=val("decay"),s=val("sustain"),r=val("release"),det=val("detune"),pw=val("pulseWidth"),mix=val("waveMix"),lfoRate=val("lfoRate"),lfoDepth=val("lfoDepth"),fe=val("filterEnvAmount"),fa=val("filterAttack"),fd=val("filterDecay"),fs=val("filterSustain"),sync=val("hardSync");
  echo86::StereoEffects::Parameters fx;
  fx.chorusMix=val("chorusMix");fx.chorusRate=val("chorusRate");fx.chorusDepth=val("chorusDepth");
  fx.delayMix=val("delayMix");fx.delayTime=val("delayTime");fx.delayFeedback=val("delayFeedback");
  fx.reverbMix=val("reverbMix");fx.reverbSize=val("reverbSize");
  for(int i=start;i<end;++i){float lfo=std::sin(float(lfoPhase)); lfoPhase+=2.0*echo86::pi*lfoRate/synth.sampleRate; if(lfoPhase>=2*echo86::pi)lfoPhase-=2*echo86::pi;
   float modCutoff=echo86::clamp(cutoff*std::pow(2.f,lfo*lfoDepth*2.f),60.f,16000.f);
   float x=synth.sample(modCutoff,res,a,d,s,r,det,pw,mix,fe,fa,fd,fs,sync);float left=x,right=x;effects.process(left,right,fx);if(b.getNumChannels()>0)b.setSample(0,i,left);if(b.getNumChannels()>1)b.setSample(1,i,right);}
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
 double lfoPhase=0;
 juce::AudioProcessorValueTreeState state;
 friend class Echo86Editor;
};
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Echo86Processor();}

// Original Liberty visual language: typography, vector panels and custom controls.
// No inherited PluginMaker.ai images, skin assets or facade arrangement.
class Echo86Editor final : public juce::AudioProcessorEditor {
public:
 explicit Echo86Editor(Echo86Processor& p):AudioProcessorEditor(p),processor(p){
  setSize(960,1025);
  for(const auto* category:{"TOUS","BASS","LEAD","PAD","KEYS","PLUCK","ARP","STRINGS","FX","INIT"})
   categoryList.addItem(category,categoryList.getNumItems()+1);
  categoryList.setSelectedId(1,juce::dontSendNotification);
  categoryList.onChange=[this]{refreshFactoryList();};
  addAndMakeVisible(categoryList);
  refreshFactoryList();
  presetList.onChange=[this]{
   const int index=presetList.getSelectedId()-1;
   if(index<0||index>=(int)echo86::factoryPresets.size())return;
   const auto& preset=echo86::factoryPresets[(size_t)index];
   for(size_t j=0;j<echo86::parameterIds.size();++j){
    auto* param=processor.state.getParameter(echo86::parameterIds[j]);
    if(param){
     const float normalized=param->convertTo0to1(preset.values[j]);
     param->beginChangeGesture();param->setValueNotifyingHost(normalized);param->endChangeGesture();
    }
   }
   presetName.setText(juce::String::fromUTF8(preset.name),juce::dontSendNotification);
  };
  presetName.setText("MON SON",juce::dontSendNotification);
  presetName.setSelectAllWhenFocused(true);
  addAndMakeVisible(presetName);
  saveButton.setButtonText("EXPORTER");
  loadButton.setButtonText("IMPORTER");
  saveButton.onClick=[this]{exportPreset();};
  loadButton.onClick=[this]{importPreset();};
  addAndMakeVisible(saveButton);
  addAndMakeVisible(loadButton);
  addAndMakeVisible(presetList);
  const char* ids[]={"cutoff","resonance","attack","decay","sustain","release","detune","pulseWidth","waveMix","lfoRate","lfoDepth","filterEnvAmount","filterAttack","filterDecay","filterSustain","hardSync","chorusMix","chorusRate","chorusDepth","delayMix","delayTime","delayFeedback","reverbMix","reverbSize"};
  const char* captions[]={"FRÉQUENCE","RÉSONANCE","ATTAQUE","DÉCLIN","MAINTIEN","RELÂCHEMENT","DÉTUNE","IMPULSION","FORME","VITESSE","PROFONDEUR","ENV FILTRE","ATTAQUE FILTRE","DÉCLIN FILTRE","MAINTIEN FILTRE","SYNC DCO","CHORUS","VITESSE","PROFONDEUR","DELAY","TEMPS","FEEDBACK","REVERB","TAILLE"};
  for(int i=0;i<24;++i){
   auto* knob=knobs.add(new juce::Slider());knob->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
   knob->setTextBoxStyle(juce::Slider::TextBoxBelow,false,76,19);
   knob->setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffdfaa62));
   knob->setColour(juce::Slider::rotarySliderOutlineColourId,juce::Colour(0xff2c5263));
   knob->setColour(juce::Slider::thumbColourId,juce::Colour(0xfff7ddad));
   knob->setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);
   knob->setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
   addAndMakeVisible(knob);
   auto* label=labels.add(new juce::Label());label->setText(juce::String::fromUTF8(captions[i]),juce::dontSendNotification);
   label->setJustificationType(juce::Justification::centred);
   label->setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
   label->setColour(juce::Label::textColourId,juce::Colour(0xffb6d5db));addAndMakeVisible(label);
   attachments.emplace_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,ids[i],*knob));
  }
 }
 void paint(juce::Graphics& g)override{
  auto r=getLocalBounds();g.fillAll(juce::Colour(0xff081b29));
  g.setColour(juce::Colour(0xff15384b));g.fillRoundedRectangle(r.reduced(18).toFloat(),20);
  g.setColour(juce::Colour(0xffe7b16d));g.setFont(juce::Font(juce::FontOptions(39.0f).withStyle("Bold")));
  g.drawText("ECHO 86",40,28,400,55,juce::Justification::centredLeft);
  g.setColour(juce::Colour(0xffaac6cc));g.setFont(juce::Font(juce::FontOptions(13.0f)));
  g.drawText("LIBERTY MUSIC SOFTWARE  /  ANALOG POLYSYNTH",42,80,550,24,juce::Justification::centredLeft);
  g.setColour(juce::Colour(0xff284f60));g.drawLine(40,118,920,118,1.5f);
  auto panel=[&](int x,int y,int w,int h,const juce::String& title){
   g.setColour(juce::Colour(0xff0d2838));g.fillRoundedRectangle((float)x,(float)y,(float)w,(float)h,13);
   g.setColour(juce::Colour(0xff31576a));g.drawRoundedRectangle((float)x,(float)y,(float)w,(float)h,13,1);
   g.setColour(juce::Colour(0xffe7b16d));g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
   g.drawText(title,x+16,y+11,w-32,20,juce::Justification::centredLeft);
  };
  panel(36,140,435,182,"01  /  OSCILLATEURS");
  panel(489,140,435,182,"02  /  FILTRE");
  panel(36,338,578,182,"03  /  ENVELOPPE");
  panel(632,338,292,182,"04  /  MODULATION");
  panel(36,535,888,162,"05  /  ENVELOPPE FILTRE ET SYNC");
  panel(36,712,888,163,"06  /  EFFETS STÉRÉO");
  panel(36,885,888,115,"07  /  BIBLIOTHÈQUE");
  g.setColour(juce::Colour(0xff9ab4bc));g.setFont(juce::Font(juce::FontOptions(11.0f)));
  g.drawText("16 VOIX  •  2 DCO  •  MIDI",690,75,225,25,juce::Justification::centredRight);
  g.drawText("ECHO 86  /  JUCE EDITION  /  PROTOTYPE",42,1004,870,16,juce::Justification::centredRight);
 }
 void resized()override{
  categoryList.setBounds(210,892,154,26);
  presetList.setBounds(372,892,523,26);
  presetName.setBounds(210,944,325,28);
  saveButton.setBounds(550,944,162,28);
  loadButton.setBounds(727,944,168,28);
  const juce::Rectangle<int> boxes[]={
   {75,194,105,110},{205,194,105,110},{335,194,105,110},
   {555,194,125,110},{718,194,125,110},
   {60,390,112,110},{198,390,112,110},{336,390,112,110},{474,390,112,110},
   {657,390,112,110},{790,390,112,110},
   {60,577,135,108},{228,577,135,108},{396,577,135,108},{564,577,135,108},{732,577,135,108},
   {47,753,101,106},{153,753,101,106},{259,753,101,106},{365,753,101,106},
   {471,753,101,106},{577,753,101,106},{683,753,101,106},{789,753,101,106}
  };
  // Oscillators, filter, amplitude envelope, LFO.
  const int mapped[]={6,7,8,0,1,2,3,4,5,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
  for(int i=0;i<24;++i){const auto b=boxes[i];knobs[mapped[i]]->setBounds(b.getX(),b.getY()+20,b.getWidth(),b.getHeight()-18);labels[mapped[i]]->setBounds(b.getX()-10,b.getY(),b.getWidth()+20,19);}
 }
private:
 void refreshFactoryList(){
  const juce::String selected=categoryList.getText();
  presetList.clear(juce::dontSendNotification);
  for(size_t i=0;i<echo86::factoryPresets.size();++i){
   const auto name=juce::String::fromUTF8(echo86::factoryPresets[i].name);
   if(selected=="TOUS"||name.startsWithIgnoreCase(selected+" ")||name.startsWithIgnoreCase(selected+" •"))
    presetList.addItem(name,(int)i+1);
  }
  presetList.setTextWhenNothingSelected("100 SONS D'USINE • CHOISIR UN PRESET");
 }
 juce::var currentPreset()const{
  auto* obj=new juce::DynamicObject();
  obj->setProperty("format","ECHO86-PRESET");
  obj->setProperty("version",1);
  obj->setProperty("name",presetName.getText());
  auto* values=new juce::DynamicObject();
  for(const char* id:echo86::parameterIds)
   values->setProperty(id,processor.state.getRawParameterValue(id)->load());
  obj->setProperty("parameters",juce::var(values));
  return juce::var(obj);
 }
 void exportPreset(){
  auto suggested=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
     .getChildFile(juce::File::createLegalFileName(presetName.getText())+".echo86");
  fileChooser=std::make_unique<juce::FileChooser>("Exporter un son ECHO 86",suggested,"*.echo86");
  fileChooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|
     juce::FileBrowserComponent::warnAboutOverwriting,[this](const juce::FileChooser& chooser){
    const auto file=chooser.getResult();
    if(file==juce::File{})return;
    const auto json=juce::JSON::toString(currentPreset(),true);
    if(!file.replaceWithText(json))juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"ECHO 86","Impossible d'enregistrer le preset.");
  });
 }
 void importPreset(){
  fileChooser=std::make_unique<juce::FileChooser>("Importer un son ECHO 86",juce::File{},"*.echo86");
  fileChooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
   [this](const juce::FileChooser& chooser){
    const auto file=chooser.getResult();
    if(file==juce::File{})return;
    if(file.getSize()>65536){juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"ECHO 86","Preset trop volumineux.");return;}
    const auto data=juce::JSON::parse(file.loadFileAsString());
    if(!data.isObject()||data.getProperty("format","").toString()!="ECHO86-PRESET"||
       (int)data.getProperty("version",0)!=1||!data.getProperty("parameters",juce::var()).isObject()){
     juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"ECHO 86","Fichier preset invalide.");return;
    }
    const auto values=data.getProperty("parameters",juce::var());
    // Validate every value before modifying any parameter.
    for(const char* id:echo86::parameterIds){
     auto* param=processor.state.getParameter(id);
     const auto v=values.getProperty(id,juce::var());
     if(!param||(!v.isDouble()&&!v.isInt())||!std::isfinite((double)v)){
      juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"ECHO 86","Paramètre manquant ou invalide.");return;
     }
     const float x=(float)(double)v;
     if(x<param->convertFrom0to1(0.f)||x>param->convertFrom0to1(1.f)){
      juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"ECHO 86","Valeur hors limites.");return;
     }
    }
    for(const char* id:echo86::parameterIds){
     auto* param=processor.state.getParameter(id);
     const float normalized=param->convertTo0to1((float)(double)values.getProperty(id,juce::var()));
     param->beginChangeGesture();param->setValueNotifyingHost(normalized);param->endChangeGesture();
    }
    presetList.setSelectedId(0,juce::dontSendNotification);
    presetName.setText(data.getProperty("name","MON SON").toString().substring(0,80),juce::dontSendNotification);
   });
 }
 Echo86Processor& processor;
 juce::ComboBox categoryList,presetList;
 juce::TextEditor presetName;
 juce::TextButton saveButton,loadButton;
 std::unique_ptr<juce::FileChooser> fileChooser;
 juce::OwnedArray<juce::Slider> knobs;
 juce::OwnedArray<juce::Label> labels;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
};
juce::AudioProcessorEditor* Echo86Processor::createEditor(){return new Echo86Editor(*this);}
