Dependencies on Microsoft C Runtime Libraries
---------------------------------------------
The DLLs built by this project need the Microsoft C Runtime Libraries.  
The exact version required is specified in a manifest file automatically
included inside the DLL.  You can extract it using MT.exe (from the
Windows SDK):

> mt -inputresource:Noesis.Javascript.dll;2 -out:t.manifest
> type t.manifest

If you don't include the correct version of the runtime libraries 