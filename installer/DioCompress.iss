[Setup]
AppId={{7D1F5C2A-3B6E-4C8D-9A41-5E2B7C9D1A33}
AppName=DioCompress
AppVersion=1.0.0
AppPublisher=DioOnTheBeat
DefaultDirName={commonpf64}\DioCompress
DisableDirPage=yes
DefaultGroupName=DioCompress
OutputDir=Output
OutputBaseFilename=DioCompress_Setup
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern

[Files]
Source: "..\build\DioCompress_artefacts\Release\VST3\DioCompress.vst3\*"; DestDir: "{commoncf64}\VST3\DioCompress.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\build\DioCompress_artefacts\Release\Standalone\DioCompress.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\DioCompress (Standalone)"; Filename: "{app}\DioCompress.exe"

[Messages]
FinishedLabel=DioCompress se instalo correctamente. Abre tu DAW y reescanea los plugins VST3.
