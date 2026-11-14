
// ECHO 86 / Native Hardware Editor
// All panels, text, switches, knobs and faders are JUCE components and vector drawing.
// No flattened/JPEG facade is displayed. Values are bound to APVTS, not decorative hotspots.
#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <memory>
#include <cmath>
#include <functional>
#include "PresetBrowser.h"
#include "FactoryPresetExtras.h"
#include "UILocalization.h"
#include "LfoWaveform.h"

class Echo86Editor final : public juce::AudioProcessorEditor, private juce::Timer {
    static constexpr auto extendedIds = echo86::factoryExtendedIds;
    class HardwareLook final : public juce::LookAndFeel_V4 {
    public:
        void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,
                              float a,float b,juce::Slider&) override {
            const float radius=juce::jmin(w,h)*.38f;
            const float cx=x+w*.5f, cy=y+h*.50f;
            g.setColour(juce::Colour(0xff081417));
            g.fillEllipse(cx-radius-4,cy-radius-4,2*(radius+4),2*(radius+4));
            g.setColour(juce::Colour(0xff60797a));
            g.drawEllipse(cx-radius-2,cy-radius-2,2*(radius+2),2*(radius+2),1.5f);
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff486265),cx-radius,cy-radius,
                    juce::Colour(0xff132c31),cx+radius,cy+radius,false));
            g.fillEllipse(cx-radius,cy-radius,2*radius,2*radius);
            g.setColour(juce::Colour(0xff0d2024));
            g.drawEllipse(cx-radius+4,cy-radius+4,2*(radius-4),2*(radius-4),1.2f);
            const float angle=a+pos*(b-a);
            g.setColour(juce::Colour(0xffe9e2b9));
            g.drawLine(cx+std::sin(angle)*radius*.3f,cy-std::cos(angle)*radius*.3f,
                       cx+std::sin(angle)*radius*.79f,cy-std::cos(angle)*radius*.79f,3);
        }
        void drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float sliderPos,
                               float,float,juce::Slider::SliderStyle style,juce::Slider&) override {
            if(style==juce::Slider::LinearHorizontal) {
                const float cy=y+h*.5f;
                g.setColour(juce::Colour(0xff0d1e21));
                g.fillRoundedRectangle(x+7,cy-3,w-14.f,6.f,3);
                g.setColour(juce::Colour(0xff76938b));
                g.drawLine(x+9,cy,x+w-9,cy,1.f);
                g.setColour(juce::Colour(0xffe8dfb8));
                g.fillRoundedRectangle(sliderPos-9,cy-9,18.f,18.f,2);
                g.setColour(juce::Colour(0xff264148));
                g.drawRoundedRectangle(sliderPos-9,cy-9,18.f,18.f,2,1.f);
                return;
            }
            if(style!=juce::Slider::LinearVertical)return;
            const float cx=x+w*.5f;
            g.setColour(juce::Colour(0xff07191a));
            g.fillRoundedRectangle(cx-6,y+6,12.f,(float)h-12,3);
            g.setColour(juce::Colour(0xff456368));
            g.drawLine(cx,y+9,cx,y+h-9,1.6f);
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xfff2ecc8),cx,sliderPos-7,
                       juce::Colour(0xffb1b293),cx,sliderPos+8,false));
            g.fillRoundedRectangle(cx-21,sliderPos-7,42.f,15.f,2);
            g.setColour(juce::Colour(0xff344b4c));
            g.drawRoundedRectangle(cx-21,sliderPos-7,42.f,15.f,2,1);
        }
        void drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,
                                   bool highlighted,bool down) override {
            auto r=b.getLocalBounds().toFloat().reduced(1);
            const bool active=b.getToggleState();
            const auto hi=juce::Colour(active?0xffe8d9a7:down?0xffc0c4ac:highlighted?0xfff3eecf:0xffdedec1);
            const auto lo=juce::Colour(active?0xff998354:down?0xff8d977f:0xff88938a);
            g.setGradientFill(juce::ColourGradient(hi,r.getX(),r.getY(),lo,r.getX(),r.getBottom(),false));
            g.fillRoundedRectangle(r,3);
            g.setColour(juce::Colour(0xff0b2227));
            g.drawRoundedRectangle(r,3,1.5f);
            if(active){
                g.setColour(juce::Colour(0xffb18439));
                g.fillRect(r.getX()+5,r.getBottom()-4,r.getWidth()-10,2.f);
            }
        }
        void drawButtonText(juce::Graphics& g,juce::TextButton& b,bool,bool) override {
            g.setColour(juce::Colour(0xff163039));
            g.setFont(juce::Font(juce::FontOptions(juce::jlimit(10.f,18.f,b.getHeight()*.31f))
                                 .withStyle("Bold")));
            g.drawFittedText(b.getButtonText(),b.getLocalBounds().reduced(4,1),
                             juce::Justification::centred,1,.72f);
        }
    };
    struct Knob {
        int page,x,y,w,h;
        juce::String name;
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };
    struct Switch {
        int page,x,y,w,h;
        juce::String id,title;
        float value;
        std::unique_ptr<juce::TextButton> button;
    };
    Echo86Processor& processor;
    HardwareLook look;
    std::vector<Knob> knobs;
    std::vector<Switch> switches;
    std::array<std::unique_ptr<juce::TextButton>,4> tabs;
    std::unique_ptr<juce::TextButton> presetButton,prevButton,initButton,nextButton,languageButton;
    std::unique_ptr<juce::FileChooser> chooser;
    std::unique_ptr<Echo86PresetBrowser> presetBrowser;
    int page=0, selectedPreset=-1;
    echo86::locale::Language language=readLanguage();
    bool prepared=false;
    double lfoPreviewPhase=0.0,lastPreviewTimeMs=0.0;
    static constexpr int W=1600,H=1069;
