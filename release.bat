@echo off
:: Release Procedure
:: -----------------
::  1. Checkout a fresh copy of the source.
::  2. Delete its .svn directory.
::  3. Rename directory to Noesis.Javascript-vX.X-Source
::  4. Zip up this directory to Noesis.Javascript-vX.X-Source.zip
::  5. Using Visual Studio 2008, build both Win32/x86 and x64 in Release mode.
::  6. Run Release.bat X.X 3.5
::  7. Zip up the created directory to Noesis.Javascript vX.X - Binaries for .NET4.0.zip
::  8. Upgrade to Visual Studio 2010 (see README.txt) and repeat
::  9. REbuild Win32/x86 in Release mode.
:: 10. Run Release.bat X.X 4.0
:: 11. Zip up the created directory to Noesis.Javascript vX.X - Binaries for .NET3.5.zip
::
:: We do not tag releases because svnbridge is too dumb for that.

:: Copies built binaries and C Runtime Library to a directory ready for
:: ZIPping.
if "%1"=="" goto usage
if "%2"=="" goto usage
set reldir="Noesis.Javascript v%1 - Binaries for .NET%2"

:: Delete any existing files and create directory structure.
if exist %reldir% del /s /q %reldir%
if not exist %reldir% mkdir %reldir%
if not exist %reldir%\x86 mkdir %reldir%\x86
if not "%2"=="4.0" (
    :: Cannot build x64 on .Net 4.0
    if not exist %reldir%\x64 mkdir %reldir%\x64
)

:: Copy our DLL and C runtime libraries in.
copy Release\Noesis.Javascript.dll %reldir%\x86
if errorlevel 1 goto error
if not "%2"=="4.0" (
    copy x64\Release\Noesis.Javascript.dll %reldir%\x64
    if errorlevel 1 goto error
)
if "%2"=="4.0" (
    copy "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\*.*" %reldir%\x86
) else (
    copy "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\*.*" %reldir%\x86
    copy "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\amd64\Microsoft.VC90.CRT\*.*" %reldir%\x64
)

goto end

:error
echo Build aborted
goto end
:usage
echo See instructions at the top of this file.
echo usage: release relnum .NetVer
echo e.g. release 0.5 3.5
echo e.g. release 0.5 4.0
:end
