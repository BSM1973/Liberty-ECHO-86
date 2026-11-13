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


// Faithful JUCE integration of the four originally approved ECHO 86 panels.
// The four original JPEGs are compiled as BinaryData resources. No generic
// section-based substitute skin is used.
#include <BinaryData.h>
#include <vector>
#include <memory>
#include <cmath>

class Echo86Editor final : public juce::AudioProcessorEditor {
    struct InvisibleButton final : juce::Button {
        InvisibleButton() : juce::Button("ECHO86 Hotspot") {}
        void paintButton(juce::Graphics&, bool, bool) override {}
    };

    class VintageControls final : public juce::LookAndFeel_V4 {
    public:
        void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                              float proportion, float start, float end, juce::Slider&) override {
            const float cx = x + w * .5f;
            const float cy = y + h * .50f;
            const float radius = juce::jmin(w, h) * .375f;
            const float angle = start + proportion * (end - start);
            g.setColour(juce::Colour(0xff07161a));
            g.fillEllipse(cx - radius - 3, cy - radius - 1, (radius + 3) * 2, (radius + 3) * 2);
            g.setColour(juce::Colour(0xff527477));
            g.drawEllipse(cx - radius - 1, cy - radius - 1, (radius + 1) * 2, (radius + 1) * 2, 2.0f);
            g.setGradientFill(juce::ColourGradient(
               juce::Colour(0xff344b4c),cx-radius,cy-radius,
               juce::Colour(0xff0c2329),cx+radius,cy+radius,false));
            g.fillEllipse(cx - radius + 3, cy - radius + 3, (radius - 3) * 2, (radius - 3) * 2);
            g.setColour(juce::Colour(0x552f5557));
            g.drawEllipse(cx-radius+7,cy-radius+7,(radius-7)*2,(radius-7)*2,1.2f);
            const float s = std::sin(angle), c = std::cos(angle);
            g.setColour(juce::Colour(0xfff3e3bf));
            g.drawLine(cx + s*radius*.23f, cy - c*radius*.23f,
                       cx + s*radius*.72f, cy - c*radius*.72f, 3.1f);
        }

