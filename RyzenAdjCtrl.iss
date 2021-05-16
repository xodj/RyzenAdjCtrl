[Setup] 
AppId={{D1E0507C-61B0-4BC3-9FE4-23088D68151F}
AppName=RyzenAdjCtrl
AppVersion=0.3.3.487
AppPublisher=xo.dj@ya.ru
WizardStyle=modern
DefaultDirName={autopf}\RyzenAdjCtrl
DefaultGroupName=RyzenAdjCtrl
UninstallDisplayIcon={app}\RyzenAdjCtrl.exe
Compression=lzma2
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
DisableWelcomePage=no
LicenseFile=LICENSE
PrivilegesRequired=admin 
DisableDirPage=no
DisableProgramGroupPage=no
OutputBaseFilename=RyzenAdjCtrlSetup
VersionInfoVersion=0.3.3.487
CloseApplications=no

//[Languages]
//Name: "english"; MessagesFile: "compiler:Default.isl"; LicenseFile: "LICENSE";
//Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"; LicenseFile: "LICENSE"

[Files]
Source: "Release\RyzenAdjCtrl.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\RyzenAdjCtrl"; Filename: "{app}\RyzenAdjCtrl.exe"
Name: "{group}\Uninstall"; Filename: "{uninstallexe}"
Name: "{commondesktop}\RyzenAdjCtrl"; Filename: "{app}\RyzenAdjCtrl.exe"; Tasks: desktopicon 
Name: "{userstartup}\RyzenAdjCtrl"; Filename: "{app}\RyzenAdjCtrl.exe"; Tasks: addAutoStart

[Tasks]
Name: "addWindowsTask"; Description: "Add RyzenAdjCtrl Service to windows tasks"; GroupDescription: "Autorun:"; 
Name: "addAutoStart"; Description: "Add RyzenAdjCtrl GUI to auto start"; GroupDescription: "Autorun:"; 
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; 

[Run]
Filename: {app}\RyzenAdjCtrl.exe; Parameters: "install"; StatusMsg: RyzenAdjCtrl Service windows tasks is installing. Please wait...

[UninstallRun]  
Filename: {app}\RyzenAdjCtrl.exe; Parameters: "uninstall";

[Code]
{ ///////////////////////////////////////////////////////////////////// }
function GetUninstallString(): String;
var
  sUnInstPath: String;
  sUnInstallString: String;
begin
  sUnInstPath := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{{D1E0507C-61B0-4BC3-9FE4-23088D68151F}_is1');
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  Result := sUnInstallString;
end;


{ ///////////////////////////////////////////////////////////////////// }
function IsUpgrade(): Boolean;
begin
  Result := (GetUninstallString() <> '');
end;


{ ///////////////////////////////////////////////////////////////////// }
function UnInstallOldVersion(): Integer;
var
  sUnInstallString: String;
  iResultCode: Integer;
begin
{ Return Values: }
{ 1 - uninstall string is empty }
{ 2 - error executing the UnInstallString }
{ 3 - successfully executed the UnInstallString }

  { default return value }
  Result := 0;

  { get the uninstall string of the old app }
  sUnInstallString := GetUninstallString();
  if sUnInstallString <> '' then begin
    sUnInstallString := RemoveQuotes(sUnInstallString);
    if Exec(sUnInstallString, '/SILENT /NORESTART /SUPPRESSMSGBOXES','', SW_HIDE, ewWaitUntilTerminated, iResultCode) then
      Result := 3
    else
      Result := 2;
  end else
    Result := 1;
end;

{ ///////////////////////////////////////////////////////////////////// }
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep=ssInstall) then
  begin
    if (IsUpgrade()) then
    begin
      UnInstallOldVersion();
    end;
  end;
end;