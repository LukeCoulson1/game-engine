# 2D Game Engine Professional Installer
# NSIS Script for creating Windows .exe installer

!define APP_NAME "2D Game Engine"
!define APP_VERSION "1.0.0"
!define APP_PUBLISHER "Luke Coulson"
!define APP_URL "https://github.com/LukeCoulson1/game-engine"
!define APP_DESCRIPTION "A complete 2D game engine for creating Zelda-like adventure games"

# Include modern UI
!include "MUI2.nsh"

# General settings
Name "${APP_NAME}"
OutFile "..\release\GameEngine_v${APP_VERSION}_Setup.exe"
InstallDir "$PROGRAMFILES64\${APP_NAME}"
InstallDirRegKey HKCU "Software\${APP_NAME}" ""
RequestExecutionLevel admin

# Compression
SetCompressor /SOLID lzma
SetCompressorDictSize 32

# Modern UI settings
!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TITLE "Welcome to ${APP_NAME} Setup"
!define MUI_WELCOMEPAGE_TEXT "This will install ${APP_NAME} v${APP_VERSION} on your computer.$\r$\n$\r$\nA complete 2D game engine with visual editor for creating adventure games.$\r$\n$\r$\nClick Next to continue."

# Finish page settings
!define MUI_FINISHPAGE_RUN "$INSTDIR\GameEditor.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch Game Engine Editor"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\QUICK_START.md"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "View Quick Start Guide"

# Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"
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
VIAddVersionKey "LegalCopyright" "© 2025 ${APP_PUBLISHER}"

# Installer sections
Section "Core Engine" SecCore
    SectionIn RO
    
    SetOutPath "$INSTDIR"
    
    # Main executables
    File "..\build\bin\Release\GameEditor.exe"
    File "..\build\bin\Release\GameRuntime.exe"
    
    # DLL dependencies
    File "..\build\bin\Release\SDL2.dll"
    File "..\build\bin\Release\SDL2_image.dll"
    File "..\build\bin\Release\SDL2_mixer.dll"
    File "..\build\bin\Release\libpng16.dll"
    File "..\build\bin\Release\zlib1.dll"
    File "..\build\bin\Release\ogg.dll"
    File "..\build\bin\Release\vorbis.dll"
    File "..\build\bin\Release\vorbisfile.dll"
    File "..\build\bin\Release\wavpackdll.dll"
    
    # Configuration files
    File "..\build\bin\Release\editor_config.json"
    File "..\build\bin\Release\imgui.ini"
    
    # Assets directory
    SetOutPath "$INSTDIR\assets"
    File /r "..\assets\*.*"
    
    # Documentation
    SetOutPath "$INSTDIR\docs"
    File /r "..\docs\*.*"
    
    # License and readme
    SetOutPath "$INSTDIR"
    File "..\README.md"
    File "..\LICENSE.txt"
    
    # Create quick start guide
    FileOpen $0 "$INSTDIR\QUICK_START.md" w
    FileWrite $0 "# 2D Game Engine v${APP_VERSION}$\r$\n"
    FileWrite $0 "$\r$\n"
    FileWrite $0 "## 🚀 Getting Started$\r$\n"
    FileWrite $0 "$\r$\n"
    FileWrite $0 "1. **Run the Editor**: Use the desktop shortcut or Start Menu$\r$\n"
    FileWrite $0 "2. **Create a Scene**: File > New Scene$\r$\n"
    FileWrite $0 "3. **Add Entities**: Right-click in the scene window$\r$\n"
    FileWrite $0 "4. **Move Entities**: Left-click and drag entities around$\r$\n"
    FileWrite $0 "5. **Pan Camera**: Right-click and drag to move camera$\r$\n"
    FileWrite $0 "6. **Save**: Press Ctrl+S to save your work$\r$\n"
    FileWrite $0 "$\r$\n"
    FileWrite $0 "## 🎮 Features$\r$\n"
    FileWrite $0 "$\r$\n"
    FileWrite $0 "- Visual scene editor with drag-and-drop$\r$\n"
    FileWrite $0 "- Real-time entity manipulation$\r$\n"
    FileWrite $0 "- Component-based architecture$\r$\n"
    FileWrite $0 "- Built-in asset management$\r$\n"
    FileWrite $0 "- Professional development tools$\r$\n"
    FileWrite $0 "$\r$\n"
    FileWrite $0 "## 🔗 Support$\r$\n"
    FileWrite $0 "$\r$\n"
    FileWrite $0 "Visit: ${APP_URL}$\r$\n"
    FileWrite $0 "Issues: ${APP_URL}/issues$\r$\n"
    FileClose $0
    
    # Write installation info
    WriteRegStr HKCU "Software\${APP_NAME}" "" $INSTDIR
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoRepair" 1
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "Publisher" "${APP_PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayVersion" "${APP_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "URLInfoAbout" "${APP_URL}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "HelpLink" "${APP_URL}/issues"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "URLUpdateInfo" "${APP_URL}/releases"
    
    # Create uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Section "Desktop Shortcuts" SecDesktop
    CreateShortCut "$DESKTOP\Game Engine Editor.lnk" "$INSTDIR\GameEditor.exe" "" "$INSTDIR\GameEditor.exe" 0
    CreateShortCut "$DESKTOP\Game Runtime.lnk" "$INSTDIR\GameRuntime.exe" "" "$INSTDIR\GameRuntime.exe" 0
SectionEnd

Section "Start Menu Shortcuts" SecStartMenu
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Game Engine Editor.lnk" "$INSTDIR\GameEditor.exe"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Game Runtime.lnk" "$INSTDIR\GameRuntime.exe"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Quick Start Guide.lnk" "$INSTDIR\QUICK_START.md"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Documentation.lnk" "$INSTDIR\docs\README.md"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Visit Website.lnk" "${APP_URL}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe"
SectionEnd

Section "Examples & Templates" SecExamples
    SetOutPath "$INSTDIR\examples"
    File /r "..\examples\*.*"
SectionEnd

# Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "Core engine files, editor, and runtime (Required)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Create desktop shortcuts for quick access"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} "Create Start Menu shortcuts and links"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecExamples} "Example games and project templates"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

# Uninstaller
Section "Uninstall"
    # Remove files
    Delete "$INSTDIR\GameEditor.exe"
    Delete "$INSTDIR\GameRuntime.exe"
    Delete "$INSTDIR\*.dll"
    Delete "$INSTDIR\*.json"
    Delete "$INSTDIR\*.ini"
    Delete "$INSTDIR\*.md"
    Delete "$INSTDIR\*.txt"
    Delete "$INSTDIR\uninstall.exe"
    
    # Remove directories
    RMDir /r "$INSTDIR\assets"
    RMDir /r "$INSTDIR\docs"
    RMDir /r "$INSTDIR\examples"
    RMDir "$INSTDIR"
    
    # Remove shortcuts
    Delete "$DESKTOP\Game Engine Editor.lnk"
    Delete "$DESKTOP\Game Runtime.lnk"
    RMDir /r "$SMPROGRAMS\${APP_NAME}"
    
    # Remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
    DeleteRegKey HKCU "Software\${APP_NAME}"
SectionEnd
