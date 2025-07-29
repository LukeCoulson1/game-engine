# Release Creation Guide

This guide explains how to create official releases of the 2D Game Engine for distribution on GitHub and other platforms.

## 🚀 Quick Release (Automated)

For a fully automated release process:

```batch
build_official_release.bat
```

This script will:
1. Build the project
2. Run tests
3. Create release packages
4. Optionally create GitHub release

## 📦 Manual Release Process

### Step 1: Prepare the Build

```batch
# Clean and build the project
build.bat
```

### Step 2: Run Tests

```batch
# Run the complete test suite
test_all.bat
```

### Step 3: Create Release Packages

```batch
# Create portable ZIP and installer
create_release.bat
```

### Step 4: Create GitHub Release

```powershell
# Create GitHub release with packages
.\scripts\create_github_release.ps1 -GitHubToken "your_token" -Repository "yourusername/game-engine"
```

## 📋 Release Checklist

Before creating a release, ensure:

- [ ] All tests pass (`test_all.bat`)
- [ ] Documentation is up to date
- [ ] Version numbers are updated
- [ ] Release notes are prepared
- [ ] All dependencies are included
- [ ] Examples and templates work
- [ ] Build works on clean system

## 📁 Release Package Contents

### Portable ZIP Package
```
GameEngine_v1.0.0/
├── GameEditor.exe          # Visual editor
├── GameRuntime.exe         # Game runtime
├── *.dll                   # Dependencies (SDL2, etc.)
├── editor_config.json      # Editor configuration
├── imgui.ini              # GUI layout
├── assets/                # Game assets
├── docs/                  # Documentation
├── examples/              # Example projects
├── README.md              # Main readme
├── LICENSE.txt            # License file
├── QUICK_START.md         # Quick start guide
└── VERSION.txt            # Version info
```

### Windows Installer
- Professional NSIS installer
- Start menu shortcuts
- Desktop shortcuts
- Uninstaller
- Registry entries
- Component selection

## 🛠️ Installation Requirements

### For Building Releases
- Windows 10/11
- Visual Studio 2022
- vcpkg package manager
- PowerShell 5.0+
- NSIS (for installer creation)
- GitHub CLI (for automated releases)

### For End Users
- Windows 10/11
- Visual C++ Redistributable 2022
- DirectX 11 compatible graphics
- 4GB RAM minimum
- 500MB free disk space

## 🔧 Customization

### Version Configuration
Edit these files to update version numbers:
- `create_release.bat` - Set VERSION variable
- `installer\GameEngine.nsi` - Update APP_VERSION
- `scripts\create_github_release.ps1` - Update default version

### Installer Customization
Edit `installer\GameEngine.nsi` to modify:
- Installation components
- File associations
- Registry entries
- Shortcuts and icons
- License and readme

### Package Customization
Edit `create_release.bat` to modify:
- Included files and folders
- Package structure
- Generated documentation
- Compression settings

## 🐛 Troubleshooting

### Common Issues

**Build Fails**
- Ensure Visual Studio 2022 is installed
- Run `setup.bat` to initialize vcpkg
- Check that all dependencies are available

**Missing DLLs**
- Verify vcpkg packages are built correctly
- Check that DLL copying logic in CMake works
- Manually copy missing DLLs to output directory

**Installer Issues**
- Install NSIS from https://nsis.sourceforge.io/
- Ensure all referenced files exist
- Check installer script syntax

**GitHub Release Fails**
- Install GitHub CLI: https://cli.github.com/
- Authenticate: `gh auth login`
- Check repository permissions
- Verify token has release permissions

## 📝 Release Notes Template

```markdown
# 2D Game Engine v1.0.0

## 🚀 New Features
- Feature 1 description
- Feature 2 description

## 🐛 Bug Fixes
- Fix 1 description
- Fix 2 description

## 📦 Installation
Download the installer or portable version from the releases page.

## 🎯 System Requirements
- Windows 10/11
- DirectX 11 compatible graphics card
- 4GB RAM, 500MB disk space
```

## 🔄 Automated CI/CD (Future)

Consider setting up GitHub Actions for automated releases:

```yaml
# .github/workflows/release.yml
name: Create Release

on:
  push:
    tags:
      - 'v*'

jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build Project
        run: build.bat
      - name: Create Release
        run: create_release.bat
      - name: Upload Release Assets
        uses: actions/upload-release-asset@v1
```

## 📞 Support

For questions about the release process:
1. Check this guide first
2. Review the troubleshooting section
3. Create an issue on GitHub
4. Contact the development team

---

**Happy Releasing! 🎉**