        void drawLinearSlider(juce::Graphics& g, int x,int y,int w,int h,
                              float sliderPos,float,float,juce::Slider::SliderStyle style,juce::Slider&) override {
            if(style != juce::Slider::LinearVertical) return;
            const float cx=x+w*.5f;
            g.setColour(juce::Colour(0xff111f21));
            g.fillRoundedRectangle(cx-6,y+2,12.f,(float)h-4,2.f);
            g.setColour(juce::Colour(0xff46666b));
            g.drawLine(cx,y+6,cx,y+h-6,2);
            g.setColour(juce::Colour(0xffe8dfb8));
            g.fillRoundedRectangle(cx-20,sliderPos-7,40,14,1.8f);
            g.setColour(juce::Colour(0xff8f9a8e));
            g.drawRoundedRectangle(cx-20,sliderPos-7,40,14,1.8f,1.3f);
        }
    };

    struct Hotspot {
        int page, x, y, w, h;
        std::unique_ptr<InvisibleButton> button;
    };

    struct Control {
        int page, cx, cy;
        bool vertical;
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

public:
    explicit Echo86Editor(Echo86Processor& processorToEdit)
        : juce::AudioProcessorEditor(processorToEdit), processor(processorToEdit)
    {
        backgrounds = {
          juce::ImageFileFormat::loadFrom(BinaryData::voice_jpg, BinaryData::voice_jpgSize),
          juce::ImageFileFormat::loadFrom(BinaryData::envelopes_jpg, BinaryData::envelopes_jpgSize),
          juce::ImageFileFormat::loadFrom(BinaryData::modulation_jpg, BinaryData::modulation_jpgSize),
          juce::ImageFileFormat::loadFrom(BinaryData::performance_jpg, BinaryData::performance_jpgSize)
        };

        // Tabs match the four navigation buttons on the original facades.
        makeButton(-1, 37,190,326,50,[this]{selectPage(0);});
        makeButton(-1,375,190,326,50,[this]{selectPage(1);});
        makeButton(-1,712,190,326,50,[this]{selectPage(2);});
        makeButton(-1,1050,190,418,50,[this]{selectPage(3);});

        // Original preset controls are fully interactive.
        makeButton(-1,1252,37,215,51,[this]{showFactoryPresets();});
        makeButton(-1,1251,94,64,39,[this]{stepPreset(-1);});
        makeButton(-1,1318,94,82,39,[this]{applyPreset(0);});
        makeButton(-1,1404,94,64,39,[this]{stepPreset(+1);});

        // Voice / two-oscillator and four-pole filter page.
        addKnob(0,526,439,"pulseWidth","OSC 1 / Pulse width");
        addKnob(0,660,439,"waveMix","OSC 1 / Saw-Pulse mix");
        addKnob(0,1117,439,"detune","OSC 2 / Fine detune");
        addKnob(0,536,717,"cutoff","Filter cutoff");
        addKnob(0,660,717,"resonance","Filter resonance");
        addKnob(0,783,717,"filterEnvAmount","Filter envelope amount");
        addKnob(0,1146,717,"lfoDepth","LFO depth");
        addKnob(0,1389,717,"hardSync","Oscillator sync amount");

        // Envelope page. The values that are currently connected to the sound
        // engine have live sliders; all original graphics remain unchanged.
        addFader(1,541,408,"attack","Amp attack");
        addFader(1,708,408,"decay","Amp decay");
        addFader(1,1209,408,"sustain","Amp sustain");
        addFader(1,1377,408,"release","Amp release");
        addFader(1,541,714,"filterAttack","Filter attack");
        addFader(1,709,714,"filterDecay","Filter decay");
        addFader(1,1209,714,"filterSustain","Filter sustain");

        // Modulation, preserving the approved page layout.
        addKnob(2,143,482,"lfoRate","LFO rate");
        addKnob(2,310,482,"lfoDepth","LFO depth");
        addKnob(2,863,402,"lfoDepth","LFO to filter");

        // Vintage chorus and performance page.
        addKnob(3,942,471,"chorusRate","Chorus rate");
        addKnob(3,1082,471,"chorusDepth","Chorus depth");
        addKnob(3,1379,471,"chorusMix","Chorus mix");

        // Waveform selectors map the supported pulse/saw engine modes.
        makeButton(0,57,311,106,51,[this]{setParameter("waveMix",0.0f);});
        makeButton(0,282,311,107,51,[this]{setParameter("waveMix",1.0f);});
        makeButton(0,780,521,79,32,[this]{setParameter("hardSync",0.0f);});
        makeButton(0,864,521,81,32,[this]{setParameter("hardSync",1.0f);});
        makeButton(3,870,352,114,39,[this]{setParameter("chorusMix",0.0f);});
        makeButton(3,996,352,113,39,[this]{setParameter("chorusMix",0.35f);});

        setResizable(true,true);
        if (auto* constrainer=getConstrainer())
            constrainer->setFixedAspectRatio(1600.0/1069.0);
        setResizeLimits(1000,668,1600,1069);
        // JUCE immediately calls resized(), so every control exists first.
        setSize(1440,962);
        selectPage(0);
    }

    ~Echo86Editor() override {
        for(auto& control:controls)
            control.slider->setLookAndFeel(nullptr);
    }

    void paint(juce::Graphics& g) override {
        if(backgrounds[(size_t)page].isValid()) {
            g.drawImageWithin(backgrounds[(size_t)page], 0,0,getWidth(),getHeight(),
                              juce::RectanglePlacement::stretchToFit);
        } else {
            g.fillAll(juce::Colour(0xff212c2d));
        }
        // Only the original amber LCD name changes dynamically.
        g.saveState();
        g.addTransform(juce::AffineTransform::scale(
            (float)getWidth()/1600.f,(float)getHeight()/1069.f));
        if(currentPreset>=0) {
            g.setColour(juce::Colour(0xffc99743));
            g.fillRect(671,82,464,54);
            g.setColour(juce::Colour(0xff30271d));
            g.setFont(juce::Font(juce::FontOptions(28.0f).withStyle("Bold")));
            g.drawText(juce::String::fromUTF8(echo86::factoryPresets[(size_t)currentPreset].name),
                       679,86,449,34,juce::Justification::centredLeft,true);
            g.setFont(juce::Font(juce::FontOptions(12.0f)));
            g.drawText("PROGRAM "+juce::String(currentPreset+1).paddedLeft('0',3),681,118,300,16,
                       juce::Justification::centredLeft);
        }
        g.restoreState();
    }

    void resized() override {
        if(hotspots.empty() || controls.empty()) return;
        const float sx=(float)getWidth()/1600.f,sy=(float)getHeight()/1069.f;
        for(auto& h:hotspots) {
            h.button->setBounds(juce::roundToInt(h.x*sx),juce::roundToInt(h.y*sy),
                                juce::roundToInt(h.w*sx),juce::roundToInt(h.h*sy));
        }
        for(auto& c:controls) {
            if(c.vertical) {
                c.slider->setBounds(juce::roundToInt((c.cx-39)*sx),
                                    juce::roundToInt((c.cy-66)*sy),
                                    juce::roundToInt(78*sx),juce::roundToInt(167*sy));
            } else {
                c.slider->setBounds(juce::roundToInt((c.cx-45)*sx),
                                    juce::roundToInt((c.cy-46)*sy),
                                    juce::roundToInt(90*sx),juce::roundToInt(116*sy));
            }
        }
    }

private:
    void makeButton(int pg,int x,int y,int w,int h,std::function<void()> click) {
        Hotspot s{pg,x,y,w,h,std::make_unique<InvisibleButton>()};
        s.button->onClick=std::move(click);
        addAndMakeVisible(*s.button);
        hotspots.emplace_back(std::move(s));
    }

    void addKnob(int pg,int cx,int cy,const char* id,const char* tooltip) {
        addControl(pg,cx,cy,id,tooltip,false);
    }
    void addFader(int pg,int cx,int cy,const char* id,const char* tooltip) {
        addControl(pg,cx,cy,id,tooltip,true);
    }
    void addControl(int pg,int cx,int cy,const char* id,const char* tooltip,bool vertical) {
        Control c{pg,cx,cy,vertical,std::make_unique<juce::Slider>(),nullptr};
        auto& s=*c.slider;
        s.setSliderStyle(vertical?juce::Slider::LinearVertical:
                                 juce::Slider::RotaryHorizontalVerticalDrag);
        s.setRotaryParameters(juce::MathConstants<float>::pi*1.22f,
                             juce::MathConstants<float>::pi*2.78f,true);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,76,19);
        s.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff141f21));
        s.setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffdae3da));
        s.setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff3b4b4d));
        s.setTooltip(tooltip);
        s.setLookAndFeel(&vintageLook);
        c.attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,id,s);
        addAndMakeVisible(s);
        controls.emplace_back(std::move(c));
    }

    void selectPage(int target) {
        page=juce::jlimit(0,3,target);
        for(auto& c:controls)c.slider->setVisible(c.page==page);
        for(auto& h:hotspots)h.button->setVisible(h.page<0||h.page==page);
        repaint();
    }

    void setParameter(const char* id,float value) {
        if(auto* parameter=processor.state.getParameter(id)){
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
            parameter->endChangeGesture();
        }
    }

    void applyPreset(int index) {
        if(index<0||index>=(int)echo86::factoryPresets.size())return;
        const auto& preset=echo86::factoryPresets[(size_t)index];
        for(size_t i=0;i<echo86::parameterIds.size();++i)
            setParameter(echo86::parameterIds[i],preset.values[i]);
        currentPreset=index;
        repaint();
    }

    void stepPreset(int delta) {
        const int n=(int)echo86::factoryPresets.size();
        applyPreset((currentPreset+n+delta)%n);
    }

    void showFactoryPresets() {
        juce::PopupMenu menu;
        const char* families[]={"INIT","BASS","LEAD","PAD","KEYS","PLUCK","ARP","STRINGS","FX"};
        for(auto* family:families){
            juce::PopupMenu subgroup;
            const juce::String prefix=juce::String(family)+" ";
            for(int i=0;i<(int)echo86::factoryPresets.size();++i) {
                const auto name=juce::String::fromUTF8(echo86::factoryPresets[(size_t)i].name);
                if(name.startsWithIgnoreCase(prefix)){
                    subgroup.addItem(i+1,name,i!=currentPreset);
                }
            }
            if(subgroup.getNumItems()>0)menu.addSubMenu(family,subgroup);
        }
        menu.addSeparator();
        menu.addItem(1001,"IMPORT USER PRESET...");
        menu.addItem(1002,"EXPORT USER PRESET...");
        juce::Component::SafePointer<Echo86Editor> safe(this);
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
           [safe](int result) {
              if(safe==nullptr)return;
              if(result>=1&&result<=100)safe->applyPreset(result-1);
              else if(result==1001)safe->importPreset();
              else if(result==1002)safe->exportPreset();
           });
    }

    juce::var serializePreset() const {
        auto* root=new juce::DynamicObject();
        root->setProperty("format","ECHO86-PRESET");
        root->setProperty("version",1);
        root->setProperty("name",currentPreset<0?"Init":
                          juce::String::fromUTF8(echo86::factoryPresets[(size_t)currentPreset].name));
        auto* values=new juce::DynamicObject();
        for(const char* id:echo86::parameterIds)
            values->setProperty(id,processor.state.getRawParameterValue(id)->load());
        root->setProperty("parameters",juce::var(values));
        return juce::var(root);
    }

    void exportPreset() {
        const auto suggested=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                   .getChildFile("ECHO86-Sound.echo86");
        fileChooser=std::make_unique<juce::FileChooser>("Export ECHO 86",suggested,"*.echo86");
        juce::Component::SafePointer<Echo86Editor> safe(this);
        fileChooser->launchAsync(juce::FileBrowserComponent::saveMode|
                      juce::FileBrowserComponent::canSelectFiles|
                      juce::FileBrowserComponent::warnAboutOverwriting,
          [safe](const juce::FileChooser& fc){
            if(safe==nullptr)return;
            const auto file=fc.getResult();
            if(file==juce::File{})return;
            if(!file.replaceWithText(juce::JSON::toString(safe->serializePreset(),true)))
                juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                       "ECHO 86","Cannot save preset");
          });
    }

    void importPreset() {
        fileChooser=std::make_unique<juce::FileChooser>("Import ECHO 86",juce::File{},"*.echo86");
        juce::Component::SafePointer<Echo86Editor> safe(this);
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode|
                   juce::FileBrowserComponent::canSelectFiles,
          [safe](const juce::FileChooser& fc){
            if(safe==nullptr)return;
            const auto file=fc.getResult();
            if(file==juce::File{})return;
            if(file.getSize()>65536)return;
            const auto data=juce::JSON::parse(file.loadFileAsString());
            if(!data.isObject()||data.getProperty("format","").toString()!="ECHO86-PRESET"||
               (int)data.getProperty("version",0)!=1||
               !data.getProperty("parameters",juce::var()).isObject())return;
            const auto values=data.getProperty("parameters",juce::var());
            // Validate all values before applying any parameter.
            for(const char* id:echo86::parameterIds){
                auto* p=safe->processor.state.getParameter(id);
                const auto v=values.getProperty(id,juce::var());
                if(p==nullptr||(!v.isDouble()&&!v.isInt())||!std::isfinite((double)v))return;
                const float value=(float)(double)v;
                if(value<p->convertFrom0to1(0.f)||value>p->convertFrom0to1(1.f))return;
            }
            for(const char* id:echo86::parameterIds)
                safe->setParameter(id,(float)(double)values.getProperty(id,juce::var()));
            safe->currentPreset=-1;
            safe->repaint();
          });
    }

    Echo86Processor& processor;
    VintageControls vintageLook;
    std::array<juce::Image,4> backgrounds;
    std::vector<Hotspot> hotspots;
    std::vector<Control> controls;
    std::unique_ptr<juce::FileChooser> fileChooser;
    int page=0;
    int currentPreset=-1;
};
juce::AudioProcessorEditor* Echo86Processor::createEditor(){return new Echo86Editor(*this);}