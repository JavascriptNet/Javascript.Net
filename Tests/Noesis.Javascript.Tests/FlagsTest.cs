using System;
using FluentAssertions;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class FlagsTest
    {
        [AssemblyInitialize]
        public static void GlobalTestInitialize(TestContext testContext)
        {
            // this method must only be called once before V8 is initialized (i.e. before `UnmanagedInitialisation` has run)
            JavascriptContext.IsV8Initialized.Should().BeFalse("V8 was already initialized which shouldn't be the case here!");
            JavascriptContext.SetFlags("--stack_size 256");
        }

        [TestMethod]
        public void CannotSetFlagsAfterV8IsInitialized()
        {
            using (var context = new JavascriptContext()) // create a dummy context to ensure V8 is definitely initialized before running the test code
            {
                JavascriptContext.IsV8Initialized.Should().BeTrue();
                Action action = () => JavascriptContext.SetFlags("--use-strict");
                action.Should().ThrowExactly<InvalidOperationException>().WithMessage("Flags can only be set once before the first context and therefore V8 is initialized.");
            }
        }
    }
}
