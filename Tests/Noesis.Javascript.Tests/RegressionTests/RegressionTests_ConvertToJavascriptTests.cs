using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace Noesis.Javascript.Tests
{
    public partial class RegressionTests
    {
        class ConvertToJavascriptTests
        {
            public delegate string SampleDelegate(string iString);

            public static string MyDelegateMethod(string iString)
            {
                return iString + "--- Modify in the MyDelegateMethod of .NET";
            }
        }

        public static string RunConvertToJavascriptTests()
        {
            using(JavascriptContext context = new JavascriptContext())
            {
                // Initialization
                StreamReader fileReader = new StreamReader("../../RegressionTests/Scripts/ConvertToJavascriptTests.js");
                String code = fileReader.ReadToEnd();

                // Initialize
                context.SetParameter("JavascriptTest", new JavascriptTest());

                // Test #1: .NET's float
                context.SetParameter("myFloat", 125.25);

                // Test #2: .NET's integer
                context.SetParameter("myInteger", 600);

                // Test #3: .NET's string
                context.SetParameter("myString", "This is a string from .NET");

                // Test #4: .NET's Array
                int[] myArray = new int[] { 123, 55, 666 };
                context.SetParameter("myArray", myArray);

                // Test #5: .NET's Boolean
                context.SetParameter("myBool", false);

                // Test #6: .NET's DateTime
                context.SetParameter("myDateTime", new DateTime(2010, 10, 10));

                // Test #7: .NET's Object
                context.SetParameter("myObject", new JavascriptTest());

                // Test #8: .NET's List
                List<string> myList = new List<string>() { "Foo", "Bar", "FooBar" };
                context.SetParameter("myList", myList);

                // Test #9: .NET's Dictionary (String as keys)
                Dictionary<string, int> myDictionaryByKeyString = new Dictionary<string, int>()
                {
                    {"A", 12},
                    {"C", 34},
                    {"E", 56}
                };
                context.SetParameter("myDictionaryByKeyString", myDictionaryByKeyString);

                // Test #10: .NET's Dictionary (Integer as keys) and Test #11: .NET Enumerator
                Dictionary<int, int> myDictionaryByKeyInteger = new Dictionary<int, int>()
                {
                    {2, 8888855},
                    {200, 9191955},
                    {40000, 1236555}
                };
                context.SetParameter("myDictionaryByKeyInteger", myDictionaryByKeyInteger);

                // Test #12: .NET Delegate
                ConvertToJavascriptTests.SampleDelegate MyDelegate = ConvertToJavascriptTests.MyDelegateMethod;
                context.SetParameter("myDelegate", MyDelegate);

                // Run context
                context.Run(code);
            }

            return null;
        }
    }
}
