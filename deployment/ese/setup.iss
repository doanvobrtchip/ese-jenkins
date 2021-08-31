; Inno Setup Script for EVE Screen Editor tool

#define MyAppName "EVE Screen Editor"
#define MyAppVersion "v4.2"
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
OutputBaseFilename=EVE Screen Editor {#MyAppVersion}-rc2


SetupIconFile=.\\eve-puzzle-64.ico
Compression=lzma
SolidCompression=yes

CreateAppDir=yes
DisableDirPage=no
EnableDirDoesntExistWarning=True

ChangesAssociations=yes
WizardStyle=modern
ArchitecturesAllowed=x64
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64


[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1


[Files]
Source: ".\\*.txt"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\astc_conv.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\aud_cvt.py"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\export_pico.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\export_pico_helper.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\export_pico_reg.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\export_template\*.*"; DestDir: "{app}\export_template"; Flags: recursesubdirs createallsubdirs

Source: ".\\export_common.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\export_bt81x.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\export_bt81x_helper.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\export_EVE_Arduino.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\export_ftdi_eve_hal2.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\export_GameDuino2.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\helperapi.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\img_cvt.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\png.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\pngp2pa.py"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\raw_cvt.py"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\config.json"; DestDir: "{app}"; Flags: ignoreversion


Source: ".\\astcenc.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\pngquant.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\fteditor.exe"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\bt8xxemu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\mx25lemu.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\freetype.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\ftd2xx.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\libFT4222-64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\libMPSSE.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\python38.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\zlib1__.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\Qt5Svg.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\msvcp100.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\msvcr100.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\ucrtbase.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\concrt140.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\eve_hal.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\\libpng16-16__.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\\Lib\*.*"; Excludes: "*.pyc"; DestDir: "{app}\Lib"; Flags: recursesubdirs createallsubdirs
Source: ".\\Examples\*.*"; DestDir: "{app}\Examples"; Flags: recursesubdirs createallsubdirs
Source: ".\\EVE_Hal_Library\*.*"; DestDir: "{app}\EVE_Hal_Library"; Flags: recursesubdirs createallsubdirs
Source: ".\\export_scripts\*.*"; DestDir: "{app}\export_scripts"; Flags: recursesubdirs createallsubdirs
Source: ".\\firmware\*.*"; DestDir: "{app}\firmware"; Flags: recursesubdirs createallsubdirs
Source: ".\\Manual\*.*"; DestDir: "{app}\Manual"; Flags: recursesubdirs createallsubdirs
Source: ".\\untitled\*.*"; DestDir: "{app}\untitled"; Flags: recursesubdirs createallsubdirs
Source: ".\\platforms\*.*"; DestDir: "{app}\platforms"; Flags: recursesubdirs createallsubdirs
Source: ".\\imageformats\*.*"; DestDir: "{app}\imageformats"; Flags: recursesubdirs createallsubdirs
Source: ".\\styles\*.*"; DestDir: "{app}\styles"; Flags: recursesubdirs createallsubdirs
Source: ".\\iconengines\*.*"; DestDir: "{app}\iconengines"; Flags: recursesubdirs createallsubdirs

Source: ".\\device_sync\*.*"; DestDir: "{app}\device_sync"; Flags: recursesubdirs createallsubdirs

Source: "cpu_detect.dll"; Flags: dontcopy

Source: "astcenc-sse2.exe";   DestDir: "{app}"; DestName: "astcenc.exe"; Check:  cpu_detect_0
Source: "astcenc-sse4.2.exe"; DestDir: "{app}"; DestName: "astcenc.exe"; Check:  cpu_detect_1
Source: "astcenc-avx2.exe";   DestDir: "{app}"; DestName: "astcenc.exe"; Check:  cpu_detect_2

[Code]


function cpu_detect(): Integer;
external 'cpu_detect@files:cpu_detect.dll cdecl setuponly';

function cpu_detect_0(): Boolean;
begin
  Result := cpu_detect() = 0;
end;

function cpu_detect_1(): Boolean;
begin
  Result := cpu_detect() = 1;
end;

function cpu_detect_2(): Boolean;
begin
  Result := cpu_detect() = 2;
end;

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon


[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent runascurrentuser



[UninstallDelete]
Type: files; Name: "{app}\recent_project"
Type: files; Name: "{app}\*.pyc"
Type: filesandordirs; Name: "{app}\__pycache__"
Type: filesandordirs; Name: "{app}\Examples"
Type: filesandordirs; Name: "{app}\Lib"
Type: filesandordirs; Name: "{app}\device_sync"

[Registry]
Root: HKCR; Subkey: ".ese";                             ValueData: "{#MyAppName}";          Flags: uninsdeletevalue; ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "{#MyAppName}";                     ValueData: "Program {#MyAppName}";  Flags: uninsdeletekey;   ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "{#MyAppName}\DefaultIcon";         ValueData: "{app}\{#MyAppExeName},0";                        ValueType: string;  ValueName: ""
Root: HKCR; Subkey: "{#MyAppName}\shell\open\command";  ValueData: """{app}\{#MyAppExeName}"" ""%1""";               ValueType: string;  ValueName: ""