public:
    explicit Echo86Editor(Echo86Processor& p):juce::AudioProcessorEditor(p),processor(p) {
        const char* names[]={u8"01  VOIX",u8"02  ENVELOPPES","03  MODULATION",u8"04  PERFORMANCE / FX"};
        for(int i=0;i<4;++i){
            tabs[(size_t)i]=std::make_unique<juce::TextButton>(juce::String::fromUTF8(names[i]));
            tabs[(size_t)i]->setLookAndFeel(&look);
            tabs[(size_t)i]->onClick=[this,i]{setPage(i);};
            addAndMakeVisible(*tabs[(size_t)i]);
        }
        auto top=[&](std::unique_ptr<juce::TextButton>& target,const char* title,
                     std::function<void()> onClick){
            target=std::make_unique<juce::TextButton>(juce::String::fromUTF8(title));
            target->setLookAndFeel(&look);
            target->onClick=std::move(onClick);
            addAndMakeVisible(*target);
        };
        top(presetButton,"PRESETS",[this]{openPresets();});
        top(prevButton,"<",[this]{nextPreset(-1);});
        top(initButton,"INIT",[this]{loadPreset(0);});
        top(nextButton,">",[this]{nextPreset(1);});
        top(languageButton,u8"LANGUE : FR",[this]{showLanguageMenu();});

        // VOICE — original dual oscillator / mixer / analog filter arrangement
        const char* modes[]={"SAW","SQUARE","PULSE","TRIANGLE","SINE","SAW + PW"};
        for(int n=0;n<6;++n){
            addSwitch(0,60+n*118,332,108,52,modes[n],"osc1Wave",(float)n);
            addSwitch(0,830+n*118,332,108,52,modes[n],"osc2Wave",(float)n);
        }
        addKnob(0,125,459,"OCTAVE","osc1Oct"); addKnob(0,270,459,u8"DEMI-TON","osc1Semi");
        addKnob(0,415,459,u8"ACCORD FIN","osc1Fine"); addKnob(0,560,459,u8"LARGEUR PULSE","pulseWidth");
        addKnob(0,705,459,"PWM","lfoPwm1");
        addKnob(0,895,459,"OCTAVE","osc2Oct");addKnob(0,1040,459,u8"DEMI-TON","osc2Semi");
        addKnob(0,1185,459,u8"ACCORD FIN","osc2Fine");addKnob(0,1330,459,u8"LARGEUR PULSE","pulseWidth");
        addKnob(0,1475,459,"PWM","lfoPwm2");
        addSwitch(0,830,556,115,35,u8"LIBRE","hardSync",0);
        addSwitch(0,956,556,115,35,"SYNC","hardSync",1);
        addHorizontal(0,1175,572,"CROSS MOD","crossMod",180);
        addHorizontal(0,1435,572,u8"ENV -> HAUTEUR","envPitch2",180);
        addFader(0,108,818,"OSC 1","osc1Level");addFader(0,210,818,"OSC 2","osc2Level");
        addFader(0,312,818,"NOISE","noiseLevel");addKnob(0,408,766,u8"COULEUR","noiseColour");
        const int filterX[]={570,700,830,960,1090,1220,1350,1480};
        const char* filterNames[]={u8"FRÉQUENCE",u8"RÉSONANCE",u8"PROF. ENV.",u8"SUIVI CLAVIER",
            u8"VÉLOCITÉ",u8"LFO -> FILTRE",u8"FM AUDIO","DRIVE"};
        const char* filterIds[]={"cutoff","resonance","filterEnvAmount","keyTrack",
            "velocityToFilter","lfoFilterDepth","filterAudioFM","filterDrive"};
        for(int i=0;i<8;++i)addKnob(0,filterX[i],774,filterNames[i],filterIds[i]);

        // ENVELOPES — the original two six-stage envelopes, not bitmap faders
        const int faderX[]={576,753,930,1107,1284,1461};
        const char* envNames[]={u8"ATTAQUE",u8"DÉCLIN 1",u8"RUPTURE",u8"DÉCLIN 2",u8"MAINTIEN",u8"RELÂCHEMENT"};
        const char* ampIds[]={"attack","decay","ampBreak","ampDecay2","sustain","release"};
        const char* filIds[]={"filterAttack","filterDecay","filterBreak","filterDecay2","filterSustain","filterRelease"};
        for(int i=0;i<6;++i){
            addFader(1,faderX[i],470,envNames[i],ampIds[i]);
            addFader(1,faderX[i],787,envNames[i],filIds[i]);
        }

        // MODULATION — waveform, rate/depth, pitch and routing
        const char* lfoModes[]={"SINE","TRIANGLE","SQUARE","S & H"};
        for(int i=0;i<4;++i)addSwitch(2,64+i*176,353,165,60,lfoModes[i],"lfoWave",(float)i);
        addKnob(2,153,521,u8"VITESSE","lfoRate");
        addKnob(2,333,521,u8"PROFONDEUR","lfoDepth");
        addKnob(2,513,521,u8"HAUTEUR","lfoPitch");
        addKnob(2,693,521,u8"RETARD","lfoDelay");
        addKnob(2,918,423,u8"LFO -> FILTRE","lfoFilterDepth");
        addKnob(2,1095,423,"LFO -> PWM 1","lfoPwm1");
        addKnob(2,1272,423,"LFO -> PWM 2","lfoPwm2");
        addKnob(2,1448,423,"ENV. 2 -> OSC. 2","envPitch2");
        addKnob(2,955,768,u8"VIBRATO MOLETTE","wheelVibrato");
        addKnob(2,1188,768,u8"MOLETTE -> VCF","wheelFilter");
        addKnob(2,1424,768,u8"PRESSION -> VCF","pressureFilter");

        // PERFORMANCE — voice, expression, chorus, vintage character
        addSwitch(3,66,372,212,54,"POLY","monoMode",0);
        addSwitch(3,291,372,212,54,u8"MONO / NOTE BASSE","monoMode",1);
        const int perfX[]={143,302,461,620,779};
        const char* perfNames[]={"PORTAMENTO",u8"TRANSPOSITION",u8"ACCORD GÉNÉRAL",u8"VÉLOCITÉ",u8"PLAGE PITCH"};
        const char* perfIds[]={"portamento","transpose","masterTune","velocityResponse","bendRange"};
        for(int i=0;i<5;++i)addKnob(3,perfX[i],544,perfNames[i],perfIds[i]);
        addKnob(3,195,717,u8"VIBRATO MOLETTE","wheelVibrato");
        addKnob(3,461,717,u8"MOLETTE -> VCF","wheelFilter");
        addKnob(3,726,717,u8"PRESSION -> VCF","pressureFilter");
        addSwitch(3,925,373,123,54,"OFF","chorusEnabled",0);
        addSwitch(3,1060,373,123,54,"ON","chorusEnabled",1);
        addKnob(3,1000,548,u8"VITESSE","chorusRate");
        addKnob(3,1156,548,u8"PROFONDEUR","chorusDepth");
        addKnob(3,1312,548,u8"LARGEUR","chorusWidth");
        addKnob(3,1468,548,u8"MÉLANGE","chorusMix");
        addKnob(3,1122,791,u8"ÂGE VINTAGE","vintageAge");
        addKnob(3,1414,791,u8"ÉCART VOIX","voiceSpread");
        for(int i=0;i<4;++i)tabs[(size_t)i]->setClickingTogglesState(false);
        addKnob(-1,1510,1001,u8"VOLUME GÉNÉRAL","masterLevel",82,104);
        // The professional browser is a real, interactive JUCE overlay.
        // It is created last so the native layout never touches incomplete components.
        presetBrowser=std::make_unique<Echo86PresetBrowser>(
            Echo86PresetBrowser::Callbacks {
                [this](int presetIndex) { loadPreset(presetIndex); },
                [this](const juce::File& file) { return loadUserPresetFile(file); },
                [this](const juce::String& name,const juce::String& category) {
                    return saveUserPreset(name,category);
                },
                [this] { exportPreset(); },
                [this] { if(presetBrowser) presetBrowser->setVisible(false); }
            });
        addAndMakeVisible(*presetBrowser);
        presetBrowser->setVisible(false);
        refreshLanguage();
        prepared=true;
        setResizable(true,true);
        if(auto* c=getConstrainer())c->setFixedAspectRatio((double)W/H);
        setResizeLimits(1050,702,1600,1069);
        setSize(1450,969);
        setPage(0);
        startTimerHz(8);
    }
    ~Echo86Editor()override{
        stopTimer();
        for(auto& b:tabs)b->setLookAndFeel(nullptr);
        for(auto* b:{presetButton.get(),prevButton.get(),initButton.get(),nextButton.get(),languageButton.get()})
            if(b)b->setLookAndFeel(nullptr);
        for(auto& c:knobs)c.slider->setLookAndFeel(nullptr);
        for(auto& b:switches)b.button->setLookAndFeel(nullptr);
    }
