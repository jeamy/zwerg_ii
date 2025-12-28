@echo off
rem zwergII - Windows Release Build
rem Builds a Release binary and creates a minimal dist folder.
rem Uses windeployqt if available; otherwise falls back to a minimal manual copy.

setlocal ENABLEDELAYEDEXPANSION

set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%"

if /I "%~1"=="--debug" set "ZWERG_BAT_DEBUG=1"
if "%ZWERG_BAT_DEBUG%"=="1" echo on

set "PROJECT_DIR=%SCRIPT_DIR%"
set "BUILD_DIR=%PROJECT_DIR%build-windows-release"
set "DIST_DIR=%PROJECT_DIR%dist\windows"
set "BUILD_TYPE=Release"
set "BIN_NAME=DwarfController.exe"

rem Optional defaults for Qt (adjust to your setup or override in the calling shell)
rem Qt6_DIR: folder containing Qt6Config.cmake
rem CMAKE_PREFIX_PATH: Qt prefix (e.g. ...\mingw_64)
if not defined Qt6_DIR (
  set "Qt6_DIR=C:\Qt\6.10.1\mingw_64\lib\cmake\Qt6"
)
if not defined CMAKE_PREFIX_PATH (
  set "CMAKE_PREFIX_PATH=C:\Qt\6.10.1\mingw_64"
)
if not defined PROTOC_PREFIX_PATH (
  set "PROTOC_PREFIX_PATH=G:\Download\protoc"
)

echo === zwergII - Windows Release Build ===
echo.

rem ===========================================================================
rem [0] Dependency checks (no auto-install)
rem ===========================================================================
set "MISSING_DEPS="

where cmake >NUL 2>&1
if errorlevel 1 set MISSING_DEPS=!MISSING_DEPS! cmake

rem Compiler (MSVC cl.exe OR MinGW g++)
where cl >NUL 2>&1
if errorlevel 1 (
  where g++ >NUL 2>&1
  if errorlevel 1 set MISSING_DEPS=!MISSING_DEPS! compiler
)

rem Qt: qmake6 OR Qt6_DIR/CMAKE_PREFIX_PATH
where qmake6 >NUL 2>&1
if errorlevel 1 if not defined Qt6_DIR if not defined CMAKE_PREFIX_PATH set "MISSING_DEPS=!MISSING_DEPS! qt6"

rem protoc: in PATH OR PROTOC_PREFIX_PATH
where protoc >NUL 2>&1
if errorlevel 1 if not defined PROTOC_PREFIX_PATH set "MISSING_DEPS=!MISSING_DEPS! protoc"

if not "%MISSING_DEPS%"=="" (
  echo ERROR: Missing dependencies: %MISSING_DEPS%
  echo.
  echo Please install the missing components and retry.
  echo.
  echo Suggested installs:
  echo   - CMake: https://cmake.org/download/  or: winget install Kitware.CMake
  echo   - Qt6:   https://www.qt.io/download-qt-installer  or set Qt6_DIR / CMAKE_PREFIX_PATH
  echo   - Protobuf protoc: via vcpkg or official releases  or set PROTOC_PREFIX_PATH
  echo.
  exit /B 1
)

echo All dependencies found.
echo.

rem Add PROTOC_PREFIX_PATH to PATH if defined
if defined PROTOC_PREFIX_PATH (
  echo Adding PROTOC_PREFIX_PATH to PATH: %PROTOC_PREFIX_PATH%\bin
  set "PATH=%PROTOC_PREFIX_PATH%\bin;%PATH%"
)

rem ===========================================================================
rem [1] Configure
rem ===========================================================================
echo [1/3] Configure (CMake)...

set CMAKE_GENERATOR=
where g++ >NUL 2>&1
if not errorlevel 1 (
  echo Detected MinGW, using MinGW Makefiles generator
  set CMAKE_GENERATOR=-G "MinGW Makefiles"
) else (
  where cl >NUL 2>&1
  if not errorlevel 1 (
    echo Detected MSVC, using default Visual Studio generator
  )
)

cmake -S "%PROJECT_DIR%" -B "%BUILD_DIR%" %CMAKE_GENERATOR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 goto error

rem ===========================================================================
rem [2] Build
rem ===========================================================================
echo.
echo [2/3] Build...
cmake --build "%BUILD_DIR%" --config %BUILD_TYPE%
if errorlevel 1 goto error

rem ===========================================================================
rem [3] Dist
rem ===========================================================================
echo.
echo [3/3] Create dist directory...
if exist "%DIST_DIR%" rmdir /S /Q "%DIST_DIR%"
mkdir "%DIST_DIR%"

