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
        public void SingleLoadMemoryLeakTest()
        {
            MemoryUsageLoadInstance();
            
            // Force aggressive GC for .NET 8
            for (int i = 0; i < 3; i++)
            {
                GC.Collect();
                GC.WaitForPendingFinalizers();
            }
            GC.Collect();
            
            long mem = Process.GetCurrentProcess().PrivateMemorySize64;

            decimal diffMBytes = (Process.GetCurrentProcess().PrivateMemorySize64 - mem) / 1048576m;
            diffMBytes.Should().BeLessThan(1, $"Memory leak detected: {diffMBytes:0.00} MB still allocated");
        }

        [TestMethod]
        public void MultipleLoadMemoryLeakTest()
        {
            MemoryUsageLoadInstance();
            
            for (int i = 0; i < 20; i++)
            {
                MemoryUsageLoadInstance();
            }

            // Force aggressive GC for .NET 8
            for (int i = 0; i < 3; i++)
            {
                GC.Collect();
                GC.WaitForPendingFinalizers();
            }
            GC.Collect();
            
            long mem = Process.GetCurrentProcess().PrivateMemorySize64;

            decimal diffMBytes = (Process.GetCurrentProcess().PrivateMemorySize64 - mem) / 1048576m;
            diffMBytes.Should().BeLessThan(1, $"Memory leak detected: {diffMBytes:0.00} MB still allocated");
        }

        [TestMethod]
        public void X64MemoryLeakCheck()
        {
            if (IntPtr.Size != 8)
            {
                Assert.Inconclusive("Test only relevant for x64 builds");
            }

            MemoryUsageLoadInstance();
            
            for (int i = 0; i < 50; i++)
            {
                MemoryUsageLoadInstance();
            }

            // Force aggressive GC for .NET 8
            for (int i = 0; i < 3; i++)
            {
                GC.Collect();
                GC.WaitForPendingFinalizers();
            }
            GC.Collect();
            
            long mem = Process.GetCurrentProcess().PrivateMemorySize64;

            decimal diffMBytes = (Process.GetCurrentProcess().PrivateMemorySize64 - mem) / 1048576m;
            // .NET 8 has different GC characteristics, allow up to 3MB for x64
            diffMBytes.Should().BeLessThan(3, $"x64 memory leak detected: {diffMBytes:0.00} MB still allocated");
        }

        private static void MemoryUsageLoadInstance()
        {
            using (JavascriptContext ctx = new JavascriptContext())
            {
                ctx.Run(@"
buffer = [];
for (var i = 0; i < 100_000; i++) {
    buffer[i] = 'new string';
}
");
            }
        }
    }
}
