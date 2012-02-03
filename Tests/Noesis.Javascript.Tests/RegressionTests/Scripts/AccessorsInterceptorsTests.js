////////////////////////////////////////////////////////////////////////////////////////////////////
// File: AccessorsInterceptorsTests.js
// 
// Copyright 2010 Noesis Innovation Inc. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////////////////////////

// Test #1: Accessing an element in a .NET Array
JavascriptTest.PrintTestName("Accessing an element in a .NET's Array");
JavascriptTest.Assert(myArray[2] == 2555);

// Test #2: Accessing by index an property in a .NET Object
JavascriptTest.PrintTestName("Accessing by index an property in a .NET Object");
var jsInteger = 1000;
JavascriptTest.Assert(myObject[jsInteger] == (jsInteger + "===You just pass in the string indexer of JavascriptTest"));

// Test #3: Accessing by name an property in a .NET Object
JavascriptTest.PrintTestName("Accessing by name an property in a .NET Object");
JavascriptTest.Assert(myObject.MyProperty == "This is the string return by \"My Property\"");

// Test #4: Setting by index an value in a .NET Array
JavascriptTest.PrintTestName("Setting by index an value in a .NET Array");
myArray[2] = 123456789;
JavascriptTest.Assert(myArray[2] == 123456789);

// Test #5: Setting by index an value in a .NET Object
JavascriptTest.PrintTestName("Setting by index an value in a .NET Object");
myObject[20] = "The Value is now set";
JavascriptTest.Assert(myObject.Value == "20-----The Value is now set");

// Test #6: Setting by name an property in a .NET Object
JavascriptTest.PrintTestName("Setting by name an property in a .NET Object");
JavascriptTest.Value = "I just changed the property";
JavascriptTest.Assert(JavascriptTest.Value == "I just changed the property -- indeed you do");

// Test #7: Chain of functions call on the same .NET object