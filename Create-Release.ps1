#!/usr/bin/env pwsh
# PowerShell Release Builder for 2D Game Engine

param(
    [string]$Version = "1.0.0"
)

Write-Host "2D Game Engine - Release Builder" -ForegroundColor Green
Write-Host "=================================" -ForegroundColor Green
Write-Host "Version: $Version" -ForegroundColor Cyan
Write-Host "Working Directory: $(Get-Location)" -ForegroundColor Cyan
Write-Host ""

# Configuration
$ReleaseDir = "release"
$PackageName = "GameEngine_v$Version"
$PackagePath = "$ReleaseDir\$PackageName"

# Clean and create directories
Write-Host "[1/4] Preparing directories..." -ForegroundColor Yellow
if (Test-Path $ReleaseDir) {
    Remove-Item $ReleaseDir -Recurse -Force
}
New-Item -ItemType Directory -Path $ReleaseDir -Force | Out-Null
New-Item -ItemType Directory -Path $PackagePath -Force | Out-Null

# Build the project
Write-Host "[2/4] Building project..." -ForegroundColor Yellow
& ".\build.bat"
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

# Verify build output
Write-Host "[3/4] Copying files..." -ForegroundColor Yellow
$BuildPath = "build\bin\Release"

if (Test-Path "$BuildPath\GameEditor.exe") {
    Write-Host "✅ GameEditor.exe found" -ForegroundColor Green
    Copy-Item "$BuildPath\GameEditor.exe" $PackagePath
} else {
    Write-Host "❌ GameEditor.exe not found" -ForegroundColor Red
    exit 1
}

if (Test-Path "$BuildPath\GameRuntime.exe") {
    Write-Host "✅ GameRuntime.exe found" -ForegroundColor Green
    Copy-Item "$BuildPath\GameRuntime.exe" $PackagePath
}

# Copy DLLs
Write-Host "  Copying DLL dependencies..." -ForegroundColor White
Get-ChildItem "$BuildPath\*.dll" | ForEach-Object {
    Copy-Item $_.FullName $PackagePath
    Write-Host "    - $($_.Name)" -ForegroundColor Gray
}

# Copy config files
Write-Host "  Copying configuration files..." -ForegroundColor White
@("editor_config.json", "imgui.ini") | ForEach-Object {
    $file = "$BuildPath\$_"
    if (Test-Path $file) {
        Copy-Item $file $PackagePath
        Write-Host "    - $_" -ForegroundColor Gray
    }
}

# Copy optional directories
@("assets", "docs", "examples") | ForEach-Object {
    if (Test-Path $_) {
        Copy-Item $_ "$PackagePath\$_" -Recurse
        Write-Host "  ✅ $_ folder copied" -ForegroundColor Green
    } else {
        Write-Host "  ⚠️  $_ folder not found" -ForegroundColor Yellow
    }
}

# Copy root files
@("README.md", "LICENSE.txt") | ForEach-Object {
    if (Test-Path $_) {
        Copy-Item $_ $PackagePath
        Write-Host "  ✅ $_ copied" -ForegroundColor Green
    }
}

# Create documentation
Write-Host "  Creating quick start guide..." -ForegroundColor White
$quickStart = @"
# 2D Game Engine v$Version

## 🚀 Quick Start

1. **Run the Editor**: Double-click ``GameEditor.exe``
2. **Create a Scene**: File > New Scene
3. **Add Entities**: Right-click in the scene window
4. **Move Entities**: Left-click and drag entities around
5. **Pan Camera**: Right-click and drag to move the camera
6. **Zoom**: Use mouse wheel to zoom in/out
7. **Save**: Press Ctrl+S to save your scene

## 🎮 What's Included

- **GameEditor.exe** - Visual scene editor with GUI
- **GameRuntime.exe** - Game runtime for playing games
- **Assets folder** - Sample textures and resources
- **Documentation** - Complete user guides and API docs
- **All DLL dependencies** - SDL2, audio, and graphics libraries

## 🖥️ System Requirements

- Windows 10 or 11 (64-bit)
- DirectX 11 compatible graphics card
- 4GB RAM (8GB recommended)
- 500MB free disk space
- Visual C++ Redistributable 2022 (usually pre-installed)

## 📖 Documentation

For complete documentation, tutorials, and examples:
- Check the ``docs`` folder included in this package
- Visit: https://github.com/yourusername/game-engine

## 🐛 Support

Found a bug? Have a question? 
Please create an issue on our GitHub repository.

**Happy Game Making! 🎉**
"@

$quickStart | Out-File -FilePath "$PackagePath\QUICK_START.md" -Encoding UTF8

# Create version info
$versionInfo = @"
Game Engine Version: $Version
Build Date: $(Get-Date -Format "yyyy-MM-dd")
Build Time: $(Get-Date -Format "HH:mm:ss")
Platform: Windows x64

Build Configuration: Release
Compiler: Microsoft Visual C++ 2022
Dependencies: SDL2, Dear ImGui, vcpkg
"@

$versionInfo | Out-File -FilePath "$PackagePath\VERSION.txt" -Encoding UTF8

# Create ZIP package
Write-Host "[4/4] Creating distribution package..." -ForegroundColor Yellow
Push-Location $ReleaseDir

try {
    Compress-Archive -Path $PackageName -DestinationPath "$PackageName`_Portable.zip" -Force
    Write-Host "✅ ZIP package created successfully" -ForegroundColor Green
} catch {
    Write-Host "❌ Failed to create ZIP package: $($_.Exception.Message)" -ForegroundColor Red
}

Pop-Location

# Final summary
Write-Host ""
Write-Host "===========================================" -ForegroundColor Green
Write-Host "🎉 Release Package Complete!" -ForegroundColor Green
Write-Host "===========================================" -ForegroundColor Green
Write-Host ""

Write-Host "📦 Package Contents:" -ForegroundColor Cyan
Get-ChildItem $PackagePath | ForEach-Object {
    Write-Host "  - $($_.Name)" -ForegroundColor White
}

Write-Host ""
Write-Host "📊 Package Statistics:" -ForegroundColor Cyan
$fileCount = (Get-ChildItem $PackagePath -Recurse -File).Count
$totalSize = (Get-ChildItem $PackagePath -Recurse -File | Measure-Object -Property Length -Sum).Sum
Write-Host "  Files: $fileCount" -ForegroundColor White
Write-Host "  Total Size: $([math]::Round($totalSize / 1MB, 2)) MB" -ForegroundColor White

Write-Host ""
Write-Host "🗂️ Distribution Files:" -ForegroundColor Cyan
Get-ChildItem $ReleaseDir | ForEach-Object {
    Write-Host "  - $($_.Name)" -ForegroundColor White
}

Write-Host ""
Write-Host "✅ Ready for GitHub release!" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Test the GameEditor.exe in the package folder" -ForegroundColor White
Write-Host "2. Upload $PackageName`_Portable.zip to GitHub releases" -ForegroundColor White
Write-Host "3. Create release notes based on QUICK_START.md" -ForegroundColor White
Write-Host ""
