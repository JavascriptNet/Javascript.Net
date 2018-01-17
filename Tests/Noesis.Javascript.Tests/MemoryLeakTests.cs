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
            MemoryUsageLoadInstance();
            long mem = Process.GetCurrentProcess().PrivateMemorySize64;

            for (int i = 0; i < 20; i++) {
                MemoryUsageLoadInstance();
            }
            GC.Collect();
            GC.Collect();
            decimal diffMBytes = (Process.GetCurrentProcess().PrivateMemorySize64 - mem) / 1048576m;

            diffMBytes.Should().BeLessThan(1, String.Format("{0:0.00}MB left allocated", diffMBytes));
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
