:: Build both V8 and Javascript.NET in various modes
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


call build_v8 %1 %2 %3 %4
call build_jsNet %1 %2 %3 %4 %5

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
