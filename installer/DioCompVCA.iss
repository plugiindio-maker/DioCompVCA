[Setup]
AppName=DioCompVCA
AppVersion=1.0.0
AppPublisher=DioOnTheBeat
DefaultDirName={commonpf64}\DioCompVCA
DisableDirPage=yes
DefaultGroupName=DioCompVCA
OutputDir=Output
OutputBaseFilename=DioCompVCA_Setup
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern

[Files]
Source: "..\build\DioCompVCA_artefacts\Release\VST3\DioCompVCA.vst3\*"; DestDir: "{commoncf64}\VST3\DioCompVCA.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\DioCompVCA_artefacts\Release\Standalone\DioCompVCA.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\DioCompVCA (Standalone)"; Filename: "{app}\DioCompVCA.exe"

[Messages]
FinishedLabel=DioCompVCA se instalo correctamente. Abre tu DAW y reescanea los plugins VST3.
