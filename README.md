# Liberty ECHO 86

Synthétiseur logiciel original de Liberty Music Software.

## État du projet

- 16 voix polyphoniques dans le moteur C++.
- 100 presets d'usine et navigateur par catégories.
- Chorus, delay et réverbération stéréo.
- Import et export de presets personnels.
- Tests DSP C++ réussis localement.
- Compilation JUCE macOS AU/VST3/Standalone non encore validée.

## Importer les sources

Télécharger l'archive ECHO86_JUCE_Validation_CI_v51.zip depuis la conversation de développement, la décompresser, puis ajouter le dossier `JUCE/` à la racine de ce dépôt.

## Compiler

Ouvrir l'onglet Actions, choisir « ECHO 86 macOS AU VST3 », puis « Run workflow » après l'ajout des sources.

Les binaires générés sont non signés. Ne pas distribuer commercialement avant tests DAW, signature Developer ID et notarisation.
