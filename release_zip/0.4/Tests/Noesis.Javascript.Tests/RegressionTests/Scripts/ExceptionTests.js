////////////////////////////////////////////////////////////////////////////////////////////////////
// File: ExceptionTests.js
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

switch (myCurrentTest) {
    case 1:
        // Test #1: Handle the invalid arguments exception in SetProperty(iIndex,iValue) of the v8 engine
        JavascriptTest.PrintTestName("Handle the invalid arguments exception in SetProperty(iIndex,iValue) of the v8 engine");
        JavascriptTest[666] = 5555;
        break;

    case 2:
        // Test #2: Handle the not existing properties exception in GetProperty(iIndex,iValue) of the v8 engine
        JavascriptTest.PrintTestName("Handle the not existing properties exception in GetProperty(iIndex,iValue) of the v8 engine");
        var object = myObject["Should have been an integer in here"];
        if (object == null)
            throw "Property doesn't exist";
        break;

    case 3:
        // Test #3: Handle an exception from the invoker of v8
        JavascriptTest.PrintTestName("Handle an exception from the invoker of v8");
        JavascriptTest.ThrowException();
        break;

    case 4:
        // Test #4: Handle an exception with a class with no indexer
        JavascriptTest.PrintTestName("Handle a class without indexer");
        var object = myObjectNoIndexer[20];
        if (object == null)
            throw "Indexer doesn't exist";
        break;

    case 5:
        // Test #5: Handle the not existing properties exception in GetProperty(iName,iValue) of the v8 engine
        JavascriptTest.PrintTestName("Handle the not existing properties exception in GetProperty(iName,iValue) of the v8 engine");
        var object = JavascriptTest.NotExist;
        if (object == null)
            throw "Property doesn't exist";
        break;
}

// If no exception have been throw then the test have failed
JavascriptTest.PrintFailed();