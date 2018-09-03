using System;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class ConvertFromJavascriptTests
    {
        private JavascriptContext _context;

        private class TypedPropertiesClass
        {
            public Decimal decimalValue { get; set; }
            public float? nullableFloat { get; set; }
        }

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
        public void ReadFloat()
        {
            _context.Run("var myFloat = 10.0125");
            
            _context.GetParameter("myFloat").Should().BeOfType<double>().Which.Should().Be(10.0125);
        }

        [TestMethod]
        public void AssignNullToNullableFloat()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            _context.Run("obj.nullableFloat = null");

            obj.nullableFloat.Should().Be(null);
        }

        [TestMethod]
        public void AssignValueToNullableFloat()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            _context.Run("obj.nullableFloat = 123.45");

            obj.nullableFloat.Should().Be(123.45f);
        }

        [TestMethod]
        public void AssignToDecimal()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            _context.Run("obj.decimalValue = 123.45");

            obj.decimalValue.Should().Be(123.45m);
        }

        [TestMethod]
        public void ReadInteger()
        {
            _context.Run("var myInteger = 1000");

            _context.GetParameter("myInteger").Should().BeOfType<int>().Which.Should().Be(1000);
        }

        [TestMethod]
        public void ReadString()
        {
            _context.Run("var myString = 'This is the string from Javascript'");

            _context.GetParameter("myString").Should().BeOfType<string>().Which.Should().Be("This is the string from Javascript");
        }

        [TestMethod]
        public void ReadArray()
        {
            _context.Run("var myArray = [11,22,33]");

            Array jsArray = (Array)_context.GetParameter("myArray");

            jsArray.Should().BeEquivalentTo(new[] {11, 22, 33});
        }

        [TestMethod]
        public void ReadBooleanFalse()
        {
            _context.Run("var myBool = false");

            _context.GetParameter("myBool").Should().BeOfType<bool>().Which.Should().BeFalse();
        }

        [TestMethod]
        public void ReadBooleanTrue()
        {
            _context.Run("var myBool = true");

            _context.GetParameter("myBool").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void ReadObject()
        {
            _context.Run(@"var myObject = new Object();
                           myObject.foo = 'new property';
                           myObject.bar = 123456;");

            Dictionary<string, Object> jsObject = (Dictionary<string, Object>)_context.GetParameter("myObject");

            jsObject["foo"].Should().BeOfType<string>().Which.Should().Be("new property");
            jsObject["bar"].Should().BeOfType<int>().Which.Should().Be(123456);
        }

        [TestMethod]
        public void ReadExternalObject()
        {
            _context.SetParameter("myExternal", new ConvertFromJavascriptTests());
            _context.GetParameter("myExternal").Should().BeOfType<ConvertFromJavascriptTests>();
        }

        [TestMethod]
        public void ReadDoubleByteChar()
        {
            _context.SetParameter("UniString", "呵呵呵呵呵");
            _context.GetParameter("UniString").Should().BeOfType<string>().Which.Should().Be("呵呵呵呵呵");
        }

        [TestMethod]
        public void SelfReferentialObjectDoesNotCauseStackOverflow()
        {
            _context.Run("a = {}; a.a = a");
        }

        [TestMethod]
        public void SelfReferentialArrayDoesNotCauseStackOverflow()
        {
            _context.Run("a = []; a.push(a)");
        }
    }
}
