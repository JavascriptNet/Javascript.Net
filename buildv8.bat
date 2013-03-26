:: Build v8 in various modes, using specific versions of Visual Studio.
:: Builds both v8.lib (release) and v8_g.lib (debug)
::
:: This script will trash bits of your environment, particularly your PATH.
@ECHO OFF

:: Set directory locations here.
SET python=C:\Python27
SET vs2008=C:\Program Files (x86)\Microsoft Visual Studio 9.0
SET vs2010=C:\Program Files (x86)\Microsoft Visual Studio 10.0
SET vs2012=C:\Program Files (x86)\Microsoft Visual Studio 11.0

SET sdk70=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A
SET sdk71=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A

IF EXIST "%sdk71%" (
	SET "sdk=%sdk71%"
) ELSE (
	SET "sdk=%sdk70%"
)

echo using sdk %sdk%

SET v8=%~dp0%\v8

:: Parameterise specified architecture.
IF "%1"=="x64" (
    SET "x64suffix=\x64"
    SET "adm64suffix=\amd64"
	SET "target_arch=x64"
	SET "profile=Release|x64"
) ELSE (
    IF "%1"=="ia32" (
        SET "x64suffix="
        SET "adm64suffix="
		SET "target_arch=ia32"
		SET "profile=Release|Win32"
    ) ELSE (
        ECHO usage: %0 ia32/x64 [vs2008/vs2010/vs2012]
        EXIT /b 1
    )
)

:: Locate build environment.
SET VS=
IF "%2"=="" (
	IF EXIST "%vs2012%\VC\bin\cl.exe" (
        SET "vs=%vs2012%"
		SET "GYP_MSVS_VERSION=2012"
    ) ELSE (
		IF EXIST "%vs2010%\VC\bin\cl.exe" (
			SET "vs=%vs2010%"
			SET "GYP_MSVS_VERSION=2010"
		) ELSE (
			IF EXIST "%vs2008%\VC\bin\cl.exe" (
				SET "vs=%vs2008%"
				SET "GYP_MSVS_VERSION=2008"
			) ELSE (
				ECHO Cannot find Visual Studio
				EXIT /b 1
			)
		)
	)
)

IF "%3" == "" (
	SET "msbuild_toolset="
) ELSE (
	SET "msbuild_toolset=%3"
)

IF "%vs%"=="" (
    IF "%2"=="vs2012" (
        SET "vs=%vs2012%"
		SET "GYP_MSVS_VERSION=2012"
    ) ELSE (
		IF "%2"=="vs2010" (
			SET "vs=%vs2010%"
			SET "GYP_MSVS_VERSION=2010"
		) ELSE (
			IF "%2"=="vs2008" (
				SET "vs=%vs2008%"
				SET "GYP_MSVS_VERSION=2008"
			) ELSE (
				ECHO usage: %0 ia32/x64 vs2012/vs2008/vs2010
				EXIT /b 1
			)
		)
	)
)

:: Allow building in just one mode.
IF "%4"=="" (
    SET mode=debug,release
) ELSE (
    SET mode=%4
)

:: Change to v8 directory.
IF NOT EXIST %v8%\LICENSE.v8 (
    ECHO Cannot find v8 checkout at %v8%
    EXIT /b 1
)
CD %v8%

:: Have found that sometimes x64 debug builds get stuck on writing to vc100.pdb,
:: so we clean it up.  Also VS2008 builds sometimes output gobbledegook.
:: We delete all outputs beforehand to force it to be rebuilt.

RMDIR /S /Q obj
RMDIR build\debug /s /q
RMDIR build\release /s /q
DEL *.idb
DEL *.pdb
DEL *.lib

DEL /S *.sln
DEL /S *.vcproj
DEL /S *.vcxproj
DEL /S *.vcxproj.user


:: Set environment required for scons to use the right toolset.
SET PATH=%python%;%python%\Scripts;%PATH%
SET PATH=%vs%\VC\bin%adm64suffix%;%vs%\Common7\IDE;%vs%\Common7\IDE;%vs%\Common7\Tools;%PATH%
SET INCLUDE=%vs%\VC\include;%sdk%\Include
SET LIB=%vs%\VC\lib%adm64suffix%;%sdk%\Lib%x64suffix%

echo third_party\python_26\python.exe build\gyp_v8 -D"target_arch=%target_arch%" -D"component=shared_library"
third_party\python_26\python.exe build\gyp_v8 -D"target_arch=%target_arch%" -D"component=shared_library"

