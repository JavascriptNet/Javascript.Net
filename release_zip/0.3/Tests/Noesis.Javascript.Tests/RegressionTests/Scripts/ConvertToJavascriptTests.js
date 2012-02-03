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

// Test #1: .NET's float
JavascriptTest.PrintTestName(".NET' float");
JavascriptTest.Assert(125.25 == myFloat);

// Test #2: .NET's integer
JavascriptTest.PrintTestName(".NET's integer");
JavascriptTest.Assert(600 == myInteger);

// Test #3: .NET's string
JavascriptTest.PrintTestName(".NET's string");
JavascriptTest.Assert("This is a string from .NET" == myString);

// Test #4: .NET's Array
JavascriptTest.PrintTestName(".NET's array");
var jsArray = new Array(123, 55, 666);
JavascriptTest.Assert((myArray[0] == jsArray[0]) &&
                      (myArray[1] == jsArray[1]) &&
                      (myArray[2] == jsArray[2]));

// Test #5: .NET's Boolean
JavascriptTest.PrintTestName(".NET's bool");
JavascriptTest.Assert(false == myBool);

// Test #6: .NET's DateTime
JavascriptTest.PrintTestName(".NET's date");
var jsDate = new Date(2010, 9, 9); // Days and Months are zero based.
JavascriptTest.Assert(myDateTime.getMilliseconds() == jsDate.getMilliseconds());

// Test #7: .NET's object
JavascriptTest.PrintTestName(".NET's object");
JavascriptTest.Assert("Noesis.Javascript.Tests.JavascriptTest" == myObject.ToString());

// Test #8: .NET's object
JavascriptTest.PrintTestName(".NET's List");
var jsArray2 = new Array("Foo", "Bar", "FooBar");
JavascriptTest.Assert((myList[0] == jsArray2[0]) &&
                      (myList[1] == jsArray2[1]) &&
                      (myList[2] == jsArray2[2]));

// Test #9: .NET's Dictionary (String as keys)
JavascriptTest.PrintTestName(".NET's Dictionary (String as keys)");
JavascriptTest.Assert((myDictionaryByKeyString["A"] == 12) &&
                      (myDictionaryByKeyString["C"] == 34) &&
                      (myDictionaryByKeyString["E"] == 56));

// Test #10: .NET's Dictionary (Integer as keys)
JavascriptTest.PrintTestName(".NET's Dictionary (8888855 as keys)");
JavascriptTest.Assert((myDictionaryByKeyInteger["2"] == 8888855) &&
                      (myDictionaryByKeyInteger["200"] == 9191955) &&
                      (myDictionaryByKeyInteger["40000"] == 1236555));

// Test #11: .NET Enumerator
JavascriptTest.PrintTestName(".NET Enumerator");
var counter = 0;
for (var key in myDictionaryByKeyString) {
    if (12 == myDictionaryByKeyString[key])
        counter++;
    if (34 == myDictionaryByKeyString[key])
        counter++;
    if (56 == myDictionaryByKeyString[key])
        counter++;
}
JavascriptTest.Assert(counter == 3);

// Test #12: .NET Delegate
JavascriptTest.PrintTestName(".NET Delegate");
var jsString = "The delegate that return this string";
JavascriptTest.Assert((jsString + "--- Modify in the MyDelegateMethod of .NET") == myDelegate(jsString));


                      




