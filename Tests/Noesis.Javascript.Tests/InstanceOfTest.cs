
using FluentAssertions;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class InstanceOfTest
    {
        private JavascriptContext _context = null!;

        private class TestClass
        {
            public int foo { get; set; }
        }

        [TestInitialize]
        public void SetUp()
        {
            _context = new JavascriptContext();
        }

        [TestMethod]
        public void RegisteredConstructorInstanceOfTest()
        {
            _context.SetConstructor("Test", typeof(TestClass), new Func<TestClass>(() => new TestClass()));
            _context.Run("(new Test()) instanceof Test").Should().Be(true);
        }

        [TestMethod]
        public void RegisterConstructorUsingGenericTest()
        {
            _context.SetConstructor<TestClass>("Test", new Func<TestClass>(() => new TestClass()));
            _context.Run("(new Test()) instanceof Test").Should().Be(true);
        }

        [TestMethod]
        public void RegisteredConstructorInstanceOfWorksWithCSharpObjectTest()
        {
            _context.SetConstructor("Test", typeof(TestClass), new Func<TestClass>(() => new TestClass()));
            _context.SetParameter("test", new TestClass());
            _context.Run("test instanceof Test").Should().Be(true);
        }

        [TestMethod]
        public void ManipulatingPropertyWorks()
        {
            _context.SetConstructor("Test", typeof(TestClass), new Func<TestClass>(() => new TestClass()));
            var testObject = new TestClass { foo = 42 };
            _context.SetParameter("test", testObject);
            _context.Run("test.foo").Should().Be(42);
            _context.Run("test.foo = 1");
            testObject.foo.Should().Be(1);
        }

        [TestMethod]
        public void UnregisteredObjectInstanceOfTest()
        {
            _context.SetParameter("test", new TestClass());
            _context.Invoking(x => x.Run("test instanceof Test"))
                .Should().Throw<JavascriptException>().WithMessage("ReferenceError: Test is not defined");
        }

        [TestMethod]
        public void ExtensionMethodIsPossible()
        {
            _context.SetConstructor("Test", typeof(TestClass), new Func<TestClass>(() => new TestClass()));
            _context.Run("Test.prototype.fooDuplicated = function() { return this.foo * 2; }");
            _context.Run("let x = new Test(); x.foo = 4; x.fooDuplicated() == 8").Should().Be(true);
        }
    }
}