private:
    static juce::File languageFile(){
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
          .getChildFile("Liberty Music Software").getChildFile("ECHO 86")
          .getChildFile("language.txt");
    }
    static echo86::locale::Language readLanguage(){
        const auto file=languageFile();
        if(!file.existsAsFile())return echo86::locale::Language::fr;
        const auto code=file.loadFileAsString().trim().toUpperCase();
        return echo86::locale::parse(code.toRawUTF8());
    }
    juce::String tr(const juce::String& key)const{
        return juce::String::fromUTF8(echo86::locale::translate(language,key.toRawUTF8()));
    }
    juce::String tr(const char* key)const{
        return juce::String::fromUTF8(echo86::locale::translate(language,key));
    }
    void refreshLanguage(){
        const char* names[]={u8"01  VOIX",u8"02  ENVELOPPES","03  MODULATION",u8"04  PERFORMANCE / FX"};
        for(int i=0;i<4;++i)tabs[(size_t)i]->setButtonText(tr(names[i]));
        presetButton->setButtonText(tr("PRESETS"));
        languageButton->setButtonText(tr("LANGUE")+" : "+echo86::locale::code(language));
        for(auto& item:switches)item.button->setButtonText(tr(item.title));
        for(auto& item:knobs)item.slider->setTooltip(tr(item.name));
        if(presetBrowser)presetBrowser->setLanguage(language);
        repaint();
    }
    void showLanguageMenu(){
        juce::PopupMenu menu;
        menu.addItem(1,juce::String::fromUTF8(u8"Français"),true,
                     language==echo86::locale::Language::fr);
        menu.addItem(2,"English",true,language==echo86::locale::Language::en);
        menu.addItem(3,juce::String::fromUTF8(u8"Español"),true,
                     language==echo86::locale::Language::es);
        menu.addItem(4,"Deutsch",true,language==echo86::locale::Language::de);
        menu.addItem(5,"Italiano",true,language==echo86::locale::Language::it);
        juce::Component::SafePointer<Echo86Editor> safe(this);
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(languageButton.get()),
          [safe](int choice){
            if(safe==nullptr || choice<1 || choice>5)return;
            safe->language=static_cast<echo86::locale::Language>(choice-1);
            safe->refreshLanguage();
            auto file=languageFile();
            if(file.getParentDirectory().createDirectory().wasOk())
                file.replaceWithText(echo86::locale::code(safe->language));
          });
    }
    float scaleX()const{return (float)getWidth()/W;}
    float scaleY()const{return (float)getHeight()/H;}
    juce::Rectangle<int> rect(int x,int y,int w,int h)const{
        return {juce::roundToInt(x*scaleX()),juce::roundToInt(y*scaleY()),
                juce::jmax(1,juce::roundToInt(w*scaleX())),
                juce::jmax(1,juce::roundToInt(h*scaleY()))};
    }
    void section(juce::Graphics& g,int x,int y,int w,int h,const juce::String& title,
                 const juce::String& hint={}){
        auto r=rect(x,y,w,h).toFloat();
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff2b3231),r.getX(),r.getY(),
                     juce::Colour(0xff182428),r.getX(),r.getBottom(),false));
        g.fillRoundedRectangle(r,4);
        g.setColour(juce::Colour(0xff526666));g.drawRoundedRectangle(r,4,1.35f);
        g.setColour(juce::Colour(0xffd6dcca));
        g.setFont(juce::Font(juce::FontOptions(16*scaleY()).withStyle("Bold")));
        g.drawText(tr(title),rect(x+20,y+16,w-40,22),juce::Justification::centredLeft);
        if(hint.isNotEmpty()){
            g.setColour(juce::Colour(0xff748e91));
            g.setFont(juce::Font(juce::FontOptions(10*scaleY())));
            g.drawText(tr(hint),rect(x+20,y+16,w-40,22),juce::Justification::centredRight);
        }
        g.setColour(juce::Colour(0xff3d5355));g.drawLine((x+18)*scaleX(),(y+48)*scaleY(),
                            (x+w-18)*scaleX(),(y+48)*scaleY(),1);
    }
    // All literal UI strings are decoded as UTF-8 before reaching JUCE text drawing.
    void section(juce::Graphics& g,int x,int y,int w,int h,
                 const char* title,const char* hint=""){
        section(g,x,y,w,h,juce::String::fromUTF8(title),juce::String::fromUTF8(hint));
    }
    void simpleText(juce::Graphics& g,const char* msg,int x,int y,int w,int h,
                    float size,juce::Colour color,
                    juce::Justification just=juce::Justification::centredLeft){
        simpleText(g,juce::String::fromUTF8(msg),x,y,w,h,size,color,just);
    }
    void simpleText(juce::Graphics& g,const juce::String& msg,int x,int y,int w,int h,
                     float size,juce::Colour color,juce::Justification just=juce::Justification::centredLeft){
        g.setColour(color);g.setFont(juce::Font(juce::FontOptions(size*scaleY())));
        g.drawFittedText(tr(msg),rect(x,y,w,h),just,1,.78f);
    }
    // Live APVTS envelope preview. SynthCore uses exponential responses; the
    // displayed stage times are compressed to keep both fast and slow sounds legible.
    void envelopePlot(juce::Graphics& g,int x,int y,int w,int h,bool filter){
        const auto bounds=rect(x,y,w,h).toFloat();
        g.setColour(juce::Colour(0xff122024));g.fillRoundedRectangle(bounds,2);
        g.setColour(juce::Colour(0xff46605f));g.drawRoundedRectangle(bounds,2,1);
        const auto value=[this](const char* id,float fallback){
            if(const auto* p=processor.state.getRawParameterValue(id)){
                const float v=p->load();
                if(std::isfinite(v))return v;
            }
            return fallback;
        };
        const float attack=value(filter?"filterAttack":"attack",.02f);
        const float decay1=value(filter?"filterDecay":"decay",.35f);
        const float breakpoint=juce::jlimit(0.f,1.f,
                             value(filter?"filterBreak":"ampBreak",.7f));
        const float decay2=value(filter?"filterDecay2":"ampDecay2",.32f);
        const float sustain=juce::jlimit(0.f,1.f,
                            value(filter?"filterSustain":"sustain",.55f));
        const float release=value(filter?"filterRelease":"release",.38f);

        // 6.9 exponential time constants reach 99.9% of the target level.
        const std::array<float,5> durations{{
            juce::jmax(.001f,attack*1.59f),juce::jmax(.001f,decay1*1.59f),
            juce::jmax(.001f,decay2*2.62f),.55f,
            juce::jmax(.001f,release*1.59f)
        }};
        std::array<float,6> xs{};
        std::array<float,5> widths{};
        float total=0.f;
        for(int k=0;k<5;++k){
            widths[(size_t)k]=std::sqrt(durations[(size_t)k])+.075f;
            total+=widths[(size_t)k];
        }
        const float left=bounds.getX()+18.f*scaleX();
        const float right=bounds.getRight()-18.f*scaleX();
        const float top=bounds.getY()+16.f*scaleY();
        const float bottom=bounds.getBottom()-28.f*scaleY();
        const auto py=[&](float level){
            return bottom-juce::jlimit(0.f,1.f,level)*(bottom-top);
        };
        xs[0]=left;
        for(int k=0;k<5;++k)
            xs[(size_t)k+1]=xs[(size_t)k]+(right-left)*widths[(size_t)k]/total;

        for(int i=1;i<=4;++i){
            g.setColour(juce::Colour(0x2b608080));
            const float gy=top+(bottom-top)*(float)i/5.f;
            g.drawLine(left,gy,right,gy,1);
        }
        const std::array<float,6> levels{{0.f,1.f,breakpoint,sustain,sustain,0.f}};
        juce::Path line;
        line.startNewSubPath(left,py(0));
        for(int stage=0;stage<5;++stage){
            for(int step=1;step<=30;++step){
                const float t=(float)step/30.f;
                const float shape=(1.f-std::exp(-6.907755f*t))/.999f;
                const float level=stage==3 ? sustain :
                    levels[(size_t)stage]+
                    (levels[(size_t)stage+1]-levels[(size_t)stage])*shape;
                line.lineTo(xs[(size_t)stage]+
                    (xs[(size_t)stage+1]-xs[(size_t)stage])*t,py(level));
            }
        }
        juce::Path under=line;
        under.lineTo(right,bottom);under.lineTo(left,bottom);under.closeSubPath();
        g.setGradientFill(juce::ColourGradient(juce::Colour(0x55e2bb64),left,top,
                                               juce::Colour(0x07e2bb64),left,bottom,false));
        g.fillPath(under);
        for(int i=1;i<5;++i){
            g.setColour(juce::Colour(0x586d8584));
            g.drawVerticalLine(juce::roundToInt(xs[(size_t)i]),top,bottom);
        }
        g.setColour(juce::Colour(0xffe9c16f));
        g.strokePath(line,juce::PathStrokeType(2.7f*scaleY()));
        for(int i=1;i<5;++i){
            g.setColour(juce::Colour(0xfff5dca1));
            g.fillEllipse(xs[(size_t)i]-3.2f*scaleX(),py(levels[(size_t)i])-3.2f*scaleY(),
                          6.4f*scaleX(),6.4f*scaleY());
        }
        const char* stages[]={"A","D1","D2","S","R"};
        g.setColour(juce::Colour(0xffa8c7c1));
        g.setFont(juce::Font(juce::FontOptions(11.f*scaleY())));
        for(int k=0;k<5;++k){
            const float mid=(xs[(size_t)k]+xs[(size_t)k+1])/2.f;
            g.drawText(stages[k],juce::Rectangle<float>(
                       mid-20.f*scaleX(),bottom+4.f*scaleY(),40.f*scaleX(),13.f*scaleY()),
                       juce::Justification::centred);
        }
    }
