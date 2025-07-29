#!/usr/bin/env pwsh
# GitHub Release Script for LukeCoulson1/game-engine

param(
    [Parameter(Mandatory=$false)]
    [string]$Version = "1.0.0",
    
    [Parameter(Mandatory=$false)]
    [string]$GitHubToken = "",
    
    [Parameter(Mandatory=$false)]
    [switch]$Draft = $false
)

$Repository = "LukeCoulson1/game-engine"

Write-Host "🚀 Publishing to GitHub: $Repository" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Green
Write-Host ""

# Check if files exist
$releaseDir = "release"
$portableZip = "$releaseDir\GameEngine_v$Version`_Portable.zip"
$installerExe = "$releaseDir\GameEngine_v$Version`_Setup.exe"
$checksums = "$releaseDir\checksums.txt"

Write-Host "🔍 Checking release files..." -ForegroundColor Yellow
$filesToUpload = @()

if (Test-Path $portableZip) {
    Write-Host "✅ Found: $portableZip" -ForegroundColor Green
    $filesToUpload += $portableZip
} else {
    Write-Host "❌ Missing: $portableZip" -ForegroundColor Red
}

if (Test-Path $installerExe) {
    Write-Host "✅ Found: $installerExe" -ForegroundColor Green
    $filesToUpload += $installerExe
} else {
    Write-Host "⚠️  Missing: $installerExe (will upload portable only)" -ForegroundColor Yellow
}

if (Test-Path $checksums) {
    Write-Host "✅ Found: $checksums" -ForegroundColor Green
    $filesToUpload += $checksums
}

if ($filesToUpload.Count -eq 0) {
    Write-Host "❌ No release files found! Run Build-Complete-Release.ps1 first." -ForegroundColor Red
    exit 1
}

# Check GitHub CLI
if (-not (Get-Command "gh" -ErrorAction SilentlyContinue)) {
    Write-Host "❌ GitHub CLI not found. Installing..." -ForegroundColor Red
    try {
        winget install GitHub.cli
        Write-Host "✅ GitHub CLI installed. Please restart PowerShell and run again." -ForegroundColor Green
        exit 0
    } catch {
        Write-Host "❌ Failed to install GitHub CLI. Please install manually from: https://cli.github.com/" -ForegroundColor Red
        exit 1
    }
}

# Check authentication
Write-Host "🔐 Checking GitHub authentication..." -ForegroundColor Yellow
try {
    $authCheck = & gh auth status 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Not authenticated with GitHub" -ForegroundColor Red
        Write-Host "Please run: gh auth login" -ForegroundColor White
        exit 1
    }
    Write-Host "✅ GitHub authentication OK" -ForegroundColor Green
} catch {
    Write-Host "❌ Authentication check failed" -ForegroundColor Red
    exit 1
}

# Create comprehensive release notes
Write-Host "📝 Creating release notes..." -ForegroundColor Yellow
$releaseNotes = @"
# 🎮 2D Game Engine v$Version

A complete 2D game engine for creating Zelda-like adventure games with an intuitive visual editor.

## 🆕 What's New in v$Version

### 🎯 Entity Manipulation System
- **Drag & Drop**: Click and drag entities around in the Scene Window
- **Visual Feedback**: Real-time green highlighting when dragging entities
- **Precision Control**: Smooth entity positioning with drag threshold
- **Camera Integration**: Separate controls for entity dragging vs camera panning

### 🛠️ Professional Editor
- **Dockable Interface**: Modern GUI with Dear ImGui
- **Component Inspector**: Real-time property editing
- **Scene Management**: Robust save/load with dirty state tracking
- **Asset Browser**: Easy resource management
- **Professional Workflow**: Undo/redo, multi-window support

### ⚡ Core Engine Features
- **ECS Architecture**: Entity-Component-System for flexible game objects
- **High-Performance Rendering**: SDL2 with hardware acceleration
- **Physics System**: Collision detection and response
- **Audio Engine**: Music and sound effects support
- **Input Management**: Keyboard, mouse, and controller support
- **Resource Management**: Automatic loading and caching

## 📦 Installation Options

### 🛠️ Windows Installer (Recommended)
1. Download ``GameEngine_v$Version`_Setup.exe``
2. Run the installer and follow the setup wizard
3. Launch from Start Menu or Desktop shortcuts
4. Everything is configured automatically!

### 📁 Portable Version
1. Download ``GameEngine_v$Version`_Portable.zip``
2. Extract to any folder on your system
3. Run ``GameEditor.exe`` to start creating games
4. No installation required!

## 🎮 Quick Start Guide

1. **Launch**: Run GameEditor.exe (installer) or extract and run (portable)
2. **Create**: File > New Scene to start your first project
3. **Add Entities**: Right-click in the scene window to add game objects
4. **Move Objects**: Left-click and drag entities to position them
5. **Navigate**: Right-click and drag to pan the camera view
6. **Zoom**: Use mouse wheel to zoom in and out
7. **Save**: Press Ctrl+S to save your scene

## 🖱️ Controls Reference

| Action | Control |
|--------|---------|
| Select Entity | Left Click |
| Move Entity | Left Click + Drag |
| Pan Camera | Right Click + Drag |
| Zoom | Mouse Wheel |
| Save Scene | Ctrl + S |
| New Scene | Ctrl + N |
| Open Scene | Ctrl + O |

