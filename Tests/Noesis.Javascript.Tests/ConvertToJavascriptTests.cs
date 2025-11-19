using System;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;
using System.Text.RegularExpressions;
using System.Numerics;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class ConvertToJavascriptTests
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
        public void SetFloat()
        {
            _context.SetParameter("val", 125.25f);

            _context.Run("val === 125.25").Should().BeOfType<bool>().Which.Should().BeTrue();
        }
        
        [TestMethod]
        public void SetDouble()
        {
            _context.SetParameter("val", 125.25);

            _context.Run("val === 125.25").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetDecimal()
        {
            _context.SetParameter("val", 125.25m);

            _context.Run("val === 125.25").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetInteger()
        {
            _context.SetParameter("val", 600);

            _context.Run("val === 600").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetSignedByte()
        {
            _context.SetParameter("val", (sbyte)65);

            _context.Run("val === 65").Should().BeOfType<bool>().Which.Should().BeTrue();
        }     
        
        [TestMethod]
        public void SetShort()
        {
            _context.SetParameter("val", (short)600);

            _context.Run("val === 600").Should().BeOfType<bool>().Which.Should().BeTrue();
        }
        
        [TestMethod]
        public void SetLong()
        {
            _context.SetParameter("val", (long)60012312321);

            _context.Run("val === 60012312321").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetUnsignedInteger()
        {
            _context.SetParameter("val", uint.MaxValue);

            _context.Run("val === 4294967295").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetByte()
        {
            _context.SetParameter("val", (byte)255);

            _context.Run("val === 255").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetUnsignedShort()
        {
            _context.SetParameter("val", ushort.MaxValue);

            _context.Run("val === 65535").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetUnsignedLong()
        {
            _context.SetParameter("val", ulong.MaxValue);

            _context.Run("val === 18446744073709551615").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetString()
        {
            _context.SetParameter("val", "A string from .NET");

            _context.Run("val === 'A string from .NET'").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetStringWithZeroByte()
        {
            _context.SetParameter("val", "a\0b");

            _context.Run("val === 'a\\0b'").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetStringWithMultipleZeroBytes()
        {
            _context.SetParameter("val", "a\0\0b");

            _context.Run("val === 'a\\0\\0b'").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetArray()
        {
            _context.SetParameter("val", new[]{1,2,3});

            _context.Run("val[0] == 1 && val[1] == 2 && val[2] == 3").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetBoolean()
        {
            _context.SetParameter("val", true);

            _context.Run("val === true").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetRegexWithoutECMAScriptFlagThrowsException()
        {
            Action action = () => _context.SetParameter("val", new Regex("abc"));
            action.Should().Throw<Exception>().WithMessage("Only regular expressions with the ECMAScript option can be converted.");
        }

        [TestMethod]
        public void SetRegexWithECMAScriptFlagOnly()
        {
            _context.SetParameter("val", new Regex("abc", RegexOptions.ECMAScript));

            _context.Run("val.source === 'abc'").Should().BeOfType<bool>().Which.Should().BeTrue();
            _context.Run("val.flags === ''").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetRegexWithFlags()
        {
            _context.SetParameter("val", new Regex("abc", RegexOptions.ECMAScript | RegexOptions.IgnoreCase | RegexOptions.Multiline));

            _context.Run("val.source === 'abc'").Should().BeOfType<bool>().Which.Should().BeTrue();
            _context.Run("val.flags === 'im'").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetBigInt()
        {
            _context.SetParameter("val", new BigInteger(1));
            _context.Run("val === 1n").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetLargeBigInt()
        {
            _context.SetParameter("val", BigInteger.Pow(new BigInteger(2), 222));
            _context.Run("val === 2n ** 222n").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetObject()
        {
            _context.SetParameter("val", new ConvertToJavascriptTests());

            _context.Run("val.ToString()").Should().BeOfType<string>().Which.Should().Be("Noesis.Javascript.Tests.ConvertToJavascriptTests");
        }

        [TestMethod]
        public void SetList()
        {
            List<string> myList = new List<string>() { "Foo", "Bar", "FooBar" };
            _context.SetParameter("myList", myList);

            _context.Run("(myList[0] == 'Foo') && (myList[1] == 'Bar') && (myList[2] == 'FooBar')").Should().BeOfType<bool>().Which.Should().BeTrue();
        }    
        
        [TestMethod]
        public void SetDictionary()
        {
            // Test #10: .NET's Dictionary (Integer as keys) and Test #11: .NET Enumerator
            Dictionary<int, int> myDictionaryByKeyInteger = new Dictionary<int, int>()
                {
                    {2, 8888855},
                    {200, 9191955},
                    {40000, 1236555}
                };
            _context.SetParameter("myDictionaryByKeyInteger", myDictionaryByKeyInteger);

            _context.Run("(myDictionaryByKeyInteger['2'] == 8888855) && (myDictionaryByKeyInteger['200'] == 9191955) && (myDictionaryByKeyInteger['40000'] == 1236555)").Should().BeOfType<bool>().Which.Should().BeTrue();
        }


        
        [TestMethod]
        public void EnumerateDictionary()
        {
            Dictionary<string, int> myDictionaryByKeyString = new Dictionary<string, int>()
                {
                    {"A", 12},
                    {"C", 34},
                    {"E", 56}
                };
            _context.SetParameter("myDictionaryByKeyString", myDictionaryByKeyString);

            _context.Run(@" var counter = 0;
                                        for (var key in myDictionaryByKeyString) {
                                            if (12 == myDictionaryByKeyString[key])
                                                counter++;
                                            if (34 == myDictionaryByKeyString[key])
                                                counter++;
                                            if (56 == myDictionaryByKeyString[key])
                                                counter++;
                                        }
                                        counter == 3
                                        ").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetDelegate()
        {
            _context.SetParameter("delegate", new Func<string,string>((s) => s.ToUpper()));

            _context.Run("delegate('Noesis') == 'NOESIS'").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetDelegateWithArray()
        {
            _context.SetParameter("delegate", new Func<string[], string>((a) => String.Join(" ", a)));

            _context.Run("delegate(['Big', 'dog']) == 'Big dog'").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetEnum()
        {
            _context.SetParameter("val", UriKind.Absolute);

            _context.Run("val == 'Absolute'").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetChar()
        {
            _context.SetParameter("val", 'B');

            _context.Run("val == 'B'").Should().BeOfType<bool>().Which.Should().BeTrue();
        }        

        [TestMethod]
        public void SetNullableFloatToNull()
        {
            float? val = null;
            _context.SetParameter("val", val);

            _context.Run("val === null").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetNullableFloatToValue()
        {
            float? val = 125.25f;
            _context.SetParameter("val", val);

            _context.Run("val == 125.25").Should().BeOfType<bool>().Which.Should().BeTrue();
        }
    }
}
