<img src="https://ci.appveyor.com/api/projects/status/5e8ofnu5d6d08wax/branch/master?svg=true">

Example
=======
```
// Initialize a context
using (JavascriptContext context = new JavascriptContext()) {

    // Setting external parameters for the context
    context.SetParameter("console", new SystemConsole());
    context.SetParameter("message", "Hello World !");
    context.SetParameter("number", 1);

    // Script
    string script = @"
        var i;
        for (i = 0; i < 5; i++)
            console.Print(message + ' (' + i + ')');
        number += i;
    ";

    // Running the script
    context.Run(script);

    // Getting a parameter
    Console.WriteLine("number: " + context.GetParameter("number"));
}
```
See [our wiki](https://github.com/JavascriptNet/Javascript.Net/wiki) for more information.

Nuget
=====

Old versions have been published as Noesis.JavaScript.

Publishing a newer version is a work in progress.


Redistribution
==============

Noesis.Javascript.dll needs the Microsoft C Runtime Libraries.

If you don't include the correct version of the runtime libraries
when you redistribute Noesis.Javascript.dll then you will get errors
when loading the DLL on some users machines.  (Many, but not all users
will already have it.)

Targets: .NET 8

For legacy .NET Framework 4.7.2 support, see previous versions.


Building from Source
====================

**Requirements:**
* Visual Studio 2022 (17.0+) or Visual Studio 2019 (16.4+)
* .NET 8 SDK
* Windows SDK 10.0 or later

**Steps:**

1. Open the .sln file in Visual Studio
2. Use Configuration Manager to switch the platform to x64
3. Build the solution

The project uses:
* C++/CLI with .NET Core support (`CLRSupport=NetCore`)
* .NET 8 SDK-style projects for C# code
* V8 version 9.8.177.4 via NuGet packages

**Note:** The C++/CLI project requires Windows and Visual Studio with C++/CLI support for .NET Core. This is available in Visual Studio 2019 16.4+ and Visual Studio 2022.

The following warnings are expected:

* warning LNK4248: unresolved typeref token (0100003F) for 'v8.internal.Object'; image may not run

* warning MSB3270: There was a mismatch between the processor architecture of the project being built "MSIL" and the processor architecture of the reference "C:\Users\oliver\Documents\GitHub\Javascript.Net\x64\Release\JavaScript.Net.dll", "AMD64". This mismatch may cause runtime failures. Please consider changing the targeted processor architecture of your project through the Configuration Manager so as to align the processor architectures between your project and references, or take a dependency on references with a processor architecture that matches the targeted processor architecture of your project.

Also note that when using the DLL built from source, you will need to add a Post Build Step to your consuming project, which copies the v8 DLLs and .bin files into your output directory.  See Noesis.Javascript.Tests.csproj for an example, noting that it has some extra sections manually inserted to define `V8Platform`.


Updating v8
===========
The log at https://docs.google.com/a/g7.org/document/d/1g8JFi8T_oAE_7uAri7Njtig7fKaPDfotU6huOa1alds/edit may help, if it is still being updated.


Running Tests
=============
The unit tests are standard Visual Studio - run them using the GUI.


How it Works
============
Simple values and arrays are copied between the native memory space (where v8 resides) and the Common Language Runtime (CLR) (where .Net applications live).  Complex .Net types (delegates, objects) are proxied.  When calls are made into proxied types then the parameters are copied into the CLR and the call is executed.  The results are copied back.


Internationalization
====================

??? buildv8.bat turns off internationalization when invoking gyp to avoid the need to distribute
the (large) ICU DLLs and data file.
