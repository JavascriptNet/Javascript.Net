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
        static void Main(string[] args)
        {
            // Regression Tests
            JavascriptTest javascriptTest = new JavascriptTest();
            javascriptTest.RunRegressionTests();
        }
    }
}
