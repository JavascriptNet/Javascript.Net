using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace Noesis.Javascript.Tests
{
    public partial class RegressionTests
    {
        public static string RetrieveV8Version(string js_dir)
        {
            JavascriptTest javascriptTest = new JavascriptTest();
            String version = JavascriptContext.V8Version;
            javascriptTest.Assert(!string.IsNullOrWhiteSpace(version));
            return null;
        }

    }
}