IF "%vs%"=="%vs2012%" (
	IF "%msbuild_toolset%"=="v100" (
		IF "%sdk%"=="%sdk71%" (
			echo ..\fart -r -i "*.vcxproj" "<PlatformToolset>v110</PlatformToolset>" "<PlatformToolset>Windows7.1SDK</PlatformToolset><TargetFrameworkVersion>v4.0</TargetFrameworkVersion>"
			..\fart -r -i "*.vcxproj" "<PlatformToolset>v110</PlatformToolset>" "<PlatformToolset>Windows7.1SDK</PlatformToolset><TargetFrameworkVersion>v4.0</TargetFrameworkVersion>"
		) ELSE (
			echo ..\fart -r -i "*.vcxproj" "<PlatformToolset>v110</PlatformToolset>" "<PlatformToolset>v100</PlatformToolset><TargetFrameworkVersion>v4.0</TargetFrameworkVersion>"
			..\fart -r -i "*.vcxproj" "<PlatformToolset>v110</PlatformToolset>" "<PlatformToolset>v100</PlatformToolset><TargetFrameworkVersion>v4.0</TargetFrameworkVersion>"
		)
	) ELSE (
		IF "%msbuild_toolset%"=="v90" (
			IF "%sdk%"=="%sdk71%" (
				echo ..\fart -r -i "*.vcxproj" "<PlatformToolset>v110</PlatformToolset>" "<PlatformToolset>Windows7.1SDK</PlatformToolset><TargetFrameworkVersion>v3.5</TargetFrameworkVersion>"
				..\fart -r -i "*.vcxproj" "<PlatformToolset>v110</PlatformToolset>" "<PlatformToolset>Windows7.1SDK</PlatformToolset><TargetFrameworkVersion>v3.5</TargetFrameworkVersion>"
			) ELSE (
				echo ..\fart -r -i "*.vcxproj" "<PlatformToolset>v110</PlatformToolset>" "<PlatformToolset>v90</PlatformToolset><TargetFrameworkVersion>v3.5</TargetFrameworkVersion>"
				..\fart -r -i "*.vcxproj" "<PlatformToolset>v110</PlatformToolset>" "<PlatformToolset>v90</PlatformToolset><TargetFrameworkVersion>v3.5</TargetFrameworkVersion>"
			)
		)
	)
) ELSE (
	IF "%vs%"=="%vs2010%" (
		IF "%msbuild_toolset%"=="v90" (
			IF "%sdk%"=="%sdk71%" (
				echo ..\fart -r -i "*.vcxproj" "<PlatformToolset>v100</PlatformToolset>" "<PlatformToolset>Windows7.1SDK</PlatformToolset><TargetFrameworkVersion>v3.5</TargetFrameworkVersion>"
				..\fart -r -i "*.vcxproj" "<PlatformToolset>v100</PlatformToolset>" "<PlatformToolset>Windows7.1SDK</PlatformToolset><TargetFrameworkVersion>v3.5</TargetFrameworkVersion>"
			) ELSE (
				echo ..\fart -r -i "*.vcxproj" "<PlatformToolset>v100</PlatformToolset>" "<PlatformToolset>v90</PlatformToolset><TargetFrameworkVersion>v3.5</TargetFrameworkVersion>"
				..\fart -r -i "*.vcxproj" "<PlatformToolset>v100</PlatformToolset>" "<PlatformToolset>v90</PlatformToolset><TargetFrameworkVersion>v3.5</TargetFrameworkVersion>"
			)
		) ELSE (
			IF "%sdk%"=="%sdk71%" (
				echo ..\fart -r -i "*.vcxproj" "<PlatformToolset>v100</PlatformToolset>" "<PlatformToolset>Windows7.1SDK</PlatformToolset><TargetFrameworkVersion>v4.0</TargetFrameworkVersion>"
				..\fart -r -i "*.vcxproj" "<PlatformToolset>v100</PlatformToolset>" "<PlatformToolset>Windows7.1SDK</PlatformToolset><TargetFrameworkVersion>v4.0</TargetFrameworkVersion>"
			) ELSE (
				echo ..\fart -r -i "*.vcxproj" "<PlatformToolset>v100</PlatformToolset>" "<PlatformToolset>v100</PlatformToolset><TargetFrameworkVersion>v4.0</TargetFrameworkVersion>"
				..\fart -r -i "*.vcxproj" "<PlatformToolset>v100</PlatformToolset>" "<PlatformToolset>v100</PlatformToolset><TargetFrameworkVersion>v4.0</TargetFrameworkVersion>"
			)
		)
	) ELSE (
		IF "%sdk%"=="%sdk71%" (
			echo ..\fart -r -i "*.vcxproj" "<PlatformToolset>v90</PlatformToolset>" "<PlatformToolset>Windows7.1SDK</PlatformToolset><TargetFrameworkVersion>v3.5</TargetFrameworkVersion>"
			..\fart -r -i "*.vcxproj" "<PlatformToolset>v90</PlatformToolset>" "<PlatformToolset>Windows7.1SDK</PlatformToolset><TargetFrameworkVersion>v3.5</TargetFrameworkVersion>"
		) ELSE (
			echo ..\fart -r -i "*.vcxproj" "<PlatformToolset>v90</PlatformToolset>" "<PlatformToolset>v90</PlatformToolset><TargetFrameworkVersion>v3.5</TargetFrameworkVersion>"
			..\fart -r -i "*.vcxproj" "<PlatformToolset>v90</PlatformToolset>" "<PlatformToolset>v90</PlatformToolset><TargetFrameworkVersion>v3.5</TargetFrameworkVersion>"
		)
	)
)

echo "%vs%\Common7\IDE\devenv.com" /build "%profile%" build\All.sln
"%vs%\Common7\IDE\devenv.com" /build "%profile%" build\All.sln