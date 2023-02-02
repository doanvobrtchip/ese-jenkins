; Inno Setup Script for EVE Screen Editor tool

#define RootDir = "..\.."
#define BuildDir = (RootDir)+ "\build\bin\Release"
#define DeploymentFileDir = (RootDir) + "\fteditor\deployment"

#define MyAppName "EVE Screen Editor"
#define MyAppVersion "v4.5.0_Beta"
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
InfoBeforeFile=.\Release Notes.txt
OutputBaseFilename=EVE Screen Editor {#MyAppVersion}


SetupIconFile={#RootDir}\fteditor\icons\eve-puzzle-64.ico
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
Source: ".\Release Notes.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\Manual\*.pdf"; DestDir: "{app}\Manual"; Flags: recursesubdirs createallsubdirs ignoreversion

Source: "{#RootDir}\fteditor\device_sync\*.*"; DestDir: "{app}\device_sync"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "{#RootDir}\fteditor\EVE_Hal_Library\*.*"; DestDir: "{app}\EVE_Hal_Library"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "{#RootDir}\fteditor\Examples\*.*"; DestDir: "{app}\Examples"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "{#RootDir}\fteditor\export_scripts\*.*"; DestDir: "{app}\export_scripts"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "{#RootDir}\fteditor\export_template\*.*"; Excludes: "*.gitkeep"; DestDir: "{app}\export_template"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "{#RootDir}\fteditor\firmware\*.*"; DestDir: "{app}\firmware"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "{#RootDir}\fteditor\untitled\*.*"; DestDir: "{app}\untitled"; Flags: recursesubdirs createallsubdirs ignoreversion

;Python
Source: "{#RootDir}\fteditor\astc_conv.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\aud_cvt.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\export_pico.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\export_pico_helper.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\export_pico_reg.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\export_common.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\export_bt81x.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\export_bt81x_helper.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\export_EVE_Arduino.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\export_ftdi_eve_hal2.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\export_GameDuino2.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\helperapi.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\img_cvt.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\png.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\pngp2pa.py"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RootDir}\fteditor\raw_cvt.py"; DestDir: "{app}"; Flags: ignoreversion

Source: "{#BuildDir}\fteditor.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\bt8xxemu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\mx25lemu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\ftd2xx.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\libFT4222-64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\libMPSSE.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\eve_hal.dll"; DestDir: "{app}"; Flags: ignoreversion

;Qt
Source: "{#BuildDir}\qt-deployment\*.*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

Source: "{#DeploymentFileDir}\pngquant.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#DeploymentFileDir}\config.json"; DestDir: "{app}"; Flags: ignoreversion
;PNG
Source: "{#DeploymentFileDir}\libpng16.dll"; DestDir: "{app}"; Flags: ignoreversion
;FreeType
Source: "{#DeploymentFileDir}\freetype.dll"; DestDir: "{app}"; Flags: ignoreversion
;PIL
Source: "{#DeploymentFileDir}\Python\Python310\Lib\site-packages\PIL\concrt140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#DeploymentFileDir}\Python\Python310\Lib\site-packages\PIL\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#DeploymentFileDir}\Python\Python310\Lib\site-packages\PIL\msvcp140_1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#DeploymentFileDir}\Python\Python310\Lib\site-packages\PIL\msvcp140_2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#DeploymentFileDir}\Python\Python310\Lib\site-packages\PIL\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#DeploymentFileDir}\Python\Python310\Lib\site-packages\PIL\vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion
;ASTC
Source: "{#DeploymentFileDir}\astcenc-sse2.exe";   DestDir: "{app}"; Flags: ignoreversion 
Source: "{#DeploymentFileDir}\astcenc-sse4.2.exe"; DestDir: "{app}"; Flags: ignoreversion 
Source: "{#DeploymentFileDir}\astcenc-avx2.exe";   DestDir: "{app}"; Flags: ignoreversion 
Source: "{#DeploymentFileDir}\install_astc.exe"; DestDir: "{app}"; Flags: ignoreversion deleteafterinstall; AfterInstall: RunASTCInstaller
;Python Version 3.10
Source: "{#DeploymentFileDir}\Python\Python310\python310.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#DeploymentFileDir}\Python\Python310\Lib\*.*"; Excludes: "*.pyc"; DestDir: "{app}\Lib"; Flags: recursesubdirs createallsubdirs ignoreversion

[Code]
procedure RunASTCInstaller;
var
  ResultCode: Integer;
begin
  if not Exec(ExpandConstant('{app}\install_astc.exe'), '', '', SW_SHOWNORMAL,
    ewWaitUntilTerminated, ResultCode)
  then
    MsgBox('Installing ASTC failed to run!' + #13#10 +
      SysErrorMessage(ResultCode), mbError, MB_OK);
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
; this registry key saves last project dir path
Root: HKCU; Subkey: "Software\BridgeTek\ESE"; Flags: uninsdeletekey