public:
    void paint(juce::Graphics& g)override{
        const auto full=getLocalBounds().toFloat();
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff343a39),0,0,
                         juce::Colour(0xff131c1f),0,(float)getHeight(),false));
        g.fillRect(full);
        g.setColour(juce::Colour(0xff47311e));g.fillRect(rect(0,0,22,H));
        g.fillRect(rect(W-22,0,22,H));
        // Fine brushed-metal surface: vector, crisp at any resolution.
        for(int yy=0;yy<H;yy+=5){
            g.setColour(juce::Colour(yy%10==0?0x124f6667:0x0a9da29b));
            g.drawLine(24*scaleX(),yy*scaleY(),(W-24)*scaleX(),yy*scaleY(),.7f);
        }
        simpleText(g,"L I B E R T Y   MUSIC SOFTWARE",38,33,390,29,17,juce::Colour(0xffd9dfca));
        simpleText(g,"ECHO 86",38,65,555,83,72,juce::Colour(0xffe6e4c9));
        simpleText(g,u8"SYNTHÉTISEUR ANALOGIQUE VINTAGE",39,153,540,23,14,juce::Colour(0xff82a8a8));
        auto screen=rect(695,44,610,124).toFloat();
        g.setColour(juce::Colour(0xff0a1c1e));g.fillRoundedRectangle(screen,5);
        auto lcd=rect(708,55,584,101).toFloat();
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xffefbd58),lcd.getX(),lcd.getY(),
                         juce::Colour(0xffb98436),lcd.getX(),lcd.getBottom(),false));
        g.fillRoundedRectangle(lcd,2);
        simpleText(g,tr("PROGRAM")+" "+juce::String(selectedPreset<0?0:selectedPreset+1).paddedLeft('0',3),
                          727,62,245,23,12,juce::Colour(0xff655034));
        simpleText(g,u8"100 PRESETS D'USINE",1046,62,228,23,12,juce::Colour(0xff655034),
                   juce::Justification::centredRight);
        const auto display=selectedPreset<0?juce::String("Init"):
                  readablePresetName(echo86::factoryPresets[(size_t)selectedPreset].name);
        simpleText(g,display,727,89,540,39,31,juce::Colour(0xff392d22));
        g.setColour(juce::Colour(0xff51696a));
        g.drawLine(39*scaleX(),190*scaleY(),1560*scaleX(),190*scaleY(),1.5f);
        switch(page){
          case 0:
            section(g,40,277,750,333,u8"OSCILLATEUR 1","MASTER");
            section(g,810,277,750,333,u8"OSCILLATEUR 2",u8"ESCLAVE / SYNC");
            section(g,40,629,429,292,u8"MIXEUR / BRUIT",u8"AVANT FILTRE");
            section(g,487,629,1073,292,u8"FILTRE ANALOGIQUE",u8"4 PÔLES / 24 dB");
            simpleText(g,u8"DOUBLE OSCILLATEUR",60,584,330,18,11,juce::Colour(0xff819395));
            simpleText(g,"OSC 1 -> OSC 2 / LIBRE / SYNC",830,529,470,17,11,juce::Colour(0xff839f9f));
            break;
          case 1:
            section(g,40,277,1520,315,u8"ENVELOPPE 1 / AMPLITUDE",u8"SIX ÉTAPES");
            section(g,40,611,1520,310,u8"ENVELOPPE 2 / FILTRE",u8"SIX ÉTAPES");
            envelopePlot(g,60,341,400,205,false);
            envelopePlot(g,60,675,400,205,true);
            simpleText(g,"A - D1 - BP - D2 - S - R",82,552,367,18,11,juce::Colour(0xff91a6a2));
            simpleText(g,"A - D1 - BP - D2 - S - R",82,884,367,18,11,juce::Colour(0xff91a6a2));
            break;
          case 2: {
            section(g,40,277,750,644,u8"OSCILLATEUR BASSE FRÉQUENCE","LFO");
            section(g,810,277,750,315,u8"ROUTAGES DE MODULATION",u8"CONTRÔLE DIRECT");
            section(g,810,611,750,310,u8"EXPRESSION MIDI",u8"MOLETTE / PRESSION");
            simpleText(g,u8"FORME D'ONDE",65,323,240,19,14,juce::Colour(0xffd5dbc7));
            auto r=rect(67,647,694,140);
            g.setColour(juce::Colour(0xff121e21));g.fillRoundedRectangle(r.toFloat(),3);
            g.setColour(juce::Colour(0xff4f6868));g.drawRoundedRectangle(r.toFloat(),3,1);
            const auto getLfo=[this](const char* id,float fallback){
                if(const auto* p=processor.state.getRawParameterValue(id))
                    return std::isfinite(p->load())?p->load():fallback;
                return fallback;
            };
            const int wave=juce::jlimit(0,3,juce::roundToInt(getLfo("lfoWave",0.f)));
            const float rate=juce::jlimit(.05f,16.f,getLfo("lfoRate",2.f));
            const float depth=juce::jlimit(0.f,1.f,getLfo("lfoDepth",0.f));
            const float cycles=juce::jlimit(.85f,6.f,1.f+.95f*std::sqrt(rate));
            const float left=90.f*scaleX(),right=738.f*scaleX();
            const float mid=724.f*scaleY(),amp=39.f*scaleY();
            g.setColour(juce::Colour(0x526a8a87));
            g.drawLine(left,mid,right,mid,1.f);
            juce::Path silhouette,live;
            for(int i=0;i<=480;++i){
                const float t=(float)i/480.f;
                const double phase=lfoPreviewPhase+echo86::lfoTwoPi*cycles*t;
                const float sample=echo86::lfoPreviewSample(wave,phase);
                const float xx=left+(right-left)*t;
                if(i==0){
                    silhouette.startNewSubPath(xx,mid-sample*amp*.35f);
                    live.startNewSubPath(xx,mid-sample*amp*depth);
                }else{
                    silhouette.lineTo(xx,mid-sample*amp*.35f);
                    live.lineTo(xx,mid-sample*amp*depth);
                }
            }
            g.saveState();
            g.reduceClipRegion(rect(84,679,666,98));
            g.setColour(juce::Colour(0x665c8386));
            g.strokePath(silhouette,juce::PathStrokeType(1.3f*scaleY()));
            g.setColour(juce::Colour(0xffe6c77d));
            g.strokePath(live,juce::PathStrokeType(2.4f*scaleY()));
            g.restoreState();
            const char* waveNames[]={"SINUS","TRIANGLE","CARRÉ","S & H"};
            simpleText(g,tr(waveNames[wave])+
                       " / "+juce::String(rate,2)+" Hz / "+
                       juce::String(juce::roundToInt(depth*100.f))+" %",
                       438,654,297,21,12,juce::Colour(0xffdbcaa1),
                       juce::Justification::centredRight);
            simpleText(g,u8"FORME D'ONDE LFO",82,656,340,22,13,juce::Colour(0xff819b9c));
            break;
          }
          default:
            section(g,40,277,845,644,"PERFORMANCE",u8"CONTRÔLE CLAVIER");
            section(g,903,277,657,355,u8"CHORUS STÉRÉO VINTAGE","BBD-STYLE");
            section(g,903,649,657,272,u8"CARACTÈRE VINTAGE",u8"VARIATIONS DE VOIX");
            simpleText(g,u8"MODE VOIX",65,338,260,23,14,juce::Colour(0xffd6ddcb));
            simpleText(g,"CHORUS",925,339,255,23,14,juce::Colour(0xffd6ddcb));
            break;
        }
        for(const auto& c:knobs){
            if(c.page!=page&&c.page!=-1)continue;
            // The master label is at the left of the knob as in the source facade.
            if(c.page==-1)continue;
            simpleText(g,c.name,c.x-90,c.y-(c.h==188?122:c.h<60?52:75),180,21,13,
                         juce::Colour(0xffd6decb),juce::Justification::centred);
        }
        g.setColour(juce::Colour(0xff4b6163));
        g.drawLine(39*scaleX(),944*scaleY(),1561*scaleX(),944*scaleY(),1);
        simpleText(g,"LIBERTY / E86",53,966,390,27,16,juce::Colour(0xffb7d1c8));
        simpleText(g,u8"16 VOIX / 2 DCO / MIDI / 100 PRESETS D'USINE",53,1002,1000,17,12,
                   juce::Colour(0xff91a9a5));
        simpleText(g,u8"VOLUME GÉNÉRAL",1262,969,199,26,14,juce::Colour(0xffd5e0ce),
                    juce::Justification::centredRight);
    }
    void resized()override{
        if(!prepared)return;
        const int tx[]={40,399,758,1117};
        for(int i=0;i<4;++i)tabs[(size_t)i]->setBounds(rect(tx[i],205,344,51));
        presetButton->setBounds(rect(1330,43,231,52));
        prevButton->setBounds(rect(1330,101,66,47));
        initButton->setBounds(rect(1404,101,82,47));
        nextButton->setBounds(rect(1494,101,66,47));
        languageButton->setBounds(rect(1330,155,231,30));
        for(auto& c:knobs)
            c.slider->setBounds(rect(c.x-c.w/2,c.y-c.h/2,c.w,c.h));
        for(auto& b:switches)b.button->setBounds(rect(b.x,b.y,b.w,b.h));
        if(presetBrowser)presetBrowser->setBounds(getLocalBounds());
    }
