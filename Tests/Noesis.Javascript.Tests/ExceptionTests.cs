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
        private JavascriptContext _context = null!;

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

        [TestMethod]
        public void ThrowNewError()
        {
            Action action = () => _context.Run("throw new Error('asdf');");
            action.Should().ThrowExactly<JavascriptException>().WithMessage("Error: asdf");
        }

        [TestMethod]
        public void ThrowNewErrorWithZeroByte()
        {
            Action action = () => _context.Run("throw new Error('asdf\\0qwer');");
            action.Should().ThrowExactly<JavascriptException>().WithMessage("Error: asdf\0qwer");
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
            action.Should().ThrowExactly<JavascriptException>().WithMessage("Object of type 'System.Int32' cannot be converted to type 'System.String'.");
        }

        class ClassWithMethods
        {
            public void Method(ClassWithMethods a) {}
            public void MethodThatThrows() { throw new Exception("Test C# exception"); }
            public void MethodThatThrowsWithZeroByte() { throw new Exception("Test C#\0exception"); }
        }

        [TestMethod]
        public void HandleInvalidArgumentsInMethodCall()
        {
            _context.SetParameter("obj", new ClassWithMethods());

            Action action = () => _context.Run("obj.Method('hello') /* passing string when expecting int */");
            action.Should().ThrowExactly<JavascriptException>().WithMessage("Argument mismatch for method \"Method\".");
        }

        [TestMethod]
        public void HandleExceptionWhenInvokingMethodOnManagedObject()
        {
            _context.SetParameter("obj", new ClassWithMethods());

            Action action = () => _context.Run("obj.MethodThatThrows()");
            action.Should().ThrowExactly<JavascriptException>().WithMessage("Test C# exception");
        }

        [TestMethod]
        public void HandleExceptionWhenInvokingMethodOnManagedObjectWithZeroByteInMessage()
        {
            _context.SetParameter("obj", new ClassWithMethods());

            Action action = () => _context.Run("obj.MethodThatThrowsWithZeroByte()");
            action.Should().ThrowExactly<JavascriptException>().WithMessage("Test C#\0exception");
        }

        [TestMethod]
        public void StackOverflow()
        {
            Action action = () => _context.Run("function f() { f(); }; f();");
            action.Should().ThrowExactly<JavascriptException>().WithMessage("RangeError: Maximum call stack size exceeded");
        }

        [TestMethod]
        public void ArgumentChecking()
        {
            Action action = () => _context.Run(null);
            action.Should().ThrowExactly<ArgumentNullException>();
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
            action.Should().Throw<AggregateException>("Because it was cancelled")
                .WithInnerException<JavascriptException>()
                .WithMessage("Execution Terminated");
        }
    }
}
