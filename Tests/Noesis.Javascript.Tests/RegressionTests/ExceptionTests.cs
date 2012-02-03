using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace Noesis.Javascript.Tests
{
    public partial class JavascriptTest
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

        public static bool RunExceptionTests()
        {
            try
            {
                // Initialization
                StreamReader fileReader = new StreamReader("../../RegressionTests/Scripts/ExceptionTests.js");
                String code = fileReader.ReadToEnd();

                using (JavascriptContext context = new JavascriptContext())
                {
                    // Initialize
                    Console.ForegroundColor = ConsoleColor.White;
                    Console.WriteLine("\n\n===== Starting Exceptions's tests =====");
                    Console.ResetColor();
                    context.SetParameter("JavascriptTest", new JavascriptTest());
                    context.SetParameter("myObject", new JavascriptTest());
                    context.SetParameter("myObjectNoIndexer", new JavascriptTestNoIndexer());

                    // Run the Exceptions's tests
                    for (int i = 1; i < 7; i++)
                    {
                        if (!ExceptionTests.RunException(context, code, i))
                            return false;
                    }
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
            catch(Exception exception) // .NET's exception
            {
                Console.ForegroundColor = ConsoleColor.White;
                Console.WriteLine("=== .NET's exception ===\n");
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine(exception.ToString());
                Console.ResetColor();

                return false;
            }

            // End tests
            Console.ForegroundColor = ConsoleColor.White;
            Console.WriteLine("\n\n===== End Exceptions's tests =====");
            Console.ResetColor();

            return true;
        }
    }
}
