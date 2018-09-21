using System.Threading;
using System.Diagnostics;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;

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
        public void RunIsolatesTest()
        {

            Stopwatch timer = new Stopwatch();
            timer.Start();

            Thread thread = new Thread(RunInstance);
            thread.Start();  // First instance
            RunInstance();   // Second instance
            thread.Join();

            timer.ElapsedMilliseconds.Should().BeLessThan(1999, "It took too long, they must not be running in parallel.");
        }

        static void RunInstance()
        {
            using (JavascriptContext context = new JavascriptContext()) {
                int i = (int)context.Run(@"
var started = new Date();
var i = 0;
while (new Date().getTime() - started.getTime() < 1000)
    i ++;
i;");
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
