#include "UILocalization.h"
#include "FactoryPresets.h"
#include "FactoryPresetExtras.h"
#include <cassert>
#include <cstring>
#include <set>
#include <string>
#include <iostream>
int main(){
 using namespace echo86::locale;
 static_assert(count==5);
 assert(parse("FR")==Language::fr);
 assert(parse("EN")==Language::en);
 assert(parse("ES")==Language::es);
 assert(parse("DE")==Language::de);
 assert(parse("IT")==Language::it);
 assert(parse("NOT_A_LOCALE")==Language::fr);
 assert(std::strcmp(code(Language::fr),"FR")==0);
 assert(std::strcmp(code(Language::it),"IT")==0);
 std::set<std::string> keys;
 for(const auto& entry:labels){
    assert(entry.source && *entry.source);
    assert(keys.insert(entry.source).second);
    assert(entry.en && *entry.en);
    assert(entry.es && *entry.es);
    assert(entry.de && *entry.de);
    assert(entry.it && *entry.it);
    assert(std::strcmp(translate(Language::fr,entry.source),entry.source)==0);
    assert(std::strcmp(translate(Language::en,entry.source),entry.en)==0);
    assert(std::strcmp(translate(Language::es,entry.source),entry.es)==0);
    assert(std::strcmp(translate(Language::de,entry.source),entry.de)==0);
    assert(std::strcmp(translate(Language::it,entry.source),entry.it)==0);
 }
 for(const auto* essential: {
    "01  VOIX","02  ENVELOPPES","03  MODULATION","04  PERFORMANCE / FX",
    "SYNTHÉTISEUR ANALOGIQUE VINTAGE","BIBLIOTHÈQUE DE SONS",
    "USINE","FAVORIS","UTILISATEUR","TOUS","BASS","LEAD","PAD","KEYS",
    "PLUCK","ARP","STRINGS","FX","CHARGER","ENREGISTRER","IMPORTER",
    "EXPORTER","FERMER","FRÉQUENCE","RÉSONANCE","ATTAQUE","RELÂCHEMENT",
    "FORME D'ONDE LFO","VOLUME GÉNÉRAL","LANGUE","MONO / NOTE BASSE",
    "CROSS MOD","FM AUDIO","LFO -> FILTRE"
 }){
    assert(keys.count(essential)==1);
 }
 assert(std::strcmp(translate(Language::en,"ATTAQUE"),"ATTACK")==0);
 assert(std::strcmp(translate(Language::de,"FRÉQUENCE"),"FREQUENZ")==0);
 assert(std::strcmp(translate(Language::es,"MAINTIEN"),"SOSTENIDO")==0);
 assert(std::strcmp(translate(Language::it,"RELÂCHEMENT"),"RILASCIO")==0);
 assert(std::strcmp(translate(Language::en,"lfoDepth"),"lfoDepth")==0);
 assert(std::strcmp(translate(Language::en,"crossMod"),"crossMod")==0);
 assert(std::strcmp(translate(Language::es,"UNKNOWN_LABEL"),"UNKNOWN_LABEL")==0);
 static_assert(echo86::factoryPresets.size()==100);
 static_assert(echo86::parameterIds.size()==24);
 static_assert(echo86::factoryExtendedPresets.size()==100);
 static_assert(echo86::factoryExtendedIds.size()==43);
 std::cout<<"PASS: five UI languages, complete core labels, preserved DSP parameter IDs and 100 factory presets\n";
}
