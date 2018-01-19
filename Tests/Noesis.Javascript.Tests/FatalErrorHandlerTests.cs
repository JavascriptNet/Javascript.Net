using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using FluentAssertions;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class FatalErrorHandlerTests
    {
        // Does not work because v8 terminates the entire process.  But you can still veryify that it works in the debugger.
        /*[TestMethod]*/
        public void OutOfMemory()
        {
            var signalled = false;
            JavascriptContext.FatalErrorHandler fatal_error_handler = (a, b) => {
                b.Should().Be("Allocation failed - JavaScript heap out of memory");
                signalled = true;
            };
            JavascriptContext.SetFatalErrorHandler(fatal_error_handler);
            using (var context = new JavascriptContext()) {
                context.Run("a = []; for (var i = 0; i < 2000000000; i ++) a.push(42.0)");
            }
            signalled.Should().BeTrue();
        }

        // Does not work because v8 crashes in x86 when it is no longer able to create an isolate.
        // On x64 COM gets angry because it cannot switch contexts after some time.
        /*[TestMethod]*/
        public void ErrorHandlingCreatingContexts()
        {
            // FatalErrorHandler won't work here because it is installed only once the context
            // is created.
            var contexts = new List<JavascriptContext>();
            int i;
            for (i = 0; i < 2000000000; i ++)
                contexts.Add(new JavascriptContext());
            // CRASH!
        }
    }
}
