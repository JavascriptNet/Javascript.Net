using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Diagnostics;

namespace Noesis.Javascript.Tests
{
    /// <summary>
    /// Tests that we can run multiple instances of v8 side by side, 
    /// simultaneously, courtesy of v8 Isolates.
    /// </summary>
    public partial class RegressionTests
    {
        static DateTime finishTime;

        public static string RunIsolatesTest(string js_dir)
        {
            finishTime = new DateTime().AddSeconds(1);

            Stopwatch timer = new Stopwatch();
            timer.Start();

            Thread thread = new Thread(RunInstance);
            thread.Start();  // First instance
            RunInstance();   // Second instance
            thread.Join();

            if (timer.ElapsedMilliseconds > 1500)
                return "It took too long, they must not be running in parallel.";

            return null;
        }

        static void RunInstance()
        {
            using (JavascriptContext context = new JavascriptContext()) {
                context.SetParameter("finishTime", finishTime);
                int i = (int)context.Run(@"
var started = new Date();
var i = 0;
while (new Date().getTime() - started.getTime() < 1000)
    i ++;
i;");
                Console.WriteLine(String.Format("Counted to {0}", i));
            }
        }
    }
}
