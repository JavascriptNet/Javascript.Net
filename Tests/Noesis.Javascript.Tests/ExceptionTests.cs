using System;
using NUnit.Framework;

namespace Noesis.Javascript.Tests
{
    [TestFixture]
    public class ExceptionTests
    {
        private JavascriptContext _context;

        [SetUp]
        public void SetUp()
        {
            _context = new JavascriptContext();
        }

        [TearDown]
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

        [Test]
        public void HandleInvalidArgumentsInIndexerCall()
        {
            _context.SetParameter("obj", new ClassWithIndexer());

            Assert.That(() => _context.Run("obj[1] = 123 /* passing int when expecting string */"),
                Throws.InstanceOf<JavascriptException>().With.Message.Matches("Object of type 'System.Int32' cannot be converted to type 'System.String"));
        }

        class ClassWithMethods
        {
            public void Method(ClassWithMethods a) {}
            public void MethodThatThrows() { throw new Exception("Test C# exception"); }
        }

        [Test]
        public void HandleInvalidArgumentsInMethodCall()
        {
            _context.SetParameter("obj", new ClassWithMethods());

            Assert.That(() => _context.Run("obj.Method('hello') /* passing string when expecting int */"),
                Throws.InstanceOf<JavascriptException>().With.Message.EqualTo("Argument mismatch for method \"Method\"."));
        }

        [Test]
        public void HandleExceptionWhenInvokingMethodOnManagedObject()
        {
            _context.SetParameter("obj", new ClassWithMethods());

            Assert.That(() => _context.Run("obj.MethodThatThrows()"),
                        Throws.InstanceOf<JavascriptException>().With.Message.EqualTo("Test C# exception"));
        }

        [Test]
        public void StackOverflow()
        {
            Assert.That(() => _context.Run("function f() { f(); }; f();"),
                        Throws.InstanceOf<JavascriptException>().With.Message.EqualTo("RangeError: Maximum call stack size exceeded"));
        }
    }
}