rem Copy binary
if exist "%BUILD_DIR%\%BIN_NAME%" (
  copy /Y "%BUILD_DIR%\%BIN_NAME%" "%DIST_DIR%" >NUL
) else (
  if exist "%BUILD_DIR%\%BUILD_TYPE%\%BIN_NAME%" (
    copy /Y "%BUILD_DIR%\%BUILD_TYPE%\%BIN_NAME%" "%DIST_DIR%" >NUL
  ) else (
    echo ERROR: Binary not found: %BIN_NAME%
    goto error
  )
)

rem Required runtime assets
mkdir "%DIST_DIR%\styles" 2>NUL
mkdir "%DIST_DIR%\i18n" 2>NUL
mkdir "%DIST_DIR%\data" 2>NUL

if exist "%PROJECT_DIR%styles\app.qss" copy /Y "%PROJECT_DIR%styles\app.qss" "%DIST_DIR%\styles" >NUL

for %%F in ("%PROJECT_DIR%i18n\*.qm") do (
  if exist "%%~fF" copy /Y "%%~fF" "%DIST_DIR%\i18n" >NUL
)

if exist "%PROJECT_DIR%data\stars.db" copy /Y "%PROJECT_DIR%data\stars.db" "%DIST_DIR%\data" >NUL
if exist "%PROJECT_DIR%data\constellationship.fab" copy /Y "%PROJECT_DIR%data\constellationship.fab" "%DIST_DIR%\data" >NUL

if exist "%PROJECT_DIR%resources" (
  xcopy "%PROJECT_DIR%resources" "%DIST_DIR%\resources" /E /I /Y >NUL
)

rem Qt deployment
where windeployqt >NUL 2>&1
if not errorlevel 1 (
  echo Running windeployqt...
  pushd "%DIST_DIR%"
  windeployqt "%DIST_DIR%\%BIN_NAME%" --release --no-translations
  popd
) else (
  echo NOTE: windeployqt not found - falling back to minimal manual Qt copy.
  set "QT_PREFIX=%CMAKE_PREFIX_PATH%"
  set "QT_BIN=%QT_PREFIX%\bin"

  if exist "%QT_BIN%\Qt6Core.dll" (
    echo Copying Qt runtime DLLs...
    for %%D in (Qt6Core.dll Qt6Gui.dll Qt6Widgets.dll Qt6Network.dll Qt6WebSockets.dll Qt6Multimedia.dll Qt6MultimediaWidgets.dll Qt6Sql.dll) do (
      if exist "%QT_BIN%\%%D" copy /Y "%QT_BIN%\%%D" "%DIST_DIR%" >NUL
    )

    mkdir "%DIST_DIR%\platforms" 2>NUL
    if exist "%QT_PREFIX%\plugins\platforms\qwindows.dll" (
      copy /Y "%QT_PREFIX%\plugins\platforms\qwindows.dll" "%DIST_DIR%\platforms" >NUL
    )

    if exist "%QT_PREFIX%\plugins\imageformats" (
      mkdir "%DIST_DIR%\imageformats" 2>NUL
      xcopy "%QT_PREFIX%\plugins\imageformats\*.dll" "%DIST_DIR%\imageformats" /Y >NUL
    )

    if exist "%QT_PREFIX%\plugins\styles" (
      mkdir "%DIST_DIR%\styles-qt" 2>NUL
      xcopy "%QT_PREFIX%\plugins\styles\*.dll" "%DIST_DIR%\styles-qt" /Y >NUL
    )

    if exist "%QT_PREFIX%\plugins\sqldrivers" (
      mkdir "%DIST_DIR%\sqldrivers" 2>NUL
      xcopy "%QT_PREFIX%\plugins\sqldrivers\*.dll" "%DIST_DIR%\sqldrivers" /Y >NUL
    )
  ) else (
    echo WARNING: Qt6Core.dll not found under %QT_BIN% - check CMAKE_PREFIX_PATH.
  )
)

rem Optional zip via PowerShell
set ZIP_NAME=zwergII-windows-release.zip
where powershell >NUL 2>&1
if errorlevel 1 (
  echo NOTE: PowerShell not found - zip will not be created.
) else (
  echo Creating release zip: %ZIP_NAME%
  pushd "%PROJECT_DIR%dist"
  if exist "%ZIP_NAME%" del /F /Q "%ZIP_NAME%"
  powershell -NoLogo -NoProfile -Command "Compress-Archive -Path 'windows\*' -DestinationPath '%ZIP_NAME%' -Force"
  popd
  echo Zip created: %PROJECT_DIR%dist\%ZIP_NAME%
)

echo.
echo ========================================
echo   Release build finished
echo ========================================
echo.
echo Run:
echo   %DIST_DIR%\%BIN_NAME%
echo.

goto :eof

:error
echo.
echo Build failed.
popd
exit /B 1

endlocal
