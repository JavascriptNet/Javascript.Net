## Installing

1. Clone this repository
2. Run "getv8.bat"
3. Extract "v8-build-tools.zip" contents to your current directory
4. Run "buildv8.bat" **from the command line** with the the required parameters (e.g 'buildv8 Debug ia32')

Now you can open the solution and change code, build, etc...

## Upgrading V8 

There is a problem upgrading beyond tag 3.28.50 because 3.28.51 changed an API used by Javascript.NET.
https://chromium.googlesource.com/v8/v8.git/+/3.28.51

If you want to upgrade furture then you need to replace the call with the new API equivalent.