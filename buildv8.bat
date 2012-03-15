:: Build v8 in various modes, using specific versions of Visual Studio.
:: Builds both v8.lib (release) and v8_g.lib (debug)
::
:: This script will trash bits of your environment, particularly your PATH.
@ECHO OFF

:: Set directory locations here.
SET python=C:\Python27
SET vs2008=C:\Program Files (x86)\Microsoft Visual Studio 9.0
SET vs2010=C:\Program Files (x86)\Microsoft Visual Studio 10.0
SET sdk=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A
SET v8=%~dp0%\v8

:: Parameterise specified architecture.
IF "%1"=="x64" (
    SET x64suffix=\x64
    SET adm64suffix=\amd64
) ELSE (
    IF "%1"=="ia32" (
        SET x64suffix=
        SET adm64suffix=
    ) ELSE (
        ECHO usage: %0 ia32/x64 [vs2008/vs2010]
        EXIT /b 1
    )
)

:: Locate build environment.
SET VS=
IF "%2"=="" (
    IF EXIST "%vs2010%\VC\bin\cl.exe" (
        SET "vs=%vs2010%"
    ) ELSE (
        IF EXIST "%vs2008%\VC\bin\cl.exe" (
            SET "vs=%vs2008%"
        ) ELSE (
            ECHO Cannot find Visual Studio
            EXIT /b 1
        )
    )
)
IF "%vs%"=="" (
    IF "%2"=="vs2010" (
        SET "vs=%vs2010%"
    ) ELSE (
        IF "%2"=="vs2008" (
            SET "vs=%vs2008%"
        ) ELSE (
            ECHO usage: %0 ia32/x64 vs2008/vs2010
            EXIT /b 1
        )
    )
)

:: Allow building in just one mode.
IF "%3"=="" (
    SET mode=debug,release
) ELSE (
    SET mode=%3
)

:: Change to v8 directory.
IF NOT EXIST %v8%\SConstruct (
    ECHO Cannot find v8 checkout at %v8%
    EXIT /b 1
)
CD %v8%

:: Have found that sometimes x64 debug builds get stuck on writing to vc100.pdb,
:: so we clean it up.  Also VS2008 builds sometimes output gobbledegook.
:: We delete all outputs beforehand to force it to be rebuilt.
:: scons --clean doesn't get everything.
IF EXIST v8.lib (
    RMDIR /S /Q obj
    DEL *.idb
    DEL *.pdb
    DEL *.lib
)

:: Set environment required for scons to use the right toolset.
SET PATH=
SET PATH=%python%;%python%\Scripts;%PATH%
SET PATH=%vs%\VC\bin%adm64suffix%;%vs%\Common7\IDE;%vs%\Common7\IDE;%vs%\Common7\Tools;%PATH%
SET INCLUDE=%vs%\VC\include;%sdk%\Include
SET LIB=%vs%\VC\lib%adm64suffix%;%sdk%\Lib%x64suffix%

:: Run the build.
::
:: We clean after every build so that you don't get weird results when you
:: change architecture or build tools.
scons env="PATH:%PATH%,INCLUDE:%INCLUDE%,LIB:%LIB%" -j8 arch=%1 mode=%mode% msvcrt=shared snapshot=off
