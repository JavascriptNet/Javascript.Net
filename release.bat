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
:: - VS2012 projects created by upgrading VS2010 projects.
:: - Ignore warnings like
::   JavascriptInterop.obj : warning LNK4248: unresolved typeref token (0100002A) for 'v8.internal.Object'; image may not run

if "%VC_PROJECT_ENGINE_NOT_USING_REGISTRY_FOR_INIT%"=="" (
	echo Add environment variable VC_PROJECT_ENGINE_NOT_USING_REGISTRY_FOR_INIT=1
)

if "%1"=="" (
    echo See instructions at the top of this file.
    echo usage: release relnum
    echo e.g. release 0.6
    exit /b 1
)

:: Build.
cmd /c buildv8 ia32 vs2012 v90 release
if errorlevel 1 goto error
C:\Windows\Microsoft.NET\Framework\v3.5\MSBuild.exe /t:Rebuild Noesis.Javascript.VS2008.sln /m /p:Configuration=Release /p:Platform=Win32
if errorlevel 1 goto error
cmd /c buildv8 x64 vs2008 v90 release
if errorlevel 1 goto error
C:\Windows\Microsoft.NET\Framework\v3.5\MSBuild.exe /t:Rebuild Noesis.Javascript.VS2008.sln /m /p:Configuration=Release /p:Platform=x64
if errorlevel 1 goto error
cmd /c buildv8 ia32 vs2012 v100 release
if errorlevel 1 goto error
C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe /t:Rebuild Noesis.Javascript.VS2010.sln /m /p:Configuration=Release /p:Platform=Win32
if errorlevel 1 goto error
cmd /c buildv8 x64 vs2012 v100 release
if errorlevel 1 goto error
C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe /t:Rebuild Noesis.Javascript.VS2010.sln /m /p:Configuration=Release /p:Platform=x64
if errorlevel 1 goto error
cmd /c buildv8 ia32 vs2012 v110 release
if errorlevel 1 goto error
C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe /t:Rebuild Noesis.Javascript.VS2012.sln /m /p:Configuration=Release /p:Platform=Win32 /p:VisualStudioVersion=11.0
if errorlevel 1 goto error
cmd /c buildv8 x64 vs2012 v110 release
if errorlevel 1 goto error
C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe /t:Rebuild Noesis.Javascript.VS2012.sln /m /p:Configuration=Release /p:Platform=x64 /p:VisualStudioVersion=11.0
if errorlevel 1 goto error

:: Delete any existing files and create directory structure.
set reldir="Noesis.Javascript v%1 - Binaries"
if exist %reldir% rmdir /s /q "%reldir%"
mkdir "%reldir%"
if errorlevel 1 goto error

mkdir "%reldir%\lib"
mkdir "%reldir%\lib\Net35"
mkdir "%reldir%\lib\Net40"
mkdir "%reldir%\lib\Net45"
mkdir "%reldir%\lib\Net35\x86"
mkdir "%reldir%\lib\Net35\amd64"
mkdir "%reldir%\lib\Net40\x86"
mkdir "%reldir%\lib\Net40\amd64"
mkdir "%reldir%\lib\Net45\x86"
mkdir "%reldir%\lib\Net45\amd64"

:: Copy files across.
copy README.txt "%reldir%"
if errorlevel 1 goto error
copy "VS2008\Win32\Release\bin\Noesis.Javascript.dll" "%reldir%\lib\Net35\x86\"
if errorlevel 1 goto error
copy "VS2008\x64\Release\bin\Noesis.Javascript.dll" "%reldir%\lib\Net35\amd64\"
if errorlevel 1 goto error
copy "VS2010\Win32\Release\bin\Noesis.Javascript.dll" "%reldir%\lib\Net40\x86\"
if errorlevel 1 goto error
copy "VS2010\x64\Release\bin\Noesis.Javascript.dll" "%reldir%\lib\net40\amd64\"
if errorlevel 1 goto error
copy "VS2012\Win32\Release\bin\Noesis.Javascript.dll" "%reldir%\lib\Net45\x86\"
if errorlevel 1 goto error
copy "VS2012\x64\Release\bin\Noesis.Javascript.dll" "%reldir%\lib\Net45\amd64\"
if errorlevel 1 goto error


mkdir "%reldir%\NativeBinaries"
mkdir "%reldir%\NativeBinaries\Net35"
mkdir "%reldir%\NativeBinaries\Net40"
mkdir "%reldir%\NativeBinaries\Net45"
mkdir "%reldir%\NativeBinaries\Net35\x86"
mkdir "%reldir%\NativeBinaries\Net35\amd64"
mkdir "%reldir%\NativeBinaries\Net40\x86"
mkdir "%reldir%\NativeBinaries\Net40\amd64"
mkdir "%reldir%\NativeBinaries\Net45\x86"
mkdir "%reldir%\NativeBinaries\Net45\amd64"

copy "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\*.*" "%reldir%\NativeBinaries\Net35\x86\"
copy "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\amd64\Microsoft.VC90.CRT\*.*" "%reldir%\NativeBinaries\Net35\amd64\"
copy "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\*.*" "%reldir%\NativeBinaries\Net40\x86\"
copy "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\*.*" "%reldir%\NativeBinaries\net40\amd64\"
copy "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\redist\x86\Microsoft.VC110.CRT\*.*" "%reldir%\NativeBinaries\Net45\x86\"
copy "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\redist\x64\Microsoft.VC110.CRT\*.*" "%reldir%\NativeBinaries\Net45\amd64\"

mkdir "%reldir%\Tools"
mkdir "%reldir%\Tools\Net35"
mkdir "%reldir%\Tools\Net40"
mkdir "%reldir%\Tools\Net45"

copy "Tools\Net35\*.*" "%reldir%\Tools\Net35\"
copy "Tools\Net40\*.*" "%reldir%\Tools\Net40\"
copy "Tools\Net45\*.*" "%reldir%\Tools\Net45\"

copy "Noesis.Javascript.nuspec" "%reldir%\"
goto end

:error
echo Build aborted
goto end
:usage
:end
