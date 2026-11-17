# LIBERTY RELEASE CENTER — un seul atelier de distribution

Ce système est hébergé dans **BSM1973/Liberty-ECHO-86**, car c'est là que
les six secrets de signature et de notarisation Apple ont déjà fonctionné.
Il n'est donc pas nécessaire de les copier dans chaque dépôt Liberty.

## Une manipulation pour publier

1. GitHub → dépôt **Liberty-ECHO-86** → **Actions**.
2. Choisir **LIBERTY RELEASE CENTER**, puis **Run workflow**.
3. Sélectionner `pedal-legends`, `echo-86` ou `organix`.
4. Laisser le champ Version vide pour la version enregistrée.
5. Cliquer sur **Run workflow**. Les compilations macOS et Windows démarrent.

À la fin :
- `LIBERTY-<plugin>-macOS-SIGNED-NOTARIZED` contient le `.pkg` universel
  AU et VST3 **signé Developer ID**, **notarisé Apple** et avec ticket agrafé.
- `LIBERTY-<plugin>-Windows-x64-EXE-UNSIGNED` contient le programme
  d'installation VST3 Inno Setup, avec désinstallateur Windows. Le fichier
  est **non signé Authenticode** (à signer séparément pour une distribution
  professionnelle sans avertissement SmartScreen).

Une notarisation Apple rejetée fait échouer le travail ; aucun paquet ne
porte le nom « SIGNED-NOTARIZED » sans réception du statut **Accepted** et
validation des commandes `stapler`, `spctl` et `pkgutil`.

## Préenregistrés

- PEDAL LEGENDS → dépôt `BSM1973/Liberty-Pedal-Legends`, version 0.6.1,
  80 pédales et 400 presets, AU/VST3 Mac et VST3 Windows.
- ECHO 86 → dépôt `BSM1973/Liberty-ECHO-86`, version 0.1.0,
  AU/VST3 Mac et VST3 + Standalone Windows.
- ORGANIX → dépôt `BSM1973/Liberty-Organix`, version 1.3.5,
  AU/VST3 macOS universel (Apple Silicon + Intel), MIDI Learn. Le Release Center
  compile et notarie ORGANIX **avec les secrets déjà fonctionnels d'ECHO 86**.
  Pour ORGANIX, le job Windows est désactivé jusqu'à validation d'un
  installateur Windows natif.

## Les 20 prochains plugins

Ajouter une entrée dans `ReleaseCenter/catalog.json`, avec les champs
`repository`, `product_name`, `cmake_source`, `target`, `version`,
`juce_checkout` et `standalone`, puis ajouter son identifiant à la liste
`options` de l'input `plugin` dans le workflow. **Pas de certificats
supplémentaires ni de secrets par dépôt.**

La compilation nécessite actuellement des dépôts publics GitHub
`BSM1973/*`, sous `main` et basés sur JUCE + CMake. D'autres structures
de plugin nécessiteront un profil de build spécifique.

## Sécurité et limites

Les certificats et le mot de passe Apple restent dans les secrets du dépôt
ECHO 86 et ne doivent jamais être publiés ou envoyés par message.
La compilation des sources externes est faite **sans secrets Apple**, puis
les binaires compilés sont transférés dans un job macOS distinct qui signe
et notarie. Ne jamais utiliser ce système pour signer du code non fiable.

Les packages restent à **installer et tester dans Ableton Live, Logic et
autres DAW** avant livraison commerciale. Le packaging ne valide pas, à lui
seul, le comportement audio du plugin ou toutes les configurations système.

## ORGANIX : automatisation sans recopie des secrets

Lors de l'intégration initiale d'ORGANIX, une modification vérifiée du catalogue
sur `main` déclenche directement le Release Center pour **ORGANIX**.
Le build, la signature, la soumission Apple et le `.pkg` sont réalisés
dans la même chaîne, sans configurer de secrets dans le dépôt ORGANIX.
Le succès est conditionné à `notarytool: Accepted`, `stapler`, Gatekeeper
et vérification des signatures. Si une étape échoue, il n'y a pas d'Artifact
commercial `SIGNED-NOTARIZED`.

Pour les versions suivantes, l'entrée `organix` est sélectionnable dans
le même menu central. Aucun fichier de certificats ou mot de passe ne doit
être ajouté au dépôt ni envoyé dans la conversation.
