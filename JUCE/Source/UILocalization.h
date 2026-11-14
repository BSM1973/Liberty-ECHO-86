#pragma once
// ECHO 86 interface translations: display-only, never alters APVTS parameter IDs.
#include <cstring>
#include <cstddef>
namespace echo86 { namespace locale {
enum class Language { fr=0,en=1,es=2,de=3,it=4 };
struct Entry { const char* source; const char* en; const char* es; const char* de; const char* it; };
inline constexpr Entry labels[] = {
    {"01  VOIX","01  VOICE","01  VOZ","01  STIMMEN","01  VOCI"},
    {"02  ENVELOPPES","02  ENVELOPES","02  ENVOLVENTES","02  HÜLLKURVEN","02  INVILUPPI"},
    {"03  MODULATION","03  MODULATION","03  MODULACIÓN","03  MODULATION","03  MODULAZIONE"},
    {"04  PERFORMANCE / FX","04  PERFORMANCE / FX","04  INTERPRETACIÓN / FX","04  SPIELHILFEN / FX","04  ESECUZIONE / FX"},
    {"PRESETS","PRESETS","PRESETS","PRESETS","PRESET"},
    {"LANGUE","LANGUAGE","IDIOMA","SPRACHE","LINGUA"},
    {"SYNTHÉTISEUR ANALOGIQUE VINTAGE","VINTAGE ANALOG SYNTHESIZER","SINTETIZADOR ANALÓGICO VINTAGE","VINTAGE-ANALOGSYNTHESIZER","SINTETIZZATORE ANALOGICO VINTAGE"},
    {"PROGRAM","PROGRAM","PROGRAMA","PROGRAMM","PROGRAMMA"},
    {"100 PRESETS D'USINE","100 FACTORY PRESETS","100 PRESETS DE FÁBRICA","100 WERKSPRESETS","100 PRESET DI FABBRICA"},
    {"OSCILLATEUR 1","OSCILLATOR 1","OSCILADOR 1","OSZILLATOR 1","OSCILLATORE 1"},
    {"OSCILLATEUR 2","OSCILLATOR 2","OSCILADOR 2","OSZILLATOR 2","OSCILLATORE 2"},
    {"MASTER","MASTER","PRINCIPAL","MASTER","PRINCIPALE"},
    {"ESCLAVE / SYNC","SLAVE / SYNC","ESCLAVO / SINC.","SLAVE / SYNC","SLAVE / SYNC"},
    {"MIXEUR / BRUIT","MIXER / NOISE","MEZCLADOR / RUIDO","MISCHER / RAUSCHEN","MIXER / RUMORE"},
    {"AVANT FILTRE","PRE-FILTER","PREFILTRO","VOR FILTER","PRE-FILTRO"},
    {"FILTRE ANALOGIQUE","ANALOG FILTER","FILTRO ANALÓGICO","ANALOGFILTER","FILTRO ANALOGICO"},
    {"4 PÔLES / 24 dB","4-POLE / 24 dB","4 POLOS / 24 dB","4-POL / 24 dB","4 POLI / 24 dB"},
    {"DOUBLE OSCILLATEUR","DUAL OSCILLATOR","DOBLE OSCILADOR","DOPPELOSZILLATOR","DOPPIO OSCILLATORE"},
    {"OSC 1 -> OSC 2 / LIBRE / SYNC","OSC 1 -> OSC 2 / FREE / SYNC","OSC 1 -> OSC 2 / LIBRE / SYNC","OSC 1 -> OSC 2 / FREI / SYNC","OSC 1 -> OSC 2 / LIBERO / SYNC"},
    {"ENVELOPPE 1 / AMPLITUDE","ENVELOPE 1 / AMP","ENVOLVENTE 1 / AMPLITUD","HÜLLKURVE 1 / AMP","INVILUPPO 1 / AMP"},
    {"ENVELOPPE 2 / FILTRE","ENVELOPE 2 / FILTER","ENVOLVENTE 2 / FILTRO","HÜLLKURVE 2 / FILTER","INVILUPPO 2 / FILTRO"},
    {"SIX ÉTAPES","SIX STAGES","SEIS ETAPAS","SECHS PHASEN","SEI FASI"},
    {"OSCILLATEUR BASSE FRÉQUENCE","LOW-FREQUENCY OSCILLATOR","OSCILADOR DE BAJA FRECUENCIA","NIEDERFREQUENZ-OSZILLATOR","OSCILLATORE A BASSA FREQUENZA"},
    {"ROUTAGES DE MODULATION","MODULATION ROUTING","RUTAS DE MODULACIÓN","MODULATIONSROUTING","PERCORSI DI MODULAZIONE"},
    {"CONTRÔLE DIRECT","DIRECT CONTROL","CONTROL DIRECTO","DIREKTSTEUERUNG","CONTROLLO DIRETTO"},
    {"EXPRESSION MIDI","MIDI EXPRESSION","EXPRESIÓN MIDI","MIDI-EXPRESSION","ESPRESSIONE MIDI"},
    {"MOLETTE / PRESSION","WHEEL / PRESSURE","RUEDA / PRESIÓN","RAD / DRUCK","ROTELLA / PRESSIONE"},
    {"FORME D'ONDE","WAVEFORM","FORMA DE ONDA","WELLENFORM","FORMA D'ONDA"},
    {"FORME D'ONDE LFO","LFO WAVEFORM","FORMA DE ONDA LFO","LFO-WELLENFORM","FORMA D'ONDA LFO"},
    {"PERFORMANCE","PERFORMANCE","INTERPRETACIÓN","SPIELHILFEN","ESECUZIONE"},
    {"CONTRÔLE CLAVIER","KEYBOARD CONTROL","CONTROL DEL TECLADO","TASTATURSTEUERUNG","CONTROLLO TASTIERA"},
    {"CHORUS STÉRÉO VINTAGE","VINTAGE STEREO CHORUS","CHORUS ESTÉREO VINTAGE","VINTAGE-STEREOCHORUS","CHORUS STEREO VINTAGE"},
    {"CARACTÈRE VINTAGE","VINTAGE CHARACTER","CARÁCTER VINTAGE","VINTAGE-CHARAKTER","CARATTERE VINTAGE"},
    {"VARIATIONS DE VOIX","VOICE VARIATIONS","VARIACIONES DE VOZ","STIMMENVARIATIONEN","VARIAZIONI DELLE VOCI"},
    {"MODE VOIX","VOICE MODE","MODO DE VOCES","STIMMENMODUS","MODALITÀ VOCI"},
    {"CHORUS","CHORUS","CHORUS","CHORUS","CHORUS"},
    {"LIBERTY / E86","LIBERTY / E86","LIBERTY / E86","LIBERTY / E86","LIBERTY / E86"},
    {"16 VOIX / 2 DCO / MIDI / 100 PRESETS D'USINE","16 VOICES / 2 DCO / MIDI / 100 FACTORY PRESETS","16 VOCES / 2 DCO / MIDI / 100 PRESETS DE FÁBRICA","16 STIMMEN / 2 DCO / MIDI / 100 WERKSPRESETS","16 VOCI / 2 DCO / MIDI / 100 PRESET DI FABBRICA"},
    {"VOLUME GÉNÉRAL","MASTER VOLUME","VOLUMEN GENERAL","GESAMTLAUTSTÄRKE","VOLUME GENERALE"},
    {"OCTAVE","OCTAVE","OCTAVA","OKTAVE","OTTAVA"},
    {"DEMI-TON","SEMITONE","SEMITONO","HALBTON","SEMITONO"},
    {"ACCORD FIN","FINE TUNE","AJUSTE FINO","FEINSTIMMUNG","ACCORDATURA FINE"},
    {"LARGEUR PULSE","PULSE WIDTH","ANCHO DE PULSO","PULSBREITE","LARGHEZZA IMPULSO"},
    {"PWM","PWM","PWM","PWM","PWM"},
    {"LIBRE","FREE","LIBRE","FREI","LIBERO"},
    {"SYNC","SYNC","SYNC","SYNC","SYNC"},
    {"CROSS MOD","CROSS MOD","MOD. CRUZADA","KREUZMOD.","MOD. INCROCIATA"},
    {"ENV -> HAUTEUR","ENV -> PITCH","ENV -> TONO","HÜLLK. -> TONHÖHE","INV. -> INTONAZIONE"},
    {"OSC 1","OSC 1","OSC 1","OSC 1","OSC 1"},
    {"OSC 2","OSC 2","OSC 2","OSC 2","OSC 2"},
    {"NOISE","NOISE","RUIDO","RAUSCHEN","RUMORE"},
    {"COULEUR","COLOR","COLOR","KLANGFARBE","COLORE"},
    {"FRÉQUENCE","CUTOFF","FRECUENCIA","FREQUENZ","FREQUENZA"},
    {"RÉSONANCE","RESONANCE","RESONANCIA","RESONANZ","RISONANZA"},
    {"PROF. ENV.","ENV DEPTH","PROF. ENV.","HÜLLK. TIEFE","PROF. INV."},
    {"SUIVI CLAVIER","KEY TRACK","SEGUIM. TECLADO","KEY-TRACKING","KEY TRACK"},
    {"VÉLOCITÉ","VELOCITY","VELOCIDAD","ANSCHLAGDYNAMIK","VELOCITÀ"},
    {"LFO -> FILTRE","LFO -> FILTER","LFO -> FILTRO","LFO -> FILTER","LFO -> FILTRO"},
    {"FM AUDIO","AUDIO FM","FM AUDIO","AUDIO-FM","FM AUDIO"},
    {"DRIVE","DRIVE","SATURACIÓN","DRIVE","DRIVE"},
    {"ATTAQUE","ATTACK","ATAQUE","ATTACK","ATTACCO"},
    {"DÉCLIN 1","DECAY 1","DECAIMIENTO 1","DECAY 1","DECAY 1"},
    {"RUPTURE","BREAKPOINT","PUNTO DE QUIEBRE","KNICKPUNKT","PUNTO DI ROTTURA"},
    {"DÉCLIN 2","DECAY 2","DECAIMIENTO 2","DECAY 2","DECAY 2"},
    {"MAINTIEN","SUSTAIN","SOSTENIDO","SUSTAIN","SUSTAIN"},
    {"RELÂCHEMENT","RELEASE","LIBERACIÓN","RELEASE","RILASCIO"},
    {"SINE","SINE","SENO","SINUS","SENO"},
    {"TRIANGLE","TRIANGLE","TRIÁNGULO","DREIECK","TRIANGOLO"},
    {"SQUARE","SQUARE","CUADRADA","RECHTECK","QUADRA"},
    {"SAW","SAW","SIERRA","SÄGEZAHN","DENTE DI SEGA"},
    {"PULSE","PULSE","PULSO","PULS","IMPULSO"},
    {"SAW + PW","SAW + PW","SIERRA + PW","SÄGE + PW","SEGA + PW"},
    {"SINUS","SINE","SENO","SINUS","SENO"},
    {"CARRÉ","SQUARE","CUADRADA","RECHTECK","QUADRA"},
    {"S & H","S & H","S & H","S & H","S & H"},
    {"VITESSE","RATE","VELOCIDAD","RATE","VELOCITÀ"},
    {"PROFONDEUR","DEPTH","PROFUNDIDAD","TIEFE","PROFONDITÀ"},
    {"HAUTEUR","PITCH","TONO","TONHÖHE","ALTEZZA"},
    {"RETARD","DELAY","RETARDO","VERZÖGERUNG","RITARDO"},
    {"LFO -> PWM 1","LFO -> PWM 1","LFO -> PWM 1","LFO -> PWM 1","LFO -> PWM 1"},
    {"LFO -> PWM 2","LFO -> PWM 2","LFO -> PWM 2","LFO -> PWM 2","LFO -> PWM 2"},
    {"ENV. 2 -> OSC. 2","ENV. 2 -> OSC. 2","ENV. 2 -> OSC. 2","HÜLLK. 2 -> OSC. 2","INV. 2 -> OSC. 2"},
    {"VIBRATO MOLETTE","WHEEL VIBRATO","VIBRATO RUEDA","RAD-VIBRATO","VIBRATO ROTELLA"},
    {"MOLETTE -> VCF","WHEEL -> VCF","RUEDA -> VCF","RAD -> VCF","ROTELLA -> VCF"},
    {"PRESSION -> VCF","PRESSURE -> VCF","PRESIÓN -> VCF","DRUCK -> VCF","PRESSIONE -> VCF"},
    {"POLY","POLY","POLI","POLY","POLI"},
    {"MONO / NOTE BASSE","MONO / LOW NOTE","MONO / NOTA BAJA","MONO / TIEFER TON","MONO / NOTA BASSA"},
    {"PORTAMENTO","PORTAMENTO","PORTAMENTO","PORTAMENTO","PORTAMENTO"},
    {"TRANSPOSITION","TRANSPOSE","TRANSPOSICIÓN","TRANSPONIEREN","TRASPOSIZIONE"},
    {"ACCORD GÉNÉRAL","MASTER TUNE","AFINACIÓN GENERAL","GESAMTSTIMMUNG","ACCORDATURA GENERALE"},
    {"PLAGE PITCH","BEND RANGE","RANGO DE PITCH","BEND-BEREICH","INTERVALLO BEND"},
    {"OFF","OFF","APAGADO","AUS","SPENTO"},
    {"ON","ON","ACTIVADO","EIN","ACCESO"},
    {"LARGEUR","WIDTH","ANCHURA","BREITE","LARGHEZZA"},
    {"MÉLANGE","MIX","MEZCLA","MISCHUNG","MIX"},
    {"ÂGE VINTAGE","VINTAGE AGE","EDAD VINTAGE","VINTAGE-ALTER","ETÀ VINTAGE"},
    {"ÉCART VOIX","VOICE SPREAD","APERTURA VOCES","STIMMENBREITE","APERTURA VOCI"},
    {"BIBLIOTHÈQUE DE SONS","SOUND LIBRARY","BIBLIOTECA DE SONIDOS","SOUNDBIBLIOTHEK","LIBRERIA SUONI"},
    {"ECHO 86  /  100 PRESETS D'USINE  /  FAVORIS  /  CRÉATIONS PERSONNELLES","ECHO 86  /  100 FACTORY PRESETS  /  FAVORITES  /  USER SOUNDS","ECHO 86  /  100 PRESETS DE FÁBRICA  /  FAVORITOS  /  SONIDOS PROPIOS","ECHO 86  /  100 WERKSPRESETS  /  FAVORITEN  /  EIGENE SOUNDS","ECHO 86  /  100 PRESET DI FABBRICA  /  PREFERITI  /  SUONI UTENTE"},
    {"INSTRUMENTS","INSTRUMENTS","INSTRUMENTOS","INSTRUMENTE","STRUMENTI"},
    {"APERÇU DU PRESET","PRESET DETAILS","DETALLES DEL PRESET","PRESET-DETAILS","DETTAGLI PRESET"},
    {"CATÉGORIE : ","CATEGORY: ","CATEGORÍA: ","KATEGORIE: ","CATEGORIA: "},
    {"SOURCE : ","SOURCE: ","ORIGEN: ","QUELLE: ","ORIGINE: "},
    {"FAVORI : ","FAVORITE: ","FAVORITO: ","FAVORIT: ","PREFERITO: "},
    {"NOM DE TON PRESET","PRESET NAME","NOMBRE DEL PRESET","PRESET-NAME","NOME PRESET"},
    {" son(s) affiché(s)"," sound(s) shown"," sonido(s) mostrados"," Sounds angezeigt"," suoni visualizzati"},
    {"USINE","FACTORY","FÁBRICA","WERK","FABBRICA"},
    {"FAVORIS","FAVORITES","FAVORITOS","FAVORITEN","PREFERITI"},
    {"UTILISATEUR","USER","USUARIO","BENUTZER","UTENTE"},
    {"TOUS","ALL","TODOS","ALLE","TUTTI"},
    {"BASS","BASS","BAJOS","BÄSSE","BASSI"},
    {"LEAD","LEAD","LEADS","LEADS","LEAD"},
    {"PAD","PAD","PADS","PADS","PAD"},
    {"KEYS","KEYS","TECLADOS","KEYS","TASTIERE"},
    {"PLUCK","PLUCK","PUNTEADOS","PLUCKS","PLUCK"},
    {"ARP","ARP","ARPEGIOS","ARPS","ARPEGGI"},
    {"STRINGS","STRINGS","CUERDAS","STREICHER","ARCHI"},
    {"FX","FX","FX","FX","FX"},
    {"CHARGER","LOAD","CARGAR","LADEN","CARICA"},
    {"FAVORI","FAVORITE","FAVORITO","FAVORIT","PREFERITO"},
    {"ENREGISTRER","SAVE","GUARDAR","SPEICHERN","SALVA"},
    {"IMPORTER","IMPORT","IMPORTAR","IMPORTIEREN","IMPORTA"},
    {"EXPORTER","EXPORT","EXPORTAR","EXPORTIEREN","ESPORTA"},
    {"FERMER","CLOSE","CERRAR","SCHLIESSEN","CHIUDI"},
    {"Mon preset","My preset","Mi preset","Mein Preset","Il mio preset"},
    {"Rechercher un son, un instrument, un tag...","Search sounds, instruments, tags...","Buscar sonidos, instrumentos, etiquetas...","Sounds, Instrumente, Tags suchen...","Cerca suoni, strumenti, tag..."},
    {"Choisir un son","Choose a sound","Elige un sonido","Sound auswählen","Scegli un suono"},
    {"OUI","YES","SÍ","JA","SÌ"},
    {"NON","NO","NO","NEIN","NO"},
    {"Double-clic pour charger.\nEnregistre tes sons dans la bibliothèque UTILISATEUR.\nLes favoris sont conservés.","Double-click to load.\nSave sounds in your USER library.\nFavorites are remembered.","Doble clic para cargar.\nGuarda sonidos en USUARIO.\nLos favoritos se conservan.","Doppelklick zum Laden.\nSounds unter BENUTZER speichern.\nFavoriten bleiben erhalten.","Doppio clic per caricare.\nSalva i suoni in UTENTE.\nI preferiti sono conservati."},
    {"Indique un nom pour le preset.","Enter a preset name.","Introduce un nombre para el preset.","Preset-Namen eingeben.","Inserisci un nome per il preset."},
    {"Impossible d'enregistrer ce preset.","Could not save the preset.","No se pudo guardar el preset.","Preset konnte nicht gespeichert werden.","Impossibile salvare il preset."},
    {"Ce fichier de preset est invalide ou incompatible.","Invalid or incompatible preset file.","Archivo de preset inválido o incompatible.","Ungültige oder inkompatible Preset-Datei.","File preset non valido o incompatibile."},
    {"Impossible d'enregistrer les favoris.","Could not save favorites.","No se pudieron guardar los favoritos.","Favoriten konnten nicht gespeichert werden.","Impossibile salvare i preferiti."},
    {"Importer un preset ECHO 86","Import ECHO 86 preset","Importar preset de ECHO 86","ECHO 86 Preset importieren","Importa preset ECHO 86"},
    {"Fichier trop volumineux ou vide.","File is too large or empty.","Archivo demasiado grande o vacío.","Datei ist zu groß oder leer.","File troppo grande o vuoto."},
    {"Ce fichier n'est pas un preset ECHO 86.","This is not an ECHO 86 preset.","Este archivo no es un preset ECHO 86.","Keine ECHO 86 Preset-Datei.","Questo non è un preset ECHO 86."},
    {"Paramètres invalides dans le preset.","Invalid preset parameters.","Parámetros de preset inválidos.","Ungültige Preset-Parameter.","Parametri del preset non validi."},
    {"Impossible de copier ce preset.","Could not copy the preset.","No se pudo copiar el preset.","Preset konnte nicht kopiert werden.","Impossibile copiare il preset."},
    {"Exporter un son ECHO 86","Export ECHO 86 sound","Exportar sonido ECHO 86","ECHO 86 Sound exportieren","Esporta suono ECHO 86"},
    {"Importer un son ECHO 86","Import ECHO 86 sound","Importar sonido ECHO 86","ECHO 86 Sound importieren","Importa suono ECHO 86"},
    {"Impossible d'exporter le préréglage","Could not export the preset","No se pudo exportar el preset","Preset konnte nicht exportiert werden","Impossibile esportare il preset"},
};
inline constexpr int count = 5;
inline const char* code(Language l) noexcept {
    switch(l) { case Language::en:return "EN";case Language::es:return "ES";
      case Language::de:return "DE";case Language::it:return "IT";default:return "FR"; }
}
inline Language parse(const char* c) noexcept {
    if(c==nullptr)return Language::fr;
    if(std::strcmp(c,"EN")==0)return Language::en;
    if(std::strcmp(c,"ES")==0)return Language::es;
    if(std::strcmp(c,"DE")==0)return Language::de;
    if(std::strcmp(c,"IT")==0)return Language::it;
    return Language::fr;
}
inline const char* translate(Language l,const char* original) noexcept {
    if(original==nullptr)return "";
    if(l==Language::fr)return original;
    for(const auto& entry:labels){
        if(std::strcmp(entry.source,original)==0) {
            switch(l) {
                case Language::en:return entry.en;
                case Language::es:return entry.es;
                case Language::de:return entry.de;
                case Language::it:return entry.it;
                default:return original;
            }
        }
    }
    return original;
}
}} // namespace echo86::locale
