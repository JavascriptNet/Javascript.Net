using System;
using System.Globalization;
using System.Threading;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;
using System.Threading.Tasks;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class ExceptionTests
    {
        private JavascriptContext _context;

        [TestInitialize]
        public void SetUp()
        {
            _context = new JavascriptContext();
            Thread.CurrentThread.CurrentUICulture = new CultureInfo("en-US");
        }

        [TestCleanup]
        public void TearDown()
        {
            _context.Dispose();
        }

        class ClassWithIndexer
        {
            public string this[int index]
            {
                get { return ""; }
                set { }
            }
        }

        [TestMethod]
        public void HandleInvalidArgumentsInIndexerCall()
        {
            _context.SetParameter("obj", new ClassWithIndexer());

            Action action = () => _context.Run("obj[1] = 123 /* passing int when expecting string */");
            action.ShouldThrowExactly<JavascriptException>().WithMessage("Object of type 'System.Int32' cannot be converted to type 'System.String'.");
        }

        class ClassWithMethods
        {
            public void Method(ClassWithMethods a) {}
            public void MethodThatThrows() { throw new Exception("Test C# exception"); }
        }

        [TestMethod]
        public void HandleInvalidArgumentsInMethodCall()
        {
            _context.SetParameter("obj", new ClassWithMethods());

            Action action = () => _context.Run("obj.Method('hello') /* passing string when expecting int */");
            action.ShouldThrowExactly<JavascriptException>().WithMessage("Argument mismatch for method \"Method\".");
        }

        [TestMethod]
        public void HandleExceptionWhenInvokingMethodOnManagedObject()
        {
            _context.SetParameter("obj", new ClassWithMethods());

            Action action = () => _context.Run("obj.MethodThatThrows()");
            action.ShouldThrowExactly<JavascriptException>().WithMessage("Test C# exception");
        }

        [TestMethod]
        public void StackOverflow()
        {
            Action action = () => _context.Run("function f() { f(); }; f();");
            action.ShouldThrowExactly<JavascriptException>().WithMessage("RangeError: Maximum call stack size exceeded");
        }

        [TestMethod]
        public void ArgumentChecking()
        {
            Action action = () => _context.Run(null);
            action.ShouldThrowExactly<ArgumentNullException>();
        }

        [TestMethod]
        public void TerminateExecutionHasNoRaceCondition()
        {
            var task = new Task(() => {
                _context.Run("while (true) {}");
            });
            task.Start();
            _context.TerminateExecution(true);
            Action action = () => task.Wait(10 * 1000);
            action.ShouldThrow<JavascriptException>("Because it was cancelled");
        }
    }
}
