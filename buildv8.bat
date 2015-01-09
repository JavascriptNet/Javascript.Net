:: Build v8 in various modes, using specific versions of Visual Studio.
::
:: This script will trash bits of your environment, particularly your PATH.
@ECHO OFF
SET "usage=usage: %0 Debug/Release ia32/x64"

:: Set directory locations here.
SET python=C:\Python27
SET vs=C:\Program Files (x86)\Microsoft Visual Studio 12.0
SET "GYP_MSVS_VERSION=2013"
SET sdk=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A

echo using sdk %sdk%

SET v8=%~dp0%\v8

:: Allow building in just one mode.
IF "%1"=="Release" (
    SET mode=Release
) ELSE (
    SET mode=Debug
)

:: Parameterise specified architecture.
IF "%2"=="x64" (
    SET "x64suffix=\x64"
    SET "adm64suffix=\amd64"
    SET "target_arch=x64"
    SET "profile=%mode%|x64"
) ELSE (
    IF "%2"=="ia32" (
        SET "x64suffix="
        SET "adm64suffix="
        SET "target_arch=ia32"
        SET "profile=%mode%|Win32"
    ) ELSE (
        ECHO %usage%
        EXIT /b 1
    )
)

:: Change to v8 directory.
IF NOT EXIST %v8%\LICENSE.v8 (
    ECHO Cannot find v8 checkout at %v8%
    EXIT /b 1
)
CD %v8%

:: Have found that sometimes x64 debug builds get stuck on writing to vc100.pdb,
:: so we clean it up.  Also vs2013 builds sometimes output gobbledegook.
:: We delete all outputs beforehand to force it to be rebuilt.

RMDIR /S /Q obj  >nul 2>&1
RMDIR build\debug /s /q  >nul 2>&1
RMDIR build\release /s /q  >nul 2>&1
DEL *.idb  >nul 2>&1
DEL *.pdb  >nul 2>&1
DEL *.lib  >nul 2>&1

DEL /S *.sln  >nul 2>&1
DEL /S *.vcproj  >nul 2>&1
DEL /S *.vcxproj  >nul 2>&1
DEL /S *.vcxproj.user  >nul 2>&1

:: Run gyp to update the Visual Studio project files to contain links
:: to the latest v8 source files.
echo third_party\python_26\python.exe build\gyp_v8 -D"target_arch=%target_arch%" -D"component=shared_library" -Dv8_enable_i18n_support=0
third_party\python_26\python.exe build\gyp_v8 -D"target_arch=%target_arch%" -D"component=shared_library" -Dv8_enable_i18n_support=0

echo "%vs%\Common7\IDE\devenv.com" /Build "%profile%" build\All.sln
"%vs%\Common7\IDE\devenv.com" /Build "%profile%" build\All.sln

CD ..