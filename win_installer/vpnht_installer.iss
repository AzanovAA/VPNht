; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "VPNht"
#define MyAppVersion "1.13"
#define MyAppPublisher "VPNht"
#define MyAppURL "https://vpn.ht/"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{88ae5d2c-1e21-4709-b7c5-2620396fe6a7}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
;OutputDir=C:\Work\VPNht\installer
OutputBaseFilename=vpnht_1_13
SetupIconFile=vpnht.ico
UninstallDisplayIcon={app}\uninstall.ico
Compression=lzma
SolidCompression=yes
PrivilegesRequired=admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "uninstall.ico"; DestDir: "{app}"
Source: "Files\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "tap-windows-9.21.1.exe"; DestDir: "{app}"; AfterInstall: RunTapInstaller

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"; IconFilename: "{app}\uninstall.ico"
Name: "{group}\VPNht"; Filename: "{app}\VPNht.exe"; WorkingDir: "{app}"; IconFilename: "{app}\VPNht.exe"; IconIndex: 0

[Run]
Filename: "sc"; Parameters: "create VPNhtOpenVPNService binPath=""{app}\VPNhtOpenVPNService.exe"""
Filename: "{app}\subinacl"; Parameters: "/SERVICE VPNhtOpenVPNService /grant={username}=F"

[UninstallRun]
Filename: "sc"; Parameters: "stop VPNhtOpenVPNService"
Filename: "timeout"; Parameters: "5"
Filename: "sc"; Parameters: "delete VPNhtOpenVPNService"

[Code]
procedure RunTapInstaller;
var
  ResultCode: Integer;
begin
  if not Exec(ExpandConstant('{app}\tap-windows-9.21.1.exe'), '', '', SW_SHOWNORMAL,
    ewWaitUntilTerminated, ResultCode)
  then
    MsgBox('Tap drivers installer failed to run!' + #13#10 +
      SysErrorMessage(ResultCode), mbError, MB_OK);
end;