@SETLOCAL
@ECHO off

for %%a in (ia32 x64) do (
	for %%v in (vs2010 vs2012 vs2013) do (
		for %%c in (MSVC2010 MSVC2012 MSVC2013) do (
			for %%b in (Debug Release) do (
				for %%f in (4.0 4.5 4.5.1) do (
					build_both %%a %%v %%c %%b %%f
				)
			)
		)
	)
)

@ENDLOCAL
