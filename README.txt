Redistribution
--------------
Noesis.Javascript.dll needs the Microsoft C Runtime Libraries.

If you don't include the correct version of the runtime libraries
when you redistribute Noesis.Javascript.dll then you will get errors
when loading the DLL on some users machines.  (Many, but not all users
will already have it.)

Targets: .NET Framework 4.5


Building from Source
--------------------

Open the .sln file in Visual Studio and build.  I've been working using Visual Studio 2017,
but 2015 will probably work too.

The following warnings are expected:
* warning LNK4248: unresolved typeref token (0100003F) for 'v8.internal.Object'; image may not run
* warning MSB3270: There was a mismatch between the processor architecture of the project being built "MSIL" and the processor architecture of the reference "C:\Users\oliver\Documents\GitHub\Javascript.Net\x64\Release\JavaScript.Net.dll", "AMD64". This mismatch may cause runtime failures. Please consider changing the targeted processor architecture of your project through the Configuration Manager so as to align the processor architectures between your project and references, or take a dependency on references with a processor architecture that matches the targeted processor architecture of your project.


Running Tests
-------------

The unit tests are standard Visual Studio - run them using the GUI.


Nuget
-----

Old versions have been published as Noesis.JavaScript.

Publishing a newer version is a work in progress.
