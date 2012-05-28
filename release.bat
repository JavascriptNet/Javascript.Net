@echo off
:: Release Procedure
:: -----------------
::  1. Consider updating to a newer v8 branch (and update README.txt to refer to it)
::  2. Run Release.bat X.X
::  3. Zip up the created directory to Noesis.Javascript vX.X - Binaries.zip
::
:: Notes:
:: - We do not tag releases because svnbridge is too dumb for that.
:: - VS2010 projects created by upgrading VS2008 projects.
:: - Ignore warnings like
::   JavascriptInterop.obj : warning LNK4248: unresolved typeref token (0100002A) for 'v8.internal.Object'; image may not run

if "%1"=="" (
    echo See instructions at the top of this file.
    echo usage: release relnum
    echo e.g. release 0.6
    exit /b 1
)

:: Delete any existing files and create directory structure.
set reldir="Noesis.Javascript v%1 - Binaries"
if exist %reldir% rmdir /s /q %reldir%
mkdir %reldir%
if errorlevel 1 goto error
mkdir %reldir%\.Net3.5
mkdir %reldir%\.Net4.0
mkdir %reldir%\.Net3.5\x86
mkdir %reldir%\.Net3.5\x64
mkdir %reldir%\.Net4.0\x86
mkdir %reldir%\.Net4.0\x64

:: Build.
cmd /c buildv8 ia32 vs2008 release
if errorlevel 1 goto error
C:\Windows\Microsoft.NET\Framework\v3.5\MSBuild.exe /t:Rebuild Noesis.Javascript.VS2008.sln /p:Configuration=Release /p:Platform=Win32
if errorlevel 1 goto error
cmd /c buildv8 x64 vs2008 release
if errorlevel 1 goto error
C:\Windows\Microsoft.NET\Framework\v3.5\MSBuild.exe /t:Rebuild Noesis.Javascript.VS2008.sln /p:Configuration=Release /p:Platform=x64
if errorlevel 1 goto error
cmd /c buildv8 ia32 vs2010 release
if errorlevel 1 goto error
C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe /t:Rebuild Noesis.Javascript.VS2010.sln /p:Configuration=Release /p:Platform=Win32
if errorlevel 1 goto error
cmd /c buildv8 x64 vs2010 release
if errorlevel 1 goto error
C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe /t:Rebuild Noesis.Javascript.VS2010.sln /p:Configuration=Release /p:Platform=x64
if errorlevel 1 goto error

:: Copy files across.
copy README.txt %reldir%
if errorlevel 1 goto error
copy Release\Noesis.Javascript.dll %reldir%\.Net3.5\x86
if errorlevel 1 goto error
copy x64\Release\Noesis.Javascript.dll %reldir%\.Net3.5\x64
if errorlevel 1 goto error
copy Release\VS2010\Noesis.Javascript.dll %reldir%\.Net4.0\x86
if errorlevel 1 goto error
copy x64\VS2010\Release\Noesis.Javascript.dll %reldir%\.Net4.0\x64
if errorlevel 1 goto error
copy "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\*.*" %reldir%\.Net3.5\x86
copy "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\amd64\Microsoft.VC90.CRT\*.*" %reldir%\.Net3.5\x64
copy "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\*.*" %reldir%\.Net4.0\x86
copy "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\*.*" %reldir%\.Net4.0\x64

goto end

:error
echo Build aborted
goto end
:usage
:end
