using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace Noesis.Javascript.Tests
{
    public partial class RegressionTests
    {
        class ExceptionTests
        {
            public static bool RunException(JavascriptContext iContext, string iCode, int iNumberTest, string expected_exception_message, out string failure)
            {
                try
                {
                    iContext.SetParameter("myCurrentTest", iNumberTest);
                    iContext.Run(iCode);
                }
                catch (Exception ex)
                {
                    if (ex.Message != expected_exception_message) {
                        failure = String.Format("Expected '{0}' exception, but got {1}: {2}",
                                                expected_exception_message, ex.GetType().Name, ex.Message);
                        return false;
                    }

                    Console.ForegroundColor = ConsoleColor.Green;
                    Console.Write("SUCCESS");
                    Console.ResetColor();

                    failure = null;
                    return true;
                }

                failure = String.Format("Exception '{0}' not generated", expected_exception_message);
                return false;
            }
        }

        public static string RunExceptionTests(string js_dir)
        {
            // Initialization
            StreamReader fileReader = new StreamReader(Path.Combine(js_dir, "ExceptionTests.js"));
            String code = fileReader.ReadToEnd();

            using (JavascriptContext context = new JavascriptContext())
            {
                // Initialize
                context.SetParameter("JavascriptTest", new JavascriptTest());
                context.SetParameter("myObject", new JavascriptTest());
                context.SetParameter("myObjectNoIndexer", new JavascriptTestNoIndexer());
                context.SetParameter("objectRejectUnknownProperties", new Object(), SetParameterOptions.RejectUnknownProperties);

                // Run the Exceptions's tests
                string failure;
                // This is an unhelpful error, which I think occurs because the
                // indexed property types are not matching.
                if (!ExceptionTests.RunException(context, code, 1, "Method 'Noesis.Javascript.Tests.JavascriptTest.Item' not found.", out failure))
                    return failure;
                if (!ExceptionTests.RunException(context, code, 2, "Property doesn't exist", out failure))
                    return failure;
                if (!ExceptionTests.RunException(context, code, 3, "Test C# exception", out failure))
                    return failure;
                if (!ExceptionTests.RunException(context, code, 4, "Indexer doesn't exist", out failure))
                    return failure;
                if (!ExceptionTests.RunException(context, code, 5, "Unknown member: NotExist", out failure))
                    return failure;
                if (!ExceptionTests.RunException(context, code, 6, "Unknown member: NotExist", out failure))
                    return failure;
            }

            return null;
        }
    }
}
