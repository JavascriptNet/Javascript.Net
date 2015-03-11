:: Build v8 in various modes
::
@SETLOCAL
@ECHO off

set args-abspath=%~dp0
SET args-arch=%1
SET args-vs=%2
SET args-toolset=%3
SET args-mode=%4
IF NOT DEFINED args-arch GOTO :helpAndExit
IF NOT DEFINED args-vs GOTO :helpAndExit
IF NOT DEFINED args-toolset GOTO :helpAndExit
IF NOT DEFINED args-mode GOTO :helpAndExit


:: Deal with build mode
IF /I "%args-mode%"=="Release" SET mode=Release
IF /I "%args-mode%"=="Debug" SET mode=Debug
IF NOT DEFINED mode GOTO :helpAndExit
echo Build mode: %mode%


:: Deal with architecture
IF "%args-arch%"=="x64" (
    SET "x64suffix=\x64"
    SET "adm64suffix=\amd64"
    SET "target_arch=x64"
    SET "profile=%mode%^|x64"
)
IF "%args-arch%"=="ia32" (
	SET "x64suffix="
	SET "adm64suffix="
	SET "target_arch=ia32"
	SET "profile=%mode%^|Win32"
)
IF NOT DEFINED target_arch GOTO :helpAndExit
echo Target architecture: %target_arch%
echo Build profile: %profile%


:: Deal with Visual Studio
IF DEFINED ProgramFiles(x86) (
	SET "vsPathPrefix=%ProgramFiles(x86)%"
) ELSE (
	SET "vsPathPrefix=%ProgramFiles%"
)

SET vs2008=%vsPathPrefix%\Microsoft Visual Studio 9.0
SET vs2010=%vsPathPrefix%\Microsoft Visual Studio 10.0
SET vs2012=%vsPathPrefix%\Microsoft Visual Studio 11.0
SET vs2013=%vsPathPrefix%\Microsoft Visual Studio 12.0
IF /I "%args-vs%"=="vs2008" SET vs=%vs2008%
IF /I "%args-vs%"=="vs2010" SET vs=%vs2010%
IF /I "%args-vs%"=="vs2012" SET vs=%vs2012%
IF /I "%args-vs%"=="vs2013" SET vs=%vs2013%
IF NOT DEFINED vs GOTO :helpAndExit
echo Using Visual Studio: %vs%


:: Deal with toolset
::
:: building v8 relies on %GYP_MSVS_VERSION%
SET "GYP_MSVS_VERSION=%args-toolset:~-4%"
echo GYP_MSVS_VERSION is set to: %GYP_MSVS_VERSION%


ECHO Building v8

:: drop custom python vars if any
SET PYTHONHOME=
SET PYTHONPATH=
SET PYTHON=

:: will use python from here if you don't have one
PATH %PATH%;%args-abspath%\depot_tools

:: Change to v8 directory.
IF NOT EXIST v8\LICENSE.v8 (
    ECHO Cannot find v8 checkout at ./v8
    EXIT /b 1
)
pushd v8

ECHO cleaning up
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
ECHO on
CALL python build\gyp_v8 -D"target_arch=%target_arch%" -D"component=shared_library" -Dv8_enable_i18n_support=0
"%vs%\Common7\IDE\devenv.com" /Build "%profile%" build\All.sln
@ECHO off
popd

ECHO Finished.
ENDLOCAL
EXIT /b 0

:helpAndExit
	ECHO usage: %0 Target_architecture VS_version MSVC_version Build_mode
	ECHO    Target_architecture:  ia32 ^| x64
	ECHO    VS_version:           vs2008 ^| vs2010 ^| vs2012 ^| vs2013
	ECHO    MSVC_version:         MSVC2008 ^| MSVC2010 ^| MSVC2012 ^| MSVC2013
	ECHO    Build_mode:           Debug ^| Release
	ECHO.
	ECHO Note: see README if you have troubles building with MSVC different from your VS version (e.g. vs2013 and MSVC2010)
	ENDLOCAL
	EXIT /b 1
