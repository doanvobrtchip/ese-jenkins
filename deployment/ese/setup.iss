; Inno Setup Script for EVE Screen Editor tool

#define MyAppName "EVE Screen Editor"
#define MyAppVersion "V3.0.0 RC1"
#define MyAppPublisher "BridgeTek Pte Ltd"
#define MyAppURL "http://brtchip.com/utilities/#evescreeneditor"
#define MyAppExeName "fteditor.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{7643B3D8-319C-4FBE-97EA-B452EE3C3F03}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={sd}\Users\Public\Documents\{#MyAppName}
DefaultGroupName={#MyAppName}
InfoBeforeFile=.\\prerelease.txt
OutputBaseFilename=EVE Screen Editor {#MyAppVersion}
;SetupIconFile=.\\eve_editor.ico
Compression=lzma
SolidCompression=yes



CreateAppDir=yes
DisableDirPage=no
EnableDirDoesntExistWarning=True
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1



[Files]
Source: ".\\prerelease.txt"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\fteditor.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\pngquant.exe"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\*.py"; DestDir: "{app}"; Flags: ignoreversion


Source: ".\\EVE Screen Editor User Guide.pdf"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\setup.iss"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\Lib\*.*"; DestDir: "{app}\Lib"; Flags: recursesubdirs createallsubdirs
Source: ".\\Examples\*.*"; DestDir: "{app}\Examples"; Flags: recursesubdirs createallsubdirs
Source: ".\\export_scripts\*.*"; DestDir: "{app}\export_scripts"; Flags: recursesubdirs createallsubdirs
Source: ".\\untitled\*.*"; DestDir: "{app}\untitled"; Flags: recursesubdirs createallsubdirs
Source: ".\\platforms\*.*"; DestDir: "{app}\platforms"; Flags: recursesubdirs createallsubdirs
Source: ".\\imageformats\*.*"; DestDir: "{app}\imageformats"; Flags: recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon


[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent runascurrentuser

