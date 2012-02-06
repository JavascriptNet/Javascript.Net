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
            public static bool RunException(JavascriptContext iContext, string iCode, int iNumberTest)
            {
                try
                {
                    iContext.SetParameter("myCurrentTest", iNumberTest);
                    iContext.Run(iCode);
                }
                catch (JavascriptException) // Javascript's exception
                {
                    Console.ForegroundColor = ConsoleColor.Green;
                    Console.Write("SUCCESS");
                    Console.ResetColor();

                    return true;
                }
                catch (Exception) // .NET's exception
                {
                    Console.ForegroundColor = ConsoleColor.Green;
                    Console.Write("SUCCESS");
                    Console.ResetColor();

                    return true;
                }

                return false;
            }
        }

        public static string RunExceptionTests()
        {
            // Initialization
            StreamReader fileReader = new StreamReader("../../RegressionTests/Scripts/ExceptionTests.js");
            String code = fileReader.ReadToEnd();

            using (JavascriptContext context = new JavascriptContext())
            {
                // Initialize
                context.SetParameter("JavascriptTest", new JavascriptTest());
                context.SetParameter("myObject", new JavascriptTest());
                context.SetParameter("myObjectNoIndexer", new JavascriptTestNoIndexer());

                // Run the Exceptions's tests
                for (int i = 1; i <= 5; i++)
                {
                    if (!ExceptionTests.RunException(context, code, i))
                        return "Exception not generated";
                }
            }

            return null;
        }
    }
}
