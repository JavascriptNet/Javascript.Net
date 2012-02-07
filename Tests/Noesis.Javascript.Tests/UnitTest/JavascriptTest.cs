using System;
using System.Reflection;
using System.Threading;
using System.IO;


namespace Noesis.Javascript.Tests
{
    public partial class JavascriptTest
    {
        #region Construction

        public JavascriptTest()
        {
            mValue = "Default Value";
            mCounter = 1;
        }

        #endregion

        #region Methods

        public void Print(string iString)
        {
            Console.WriteLine(iString);
        }

        public void Assert(bool iBoolean)
        {
            if (iBoolean)
            {
                Console.ForegroundColor = ConsoleColor.Green;
                Console.Write("SUCCESS");
            }
            else
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.Write("FAILED");
            }
            Console.ResetColor();
        }

        public void PrintTestName(String iString)
        {
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.Write("\nTest #{1}: \"{0}\": ", iString, mCounter);
            mCounter++;
            Console.ResetColor();
        }

        public void PrintFailed()
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.Write("FAILED");
        }

        public void ThrowException()
        {
            throw new Exception("\n\n\"Throwing an exception to test the exception's management.\"\n\n");
        }

        public JavascriptTest GetClass()
        {
            return this;
        }

        public void RunRegressionTests()
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

        #endregion

        #region Properties

        public string this[int iIndex]
        {
            get { return (iIndex + "===You just pass in the string indexer of JavascriptTest"); }
            set { mValue = iIndex + "-----" + value; }
        }

        public string Value
        {
            get { return mValue; }
            set { mValue = value + " -- indeed you do"; }
        }

        public string MyProperty
        {
            get { return "This is the string return by \"My Property\""; } 
        }

        #endregion

        #region Data Members

        private string mValue;
        private int mCounter;

        #endregion
    }

    public class JavascriptTestNoIndexer
    {
    }
}
