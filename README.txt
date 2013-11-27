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

Visual Stdio 2010+ is more flexible about where it finds its DLLs 
(http://mariusbancila.ro/blog/2010/03/24/visual-studio-2010-changes-for-vc-part-5/)
so you need not worry about the manifest, but you should still redistribute the
runtime library because the user may not have it.


Building from Source
--------------------

1. Fork and get a local copy of the github repository.
   This will automatically check out a known good copy (perhaps old)
   of the v8 source code.

2. Follow Visual Studio build instructions at
    http://code.google.com/p/v8/wiki/BuildingWithGYP
   At the time of writing you also needed to read
    https://code.google.com/p/v8/issues/detail?id=2973

3. Run build.bat to build v8 for your preferred architecture and build
   environment.  e.g.
   > buildv8 ia32 vs2012
   
4. Load the Visual Studio Solution file corresponding to your version of
   Visual Studio.

5. Switch the architecture to match (x64/Win32) the v8 build you made.

6. Build.


Running Tests
-------------

Visual Studio may download nunit for you (though not if using VS 2008).

There may be a better way to do this, but all I've been able to figure
out for running it is something like:

> packages\NUnit.Runners.2.6.3\tools\nunit-console-x86 Tests\Noesis.Javascript.Tests\bin\VS2010\x86\Debug\Noesis.Javascript.Tests.dll


Known Problems
--------------
See Issues on GitHub.
