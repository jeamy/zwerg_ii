@echo off
rem zwergII - Windows Release Build
rem Builds a Release binary and creates a minimal dist folder.
rem Uses windeployqt if available; otherwise falls back to a minimal manual copy.

setlocal ENABLEDELAYEDEXPANSION

set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%"

if /I "%~1"=="--debug" set "ZWERG_BAT_DEBUG=1"
if /I "%~1"=="--install" set "ZWERG_BAT_INSTALL=1"
if /I "%~2"=="--install" set "ZWERG_BAT_INSTALL=1"
if not defined ZWERG_BAT_INSTALL set "ZWERG_BAT_INSTALL=0"
if "%ZWERG_BAT_DEBUG%"=="1" echo on

set "PROJECT_DIR=%SCRIPT_DIR%"
if "%PROJECT_DIR:~-1%"=="\" set "PROJECT_DIR=%PROJECT_DIR:~0,-1%"
set "BUILD_DIR=%PROJECT_DIR%\build-windows-release"
set "DIST_DIR=%PROJECT_DIR%\dist\windows"
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
if not defined Qt6WebSockets_DIR (
  set "Qt6WebSockets_DIR=%CMAKE_PREFIX_PATH%\lib\cmake\Qt6WebSockets"
)
if not defined PROTOC_PREFIX_PATH (
  set "PROTOC_PREFIX_PATH=G:\Download\protoc"
)
if not defined PROTOBUF_PREFIX_PATH (
  set "PROTOBUF_PREFIX_PATH=%PROTOC_PREFIX_PATH%"
)
if not defined Protobuf_INCLUDE_DIR (
  set "Protobuf_INCLUDE_DIR=%PROTOBUF_PREFIX_PATH%\include"
)
if not defined Protobuf_PROTOC_EXECUTABLE (
  set "Protobuf_PROTOC_EXECUTABLE=%PROTOC_PREFIX_PATH%\bin\protoc.exe"
)
if not defined Protobuf_LIBRARY (
  if exist "%PROTOBUF_PREFIX_PATH%\lib\libprotobuf.dll.a" set "Protobuf_LIBRARY=%PROTOBUF_PREFIX_PATH%\lib\libprotobuf.dll.a"
  if not defined Protobuf_LIBRARY if exist "%PROTOBUF_PREFIX_PATH%\lib\libprotobuf.a" set "Protobuf_LIBRARY=%PROTOBUF_PREFIX_PATH%\lib\libprotobuf.a"
  if not defined Protobuf_LIBRARY if exist "%PROTOBUF_PREFIX_PATH%\lib\protobuf.lib" set "Protobuf_LIBRARY=%PROTOBUF_PREFIX_PATH%\lib\protobuf.lib"
)
if not defined Vulkan_INCLUDE_DIR (
  if defined VULKAN_SDK (
    set "Vulkan_INCLUDE_DIR=%VULKAN_SDK%\Include"
  )
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

rem Qt WebSockets: needed by find_package(Qt6 COMPONENTS WebSockets)
if exist "%Qt6WebSockets_DIR%\Qt6WebSocketsConfig.cmake" (
  rem ok
) else (
  set "MISSING_DEPS=!MISSING_DEPS! qt6websockets"
)

rem protoc: in PATH OR PROTOC_PREFIX_PATH
where protoc >NUL 2>&1
if errorlevel 1 if not defined PROTOC_PREFIX_PATH set "MISSING_DEPS=!MISSING_DEPS! protoc"

rem Protobuf SDK (headers + libs) needed by find_package(Protobuf)
if exist "%Protobuf_INCLUDE_DIR%\google\protobuf\message.h" (
  rem ok
) else (
  set "MISSING_DEPS=!MISSING_DEPS! protobuf-headers"
)
if defined Protobuf_LIBRARY (
  rem ok
) else (
  set "MISSING_DEPS=!MISSING_DEPS! protobuf-lib"
)

rem Vulkan headers (optional for runtime, but required by some Qt/CMake configs)
if defined Vulkan_INCLUDE_DIR (
  if not "%Vulkan_INCLUDE_DIR%"=="" (
    if exist "%Vulkan_INCLUDE_DIR%\vulkan\vulkan.h" (
      rem ok
    ) else (
      set "MISSING_DEPS=!MISSING_DEPS! vulkan"
    )
  ) else (
    set "MISSING_DEPS=!MISSING_DEPS! vulkan"
  )
) else (
  set "MISSING_DEPS=!MISSING_DEPS! vulkan"
)

if not "%MISSING_DEPS%"=="" (
  echo ERROR: Missing dependencies: %MISSING_DEPS%
  echo.
  echo Please install the missing components and retry.
  echo.
  echo Suggested installs:
  echo   - CMake: https://cmake.org/download/  or: winget install Kitware.CMake
  echo   - Qt6:   https://www.qt.io/download-qt-installer  or set Qt6_DIR / CMAKE_PREFIX_PATH
  echo   - Protobuf protoc: via vcpkg or official releases  or set PROTOC_PREFIX_PATH
  echo   - Protobuf libs: install protobuf development package via vcpkg or MSYS2 and set PROTOBUF_PREFIX_PATH.
  echo   - Vulkan headers: install Vulkan SDK and set VULKAN_SDK.
  echo.
  if "%ZWERG_BAT_INSTALL%"=="1" (
    where winget >NUL 2>&1
    if errorlevel 1 (
      echo Install mode requested but winget was not found.
      echo.
    ) else (
      echo Interactive install mode enabled.
      echo.
      echo Selected packages will be installed using winget.
      echo.
      echo Note: Qt and Protobuf libs are not auto-installed here.
      echo.
      echo.
      echo Installing optional packages.
      echo.
      echo.
      if not "x%MISSING_DEPS:cmake=%"=="x%MISSING_DEPS%" call :maybe_install "CMake" "Kitware.CMake"
      if not "x%MISSING_DEPS:vulkan=%"=="x%MISSING_DEPS%" call :maybe_install "Vulkan SDK" "KhronosGroup.VulkanSDK"
      echo.
    )
  )
  echo Details for missing items:
  echo.
  if not "x%MISSING_DEPS:cmake=%"=="x%MISSING_DEPS%" (
    echo cmake: install via winget install Kitware.CMake or https://cmake.org/download/
  )
  if not "x%MISSING_DEPS:compiler=%"=="x%MISSING_DEPS%" (
    echo compiler: install MinGW via MSYS2 or MSVC Build Tools.
    echo compiler: MSYS2 https://www.msys2.org/ then pacman -S mingw-w64-x86_64-gcc
    echo compiler: MSVC https://visualstudio.microsoft.com/downloads/ then Build Tools.
  )
  if not "x%MISSING_DEPS:qt6=%"=="x%MISSING_DEPS%" (
    echo qt6: install Qt 6 and set CMAKE_PREFIX_PATH or Qt6_DIR.
  )
  if not "x%MISSING_DEPS:qt6websockets=%"=="x%MISSING_DEPS%" (
    echo qt6websockets: install Qt WebSockets component in Qt Maintenance Tool.
    echo qt6websockets: expected %Qt6WebSockets_DIR%\Qt6WebSocketsConfig.cmake
  )
  if not "x%MISSING_DEPS:protoc=%"=="x%MISSING_DEPS%" (
    echo protoc: download protoc and set PROTOC_PREFIX_PATH to the folder containing bin\protoc.exe
  )
  if not "x%MISSING_DEPS:protobuf-headers=%"=="x%MISSING_DEPS%" (
    echo protobuf headers: need include\google\protobuf\message.h under PROTOBUF_PREFIX_PATH
  )
  if not "x%MISSING_DEPS:protobuf-lib=%"=="x%MISSING_DEPS%" (
    echo protobuf libs: need libprotobuf library under PROTOBUF_PREFIX_PATH\lib
    echo protobuf libs: easiest via vcpkg install protobuf or MSYS2 mingw-w64-x86_64-protobuf
  )
  if not "x%MISSING_DEPS:vulkan=%"=="x%MISSING_DEPS%" (
    echo vulkan: install Vulkan SDK and set VULKAN_SDK. Header should be at %%VULKAN_SDK%%\Include\vulkan\vulkan.h
  )
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

set "CMAKE_GEN="
where g++ >NUL 2>&1
if not errorlevel 1 (
  echo Detected MinGW, using MinGW Makefiles generator
  set "CMAKE_GEN=MinGW Makefiles"
) else (
  where cl >NUL 2>&1
  if not errorlevel 1 (
    echo Detected MSVC, using default Visual Studio generator
  )
)

if defined CMAKE_GEN (
  if defined Vulkan_INCLUDE_DIR (
    cmake -S "%PROJECT_DIR%" -B "%BUILD_DIR%" -G "%CMAKE_GEN%" -DQt6WebSockets_DIR="%Qt6WebSockets_DIR%" -DVulkan_INCLUDE_DIR="%Vulkan_INCLUDE_DIR%" -DProtobuf_PROTOC_EXECUTABLE="%Protobuf_PROTOC_EXECUTABLE%" -DProtobuf_INCLUDE_DIR="%Protobuf_INCLUDE_DIR%" -DProtobuf_LIBRARY="%Protobuf_LIBRARY%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
  ) else (
    cmake -S "%PROJECT_DIR%" -B "%BUILD_DIR%" -G "%CMAKE_GEN%" -DQt6WebSockets_DIR="%Qt6WebSockets_DIR%" -DProtobuf_PROTOC_EXECUTABLE="%Protobuf_PROTOC_EXECUTABLE%" -DProtobuf_INCLUDE_DIR="%Protobuf_INCLUDE_DIR%" -DProtobuf_LIBRARY="%Protobuf_LIBRARY%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
  )
) else (
  if defined Vulkan_INCLUDE_DIR (
    cmake -S "%PROJECT_DIR%" -B "%BUILD_DIR%" -DQt6WebSockets_DIR="%Qt6WebSockets_DIR%" -DVulkan_INCLUDE_DIR="%Vulkan_INCLUDE_DIR%" -DProtobuf_PROTOC_EXECUTABLE="%Protobuf_PROTOC_EXECUTABLE%" -DProtobuf_INCLUDE_DIR="%Protobuf_INCLUDE_DIR%" -DProtobuf_LIBRARY="%Protobuf_LIBRARY%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
  ) else (
    cmake -S "%PROJECT_DIR%" -B "%BUILD_DIR%" -DQt6WebSockets_DIR="%Qt6WebSockets_DIR%" -DProtobuf_PROTOC_EXECUTABLE="%Protobuf_PROTOC_EXECUTABLE%" -DProtobuf_INCLUDE_DIR="%Protobuf_INCLUDE_DIR%" -DProtobuf_LIBRARY="%Protobuf_LIBRARY%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
  )
)
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

:maybe_install
set "PKG_NAME=%~1"
set "WINGET_ID=%~2"
echo.
echo Install %PKG_NAME% now?
choice /C YN /N /M "[Y]es / [N]o: "
if errorlevel 2 goto :eof
echo Running: winget install --id %WINGET_ID% -e
winget install --id %WINGET_ID% -e
goto :eof

:error
echo.
echo Build failed.
popd
exit /B 1

endlocal
