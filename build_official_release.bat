@echo off
echo 2D Game Engine - Official Release Builder
echo ==========================================
echo.

:: Configuration
set VERSION=1.0.0
set GITHUB_REPO=yourusername/game-engine
set RELEASE_TITLE=2D Game Engine v%VERSION% - Official Release

echo Version: %VERSION%
echo Repository: %GITHUB_REPO%
echo.

:: Step 1: Build the project
echo [1/5] Building project...
echo -------------------------
call build.bat
if errorlevel 1 (
    echo Error: Build failed!
    pause
    exit /b 1
)
echo ✅ Build completed successfully!
echo.

:: Step 2: Run tests (if available)
echo [2/5] Running tests...
echo ----------------------
if exist test_all.bat (
    call test_all.bat
    if errorlevel 1 (
        echo Warning: Some tests failed. Continue anyway? (Y/N)
        set /p CONTINUE=
        if /i not "%CONTINUE%"=="Y" exit /b 1
    )
    echo ✅ Tests completed!
) else (
    echo ℹ️  No test suite found, skipping tests.
)
echo.

:: Step 3: Create release packages
echo [3/5] Creating release packages...
echo -----------------------------------
call create_release.bat
if errorlevel 1 (
    echo Error: Release package creation failed!
    pause
    exit /b 1
)
echo ✅ Release packages created!
echo.

:: Step 4: Verify release files
echo [4/5] Verifying release files...
echo --------------------------------
set RELEASE_DIR=release
set PACKAGE_NAME=GameEngine_v%VERSION%

if not exist "%RELEASE_DIR%\%PACKAGE_NAME%_Portable.zip" (
    echo Error: Portable ZIP not found!
    pause
    exit /b 1
)

if exist "%RELEASE_DIR%\GameEngine_v%VERSION%_Setup.exe" (
    echo ✅ Found installer and portable package
) else (
    echo ⚠️  Found portable package only (no installer)
)

:: Check package contents
echo.
echo Package contents:
powershell -command "Get-ChildItem -Path '%RELEASE_DIR%\%PACKAGE_NAME%' -Recurse | Measure-Object | Select-Object Count"
echo.

:: Step 5: GitHub release (optional)
echo [5/5] GitHub Release Options
echo -----------------------------
echo.
echo Would you like to create a GitHub release? (Y/N)
set /p CREATE_GITHUB_RELEASE=

if /i "%CREATE_GITHUB_RELEASE%"=="Y" (
    echo.
    echo GitHub Release Setup:
    echo ---------------------
    echo 1. Make sure you have GitHub CLI installed (gh)
    echo 2. Make sure you're authenticated with GitHub
    echo 3. Have your GitHub token ready
    echo.
    echo Continue with GitHub release? (Y/N)
    set /p GITHUB_CONTINUE=
    
    if /i "%GITHUB_CONTINUE%"=="Y" (
        echo.
        set /p GITHUB_TOKEN=Enter your GitHub token: 
        
        echo Creating GitHub release...
        powershell -ExecutionPolicy Bypass -File "scripts\create_github_release.ps1" -GitHubToken "%GITHUB_TOKEN%" -Repository "%GITHUB_REPO%" -Version "%VERSION%"
        
        if errorlevel 1 (
            echo ⚠️  GitHub release creation failed, but local packages are ready
        ) else (
            echo ✅ GitHub release created successfully!
        )
    )
)

:: Summary
echo.
echo ==========================================
echo 🎉 Release v%VERSION% Ready!
echo ==========================================
echo.
echo 📦 Release files created:
echo   - %RELEASE_DIR%\%PACKAGE_NAME%_Portable.zip
if exist "%RELEASE_DIR%\GameEngine_v%VERSION%_Setup.exe" echo   - %RELEASE_DIR%\GameEngine_v%VERSION%_Setup.exe
echo.
echo 🚀 Next steps:
echo   1. Test the release packages on a clean system
echo   2. Upload to GitHub releases (if not done automatically)
echo   3. Update documentation with release notes
echo   4. Announce the release to your community!
echo.

:: Open release folder
echo Opening release folder...
explorer "%RELEASE_DIR%"

echo.
echo Release creation complete!
pause
