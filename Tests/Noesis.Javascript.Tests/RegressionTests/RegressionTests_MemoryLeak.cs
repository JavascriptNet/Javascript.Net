using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace Noesis.Javascript.Tests
{
    public partial class RegressionTests
    {
        public static string RunMemoryLeakTest(string js_dir)
        {
            MemoryUsageLoadInstance();
            long mem = Process.GetCurrentProcess().PrivateMemorySize64;

            for (int i = 0; i < 20; i++) {
                MemoryUsageLoadInstance();
            }
            JavascriptContext.Collect();
            GC.Collect();
            GC.Collect();
            decimal diffMBytes = (Process.GetCurrentProcess().PrivateMemorySize64 - mem) / 1048576m;

            if (diffMBytes >= 1) // Allow 1 MB
                return String.Format("{0:0.00}MB left allocated", diffMBytes);
            return null;
        }

        private static void MemoryUsageLoadInstance()
        {
            using (JavascriptContext ctx = new JavascriptContext()) {
                ctx.Run(
                @"
buffer = [];
for (var i = 0; i < 100000; i++) {
buffer[i] = 'new string';
}
");
            }
        }
    }
}
