using System;
using System.Diagnostics;
using NUnit.Framework;

namespace Noesis.Javascript.Tests
{
    [TestFixture]
    public class MemoryLeakTests
    {
        [Test]
        public void RunMemoryLeakTest()
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

            Assert.That(diffMBytes < 1, String.Format("{0:0.00}MB left allocated", diffMBytes));
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
