; Inno Setup Script for EVE Screen Editor tool

#define MyAppName "EVE Screen Editor 3.0"
#define MyAppVersion "V3.0.0"
#define MyAppPublisher "Bridgetek Pte Ltd"
#define MyAppURL "http://brtchip.com/eve/"
#define MyAppExeName "fteditor.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{B800403A-63B9-43C0-A3CA-E0555DD8CF76}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={sd}\Users\Public\Documents\{#MyAppName}
DefaultGroupName={#MyAppName}
;InfoBeforeFile=.\\prerelease.txt
OutputBaseFilename=EVE Screen Editor {#MyAppVersion}
SetupIconFile=.\\eve_editor.ico
Compression=lzma
SolidCompression=yes
CreateAppDir=yes
DisableDirPage=no
EnableDirDoesntExistWarning=True
ChangesAssociations=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: ".\\*.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\*.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\Manual\*.*"; DestDir: "{app}\Manual"; Flags: recursesubdirs createallsubdirs

Source: ".\\Lib\*.*"; DestDir: "{app}\Lib"; Flags: recursesubdirs createallsubdirs
Source: ".\\Examples\*.*"; DestDir: "{app}\Examples"; Flags: recursesubdirs createallsubdirs
Source: ".\\export_scripts\*.*"; DestDir: "{app}\export_scripts"; Flags: recursesubdirs createallsubdirs
Source: ".\\untitled\*.*"; DestDir: "{app}\untitled"; Flags: recursesubdirs createallsubdirs
Source: ".\\EVE_Hal_Library\*.*"; DestDir: "{app}\EVE_Hal_Library"; Flags: recursesubdirs createallsubdirs
Source: ".\\platforms\*.*"; DestDir: "{app}\platforms"; Flags: recursesubdirs createallsubdirs
Source: ".\\imageformats\*.*"; DestDir: "{app}\imageformats"; Flags: recursesubdirs createallsubdirs
Source: ".\\firmware\*.*"; DestDir: "{app}\firmware"; Flags: recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[UninstallDelete]
Type: files; Name: "{app}\*.pyc"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent runascurrentuser

[Registry]

Root: HKCR; Subkey: ".ese";                             ValueData: "{#MyAppName}";          Flags: uninsdeletevalue; ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "{#MyAppName}";                     ValueData: "Program {#MyAppName}";  Flags: uninsdeletekey;   ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "{#MyAppName}\DefaultIcon";         ValueData: "{app}\{#MyAppExeName},0";                        ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "{#MyAppName}\shell\open\command";  ValueData: """{app}\{#MyAppExeName}"" ""%1""";               ValueType: string;  ValueName: ""