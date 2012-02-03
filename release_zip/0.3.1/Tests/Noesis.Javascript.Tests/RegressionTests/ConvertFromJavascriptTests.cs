using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace Noesis.Javascript.Tests
{
    public partial class JavascriptTest
    {
        public static bool RunConvertFromJavascriptTests()
        {
            try
            {
                using (JavascriptContext context = new JavascriptContext())
                {
                    // Initialization
                    StreamReader fileReader = new StreamReader("../../RegressionTests/Scripts/ConvertFromJavascriptTests.js");
                    String code = fileReader.ReadToEnd();
                    JavascriptTest javascriptTest = new JavascriptTest();


                    // Initialize
                    Console.ForegroundColor = ConsoleColor.White;
                    Console.WriteLine("===== Starting ConvertFromJavascript's tests =====");
                    Console.ResetColor();
                    context.SetParameter("JavascriptTest", new JavascriptTest());

                    // Run context
                    context.Run(code);

                    // Test #1: Javscript's float
                    javascriptTest.PrintTestName("Javascript' float");
                    javascriptTest.Assert(10.0125 == ((double)context.GetParameter("myFloat")));

                    // Test #2: Javascript's integer
                    javascriptTest.PrintTestName("Javascript's integer");
                    javascriptTest.Assert(1000 == ((int)context.GetParameter("myInteger")));

                    // Test #3: Javascript's string
                    javascriptTest.PrintTestName("Javascript's string");
                    javascriptTest.Assert("This is the string from Javascript" == ((string)context.GetParameter("myString")));

                    // Test #4: Javascript's array
                    javascriptTest.PrintTestName("Javascript's array");
                    int[] myArray = new int[] { 123, 55, 666 };
                    Array jsArray = (Array)context.GetParameter("myArray");
                    javascriptTest.Assert((myArray[0] == (int)jsArray.GetValue(0)) &&
                                          (myArray[1] == (int)jsArray.GetValue(1)) &&
                                          (myArray[2] == (int)jsArray.GetValue(2)));

                    // Test #5: Javascript's boolean
                    javascriptTest.PrintTestName("Javascript's bool");
                    javascriptTest.Assert(false == ((bool)context.GetParameter("myBool")));

                    // Test #6: Javascript's date
                    javascriptTest.PrintTestName("Javascript's date");
                    DateTime myDate = new DateTime(2010, 10, 10);
                    DateTime jsDate = (DateTime)context.GetParameter("myDate");
                    javascriptTest.Assert(myDate == jsDate);

                    // Test #7: Javascript's object
                    javascriptTest.PrintTestName("Javascript's object");
                    Dictionary<string, Object> jsObject = (Dictionary<string, Object>)context.GetParameter("myObject");
                    javascriptTest.Assert(((string)jsObject["foo"] == "new property") && ((int)jsObject["bar"] == 123456));

                    // Test #8: .NET's object (External)
                    javascriptTest.PrintTestName(".NET's object (External)");
                    JavascriptTest jsTest = (JavascriptTest)context.GetParameter("myExternal");
                    javascriptTest.Assert("Noesis.Javascript.Tests.JavascriptTest" == jsTest.ToString());

                    // Test #9: Create new property for Javascript's object
                    javascriptTest.PrintTestName("Create new property for Javascript's object");
                    Dictionary<string, object> jsObject2 = (Dictionary<string, object>)context.GetParameter("myJSObject");
                    javascriptTest.Assert(123 == (int)jsObject2["newproperty"]);

                    // End tests
                    Console.ForegroundColor = ConsoleColor.White;
                    Console.WriteLine("\n\n===== End ConvertFromJavascript's tests =====");
                    Console.ResetColor();
                }
            }
            catch (JavascriptException exception) // Javascript's exception
            {
                Console.ForegroundColor = ConsoleColor.White;
                Console.Write("\n=== Javascript's exception ===\n");
                Console.ForegroundColor = ConsoleColor.Red;
                Console.Write(
                    "{0}, line {1}: {2}",
                    exception.Source,
                    exception.Line,
                    exception.Message
                );
                Console.ResetColor();
                return false;
            }
            catch (Exception exception) // .NET's exception
            {
                Console.ForegroundColor = ConsoleColor.White;
                Console.WriteLine("=== .NET's exception ===\n");
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine(exception.ToString());
                Console.ResetColor();
                return false;
            }

            return true;
        }
    }
}
