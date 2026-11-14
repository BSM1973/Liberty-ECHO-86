#define MyAppName "ECHO 86"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "Liberty Music Software"
#ifndef BuildRoot
  #error BuildRoot must point to a staging folder containing VST3 and Standalone.
#endif

[Setup]
AppId={{A0B4E860-86BB-4E86-8E86-BA7C48600086}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\Liberty Music Software\ECHO 86
DefaultGroupName=Liberty Music Software
DisableProgramGroupPage=yes
LicenseFile=
OutputDir=..\..\installer-output
OutputBaseFilename=ECHO86-Windows-x64-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayName=ECHO 86 (VST3 + Standalone)
SetupLogging=yes

[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full"; Description: "VST3 + Standalone"
Name: "compact"; Description: "VST3 seulement"
Name: "custom"; Description: "Personnalisé"; Flags: iscustom

[Components]
Name: "vst3"; Description: "Plugin VST3 64 bits"; Types: full compact custom; Flags: fixed
Name: "standalone"; Description: "Application autonome (Standalone)"; Types: full

[Files]
Source: "{#BuildRoot}\VST3\ECHO 86.vst3\*"; DestDir: "{commoncf64}\VST3\ECHO 86.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: vst3
Source: "{#BuildRoot}\Standalone\ECHO 86.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: standalone

[Icons]
Name: "{group}\ECHO 86"; Filename: "{app}\ECHO 86.exe"; Components: standalone
Name: "{group}\Désinstaller ECHO 86"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\ECHO 86.exe"; Description: "Lancer ECHO 86"; Flags: nowait postinstall skipifsilent; Components: standalone

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
end;
