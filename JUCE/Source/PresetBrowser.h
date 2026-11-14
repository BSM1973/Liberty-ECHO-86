#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <functional>
#include <cmath>
#include "FactoryPresets.h"
#include "UILocalization.h"

// Non-modal in-editor overlay: remains functional in AU, VST3 and Standalone
// without creating unmanaged native windows in plugin hosts.
class Echo86PresetBrowser final : public juce::Component, private juce::ListBoxModel {
public:
    struct Callbacks {
        std::function<void(int)> loadFactory;
        std::function<bool(const juce::File&)> loadUser;
        std::function<bool(const juce::String&, const juce::String&)> saveUser;
        std::function<void()> exportUser;
        std::function<void()> close;
    };

    explicit Echo86PresetBrowser(Callbacks callbacksToUse)
        : callbacks(std::move(callbacksToUse)),
          presetFolder(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
             .getChildFile("Liberty Music Software").getChildFile("ECHO 86").getChildFile("Presets")),
          favoritesFile(presetFolder.getParentDirectory().getChildFile("favorites.txt")),
          browserList("ECHO86 preset list", this) {
        setOpaque(true);
        presetFolder.createDirectory();
        if (favoritesFile.existsAsFile())
            favoritesFile.readLines(favorites);
        favorites.removeEmptyStrings();

        search.setTextToShowWhenEmpty(tr(u8"Rechercher un son, un instrument, un tag..."),
                                    juce::Colour(0xff839ca0));
        search.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff111f25));
        search.setColour(juce::TextEditor::textColourId, juce::Colour(0xfff1e6cb));
        search.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff587078));
        search.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xffd9a65e));
        search.onTextChange = [this] { refreshRows(); };
        addAndMakeVisible(search);

        static const char* sourceNames[] = {"USINE", "FAVORIS", "UTILISATEUR"};
        for (int i=0;i<3;++i) {
            sourceButtons[(size_t)i].setButtonText(sourceNames[i]);
            styleButton(sourceButtons[(size_t)i]);
            sourceButtons[(size_t)i].onClick = [this,i] {
                source=i; refreshButtons(); refreshRows();
            };
            addAndMakeVisible(sourceButtons[(size_t)i]);
        }

        static const char* families[] = {"TOUS", "BASS", "LEAD", "PAD", "KEYS",
                                         "PLUCK", "ARP", "STRINGS", "FX"};
        for (int i=0;i<9;++i) {
            familyButtons[(size_t)i].setButtonText(families[i]);
            styleButton(familyButtons[(size_t)i]);
            familyButtons[(size_t)i].onClick = [this,i] {
                categoryIndex=i; refreshButtons(); refreshRows();
            };
            addAndMakeVisible(familyButtons[(size_t)i]);
        }

        browserList.setModel(this);
        browserList.setRowHeight(47);
        browserList.setMultipleSelectionEnabled(false);
        browserList.setColour(juce::ListBox::backgroundColourId,juce::Colour(0xff102129));
        browserList.setColour(juce::ListBox::outlineColourId,juce::Colour(0xff34515b));
        browserList.setOutlineThickness(1);
        addAndMakeVisible(browserList);

        nameField.setText(tr(u8"Mon preset"),juce::dontSendNotification);
        nameField.setColour(juce::TextEditor::backgroundColourId,juce::Colour(0xff0b1b22));
        nameField.setColour(juce::TextEditor::textColourId,juce::Colour(0xfff2e3c6));
        nameField.setColour(juce::TextEditor::outlineColourId,juce::Colour(0xff47626b));
        addAndMakeVisible(nameField);

        loadButton.setButtonText("CHARGER");
        favoriteButton.setButtonText("FAVORI");
        saveButton.setButtonText("ENREGISTRER");
        importButton.setButtonText("IMPORTER");
        exportButton.setButtonText("EXPORTER");
        closeButton.setButtonText("FERMER");
        for (auto* b : {&loadButton,&favoriteButton,&saveButton,&importButton,&exportButton,&closeButton}) {
            styleButton(*b);
            addAndMakeVisible(*b);
        }

        loadButton.onClick = [this] { loadSelection(); };
        favoriteButton.onClick = [this] { toggleFavorite(); };
        saveButton.onClick = [this] {
            const auto name=nameField.getText().trim();
            if(name.isEmpty()) { showError(tr(u8"Indique un nom pour le preset."));return; }
            const auto cat=categoryIndex==0 ? selectedCategory() : juce::String(familyCode(categoryIndex));
            if(callbacks.saveUser && callbacks.saveUser(name,cat)) {
                source=2;categoryIndex=0;search.clear();refreshButtons();refreshRows();
            } else showError(tr(u8"Impossible d'enregistrer ce preset."));
        };
        importButton.onClick = [this] { importUserPreset(); };
        exportButton.onClick = [this] { if(callbacks.exportUser) callbacks.exportUser(); };
        closeButton.onClick = [this] { if(callbacks.close) callbacks.close(); };
        refreshButtons();
        refreshRows();
    }

    // Display language is independent of stable source/category/preset identifiers.
    void setLanguage(echo86::locale::Language next){
        const auto previous=language;
        const auto oldDefault=juce::String::fromUTF8(echo86::locale::translate(previous,"Mon preset"));
        const bool untouchedName=nameField.getText()==oldDefault;
        language=next;
        const char* sourceKeys[]={"USINE","FAVORIS","UTILISATEUR"};
        for(int i=0;i<3;++i)sourceButtons[(size_t)i].setButtonText(tr(sourceKeys[i]));
        for(int i=0;i<9;++i)familyButtons[(size_t)i].setButtonText(tr(familyCode(i)));
        loadButton.setButtonText(tr("CHARGER"));
        favoriteButton.setButtonText(tr("FAVORI"));
        saveButton.setButtonText(tr("ENREGISTRER"));
        importButton.setButtonText(tr("IMPORTER"));
        exportButton.setButtonText(tr("EXPORTER"));
        closeButton.setButtonText(tr("FERMER"));
        search.setTextToShowWhenEmpty(tr("Rechercher un son, un instrument, un tag..."),
                                      juce::Colour(0xff839ca0));
        if(untouchedName)nameField.setText(tr("Mon preset"),juce::dontSendNotification);
        refreshRows();
        repaint();
    }
    void reload() { refreshRows(); }

    bool keyPressed(const juce::KeyPress& key) override {
        if (key==juce::KeyPress::escapeKey) { if(callbacks.close) callbacks.close();return true; }
        return false;
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff09151c));
        auto r=panelBounds().toFloat();
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff243a42),r.getX(),r.getY(),
                                              juce::Colour(0xff101e27),r.getX(),r.getBottom(),false));
        g.fillRoundedRectangle(r,14);
        g.setColour(juce::Colour(0xff658389));g.drawRoundedRectangle(r.reduced(.5f),14,1.4f);
        g.setColour(juce::Colour(0xffd8a25e));
        g.setFont(juce::Font(juce::FontOptions(32.f).withStyle("Bold")));
        g.drawText(tr(u8"BIBLIOTHÈQUE DE SONS"),
                   r.getX()+27,r.getY()+18,r.getWidth()-60,43,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xffa6c0c2));
        g.setFont(juce::Font(juce::FontOptions(13.f)));
        g.drawText(tr(u8"ECHO 86  /  100 PRESETS D'USINE  /  FAVORIS  /  CRÉATIONS PERSONNELLES"),
                   r.getX()+30,r.getY()+64,r.getWidth()-60,22,juce::Justification::centredLeft);
        const auto p=panelBounds();
        const int left=p.getX()+28, top=p.getY()+181;
        g.setColour(juce::Colour(0xffe6d9b8));
        g.setFont(juce::Font(juce::FontOptions(15.f).withStyle("Bold")));
        g.drawText(tr("INSTRUMENTS"),left,top-34,195,25,juce::Justification::centredLeft);
        const int rightX=p.getRight()-290;
        g.drawText(tr(u8"APERÇU DU PRESET"),rightX,top-34,260,25,
                   juce::Justification::centredLeft);
        auto info=juce::Rectangle<int>(rightX,top+8,260,juce::jmax(90,p.getBottom()-top-178));
        g.setColour(juce::Colour(0xff0c2029));g.fillRoundedRectangle(info.toFloat(),8);
        g.setColour(juce::Colour(0xff375a66));g.drawRoundedRectangle(info.toFloat(),8,1);

        const auto row=selectedRow();
        g.setColour(juce::Colour(0xffdfb77a));
        g.setFont(juce::Font(juce::FontOptions(19.f).withStyle("Bold")));
        g.drawFittedText(row?row->name:tr(u8"Choisir un son"),
                         info.reduced(18,14).removeFromTop(65),
                         juce::Justification::topLeft,3,.85f);
        g.setFont(juce::Font(juce::FontOptions(13.f)));
        g.setColour(juce::Colour(0xffbbd0d1));
        const int ix=info.getX()+18, iy=info.getY()+86;
        g.drawText(tr(u8"CATÉGORIE : ") + (row?tr(row->category.toRawUTF8()):juce::String("-")),
                   ix,iy,info.getWidth()-36,24,juce::Justification::centredLeft);
        g.drawText(tr(u8"SOURCE : ") +
                   (row?tr(row->factoryIndex>=0?"USINE":"UTILISATEUR"):juce::String("-")),
                   ix,iy+36,info.getWidth()-36,24,juce::Justification::centredLeft);
        g.drawText(tr(u8"FAVORI : ") +
                   (row?tr(favorites.contains(row->key)?"OUI":"NON"):juce::String("-")),
                   ix,iy+72,info.getWidth()-36,24,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff8ba9b0));
        g.drawFittedText(tr(u8"Double-clic pour charger.\nEnregistre tes sons dans la bibliothèque UTILISATEUR.\nLes favoris sont conservés."),
                   ix,iy+129,info.getWidth()-36,120,juce::Justification::topLeft,6,0.9f);
        g.setColour(juce::Colour(0xffbbccce));
        g.setFont(juce::Font(juce::FontOptions(12.f)));
        const int bottomY=p.getBottom()-133;
        g.drawText(tr(u8"NOM DE TON PRESET"),
                   left,bottomY-14,300,18,juce::Justification::centredLeft);
        g.drawText(juce::String(rows.size())+tr(u8" son(s) affiché(s)"),
                   p.getRight()-270,bottomY-14,242,18,juce::Justification::centredRight);
    }

    void resized() override {
        const auto p=panelBounds();
        const int x=p.getX()+28,y=p.getY()+108;
        const int right=p.getRight()-28;
        const int sourceW=juce::jmin(140,(right-x-30)/3);
        for(int i=0;i<3;++i)
            sourceButtons[(size_t)i].setBounds(x+i*(sourceW+11),y,sourceW,40);

        const int bodyTop=p.getY()+189;
        for(int i=0;i<9;++i)
            familyButtons[(size_t)i].setBounds(x,bodyTop+i*juce::jmax(32,juce::jmin(42,(p.getHeight()-370)/9)),192,33);

        const int listX=x+212;
        const int infoX=p.getRight()-290;
        search.setBounds(listX,p.getY()+164,infoX-listX-17,38);
        browserList.setBounds(listX,bodyTop+15,infoX-listX-17,
                              juce::jmax(100,p.getBottom()-bodyTop-162));
        const int bottomY=p.getBottom()-133;
        nameField.setBounds(x,bottomY+12,juce::jmin(440,p.getWidth()/3),36);
        const int buttonY=p.getBottom()-67;
        const int gap=11,w=(p.getWidth()-56-gap*5)/6;
        juce::TextButton* bs[]={&loadButton,&favoriteButton,&saveButton,&importButton,&exportButton,&closeButton};
        for(int i=0;i<6;++i)bs[i]->setBounds(x+i*(w+gap),buttonY,w,38);
    }

private:
    struct Entry {
        int factoryIndex=-1;
        juce::File file;
        juce::String name,category,key;
    };
    static const char* familyCode(int index){
        static const char* keys[]={"TOUS","BASS","LEAD","PAD","KEYS","PLUCK","ARP","STRINGS","FX"};
        return keys[juce::jlimit(0,8,index)];
    }
    juce::String tr(const char* source)const{
        return juce::String::fromUTF8(echo86::locale::translate(language,source));
    }
    echo86::locale::Language language=echo86::locale::Language::fr;
    Callbacks callbacks;
    juce::File presetFolder,favoritesFile;
    juce::StringArray favorites;
    std::vector<Entry> rows;
    juce::TextEditor search,nameField;
    std::array<juce::TextButton,3> sourceButtons;
    std::array<juce::TextButton,9> familyButtons;
    juce::ListBox browserList;
    juce::TextButton loadButton,favoriteButton,saveButton,importButton,exportButton,closeButton;
    std::unique_ptr<juce::FileChooser> chooser;
    int source=0,categoryIndex=0;

    static void styleButton(juce::TextButton& b) {
        b.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff28424c));
        b.setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xffa4773f));
        b.setColour(juce::TextButton::textColourOffId,juce::Colour(0xffe4e7d6));
        b.setColour(juce::TextButton::textColourOnId,juce::Colour(0xff121e24));
        b.setClickingTogglesState(false);
    }
    juce::Rectangle<int> panelBounds() const {
        return getLocalBounds().reduced(juce::jmax(12,getWidth()/36),
                                       juce::jmax(12,getHeight()/25));
    }
    void refreshButtons() {
        for(int i=0;i<3;++i)sourceButtons[(size_t)i].setToggleState(i==source,juce::dontSendNotification);
        for(int i=0;i<9;++i)familyButtons[(size_t)i].setToggleState(i==categoryIndex,juce::dontSendNotification);
    }
    void refreshRows() {
        rows.clear();
        const auto query=search.getText().trim();
        const auto selectedFamily=juce::String(familyCode(categoryIndex));
        auto accept=[&](Entry e){
            if(source==0 && e.factoryIndex<0)return;
            if(source==2 && e.factoryIndex>=0)return;
            if(source==1 && !favorites.contains(e.key))return;
            if(categoryIndex!=0 && e.category!=selectedFamily)return;
            if(query.isNotEmpty() && !e.name.containsIgnoreCase(query) &&
               !e.category.containsIgnoreCase(query) &&
               !tr(e.category.toRawUTF8()).containsIgnoreCase(query))return;
            rows.push_back(std::move(e));
        };
        for(int i=0;i<(int)echo86::factoryPresets.size();++i){
            auto name=juce::String::fromUTF8(echo86::factoryPresets[(size_t)i].name)
                        .replace(juce::String::fromUTF8(u8" • ")," - ");
            const auto cat=name.upToFirstOccurrenceOf(" ",false,false).toUpperCase();
            accept(Entry{i,{},name,cat,"factory:"+juce::String(i)});
        }
        if(presetFolder.isDirectory()) {
            for(const auto& f:presetFolder.findChildFiles(juce::File::findFiles,false,"*.echo86")) {
                if(f.getSize()<=0||f.getSize()>65536)continue;
                const auto data=juce::JSON::parse(f.loadFileAsString());
                if(!data.isObject() || data.getProperty("format","").toString()!="ECHO86-PRESET")continue;
                auto n=data.getProperty("name",f.getFileNameWithoutExtension()).toString();
                if(n.isEmpty())n=f.getFileNameWithoutExtension();
                const auto cat=data.getProperty("category","USER").toString().toUpperCase();
                accept(Entry{-1,f,n,cat,"user:"+f.getFileName()});
            }
        }
        browserList.updateContent();
        if(rows.empty())browserList.deselectAllRows();
        else browserList.selectRow(0);
        loadButton.setEnabled(!rows.empty());
        favoriteButton.setEnabled(!rows.empty());
        repaint();
    }
    const Entry* selectedRow() const {
        const int i=browserList.getSelectedRow();
        return i>=0 && i<(int)rows.size()?&rows[(size_t)i]:nullptr;
    }
    juce::String selectedCategory()const {
        if(const auto* entry=selectedRow())return entry->category;
        return "USER";
    }
    int getNumRows()override { return (int)rows.size(); }
    void paintListBoxItem(int i,juce::Graphics& g,int w,int h,bool selected)override {
        if(i<0||i>=(int)rows.size())return;
        const auto& entry=rows[(size_t)i];
        if(selected)g.fillAll(juce::Colour(0xff43606c));
        else if(i%2==1)g.fillAll(juce::Colour(0xff172e37));
        g.setColour(juce::Colour(0xffdbe8e0));
        g.setFont(juce::Font(juce::FontOptions(16.f).withStyle(selected?"Bold":"Regular")));
        g.drawFittedText(entry.name,juce::Rectangle<int>(17,4,w-135,h-8),
                         juce::Justification::centredLeft,1,.85f);
        g.setColour(juce::Colour(0xffd8ab6b));
        g.setFont(juce::Font(juce::FontOptions(12.f)));
        g.drawFittedText(tr(entry.category.toRawUTF8()),juce::Rectangle<int>(w-118,4,83,h-8),juce::Justification::centredRight,1,.7f);
        if(favorites.contains(entry.key)){
            g.setColour(juce::Colour(0xffecc076));
            g.setFont(juce::Font(juce::FontOptions(18.f).withStyle("Bold")));
            g.drawText("*",w-25,8,20,h-16,juce::Justification::centred);
        }
        g.setColour(juce::Colour(0xff2f4a52));
        g.drawLine(0,(float)h-1,(float)w,(float)h-1,1);
    }
    void selectedRowsChanged(int)override { repaint(); }
    void listBoxItemDoubleClicked(int,const juce::MouseEvent&)override {loadSelection();}
    void returnKeyPressed(int)override {loadSelection();}

    void loadSelection(){
        const auto* e=selectedRow();if(!e)return;
        bool success=false;
        if(e->factoryIndex>=0) {
            if(callbacks.loadFactory){callbacks.loadFactory(e->factoryIndex);success=true;}
        } else if(callbacks.loadUser) success=callbacks.loadUser(e->file);
        if(success){if(callbacks.close)callbacks.close();}
        else showError(tr(u8"Ce fichier de preset est invalide ou incompatible."));
    }
    void toggleFavorite(){
        const auto* e=selectedRow();if(!e)return;
        if(favorites.contains(e->key))favorites.removeString(e->key);
        else favorites.addIfNotAlreadyThere(e->key);
        if(!favoritesFile.replaceWithText(favorites.joinIntoString("\n")))
            showError(tr(u8"Impossible d'enregistrer les favoris."));
        if(source==1)refreshRows();
        else {browserList.repaint();repaint();}
    }
    void importUserPreset(){
        chooser=std::make_unique<juce::FileChooser>(
          tr(u8"Importer un preset ECHO 86"),juce::File{},"*.echo86");
        juce::Component::SafePointer<Echo86PresetBrowser> safe(this);
        chooser->launchAsync(juce::FileBrowserComponent::openMode|
                             juce::FileBrowserComponent::canSelectFiles,
            [safe](const juce::FileChooser& fc){
                if(safe==nullptr)return;
                auto f=fc.getResult();
                if(f==juce::File{})return;
                if(f.getSize()<=0||f.getSize()>65536){
                    safe->showError(safe->tr(u8"Fichier trop volumineux ou vide."));return;
                }
                const auto data=juce::JSON::parse(f.loadFileAsString());
                if(!data.isObject() || data.getProperty("format","").toString()!="ECHO86-PRESET" ||
                   (int)data.getProperty("version",0)!=1 ||
                   !data.getProperty("parameters",juce::var()).isObject()){
                    safe->showError(safe->tr(u8"Ce fichier n'est pas un preset ECHO 86."));return;
                }
                const auto values=data.getProperty("parameters",juce::var());
                for(const auto* id:echo86::parameterIds){
                    const auto v=values.getProperty(id,juce::var());
                    if((!v.isDouble()&&!v.isInt()) || !std::isfinite((double)v)){
                        safe->showError(safe->tr(u8"Paramètres invalides dans le preset."));
                        return;
                    }
                }
                const auto file=safe->presetFolder.getChildFile(
                       juce::File::createLegalFileName(f.getFileNameWithoutExtension())+".echo86")
                       .getNonexistentSibling();
                if(!f.copyFileTo(file)) {
                    safe->showError(safe->tr(u8"Impossible de copier ce preset."));return;
                }
                safe->source=2;safe->categoryIndex=0;safe->search.clear();
                safe->refreshButtons();safe->refreshRows();
            });
    }
    void showError(const juce::String& message){
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"ECHO 86",message);
    }
};