## 🖥️ System Requirements

- **Operating System**: Windows 10 or 11 (64-bit)
- **Graphics**: DirectX 11 compatible graphics card
- **Memory**: 4GB RAM minimum (8GB recommended)
- **Storage**: 500MB free disk space
- **Runtime**: Visual C++ Redistributable 2022 (usually pre-installed)

## 📋 What's Included

### 🎮 Applications
- **GameEditor.exe** - Visual scene editor with professional GUI
- **GameRuntime.exe** - Standalone game runtime for testing

### 📁 Resources
- **Complete Documentation** - User guides, tutorials, API reference
- **Example Projects** - Sample games and templates
- **Asset Library** - Textures, sprites, and audio files
- **All Dependencies** - SDL2, audio libraries, no additional downloads

### 🔧 Development Tools
- **Project Templates** - Ready-to-use game templates
- **Asset Pipeline** - Import and manage game resources
- **Debug Console** - Real-time logging and debugging
- **Performance Profiler** - Optimize your games

## 🔒 File Verification

Use the provided ``checksums.txt`` to verify file integrity:
```bash
# Windows (PowerShell)
Get-FileHash GameEngine_v$Version`_Setup.exe -Algorithm SHA256
Get-FileHash GameEngine_v$Version`_Portable.zip -Algorithm SHA256
```

## 🐛 Known Issues & Limitations

- Currently Windows-only (Linux/Mac support planned for v2.0)
- Large textures (>4K) may impact performance on older hardware
- Some advanced OpenGL features require recent drivers

## 📖 Documentation & Support

- **📚 User Guide**: Included in docs folder or [online](https://github.com/$Repository/wiki)
- **🐛 Bug Reports**: [Create an issue](https://github.com/$Repository/issues)
- **💡 Feature Requests**: [Discussions](https://github.com/$Repository/discussions)
- **❓ Questions**: Check documentation or ask in discussions

## 🙏 Acknowledgments

Built with amazing open-source libraries:
- **SDL2** - Cross-platform multimedia library
- **Dear ImGui** - Immediate mode GUI framework
- **vcpkg** - C++ package management
- **CMake** - Build system generator

Special thanks to the game development community for feedback and inspiration!

## 🎯 What's Next?

- Cross-platform support (Linux, macOS)
- Advanced scripting system
- Visual shader editor
- Networking capabilities
- Asset store integration

---

**Ready to create your next adventure game? Download and start building! 🎉**

*For support, visit: https://github.com/$Repository*
"@

$releaseNotesFile = "RELEASE_NOTES_v$Version.md"
$releaseNotes | Out-File -FilePath $releaseNotesFile -Encoding UTF8

# Create the GitHub release
Write-Host "🌐 Creating GitHub release..." -ForegroundColor Yellow

$tagName = "v$Version"
$releaseTitle = "2D Game Engine v$Version - Complete Development Suite"

try {
    # Build the command
    $releaseArgs = @(
        "release", "create", $tagName,
        "--repo", $Repository,
        "--title", $releaseTitle,
        "--notes-file", $releaseNotesFile
    )
    
    # Add files
    foreach ($file in $filesToUpload) {
        $releaseArgs += $file
    }
    
    # Add draft flag if requested
    if ($Draft) {
        $releaseArgs += "--draft"
        Write-Host "  Creating as draft..." -ForegroundColor Yellow
    }
    
    # Execute the command
    & gh @releaseArgs
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ GitHub release created successfully!" -ForegroundColor Green
        
        $releaseUrl = "https://github.com/$Repository/releases/tag/$tagName"
        Write-Host ""
        Write-Host "🔗 Release URL: $releaseUrl" -ForegroundColor Cyan
        
        # Open the release page
        Start-Process $releaseUrl
        
    } else {
        Write-Host "❌ GitHub release creation failed" -ForegroundColor Red
        exit 1
    }
    
} catch {
    Write-Host "❌ Error creating GitHub release: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# Final success message
Write-Host ""
Write-Host "🎉 Release v$Version Published Successfully!" -ForegroundColor Green
Write-Host "===========================================" -ForegroundColor Green
Write-Host ""

Write-Host "📊 Release Summary:" -ForegroundColor Cyan
Write-Host "  Repository: $Repository" -ForegroundColor White
Write-Host "  Version: $Version" -ForegroundColor White
Write-Host "  Tag: $tagName" -ForegroundColor White
Write-Host "  Files uploaded: $($filesToUpload.Count)" -ForegroundColor White
if ($Draft) {
    Write-Host "  Status: Draft (ready to publish)" -ForegroundColor Yellow
} else {
    Write-Host "  Status: Published and live!" -ForegroundColor Green
}

Write-Host ""
Write-Host "🚀 Next Steps:" -ForegroundColor Cyan
Write-Host "  1. 🧪 Test downloads on a clean system" -ForegroundColor White
Write-Host "  2. 📢 Share on social media and forums" -ForegroundColor White
Write-Host "  3. 📝 Update project documentation" -ForegroundColor White
Write-Host "  4. 🎯 Start planning v$([int]$Version.Split('.')[0] + 1).0.0 features!" -ForegroundColor White

Write-Host ""
Write-Host "✨ Congratulations on your release! ✨" -ForegroundColor Green
