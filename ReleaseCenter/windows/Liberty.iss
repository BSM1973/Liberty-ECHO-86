#define ProductName GetEnv("LIBERTY_PRODUCT_NAME")
#define ProductSlug GetEnv("LIBERTY_SLUG")
#define ProductVersion GetEnv("LIBERTY_VERSION")
#define Vst3Source GetEnv("LIBERTY_VST3_BUNDLE")
#define StandaloneExe GetEnv("LIBERTY_STANDALONE_EXE")
#define InstallerOutput GetEnv("LIBERTY_OUTPUT_DIR")

[Setup]
AppId=LibertyMusicSoftware.ReleaseCenter.{#ProductSlug}
AppName={#ProductName}
AppVersion={#ProductVersion}
AppPublisher=Liberty Music Software
AppPublisherURL=https://libertymusicsoftware.com
DefaultDirName={autopf}\Liberty Music Software\{#ProductName}
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
OutputDir={#InstallerOutput}
OutputBaseFilename=LIBERTY-{#ProductSlug}-Windows-x64-v{#ProductVersion}-UNSIGNED
Compression=lzma2
SolidCompression=yes
Uninstallable=yes
SetupLogging=yes
UsePreviousAppDir=no

[Files]
Source: "{#Vst3Source}\*"; DestDir: "{commoncf64}\VST3\{#ProductName}.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
#if StandaloneExe != ""
Source: "{#StandaloneExe}"; DestDir: "{app}"; Flags: ignoreversion
#endif

[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
