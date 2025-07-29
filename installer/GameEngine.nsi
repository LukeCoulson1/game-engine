# 2D Game Engine Installer Script
# NSIS Script for creating Windows installer

!define APP_NAME "2D Game Engine"
!define APP_VERSION "1.0.0"
!define APP_PUBLISHER "Luke Coulson"
!define APP_URL "https://github.com/LukeCoulson1/game-engine"
!define APP_DESCRIPTION "A complete 2D game engine for creating Zelda-like adventure games"

# Include modern UI
!include "MUI2.nsh"
!include "FileAssociation.nsh"

# General settings
Name "${APP_NAME}"
OutFile "GameEngine_v${APP_VERSION}_Setup.exe"
InstallDir "$PROGRAMFILES\${APP_NAME}"
InstallDirRegKey HKCU "Software\${APP_NAME}" ""
RequestExecutionLevel admin

# Interface settings
!define MUI_ABORTWARNING
!define MUI_ICON "assets\icon.ico"
!define MUI_UNICON "assets\icon.ico"

# Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

# Languages
!insertmacro MUI_LANGUAGE "English"

# Version information
VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName" "${APP_NAME}"
VIAddVersionKey "ProductVersion" "${APP_VERSION}"
VIAddVersionKey "CompanyName" "${APP_PUBLISHER}"
VIAddVersionKey "FileDescription" "${APP_DESCRIPTION}"
VIAddVersionKey "FileVersion" "${APP_VERSION}"

# Installer sections
Section "Core Engine" SecCore
    SectionIn RO
    
    SetOutPath "$INSTDIR"
    
    # Main executables
    File "build\bin\Release\GameEditor.exe"
    File "build\bin\Release\GameRuntime.exe"
    
    # DLL dependencies
    File "build\bin\Release\*.dll"
    
    # Configuration files
    File "build\bin\Release\editor_config.json"
    File "build\bin\Release\imgui.ini"
    
    # Assets directory
    SetOutPath "$INSTDIR\assets"
    File /r "assets\*.*"
    
    # Documentation
    SetOutPath "$INSTDIR\docs"
    File /r "docs\*.*"
    
    # License and readme
    SetOutPath "$INSTDIR"
    File "README.md"
    File "LICENSE.txt"
    
    # Write installation info
    WriteRegStr HKCU "Software\${APP_NAME}" "" $INSTDIR
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoRepair" 1
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "Publisher" "${APP_PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayVersion" "${APP_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "URLInfoAbout" "${APP_URL}"
    
    # Create uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Section "Desktop Shortcuts" SecDesktop
    CreateShortCut "$DESKTOP\Game Engine Editor.lnk" "$INSTDIR\GameEditor.exe"
    CreateShortCut "$DESKTOP\Game Runtime.lnk" "$INSTDIR\GameRuntime.exe"
SectionEnd

Section "Start Menu Shortcuts" SecStartMenu
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Game Engine Editor.lnk" "$INSTDIR\GameEditor.exe"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Game Runtime.lnk" "$INSTDIR\GameRuntime.exe"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Documentation.lnk" "$INSTDIR\docs\README.md"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe"
SectionEnd

Section "Examples & Templates" SecExamples
    SetOutPath "$INSTDIR\examples"
    File /r "examples\*.*"
SectionEnd

Section "Development Tools" SecDev
    # Scripts and tools
    SetOutPath "$INSTDIR\scripts"
    File /r "scripts\*.*"
    
    # Build tools (if needed for modding)
    SetOutPath "$INSTDIR\tools"
    File "build.bat"
    File "setup.bat"
    File "CMakeLists.txt"
SectionEnd

# Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "Core engine files and runtime (Required)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Create desktop shortcuts for easy access"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} "Create Start Menu shortcuts"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecExamples} "Example games and project templates"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDev} "Development tools and scripts for advanced users"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

# Uninstaller
Section "Uninstall"
    # Remove files
    Delete "$INSTDIR\GameEditor.exe"
    Delete "$INSTDIR\GameRuntime.exe"
    Delete "$INSTDIR\*.dll"
    Delete "$INSTDIR\*.json"
    Delete "$INSTDIR\*.ini"
    Delete "$INSTDIR\README.md"
    Delete "$INSTDIR\LICENSE.txt"
    Delete "$INSTDIR\uninstall.exe"
    
    # Remove directories
    RMDir /r "$INSTDIR\assets"
    RMDir /r "$INSTDIR\docs"
    RMDir /r "$INSTDIR\examples"
    RMDir /r "$INSTDIR\scripts"
    RMDir /r "$INSTDIR\tools"
    RMDir "$INSTDIR"
    
    # Remove shortcuts
    Delete "$DESKTOP\Game Engine Editor.lnk"
    Delete "$DESKTOP\Game Runtime.lnk"
    RMDir /r "$SMPROGRAMS\${APP_NAME}"
    
    # Remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
    DeleteRegKey HKCU "Software\${APP_NAME}"
SectionEnd
