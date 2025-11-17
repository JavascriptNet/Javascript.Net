using System;
using FluentAssertions;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class FlagsTest
    {
        [TestMethod]
        public void CanUseEngineFlagsToSpecifyStrictMode()
        {
            JavascriptContext.SetFlags("--use_strict");
            Action action = () =>
            {
                using (var context = new JavascriptContext())
                    context.Run("globalVariable = 1;");
            };
            action.Should().ThrowExactly<JavascriptException>().WithMessage("ReferenceError: globalVariable is not defined");
            JavascriptContext.SetFlags("--nouse_strict");
        }
    }
}
