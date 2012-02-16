Build Dependencies
------------------
You need to have Python (CPython) installed for the v8 build.


Redistribution
--------------
Noesis.Javascript.dll needs the Microsoft C Runtime Libraries.
The exact version required is specified in a manifest file automatically
included inside the DLL.  You can extract it using 
MT.exe (from the Windows SDK):

> mt -inputresource:Noesis.Javascript.dll;2 -out:t.manifest
> type t.manifest

If you don't include the correct version of the runtime libraries
when you redistribute Noesis.Javascript.dll then you will get errors
when loading the DLL on some users machines.  (Many, but not all users
will already have it.)

Visual Stdio 2010 is more flexible about where it finds its DLLs 
(http://mariusbancila.ro/blog/2010/03/24/visual-studio-2010-changes-for-vc-part-5/)
so you need not worry about the manifest, but you should still redistribute the
runtime library because the user may not have it.


Using Visual Studio 2010
------------------------
1. Allow Visual Studio to automatically upgrade the project files.
2. Switch Noesis.Javascript.Tests to the .Net 4 platform target.

WARNING! You get a warning C4789 when building in x64 mode.
Use x86.  Hopefully a newer v8 release will fix this (perhaps when
we switch to building v8 using scons).
