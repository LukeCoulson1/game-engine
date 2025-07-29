# 📤 Manual GitHub Release Upload Guide

## 🎯 Quick Upload Process

### Step 1: Go to Your GitHub Repository
1. Open: https://github.com/LukeCoulson1/game-engine
2. Click **"Releases"** (on the right side)
3. Click **"Create a new release"**

### Step 2: Fill in Release Information

**Tag version:** `v1.0.0`
**Release title:** `2D Game Engine v1.0.0 - Complete Development Suite`

**Description:** (Copy and paste this)

```markdown
# 🎮 2D Game Engine v1.0.0

A complete 2D game engine for creating Zelda-like adventure games with an intuitive visual editor.

## 🆕 What's New in v1.0.0

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

## 📦 Installation Options

### 🛠️ Windows Installer (Recommended)
1. Download `GameEngine_v1.0.0_Setup.exe`
2. Run the installer and follow the setup wizard
3. Launch from Start Menu or Desktop shortcuts

### 📁 Portable Version
1. Download `GameEngine_v1.0.0_Portable.zip`
2. Extract to any folder on your system
3. Run `GameEditor.exe` to start creating games

## 🎮 Quick Start Guide

1. **Launch**: Run GameEditor.exe
2. **Create**: File > New Scene
3. **Add Entities**: Right-click in the scene window
4. **Move Objects**: Left-click and drag entities to position them
5. **Navigate**: Right-click and drag to pan the camera
6. **Save**: Press Ctrl+S to save your scene

## 🖱️ Controls

- **Left Click**: Select entities
- **Left Click + Drag**: Move entities around
- **Right Click + Drag**: Pan camera view
- **Mouse Wheel**: Zoom in/out
- **Ctrl + S**: Save scene

## 🖥️ System Requirements

- Windows 10 or 11 (64-bit)
- DirectX 11 compatible graphics card
- 4GB RAM (8GB recommended)
- 500MB free disk space

## 🔒 File Verification

SHA256 checksums are provided in `checksums.txt` for security verification.

## 🐛 Support

- **Bug Reports**: [Create an issue](https://github.com/LukeCoulson1/game-engine/issues)
- **Questions**: [Start a discussion](https://github.com/LukeCoulson1/game-engine/discussions)

---

**Ready to create your next adventure game? Download and start building! 🎉**
```

### Step 3: Attach Files

In the **"Attach binaries"** section at the bottom:

1. **Drag and drop** these files from your `release` folder:
   - `GameEngine_v1.0.0_Setup.exe` (Windows Installer)
   - `GameEngine_v1.0.0_Portable.zip` (Portable Version)
   - `checksums.txt` (Security Checksums)

2. **Or click "selecting them"** and browse to:
   - `f:\Programming\game-engine\release\`
   - Select all three files

### Step 4: Publish

- ✅ Check **"Set as the latest release"**
- ✅ Check **"Create a discussion for this release"** (optional)
- Click **"Publish release"**

## 🎊 That's It!

Your release will be live at:
`https://github.com/LukeCoulson1/game-engine/releases/tag/v1.0.0`

## 📊 File Sizes for Reference

- `GameEngine_v1.0.0_Setup.exe`: ~1.65 MB (Windows Installer)
- `GameEngine_v1.0.0_Portable.zip`: ~2.11 MB (Portable Version)
- `checksums.txt`: ~0.1 KB (File Verification)

## 🔄 Alternative: Automated Upload

After restarting PowerShell, you can also use:
```powershell
gh auth login
.\Publish-GitHub.ps1
```

This will create the release automatically with all files and proper formatting.

---

**🚀 Your game engine with entity drag-and-drop is ready for the world!**
