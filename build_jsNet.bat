:: Build Javascript.NET in various modes
::
@SETLOCAL
@ECHO off

SET args-arch=%1
SET args-vs=%2
SET args-toolset=%3
SET args-mode=%4
SET args-framework=%5
IF NOT DEFINED args-arch GOTO :helpAndExit
IF NOT DEFINED args-vs GOTO :helpAndExit
IF NOT DEFINED args-toolset GOTO :helpAndExit
IF NOT DEFINED args-mode GOTO :helpAndExit
IF NOT DEFINED args-framework GOTO :helpAndExit


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

SET vs2010=%vsPathPrefix%\Microsoft Visual Studio 10.0
SET vs2012=%vsPathPrefix%\Microsoft Visual Studio 11.0
SET vs2013=%vsPathPrefix%\Microsoft Visual Studio 12.0
IF /I "%args-vs%"=="vs2010" SET vs=%vs2010%
IF /I "%args-vs%"=="vs2012" SET vs=%vs2012%
IF /I "%args-vs%"=="vs2013" SET vs=%vs2013%
IF NOT DEFINED vs GOTO :helpAndExit
echo Using Visual Studio: %vs%


:: Deal with toolset
::
:: building Javascript.NET relies on %PlatformToolset%
SET msvc2010=v100
SET msvc2012=v110
SET msvc2013=v120
IF /I "%args-toolset%"=="msvc2010" SET PlatformToolset=%msvc2010%
IF /I "%args-toolset%"=="msvc2012" SET PlatformToolset=%msvc2012%
IF /I "%args-toolset%"=="msvc2013" SET PlatformToolset=%msvc2013%
IF NOT DEFINED PlatformToolset GOTO :helpAndExit
echo PlatformToolset is set to %PlatformToolset%


:: Deal with framework
SET net40=4.0
SET net45=4.5
SET net451=4.5.1
SET net452=4.5.2
IF /I "%args-framework%"=="%net40%" (
	SET TargetFrameworkVersion=v%net40%
	SET TargetFrameworkProfile=Client
)
IF /I "%args-framework%"=="%net45%" (
	SET TargetFrameworkVersion=v%net45%
	SET TargetFrameworkProfile=
)
IF /I "%args-framework%"=="%net451%" (
	SET TargetFrameworkVersion=v%net451%
	SET TargetFrameworkProfile=
)
IF /I "%args-framework%"=="%net452%" (
	SET TargetFrameworkVersion=v%net452%
	SET TargetFrameworkProfile=
)
IF NOT DEFINED TargetFrameworkVersion GOTO :helpAndExit
echo TargetFrameworkVersion is set to %TargetFrameworkVersion%


ECHO Building Javascript.NET
ECHO on
"%vs%\Common7\IDE\devenv.com" /Rebuild "%profile%" Noesis.Javascript.%args-vs%.sln
@ECHO off

ECHO Finished. Check the ./Build/ directory
ENDLOCAL
EXIT /b 0

:helpAndExit
	ECHO usage: %0 Target_architecture VS_version MSVC_version Build_mode Framework_version
	ECHO    Target_architecture:  ia32 ^| x64
	ECHO    VS_version:           vs2010 ^| vs2012 ^| vs2013
	ECHO    MSVC_version:         MSVC2010 ^| MSVC2012 ^| MSVC2013
	ECHO    Build_mode:           Debug ^| Release
	ECHO    Framework_version:    4.0 ^| 4.5 ^| 4.5.1 ^| 4.5.2
	ECHO.
	ECHO Note: see README if you have troubles building with MSVC different from your VS version (e.g. vs2013 and MSVC2010)
	ENDLOCAL
	EXIT /b 1
