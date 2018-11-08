using System.Threading;
using System.Diagnostics;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;
using System;

namespace Noesis.Javascript.Tests
{
    /// <summary>
    /// Tests that we can run multiple instances of v8 side by side, 
    /// simultaneously, courtesy of v8 Isolates.
    /// </summary>
    [TestClass]
    public class IsolationTests
    {
        [TestMethod]
        public void IsolatesRunSimultaneously()
        {
            var thread1_started = new EventWaitHandle(false, EventResetMode.ManualReset);
            var thread2_started = new EventWaitHandle(false, EventResetMode.ManualReset);
            Thread thread1 = new Thread(() => RunAndCallbackIntoCsharp(() => {
                thread1_started.Set();
                thread2_started.WaitOne();
            }));
            Thread thread2 = new Thread(() => RunAndCallbackIntoCsharp(() => {
                thread2_started.Set();
                thread1_started.WaitOne();
            }));
            thread1.Start();
            thread2.Start();
            thread1.Join();
            thread2.Join();
        }

        static void RunAndCallbackIntoCsharp(Action run_in_javascript_thread)
        {
            using (JavascriptContext context = new JavascriptContext()) {
                context.SetParameter("csharp_code", run_in_javascript_thread);
                context.Run("csharp_code();");
            }
        }

        [TestMethod]
        public void LotsOfParallelInstances()
        {
            var started_threads = Enumerable.Range(0, 1000)
                                            .Select(_ => {
                                                var thread = new Thread(TaskWithAReasonableAmountOfCompilation);
                                                thread.Start();
                                                return thread;
                                            })
                                            .ToList();
            foreach (var thread in started_threads)
                thread.Join();
        }

        /// <summary>We have had a recurring AccessViolationException in CompileScript() -> NewFromTwoByte()
        /// This is an attempt to reproduce.</summary>
        static void TaskWithAReasonableAmountOfCompilation()
        {
            using (JavascriptContext context = new JavascriptContext()) {
                int i = (int)context.Run(@"
function* fibonacci() {
  var a = 0;
  var b = 1;
  while (true) {
    yield a;
    a = b;
    b = a + b;
  }
}

// Instantiates the fibonacci generator
fib = fibonacci();

// gets first 10 numbers from the Fibonacci generator starting from 0
var seq = [];
for (let i = 0; i < 10; i++) {
  seq.push(fib.next().value);
}

var i = 0;
for (i = 0; i < 1000; i ++)
    i ++;
i;");
            }
        }
    }
}
