;   TCR compliant Sample install script.
;   Put this file in the root for what you want included in the installer,
;   then put everything into the subdirectories as listed in the installer below.
;   Created by Chris Onorati.
;
;   !!NOTE!!
;   exe of your game ==MUST== match the name of your game.  For example, a game
;   called "Gooball" must have an exe to start the game called "Gooball.exe".
;
;   !!NOTE!!
; There are three steps that you must perform in order to customize this script
;   for your project (see all "TODO" comments below):
;   1) Generate a new GUID for your game (the AppId).
;   2) Replace YOUR_GAME_NAME_HERE on line 19 with your game name.
;   3) Replace 0.0.0 on line 20 with the current version number of your game.
;   4) Update the redistributables required for your game.
[Setup]

#define ApplicationName 'Chamomile Grove'
#define ApplicationVersion '0.7.11'

DisableWelcomePage=no
WizardImageFile=INSTALLERFILES\panel_image_100.bmp
WizardSmallImageFile=INSTALLERFILES\banner.bmp

AppId={{EBD3FDEB-26C3-417D-B9FB-F00C6112537F}

; Standard app data stuff
AppName={#ApplicationName}
AppVerName={#ApplicationName}
VersionInfoVersion = {#ApplicationVersion}
AppVersion = {#ApplicationVersion}
AppPublisher=DigiPen Institute of Technology
AppPublisherURL=http://www.digipen.edu/
AppSupportURL=http://www.digipen.edu/

; Default path to the file storage directory.
; {pf} is the default program files directory set by Windows
DefaultDirName={pf}\DigiPen\{#ApplicationName}

; Start menu directory
DefaultGroupName=DigiPen\{#ApplicationName}

; Output directory for the installer.
OutputDir=.\INSTALLER

; Setup executable installer
OutputBaseFilename={#ApplicationName}_Setup

; Path to the DigiPen EULA (Needed to pass TCRs)
LicenseFile=INSTALLERFILES\DigiPen_EULA.txt

; Compression scheme for the installer. Check Inno Setup help files for more options.
Compression=lzma
SolidCompression=yes

; Path to the icon for the installer (TCR check requires custom icon)
SetupIconFile=.\INSTALLERFILES\setup_icon.ico

; This allows the installer to run without admin privileges, which means you can't install
; in the Program Files, or change the registry. This is done to allow installation on Sandbox
; or other intermediate directory
PrivilegesRequired=none

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
; Creates an installer option to allow/disallow desktop shortcut
; Checked by default
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"


; This is the list of files that the installer should grab and install.
;
; Destination Directory Notes
;   {app} is the root directory that you will be installing to.
;   {temp} is a temporary directory that Windows sets that gets deleted after the
;      installer quits.
;   {userdocs} is the directory for My Documents/Documents on Windows XP, Vista, and 7.
;
; For more information on default paths to install to, check the "Constants" article
;   in the Inno Setup 5 documentation.
;
; I recommend placing any installers for required stuff (DirectX, PhysX, etc)
;   in the general structure below to keep things neat for you.
[Files]

;Chirs Onorati: Add every possible image.
Source: ".\INSTALLERFILES\panel_image_*.bmp"; Flags: dontcopy

; The game directoy is exactly what you want your install directory in program files to look like
Source: .\GAMEDIRECTORY\*; DestDir: {app}; Flags: ignoreversion recursesubdirs createallsubdirs

; Include the redistributable programs and install them to the temp directory
; TODO: Update this list with the correct redistributables for your game.
;       If your game uses DirectX, then please refer to the following forum posting:
;	        https://distance.digipen.edu/mod/forum/discuss.php?d=19674&parent=39276
; NOTE: This line is commented out because no redistributables are required for Zero Engine games.
Source: .\REDIST\*; DestDir: {tmp}; Flags: ignoreversion

; This is the list of shortcuts that the installer will setup for you.
; Of note, this will create the uninstaller automatically.
;
; Directory Notes
;   {group} is the start menu location that the game will install shortcuts to.
;   {commondesktop} is your Windows desktop directory.
[Icons]
Name: {group}\{#ApplicationName}; Filename: {app}\{#ApplicationName}.exe; WorkingDir: {app}; IconFilename: "{app}\icon.ico"
Name: {group}\{cm:UninstallProgram,{#ApplicationName}}; Filename: {uninstallexe}
Name: {commondesktop}\{#ApplicationName}; Filename: {app}\{#ApplicationName}.exe; Tasks: desktopicon; WorkingDir: {app}; IconFilename: "{app}\icon.ico"

; List of items to execute in the installer.
; Note that this will run all executables in their silent versions as required by the TCRs.
;
; The last item being run is the installer option to automatically launch the game after
;   the installer exits as required by the TCRs.
; TODO: Update this list with the correct redistributables for your game.
[Run]
Filename: {tmp}\VC_2019_redist.x64.exe; Parameters: /q /norestart; StatusMsg: Installing Visual C++ 2015 Redistributable...
Filename: {tmp}\DXSETUP.exe; Parameters: /silent; StatusMsg: Installing DirectX...
Filename: {app}\{#ApplicationName}.exe; Description: {cm:LaunchProgram,{#ApplicationName}}; Flags: nowait postinstall skipifsilent

[UninstallDelete]

[Code]

//Scale the image based on DPI of monitor screen.
function GetScalingFactor: Integer;
begin
  if WizardForm.Font.PixelsPerInch >= 192 then Result := 200
    else
  if WizardForm.Font.PixelsPerInch >= 144 then Result := 150
    else
  if WizardForm.Font.PixelsPerInch >= 120 then Result := 125
    else Result := 100;
end;

//Set image of the far left side panel.
procedure LoadEmbededScaledBitmap(Image: TBitmapImage; NameBase: string);

var Name: String;
var FileName: String;

begin
  Name := Format('%s_%d.bmp', [NameBase, GetScalingFactor]);
  ExtractTemporaryFile(Name);
  FileName := ExpandConstant('{tmp}\' + Name);
  Image.Bitmap.LoadFromFile(FileName);
  DeleteFile(FileName);
end;

//Set images based on resolution.
procedure InitializeWizard;

begin
  { If using larger scaling, load the correct size of images }
  if GetScalingFactor > 100 then 
  begin
    LoadEmbededScaledBitmap(WizardForm.WizardBitmapImage, 'panel_image');
  end;

end;