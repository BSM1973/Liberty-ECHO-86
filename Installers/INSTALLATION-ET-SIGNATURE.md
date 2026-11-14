# ECHO 86 — installateurs et signature Apple Developer

Les installateurs sont construits automatiquement sur GitHub Actions, sans modifier le synthétiseur.

## Windows x64

Workflow : **ECHO 86 Windows VST3 x64**.

Télécharger `ECHO86-Windows-x64-Installer-EXE`, extraire `ECHO86-Windows-x64-Setup.exe`, puis double-cliquer.

L'assistant Inno Setup installe automatiquement :
- VST3 dans `C:\Program Files\Common Files\VST3\ECHO 86.vst3`
- Application autonome dans `C:\Program Files\Liberty Music Software\ECHO 86\`
- Raccourci dans le menu Démarrer et désinstallation via Windows.

**L'installateur Windows n'est pas encore signé Authenticode** : Apple Developer ne signe pas les applications Windows. Une signature Microsoft/Authenticode devra être ajoutée séparément avant distribution commerciale.

## macOS Apple Silicon

Workflow : **ECHO 86 macOS AU VST3**.

L'installateur `.pkg` place :
- AU dans `/Library/Audio/Plug-Ins/Components/ECHO 86.component`
- VST3 dans `/Library/Audio/Plug-Ins/VST3/ECHO 86.vst3`
- Standalone dans `/Applications/ECHO 86.app`.

L'installation nécessite les droits administrateur macOS.

**Sans certificats**, le workflow fournit `ECHO86-macOS-Installer-TEST-unsigned`, à réserver aux tests. Avec certificats Developer ID et clés de notarisation configurés, il fournit `ECHO86-macOS-Installer-SIGNED-NOTARIZED`.

## Configurer la signature Apple une seule fois

Cette procédure se fait directement dans ton dépôt GitHub. **Ne communique jamais de certificats, mots de passe, fichiers .p8 ou clés privées dans une discussion ni dans un commit GitHub.**

1. Sur ton Mac, dans **Trousseaux d'accès**, vérifie que tu possèdes les deux identités, chacune avec sa clé privée : **Developer ID Application** et **Developer ID Installer**. Ton compte Apple Developer peut déjà posséder la première sans posséder la seconde.
2. Exporte chaque identité depuis le Trousseau au format `.p12`. Pour simplifier l'automatisation, choisis le **même mot de passe d'export** pour les deux fichiers. Conserve-les dans un emplacement privé, hors de ton dépôt GitHub.
3. Pour copier chaque fichier encodé dans le presse-papier macOS, adapte le chemin dans la commande `base64 -i "/chemin/vers/MonCertificat.p12" | tr -d '\n' | pbcopy`.
4. Dans GitHub → dépôt `BSM1973/Liberty-ECHO-86` → **Settings → Secrets and variables → Actions → New repository secret**, ajoute les trois secrets :

   - `APPLE_DEVELOPER_ID_APPLICATION_P12_BASE64` : contenu base64 du certificat d'application
   - `APPLE_DEVELOPER_ID_INSTALLER_P12_BASE64` : contenu base64 du certificat d'installateur
   - `APPLE_P12_PASSWORD` : mot de passe d'export commun aux deux `.p12`

5. Pour la **notarisation Apple**, crée ou utilise une clé API App Store Connect autorisée à soumettre pour notarisation (fichier `.p8`, Key ID et Issuer ID). Ajoute aussi :

   - `APPLE_NOTARY_API_KEY_P8_BASE64` : contenu base64 du fichier `.p8`
   - `APPLE_NOTARY_KEY_ID` : Key ID de cette clé
   - `APPLE_NOTARY_ISSUER_ID` : Issuer ID App Store Connect

6. Dans **Actions**, ouvre `ECHO 86 macOS AU VST3`, puis **Run workflow** sur la branche `main`. Le pipeline importe les certificats dans un trousseau éphémère, signe AU/VST3/Standalone avec **Developer ID Application + Hardened Runtime**, signe le `.pkg` avec **Developer ID Installer**, soumet le paquet à Apple, puis ajoute et valide le ticket de notarisation.

Si la notarisation est rejetée, le build échoue explicitement plutôt que de publier un installateur prétendument notarisé. Si les trois secrets de certificat ne sont pas encore présents, GitHub produit seulement l'installateur de test non signé.

**Important :** un certificat `Apple Development`, `Mac App Distribution` ou une signature ad hoc ne remplace pas `Developer ID Application` pour la distribution en dehors du Mac App Store.

## Vérifications finales

- Vérifier que le `.pkg` est signé et notarisé avant publication commerciale sur Liberty Music Software.
- Sur un Mac non-développeur, installer puis ouvrir le plugin dans Ableton Live, Logic Pro, etc.
- Sur Windows, exécuter le Setup, ouvrir ECHO 86 dans un DAW VST3, contrôler presets/GUI/MIDI et désinstaller.
- Si des anciens plugins AU existaient dans `~/Library/Audio/Plug-Ins/Components`, les retirer manuellement après vérification : le nouvel installateur utilise le dossier système `/Library`, pour tous les utilisateurs.

Documentation Apple :
- https://developer.apple.com/help/account/certificates/create-developer-id-certificates
- https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution
