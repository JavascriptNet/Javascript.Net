using System;
using System.Reflection;
using System.Threading;
using System.IO;


namespace Noesis.Javascript.Tests
{
    /// <summary>
    /// An object passed into v8 and called by JavaScript code in the tests.
    /// </summary>
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
            throw new Exception("Test C# exception");
        }

        public JavascriptTest GetClass()
        {
            return this;
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
