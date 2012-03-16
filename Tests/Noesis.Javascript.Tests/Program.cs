using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Reflection;
using System.Collections;

using Noesis.Javascript;
using System.Threading;

namespace Noesis.Javascript.Tests
{
    class Program
    {
        /// <summary>
        /// Each regression test is implemented as a partial part of the
        /// RegressionTests class.
        /// </summary>
        static void Main(string[] args)
        {
            MethodInfo[] memberInfos = typeof(RegressionTests).GetMethods(BindingFlags.Public | BindingFlags.Static);

            string js_dir = "../../RegressionTests/Scripts/";
            if (Path.GetFileName(Path.GetDirectoryName(Environment.CurrentDirectory)) != "bin")
                js_dir = "../" + js_dir;

            for (int i = 0; i < memberInfos.Length; i++) {
                Console.ForegroundColor = ConsoleColor.White;
                Console.WriteLine(String.Format("\n\n===== Starting {0} =====", memberInfos[i].Name));
                Console.ResetColor();

                try {
                    // Run the test.
                    string failure = (string)memberInfos[i].Invoke(null, new object[] { js_dir });

                    if (failure == null) {
                        // End tests
                        Console.ForegroundColor = ConsoleColor.White;
                        Console.WriteLine(String.Format("\n===== End {0} =====", memberInfos[i].Name));
                        Console.ResetColor();
                    } else {
                        Console.ForegroundColor = ConsoleColor.Red;
                        Console.WriteLine(String.Format("Failed: {0}", failure));
                        Console.ResetColor();
                    }
                } catch (JavascriptException exception) { // Javascript's exception
                    Console.ForegroundColor = ConsoleColor.White;
                    Console.Write("\n=== Javascript exception ===\n");
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.Write(
                        "{0}, line {1}: {2}",
                        exception.Source,
                        exception.Line,
                        exception.Message
                    );
                    Console.ResetColor();
                } catch (Exception exception) { // .NET's exception
                    Console.ForegroundColor = ConsoleColor.White;
                    Console.WriteLine("=== .NET exception ===\n");
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.WriteLine(exception.ToString());
                    Console.ResetColor();
                }
            }

            Console.WriteLine("\n");
            Thread.Sleep(2000);
        }
    }
}
