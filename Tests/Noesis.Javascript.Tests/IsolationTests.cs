using System.Threading;
using System.Diagnostics;
using NUnit.Framework;

namespace Noesis.Javascript.Tests
{
    /// <summary>
    /// Tests that we can run multiple instances of v8 side by side, 
    /// simultaneously, courtesy of v8 Isolates.
    /// </summary>
    [TestFixture]
    public class IsolationTests
    {
        [Test]
        public void RunIsolatesTest()
        {

            Stopwatch timer = new Stopwatch();
            timer.Start();

            Thread thread = new Thread(RunInstance);
            thread.Start();  // First instance
            RunInstance();   // Second instance
            thread.Join();

            Assert.That(timer.ElapsedMilliseconds, Is.LessThan(1500), "It took too long, they must not be running in parallel.");
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
    }
}
