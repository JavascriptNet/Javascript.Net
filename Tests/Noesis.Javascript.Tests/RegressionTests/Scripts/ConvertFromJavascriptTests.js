////////////////////////////////////////////////////////////////////////////////////////////////////
// File: ConvertFromJavascriptTests.js
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

// Test #1: Javscript's float
var myFloat = 10.0125;

// Test #2: Javascript's integer
var myInteger = 1000;

// Test #3: Javascript's string
var myString = "This is the string from Javascript";

// Test #4: Javascript's array
var myArray = new Array(123,55,666);

// Test #5: Javascript's bool
var myBool = false;

// Test #6: Javascript's date
var myDate = new Date(2010, 9, 10); // Months are zero based.

// Test #7: Javascript's object
var myObject = new Object();
myObject.foo = "new property";
myObject.bar = 123456;

// Test #8: .NET's object (External)
var myExternal = JavascriptTest;

// Test #9: Create new property for Javascript's object
var myJSObject = {};
myJSObject.newproperty = 123;
