using System;
using System.Collections.Generic;
using System.Linq;
using FluentAssertions;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class JavascriptFunctionTests
    {
        private JavascriptContext _context = null!;

        [TestInitialize]
        public void SetUp()
        {
            _context = new JavascriptContext();
        }

        [TestCleanup]
        public void TearDown()
        {
            _context.Dispose();
        }

        [TestMethod]
        public void GetFunctionExpressionFromJsContext()
        {
            _context.Run("a = function(a, b) { return a + b; }");

            JavascriptFunction funcObj = (JavascriptFunction)_context.GetParameter("a");
            funcObj.Should().NotBeNull();
            funcObj.Call(1, 2).Should().BeOfType<int>().Which.Should().Be(3);
        }

        [TestMethod]
        public void EqualsOperatorWorksWithFunctionObjects()
        {
            _context.Run("a = function(a, b) { return a + b; }");

            JavascriptFunction funcObj = (JavascriptFunction)_context.GetParameter("a");
            (funcObj == null).Should().BeFalse();
            (funcObj != null).Should().BeTrue();
            (null == funcObj).Should().BeFalse();
            (null != funcObj).Should().BeTrue();
        }

        [TestMethod]
        public void GetNamedFunctionFromJsContext()
        {
            _context.Run("function test(a, b) { return a + b; }");

            JavascriptFunction funcObj = (JavascriptFunction)_context.GetParameter("test");
            funcObj.Should().NotBeNull();
            funcObj.Call(1, 2).Should().BeOfType<int>().Which.Should().Be(3);
        }

        [TestMethod]
        public void GetArrowFunctionExpressionFromJsContext()
        {
            _context.Run("a = (a, b) => a + b");

            JavascriptFunction funcObj = (JavascriptFunction)_context.GetParameter("a");
            funcObj.Should().NotBeNull();
            funcObj.Call(1, 2).Should().BeOfType<int>().Which.Should().Be(3);
        }

        [TestMethod]
        public void PassFunctionToMethodInManagedObjectAndUseItToFilterAList()
        {
            _context.SetParameter("collection", new CollectionWrapper());

            var result = _context.Run("collection.Filter(x => x % 2 === 0)") as IEnumerable<int>;
            result.Should().NotBeNull();
            result.Should().BeEquivalentTo(new[] { 2, 4 });
        }

        [TestMethod]
        public void ExceptionsAreHandledAndWrappedInAJavascriptExceptionObject()
        {
            var function = _context.Run("() => { throw new Error('test'); }") as JavascriptFunction;
            function.Should().NotBeNull();
            Action action = () => function?.Call();
            action.Should().ThrowExactly<JavascriptException>().WithMessage("Error: test");
        }

        [TestMethod]
        public void ToStringShouldReturnTheFunctionDefinition()
        {
            _context.Run("function test() { return 1; }");
            var funcObj = (JavascriptFunction)_context.GetParameter("test");
            funcObj.Should().NotBeNull();
            funcObj.ToString().Should().Be("function test() { return 1; }");
        }

        [TestMethod]
        public void ToStringShouldReturnTheFunctionDefinitionForAnArrowFunction()
        {
            _context.Run("var test = (a, b) => a + b");
            var funcObj = (JavascriptFunction)_context.GetParameter("test");
            funcObj.Should().NotBeNull();
            funcObj.ToString().Should().Be("(a, b) => a + b");
        }

        [TestMethod]
        public void ToStringShouldReturnTheFunctionDefinitionWithLineBreaks()
        {
            _context.Run(@"
function test() {
    return 1;
}");
            var funcObj = (JavascriptFunction)_context.GetParameter("test");
            funcObj.Should().NotBeNull();
            funcObj.ToString().Should().Be(@"
function test() {
    return 1;
}".Trim());
        }
    }

    [TestClass]
    public class JavascriptFunctionTestsWithoutAutomaticContext
    {
        [TestMethod]
        public void CannotUseAFunctionWhenItsContextIsDisposed()
        {
            JavascriptFunction function;
            using (var context = new JavascriptContext()) {
                function = (JavascriptFunction)context.Run("() => { throw new Error('test'); }");
            }
            Action action = () => function.Call();
            action.Should().ThrowExactly<JavascriptException>().WithMessage("This function's owning JavascriptContext has been disposed");
        }

        [TestMethod]
        public void DisposingAFunction()
        {
            using (var context = new JavascriptContext()) {
                var function = (JavascriptFunction)context.Run("() => { throw new Error('test'); }");
                function.Dispose();
            }
        }
    }

    class CollectionWrapper
    {
        private IEnumerable<int> numbers = new List<int> { 1, 2, 3, 4, 5 };

        public IEnumerable<int> Filter(JavascriptFunction predicate)
        {
            return numbers.Where(x => (bool) predicate.Call(x));
        }
    }
}
