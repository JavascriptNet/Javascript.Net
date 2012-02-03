using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Reflection;
using System.Collections;

using Noesis.Javascript;

namespace Noesis.Javascript.Tests
{
    class Program
    {
        public class SystemConsole
        {
            public SystemConsole() { }

            public void Print(string iString)
            {
                Console.WriteLine(iString);
            }
        }

        static void Main(string[] args)
        {
            // Initialize the context
            JavascriptContext context = new JavascriptContext();

            // Setting the externals parameters of the context
            context.SetParameter("console", new SystemConsole());
            context.SetParameter("message", "Hello World !");
            context.SetParameter("number", 1);

            // Running the script
            context.Run("var i; for (i = 0; i < 5; i++) console.Print(message + ' (' + i + ')'); number += i;");

            // Getting a parameter
            Console.WriteLine("number: " + context.GetParameter("number"));

            // Regression Tests
            JavascriptTest javascriptTest = new JavascriptTest();
            javascriptTest.RunRegressionTests();
        }
    }
}
