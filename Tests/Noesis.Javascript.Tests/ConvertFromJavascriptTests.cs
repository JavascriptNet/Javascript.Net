using System;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;
using System.Text.RegularExpressions;
using System.Numerics;
using System.Diagnostics.CodeAnalysis;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class ConvertFromJavascriptTests
    {
        private JavascriptContext _context = null!;

        private class TypedPropertiesClass
        {
            public Decimal decimalValue { get; set; }
            public float? nullableFloat { get; set; }

            public int methodWithoutParameters()
            {
                return 1;
            }

            public int methodWithOneParameter(int i)
            {
                return i + 1;
            }

            public int methodWithEnumParameter(UriHostNameType t)
            {
                return (int)t;
            }

            public string methodWithMultipleMixedParameters(int i, string s, bool b)
            {
                return String.Format("i: {0}, s: {1}, b: {2}", i, s, b);
            }

            public string methodWithDefaultParameter(string s = "")
            {
                return s;
            }

            public string methodWithRequiredAndDefaultParameters(int i, string s = "abc", bool b = true)
            {
                return String.Format("i: {0}, s: {1}, b: {2}", i, s, b);
            }
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
        public void ReadStringWithZeroByte()
        {
            _context.Run("var myString = 'a\\0b'");

            _context.GetParameter("myString").Should().BeOfType<string>().Which.Should().Be("a\0b");
        }

        [TestMethod]
        public void ReadStringWithMultipleZeroBytes()
        {
            _context.Run("var myString = 'a\\0\\0b'");

            _context.GetParameter("myString").Should().BeOfType<string>().Which.Should().Be("a\0\0b");
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
        public void ReadRegExpLiteral()
        {
            _context.Run("var myRegExp = /abc/gim");

            var regex = _context.GetParameter("myRegExp");
            regex.Should().BeOfType<Regex>().Which.Should().BeEquivalentTo(new Regex("abc", RegexOptions.ECMAScript | RegexOptions.IgnoreCase | RegexOptions.Multiline));
        }

        [TestMethod]
        public void ReadRegExpObject()
        {
            _context.Run("var myRegExp = new RegExp('abc', 'gim')");

            var regex = _context.GetParameter("myRegExp");
            regex.Should().BeOfType<Regex>().Which.Should().BeEquivalentTo(new Regex("abc", RegexOptions.ECMAScript | RegexOptions.IgnoreCase | RegexOptions.Multiline));
        }

        [TestMethod]
        public void ReadBigIntLiteral()
        {
            var bigInt = _context.Run("1n");
            bigInt.Should().BeOfType<BigInteger>().Which.Should().Be(new BigInteger(1));
        }

        [TestMethod]
        public void ReadBigIntObject()
        {
            var bigInt = _context.Run("BigInt(1)");
            bigInt.Should().BeOfType<BigInteger>().Which.Should().Be(new BigInteger(1));
        }

        [TestMethod]
        public void ReadLargeBigIntLiteral()
        {
            var bigInt = _context.Run("2n ** 222n");
            var expected = BigInteger.Pow(new BigInteger(2), 222);
            bigInt.Should().BeOfType<BigInteger>().Which.Should().Be(expected);
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

        [TestMethod]
        public void MethodCallWithoutParameters()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            var result = _context.Run("obj.methodWithoutParameters()");

            result.Should().Be(1);
        }

        [TestMethod]
        public void MethodCallWithoutParameters_RedundantArgumentsAreIgnored()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            var result = _context.Run("obj.methodWithoutParameters(42)");

            result.Should().Be(1);
        }

        [TestMethod]
        public void MethodCallWithParameter()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            var result = _context.Run("obj.methodWithOneParameter(1)");

            result.Should().Be(2);
        }

        [TestMethod]
        public void MethodCallWithEnumParameter()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            var result = _context.Run("obj.methodWithEnumParameter('IPv4')");
            result.Should().Be((int)UriHostNameType.IPv4);
        }

        [TestMethod]
        public void MethodCallWithBadEnumParameter()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            Action action = () => _context.Run("obj.methodWithEnumParameter('dog')");
            action.Should().Throw<JavascriptException>("'dog' is not a member of the required enum")
                  .Which.Message.Should().Contain("Argument mismatch");
        }

        [TestMethod]
        public void MethodCallWithMixedParameter()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            var result = _context.Run("obj.methodWithMultipleMixedParameters(1, 'test', false)");

            result.Should().Be("i: 1, s: test, b: False");
        }

        [TestMethod]
        public void MethodCallWithDefaultParameter_PassingNoActualParameter()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            var result = _context.Run("obj.methodWithDefaultParameter()");

            result.Should().Be("");
        }

        [TestMethod]
        public void MethodCallWithDefaultParameter_PassingActualParameter()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            var result = _context.Run("obj.methodWithDefaultParameter('foo')");

            result.Should().Be("foo");
        }

        [TestMethod]
        public void MethodCallWithDefaultParameter_PassingExcplicitNullAsActualParameter()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            var result = _context.Run("obj.methodWithDefaultParameter(null)");

            result.Should().Be(null);
        }

        [TestMethod]
        public void MethodCallWithRequiredAndDefaultParameters_PassingNoOptionalActualParameter()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            var result = _context.Run("obj.methodWithRequiredAndDefaultParameters(1)");

            result.Should().Be("i: 1, s: abc, b: True");
        }

        [TestMethod]
        public void MethodCallWithRequiredAndDefaultParameters_PassingAllActualParameter()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            var result = _context.Run("obj.methodWithRequiredAndDefaultParameters(1, 'test', false)");

            result.Should().Be("i: 1, s: test, b: False");
        }

        [TestMethod]
        public void MethodCallWithRequiredAndDefaultParameters_PassingUndefinedForAllOptionalActualParameter()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            var result = _context.Run("obj.methodWithRequiredAndDefaultParameters(1, undefined, undefined)");

            result.Should().Be("i: 1, s: abc, b: True");
        }

        [TestMethod]
        public void MethodCallWithRequiredAndDefaultParameters_PassingOnlyOneOptionalActualParameterLeavingTheMiddleOneUndefined()
        {
            var obj = new TypedPropertiesClass();
            _context.SetParameter("obj", obj);
            var result = _context.Run("obj.methodWithRequiredAndDefaultParameters(1, undefined, false)");

            result.Should().Be("i: 1, s: abc, b: False");
        }
    }
}
