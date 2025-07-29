#!/usr/bin/env pwsh
# Complete Release Builder with .exe Installer

param(
    [string]$Version = "1.0.0"
)

Write-Host "🚀 2D Game Engine - Professional Release Builder" -ForegroundColor Green
Write-Host "=================================================" -ForegroundColor Green
Write-Host "Version: $Version" -ForegroundColor Cyan
Write-Host ""

# Step 1: Build the project and create portable package
Write-Host "[1/4] Creating portable package..." -ForegroundColor Yellow
& ".\Create-Release.ps1" -Version $Version
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Portable package creation failed" -ForegroundColor Red
    exit 1
}
Write-Host "✅ Portable package created" -ForegroundColor Green

# Step 2: Check for NSIS and create .exe installer
Write-Host "[2/4] Creating .exe installer..." -ForegroundColor Yellow

$nsisPath = $null
$nsisPaths = @(
    "C:\Program Files (x86)\NSIS\makensis.exe",
    "C:\Program Files\NSIS\makensis.exe",
    "C:\Tools\NSIS\makensis.exe"
)

foreach ($path in $nsisPaths) {
    if (Test-Path $path) {
        $nsisPath = $path
        break
    }
}

if ($nsisPath) {
    Write-Host "✅ NSIS found at: $nsisPath" -ForegroundColor Green
    
    # Update version in NSIS script
    $nsisScript = "installer\GameEngine_Professional.nsi"
    if (Test-Path $nsisScript) {
        # Create the installer
        Write-Host "  Building installer..." -ForegroundColor White
        & $nsisPath $nsisScript
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ .exe installer created successfully!" -ForegroundColor Green
        } else {
            Write-Host "❌ Installer creation failed" -ForegroundColor Red
        }
    } else {
        Write-Host "❌ NSIS script not found: $nsisScript" -ForegroundColor Red
    }
} else {
    Write-Host "⚠️  NSIS not found. Installing NSIS..." -ForegroundColor Yellow
    Write-Host "  Downloading NSIS..." -ForegroundColor White
    
    try {
        # Download NSIS installer
        $nsisUrl = "https://sourceforge.net/projects/nsis/files/NSIS%203/3.10/nsis-3.10-setup.exe/download"
        $nsisInstaller = "$env:TEMP\nsis-setup.exe"
        
        Invoke-WebRequest -Uri $nsisUrl -OutFile $nsisInstaller -UserAgent "Mozilla/5.0"
        
        Write-Host "  Running NSIS installer..." -ForegroundColor White
        Start-Process -FilePath $nsisInstaller -ArgumentList "/S" -Wait
        
        # Check if NSIS is now available
        if (Test-Path "C:\Program Files (x86)\NSIS\makensis.exe") {
            Write-Host "✅ NSIS installed successfully!" -ForegroundColor Green
            $nsisPath = "C:\Program Files (x86)\NSIS\makensis.exe"
            
            # Now create the installer
            & $nsisPath "installer\GameEngine_Professional.nsi"
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✅ .exe installer created!" -ForegroundColor Green
            }
        } else {
            Write-Host "❌ NSIS installation failed or path not found" -ForegroundColor Red
            Write-Host "Please install NSIS manually from: https://nsis.sourceforge.io/" -ForegroundColor White
        }
    } catch {
        Write-Host "❌ Failed to download/install NSIS: $($_.Exception.Message)" -ForegroundColor Red
        Write-Host "Please install NSIS manually from: https://nsis.sourceforge.io/" -ForegroundColor White
    }
}

# Step 3: Create checksums for security
Write-Host "[3/4] Creating checksums..." -ForegroundColor Yellow
$releaseDir = "release"
$files = @()

if (Test-Path "$releaseDir\GameEngine_v$Version`_Portable.zip") {
    $files += "$releaseDir\GameEngine_v$Version`_Portable.zip"
}

if (Test-Path "$releaseDir\GameEngine_v$Version`_Setup.exe") {
    $files += "$releaseDir\GameEngine_v$Version`_Setup.exe"
}

if ($files.Count -gt 0) {
    $checksumFile = "$releaseDir\checksums.txt"
    "" | Out-File -FilePath $checksumFile -Encoding UTF8
    
    foreach ($file in $files) {
        $hash = Get-FileHash -Path $file -Algorithm SHA256
        $filename = Split-Path $file -Leaf
        "$($hash.Hash.ToLower())  $filename" | Add-Content -Path $checksumFile -Encoding UTF8
        Write-Host "  ✅ $filename - SHA256 checksum created" -ForegroundColor Gray
    }
    
    Write-Host "✅ Checksums created: checksums.txt" -ForegroundColor Green
}

# Step 4: Summary and verification
Write-Host "[4/4] Final verification..." -ForegroundColor Yellow

Write-Host ""
Write-Host "📊 Release Summary:" -ForegroundColor Cyan
Write-Host "==================" -ForegroundColor Cyan

if (Test-Path "$releaseDir\GameEngine_v$Version`_Portable.zip") {
    $zipSize = (Get-Item "$releaseDir\GameEngine_v$Version`_Portable.zip").Length
    Write-Host "✅ Portable ZIP: GameEngine_v$Version`_Portable.zip ($([math]::Round($zipSize / 1MB, 2)) MB)" -ForegroundColor Green
}

if (Test-Path "$releaseDir\GameEngine_v$Version`_Setup.exe") {
    $exeSize = (Get-Item "$releaseDir\GameEngine_v$Version`_Setup.exe").Length
    Write-Host "✅ Windows Installer: GameEngine_v$Version`_Setup.exe ($([math]::Round($exeSize / 1MB, 2)) MB)" -ForegroundColor Green
}

if (Test-Path "$releaseDir\checksums.txt") {
    Write-Host "✅ Security checksums: checksums.txt" -ForegroundColor Green
}

Write-Host ""
Write-Host "🎯 Installation Options:" -ForegroundColor Cyan
Write-Host "========================" -ForegroundColor Cyan
Write-Host "1. 📦 Portable Version: Extract ZIP and run GameEditor.exe" -ForegroundColor White
Write-Host "2. 🛠️  Windows Installer: Run Setup.exe for full installation" -ForegroundColor White
Write-Host "3. 🔒 Verify Downloads: Use checksums.txt to verify file integrity" -ForegroundColor White

Write-Host ""
Write-Host "🚀 Ready for Release!" -ForegroundColor Green
Write-Host "=====================" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Test both installers on a clean system" -ForegroundColor White
Write-Host "2. Run: .\Release-GitHub.ps1 to upload to GitHub" -ForegroundColor White
Write-Host "3. Create release announcement" -ForegroundColor White
Write-Host ""

# Open release folder
Write-Host "📂 Opening release folder..." -ForegroundColor Cyan
Start-Process explorer.exe -ArgumentList $releaseDir
