using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace Noesis.Javascript.Tests
{
    public partial class RegressionTests
    {
        public static string GetFunctionFromJsContext(string js_dir)
        {
            JavascriptTest javascriptTest = new JavascriptTest();
            JavascriptContext context = new JavascriptContext();
            context.Run("var a = function(a,b) {return a+b;}");
            
            JavascriptFunction funcObj = context.GetParameter("a") as JavascriptFunction;

            javascriptTest.Assert(funcObj != null);

            object result = funcObj.Call(1, 2);

            javascriptTest.Assert(result is int);
            javascriptTest.Assert((int)result == 3);
            
            return null;
        }

    }
}
