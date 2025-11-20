; TCR compliant Sample install script adapted for WaterBound.

[Setup]
; !!NOTE!!: The value of AppId uniquely identifies this application.
; You can generate a new GUID in Visual Studio (Tools -> Create GUID),
; then replace the one below.
AppId={{6E54C5F5-2B5F-4B7D-9F31-8F2A4F0E9F31}

; ---- Standard app data ----
AppName=WaterBound
AppVerName=WaterBound Version 1.0
AppPublisher=DigiPen Institute of Technology
AppPublisherURL=http://www.digipen.edu/
AppSupportURL=http://www.digipen.edu/

; Default install path in Program Files (requirement 1907)
DefaultDirName={pf}\DigiPen\WaterBound

; Start menu directory
DefaultGroupName=DigiPen\WaterBound

; Output directory for the installer.
OutputDir=.\INSTALLER

; Setup executable installer
OutputBaseFilename=WaterBound_Setup

; Path to the DigiPen EULA (Needed to pass TCRs)
LicenseFile=INSTALLERFILES\DigiPen_EULA.txt

; Path to the icon for the installer (TCR check requires custom icon)
SetupIconFile=.\INSTALLERFILES\SetupIcon.ico

; Compression scheme for the installer.
Compression=lzma
SolidCompression=yes

; We want to install into Program Files, so require admin rights.
PrivilegesRequired=admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
; Creates an installer option to allow/disallow desktop shortcut (checked by default)
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

; -------------- Files to install --------------
[Files]
; Main game directory (your Release exe + assets) -> {app}
Source: ".\GAMEDIRECTORY\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

; Optional: user documents (readme, controls, etc.) -> Documents\WaterBound
Source: ".\USERDOCS\*"; DestDir: "{userdocs}\WaterBound"; Flags: ignoreversion recursesubdirs createallsubdirs

; VC++ Redistributable copied to temp
Source: ".\REDIST\VC_redist.x64.exe"; DestDir: "{tmp}"; Flags: ignoreversion

; -------------- Shortcuts --------------
[Icons]
; Start menu shortcuts
Name: "{group}\WaterBound"; Filename: "{app}\WaterBound.exe"; WorkingDir: "{app}"; IconFilename: "{app}\Icon.ico"
Name: "{group}\{cm:UninstallProgram,WaterBound}"; Filename: "{uninstallexe}"
Name: "{group}\{cm:UninstallProgram,WaterBound}"; Filename: "{uninstallexe}"; IconFilename: "{app}\Icon.ico"

; Desktop shortcut (optional task)
Name: "{commondesktop}\WaterBound"; Filename: "{app}\WaterBound.exe"; \
    Tasks: desktopicon; WorkingDir: "{app}"; IconFilename: "{app}\Icon.ico"

; -------------- Things to run during/after install --------------
[Run]
; Install Visual C++ Redistributable silently
Filename: "{tmp}\VC_redist.x64.exe"; Parameters: "/install /quiet /norestart"; \
    StatusMsg: "Installing Visual C++ 2022 Redistributable..."

; Option to launch the game after install
Filename: "{app}\WaterBound.exe"; Description: "{cm:LaunchProgram,WaterBound}"; \
    Flags: nowait postinstall skipifsilent

; To build WaterBound_Setup.exe:
; 1) Install Inno Setup
; 2) Open this .iss file
; 3) Compile
