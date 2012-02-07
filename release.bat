@echo off
:: See release instructions in README.txt.
::
:: Copies built binaries and C Runtime Library to a directory ready for
:: ZIPping.
if "%1"=="" goto usage
if "%2"=="" goto usage
set reldir="Noesis.Javascript %1 - Binaries for .NET%2"

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
    copy x64\Release\Noesis.Javascript.dll %reldir\x64
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
echo usage: release relnum .NetVer
echo e.g. release 0.5 3.5
echo e.g. release 0.5 4.0
:end
