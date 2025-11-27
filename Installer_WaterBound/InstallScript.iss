; TCR compliant Sample install script adapted for WaterBound.

[Setup]
; !!NOTE!!: The value of AppId uniquely identifies this application.
; You can generate a new GUID in Visual Studio (Tools -> Create GUID),
; then replace the one below.
AppId={{10479E23-9D27-4730-B89F-8B9E0485DFDB}

; ---- Standard app data ----
AppName=WaterBound
AppVerName=WaterBound Version 1.0
AppPublisher=DigiPen Institute of Technology
AppPublisherURL=http://www.digipen.edu/
AppSupportURL=http://www.digipen.edu/

; Default install path in Program Files
DefaultDirName={pf}\DigiPen\WaterBound

; Start menu directory
DefaultGroupName=DigiPen\WaterBound

; Output directory for the installer.
OutputDir=.\INSTALLER

; Setup executable installer
OutputBaseFilename=WaterBound_Setup

; Path to the DigiPen EULA
LicenseFile=INSTALLERFILES\DigiPen_EULA.txt

; Path to the icon for the installer 
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

; VC++ Redistributable copied to temp
Source: ".\REDIST\VC_redist.x64.exe"; DestDir: "{tmp}"; Flags: ignoreversion

; -------------- Shortcuts --------------
[Icons]

; Start menu shortcuts
Name: "{group}\WaterBound (Fullscreen)"; \
    Filename: "{app}\WaterBound.exe"; \
    WorkingDir: "{app}"; \
    IconFilename: "{app}\Icon.ico"

Name: "{group}\WaterBound (Windowed Mode)"; \
    Filename: "{app}\WaterBound.exe"; \
    Parameters: "--windowed"; \
    WorkingDir: "{app}"; \
    IconFilename: "{app}\Icon.ico"

; Single uninstaller entry (uninstalls everything)
Name: "{group}\{cm:UninstallProgram,WaterBound}"; \
    Filename: "{uninstallexe}"; \
    IconFilename: "{app}\Icon.ico"

; Desktop shortcut (fullscreen launcher)
Name: "{commondesktop}\WaterBound"; \
    Filename: "{app}\WaterBound.exe"; \
    Tasks: desktopicon; \
    WorkingDir: "{app}"; \
    IconFilename: "{app}\Icon.ico"
;Desktop shortcut (windowed launcher)
Name: "{commondesktop}\WaterBound (Windowed)"; \
    Filename: "{app}\WaterBound.exe"; \
    Parameters: "--windowed"; \
    Tasks: desktopicon; \
    WorkingDir: "{app}"; \
    IconFilename: "{app}\Icon.ico"


; -------------- Things to run during/after install --------------
[Run]
; Install Visual C++ Redistributable silently
Filename: "{tmp}\VC_redist.x64.exe"; Parameters: "/install /quiet /norestart"; \
    StatusMsg: "Installing Visual C++ 2022 Redistributable..."

; Option to launch the game after install
Filename: "{app}\WaterBound.exe"; Description: "{cm:LaunchProgram,WaterBound}"; \
    Flags: nowait postinstall skipifsilent
