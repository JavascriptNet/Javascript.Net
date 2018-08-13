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

Targets: .NET Framework 4.5


Building from Source
====================

Open the .sln file in Visual Studio, use Configuration Manager to switch to platform to x64, and build.  I've been working using Visual Studio 2017,
but 2015 will probably work too.

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


Internationalization
====================

??? buildv8.bat turns off internationalization when invoking gyp to avoid the need to distribute
the (large) ICU DLLs and data file.
