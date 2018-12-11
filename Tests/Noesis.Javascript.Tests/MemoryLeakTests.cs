using System;
using System.Diagnostics;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class MemoryLeakTests
    {
        [TestMethod]
        public void RunMemoryLeakTest()
        {
            MemoryLeakTest(MemoryUsageLoadInstance);
        }

        [TestMethod]
        public void RunFunctionMemoryLeakTest()
        {
            using (JavascriptContext ctx = new JavascriptContext())
            {
                MemoryLeakTest(() => FunctionMemoryUsageLoadInstance(ctx), iterations: 10000, maxMemoryIncreaseInMB: 5);
            }
        }

        private static void MemoryLeakTest(Action test, uint iterations = 20, uint maxMemoryIncreaseInMB = 1)
        {
            test();
            long mem = Process.GetCurrentProcess().PrivateMemorySize64;

            for (int i = 0; i < iterations; i++)
            {
                test();
            }
            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();
            decimal diffMBytes = (Process.GetCurrentProcess().PrivateMemorySize64 - mem) / 1048576m;

            diffMBytes.Should().BeLessThan(maxMemoryIncreaseInMB, String.Format("{0:0.00}MB left allocated", diffMBytes));
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

        private static void FunctionMemoryUsageLoadInstance(JavascriptContext ctx)
        {
            using (var function = ctx.Run(@"() => { return 1; }") as JavascriptFunction)
                function.Call();
        }
    }
}
