using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace Noesis.Javascript.Tests
{
    public partial class RegressionTests
    {
        public static string RunAccessorsInterceptorsTests()
        {
            using (JavascriptContext context = new JavascriptContext())
            {
                // Initialization
                StreamReader fileReader = new StreamReader("../../RegressionTests/Scripts/AccessorsInterceptorsTests.js");
                String code = fileReader.ReadToEnd();

                // Initialize
                context.SetParameter("JavascriptTest", new JavascriptTest());

                /**
                 * Test #1: Accessing an element in a .NET Array
                 * Test #4: Setting by index an value in a .NET Array
                 * 
                 */
                int[] myArray = new int[] { 151515, 666, 2555, 888, 99 };
                context.SetParameter("myArray", myArray);

                /**
                 * Test #2: Accessing by index an property in a .NET Object, 
                 * Test #3: Accessing by name an property in a .NET Object
                 * Test #5: Setting by index an value in a .NET Object
                 * Test #6: Setting by name an property in a .NET Object
                 */
                context.SetParameter("myObject", new JavascriptTest());

                // Run context
                context.Run(code);
            }

            return null;
        }
    }
}