private:
    void addKnob(int pg,int x,int y,const char* name,const char* id,
                 int w=86,int h=105){
        addControl(pg,x,y,name,id,w,h,false);
    }
    void addFader(int pg,int x,int y,const char* name,const char* id){
        addControl(pg,x,y,name,id,72,188,true);
    }
    void addHorizontal(int pg,int x,int y,const char* name,const char* id,int width){
        addControl(pg,x,y,name,id,width,47,false);
        if(!knobs.empty())knobs.back().slider->setSliderStyle(juce::Slider::LinearHorizontal);
    }
    void addControl(int pg,int x,int y,const char* name,const char* id,
                    int w,int h,bool fader){
        Knob c;
        c.page=pg;c.x=x;c.y=y;c.w=w;c.h=h;c.name=juce::String::fromUTF8(name);
        c.slider=std::make_unique<juce::Slider>();
        c.slider->setLookAndFeel(&look);
        c.slider->setSliderStyle(fader?juce::Slider::LinearVertical:
                                 juce::Slider::RotaryHorizontalVerticalDrag);
        if(!fader)c.slider->setRotaryParameters(juce::MathConstants<float>::pi*1.22f,
                         juce::MathConstants<float>::pi*2.78f,true);
        c.slider->setTextBoxStyle(juce::Slider::TextBoxBelow,false,76,22);
        c.slider->setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffd4dfd2));
        c.slider->setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff101b1e));
        c.slider->setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff495c5b));
        c.slider->setTooltip(c.name);
        if(pg==2)c.slider->onValueChange=[this]{if(page==2)repaint(rect(67,647,694,140));};
        if(pg==1)c.slider->onValueChange=[this]{
            // Immediate preview feedback during a fader drag.
            if(page==1){
                repaint(rect(60,341,400,205));
                repaint(rect(60,675,400,205));
            }
        };
        if(processor.state.getParameter(id)==nullptr)return;
        c.attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                            processor.state,id,*c.slider);
        addAndMakeVisible(*c.slider);
        knobs.push_back(std::move(c));
    }
    void addSwitch(int pg,int x,int y,int w,int h,const char* title,
                   const char* id,float value){
        Switch s;
        s.page=pg;s.x=x;s.y=y;s.w=w;s.h=h;s.id=id;s.value=value;s.title=juce::String::fromUTF8(title);
        s.button=std::make_unique<juce::TextButton>(tr(s.title));
        s.button->setLookAndFeel(&look);
        s.button->onClick=[this,parameter=juce::String(id),value]{
            setParameter(parameter,value);
            refreshSwitches();
            if(page==2)repaint(rect(67,647,694,140));
        };
        addAndMakeVisible(*s.button);
        switches.push_back(std::move(s));
    }
    void setParameter(const juce::String& id,float value){
        if(auto* p=processor.state.getParameter(id)){
            p->beginChangeGesture();
            p->setValueNotifyingHost(p->convertTo0to1(value));
            p->endChangeGesture();
        }
    }
    void setPage(int index){
        page=juce::jlimit(0,3,index);
        for(auto& c:knobs)c.slider->setVisible(c.page==page||c.page==-1);
        for(auto& s:switches)s.button->setVisible(s.page==page);
        for(int i=0;i<4;++i)tabs[(size_t)i]->setToggleState(i==page,juce::dontSendNotification);
        lastPreviewTimeMs=juce::Time::getMillisecondCounterHiRes();
        startTimerHz(page==2?30:page==1?15:8);
        repaint();
    }
    void refreshSwitches(){
        for(auto& b:switches){
            const auto* raw=processor.state.getRawParameterValue(b.id);
            if(raw!=nullptr)
                b.button->setToggleState(std::abs(raw->load()-b.value)<.01f,juce::dontSendNotification);
        }
    }
    void timerCallback()override{
        refreshSwitches();
        if(page==2){
            const double now=juce::Time::getMillisecondCounterHiRes();
            const double dt=juce::jlimit(0.0,.12,(now-lastPreviewTimeMs)/1000.0);
            lastPreviewTimeMs=now;
            const auto* raw=processor.state.getRawParameterValue("lfoRate");
            const double rate=raw?juce::jlimit(.05,16.0,(double)raw->load()):2.0;
            lfoPreviewPhase+=echo86::lfoTwoPi*rate*dt;
            if(lfoPreviewPhase>echo86::lfoTwoPi*2048.0)
                lfoPreviewPhase=std::fmod(lfoPreviewPhase,echo86::lfoTwoPi*2048.0);
            repaint(rect(67,647,694,140));
        }
        if(page==1){
            // Host automation and preset recall update both envelope plots.
            repaint(rect(60,341,400,205));
            repaint(rect(60,675,400,205));
        }
    }
    static juce::String readablePresetName(const char* utf8){
        return juce::String::fromUTF8(utf8)
             .replace(juce::String::fromUTF8(u8" • "), " - ");
    }
    void loadPreset(int index){
        if(index<0||index>=(int)echo86::factoryPresets.size())return;
        const auto& preset=echo86::factoryPresets[(size_t)index];
        const auto& extra=echo86::factoryExtendedPresets[(size_t)index];
        jassert(juce::String::fromUTF8(preset.name)==juce::String::fromUTF8(extra.name));
        for(size_t i=0;i<echo86::parameterIds.size();++i)
            setParameter(echo86::parameterIds[i],preset.values[i]);
        for(size_t i=0;i<extendedIds.size();++i)
            setParameter(extendedIds[i],extra.values[i]);
        selectedPreset=index;refreshSwitches();repaint();
    }
    void nextPreset(int delta){
        const int count=(int)echo86::factoryPresets.size();
        loadPreset((selectedPreset+count+delta)%count);
    }
    // Show the integrated professional preset browser, not a tiny popup menu.
    void openPresets(){
        if(!presetBrowser)return;
        presetBrowser->reload();
        presetBrowser->setBounds(getLocalBounds());
        presetBrowser->setVisible(true);
        presetBrowser->toFront(true);
        presetBrowser->grabKeyboardFocus();
    }

    bool saveUserPreset(const juce::String& requestedName,const juce::String& category){
        const auto name=requestedName.trim().substring(0,80);
        const auto safeName=juce::File::createLegalFileName(name);
        if(name.isEmpty()||safeName.isEmpty())return false;
        const auto dir=juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
          .getChildFile("Liberty Music Software").getChildFile("ECHO 86").getChildFile("Presets");
        if(dir.createDirectory().failed())return false;
        auto file=dir.getChildFile(safeName+".echo86");
        if(file.existsAsFile())file=file.getNonexistentSibling();
        auto data=serializePreset();
        if(auto* object=data.getDynamicObject()){
            object->setProperty("name",name);
            object->setProperty("category",category);
        }
        return file.replaceWithText(juce::JSON::toString(data,true));
    }

    // Validate the full preset before altering the audio processor state.
    bool loadUserPresetFile(const juce::File& file){
        if(!file.existsAsFile()||file.getSize()<=0||file.getSize()>65536)return false;
        const auto data=juce::JSON::parse(file.loadFileAsString());
        if(!data.isObject()||data.getProperty("format","").toString()!="ECHO86-PRESET"||
           (int)data.getProperty("version",0)!=1||
           !data.getProperty("parameters",juce::var()).isObject())return false;
        const auto values=data.getProperty("parameters",juce::var());
        auto valid=[&](const char* id,bool mandatory) {
            const auto v=values.getProperty(id,juce::var());
            if(!mandatory && v.isVoid())return true;
            const auto* p=processor.state.getParameter(id);
            if(p==nullptr||(!v.isDouble()&&!v.isInt())||!std::isfinite((double)v))return false;
            const float x=(float)(double)v;
            return x>=p->convertFrom0to1(0.f) && x<=p->convertFrom0to1(1.f);
        };
        for(const auto* id:echo86::parameterIds)if(!valid(id,true))return false;
        for(const auto* id:extendedIds)if(!valid(id,false))return false;
        for(const auto* id:echo86::parameterIds)
            setParameter(id,(float)(double)values.getProperty(id,juce::var()));
        for(const auto* id:extendedIds){
            const auto v=values.getProperty(id,juce::var());
            if(v.isVoid()){
                if(auto* p=processor.state.getParameter(id)){
                    p->beginChangeGesture();
                    p->setValueNotifyingHost(p->getDefaultValue());
                    p->endChangeGesture();
                }
            }else setParameter(id,(float)(double)v);
        }
        selectedPreset=-1;
        refreshSwitches();
        repaint();
        return true;
    }
    juce::var serializePreset()const{
        auto* root=new juce::DynamicObject();
        root->setProperty("format","ECHO86-PRESET");
        root->setProperty("version",1);
        root->setProperty("name",selectedPreset<0?"Init":
             readablePresetName(echo86::factoryPresets[(size_t)selectedPreset].name));
        auto* values=new juce::DynamicObject();
        for(auto* id:echo86::parameterIds)
            values->setProperty(id,processor.state.getRawParameterValue(id)->load());
        for(auto* id:extendedIds)
            values->setProperty(id,processor.state.getRawParameterValue(id)->load());
        root->setProperty("parameters",juce::var(values));
        return juce::var(root);
    }
    void exportPreset(){
        auto suggested=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                        .getChildFile("ECHO86-Sound.echo86");
        chooser=std::make_unique<juce::FileChooser>(juce::String::fromUTF8(u8"Exporter un son ECHO 86"),suggested,"*.echo86");
        juce::Component::SafePointer<Echo86Editor> safe(this);
        chooser->launchAsync(juce::FileBrowserComponent::saveMode|
                juce::FileBrowserComponent::canSelectFiles|
                juce::FileBrowserComponent::warnAboutOverwriting,
                [safe](const juce::FileChooser& fc){
                  if(safe==nullptr)return;
                  auto file=fc.getResult();
                  if(file==juce::File{})return;
                  if(!file.replaceWithText(juce::JSON::toString(safe->serializePreset(),true)))
                     juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                            "ECHO 86",juce::String::fromUTF8(u8"Impossible d'exporter le préréglage"));
                });
    }
    void importPreset(){
        chooser=std::make_unique<juce::FileChooser>(juce::String::fromUTF8(u8"Importer un son ECHO 86"),juce::File{},"*.echo86");
        juce::Component::SafePointer<Echo86Editor> safe(this);
        chooser->launchAsync(juce::FileBrowserComponent::openMode|
              juce::FileBrowserComponent::canSelectFiles,
              [safe](const juce::FileChooser& fc){
                 if(safe==nullptr)return;
                 const auto file=fc.getResult();
                 if(file==juce::File{}||file.getSize()>65536)return;
                 const auto data=juce::JSON::parse(file.loadFileAsString());
                 if(!data.isObject()||data.getProperty("format","").toString()!="ECHO86-PRESET"||
                    (int)data.getProperty("version",0)!=1||
                    !data.getProperty("parameters",juce::var()).isObject())return;
                 const auto values=data.getProperty("parameters",juce::var());
                 for(auto* id:echo86::parameterIds){
                     auto* p=safe->processor.state.getParameter(id);
                     const auto v=values.getProperty(id,juce::var());
                     if(p==nullptr||(!v.isDouble()&&!v.isInt())||!std::isfinite((double)v))return;
                     const float value=(float)(double)v;
                     if(value<p->convertFrom0to1(0.f)||value>p->convertFrom0to1(1.f))return;
                 }
                 // Optional fields make original 24-parameter preset files compatible.
                 for(auto* id:extendedIds){
                     const auto v=values.getProperty(id,juce::var());
                     if(v.isVoid())continue;
                     const auto* p=safe->processor.state.getParameter(id);
                     if(p==nullptr||(!v.isDouble()&&!v.isInt())||!std::isfinite((double)v))return;
                     const float value=(float)(double)v;
                     if(value<p->convertFrom0to1(0.f)||value>p->convertFrom0to1(1.f))return;
                 }
                 for(auto* id:echo86::parameterIds)
                      safe->setParameter(id,(float)(double)values.getProperty(id,juce::var()));
                 for(auto* id:extendedIds){
                     const auto v=values.getProperty(id,juce::var());
                     auto* p=safe->processor.state.getParameter(id);
                     if(p!=nullptr){
                        if(v.isVoid()){
                           p->beginChangeGesture();
                           p->setValueNotifyingHost(p->getDefaultValue());
                           p->endChangeGesture();
                        }
                        else safe->setParameter(id,(float)(double)v);
                     }
                 }
                 safe->selectedPreset=-1;
                 safe->refreshSwitches();
                 safe->repaint();
              });
    }
};
