Redistribution
==============

Noesis.Javascript.dll needs the Microsoft Visual C++ Libraries.
You can specify MSVC version when building, see below.

To check if everything is correct: exact version required is specified
in a manifest file automatically included inside the DLL.  You can extract it using 
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
====================

* You will need python 2.6+ in PATH

* `git clone --recursive` this repo. You will need depot_tools (specified as submodule) to get V8.

* run get_v8.bat to fetch required version of V8 and its build dependencies. 

* Now you can build V8 and Javascript.NET. See `build_v8.bat`, `build_jsNet.bat`, `build_both.bat` and `build_everything.bat`.

__Ignore warnings like `JavascriptInterop.obj : warning LNK4248: unresolved typeref token (0100002A) for 'v8.internal.Object'; image may not run`__


Building options
----------------

You will need to specify:

* Visual Studio version to build with: 2010, 2012, 2013.

* MSVC++ version to use.  2010, 2012, 2013.

* .NET Framework. 4.0, 4.5, 4.5.1.

* Architecture. 32 or 64 bits

* Release or Debug mode

__Note: if you are using eg. VS2013 and specified MSVC++2010, you will need VS2010 to be installed. (TODO: build with Windows SDK?)__ Related links:

http://stackoverflow.com/questions/24775363/how-to-build-with-v90-platform-toolset-in-vs2012-without-vs2008-using-windows-s

http://blogs.msdn.com/b/chuckw/archive/2013/10/03/a-brief-history-of-windows-sdks.aspx


Running Tests
=============

Visual Studio may download nunit for you.

There may be a better way to do this, but all I've been able to figure
out for running it is something like:

> packages\NUnit.Runners.2.6.3\tools\nunit-console-x86 Build\Tests\\{...}\Noesis.Javascript.Tests.dll

or

> packages\NUnit.Runners.2.6.3\tools\nunit-console Build\Tests\\{...}\Noesis.Javascript.Tests.dll

or (x64 in VS2012)

> packages\NUnit.Runners.2.6.3\tools\nunit-console Build\Tests\\{...}\Noesis.Javascript.Tests.dll


Upgrading v8
============

You can change revision of V8 in get_v8.bat or with git in subdirectory `./v8`

You can read about changes to the v8 API at
https://docs.google.com/a/g7.org/document/d/1g8JFi8T_oAE_7uAri7Njtig7fKaPDfotU6huOa1alds/edit


Internationalization
====================

buildv8.bat turns off internationalization when invoking gyp to avoid the need to distribute
the (large) ICU DLLs and data file.


Known Problems
==============

See Issues on GitHub.

Using .NET 4.5.2
=================

You will need the [.NET 4.5.2 Developer Pack](https://www.microsoft.com/en-us/download/details.aspx?id=42637) to build using .NET 4.5.2
