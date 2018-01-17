using System;
using System.Collections.Generic;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class ConvertToJavascriptTests
    {        
        private JavascriptContext _context;

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

            _context.Run("val == 125.25").Should().BeOfType<bool>().Which.Should().BeTrue();
        }
        
        [TestMethod]
        public void SetDouble()
        {
            _context.SetParameter("val", 125.25);

            _context.Run("val == 125.25").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetInteger()
        {
            _context.SetParameter("val", 600);

            _context.Run("val == 600").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetSignedByte()
        {
            _context.SetParameter("val", (sbyte)65);

            _context.Run("val == 65").Should().BeOfType<bool>().Which.Should().BeTrue();
        }     
        
        [TestMethod]
        public void SetShort()
        {
            _context.SetParameter("val", (short)600);

            _context.Run("val == 600").Should().BeOfType<bool>().Which.Should().BeTrue();
        }
        
        [TestMethod]
        public void SetLong()
        {
            _context.SetParameter("val", (long)60012312321);

            _context.Run("val == 60012312321").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetUnsignedInteger()
        {
            _context.SetParameter("val", uint.MaxValue);

            _context.Run("val == 4294967295").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetByte()
        {
            _context.SetParameter("val", (byte)255);

            _context.Run("val == 255").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetUnsignedShort()
        {
            _context.SetParameter("val", ushort.MaxValue);

            _context.Run("val == 65535").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetUnsignedLong()
        {
            _context.SetParameter("val", ulong.MaxValue);

            _context.Run("val == 18446744073709551615").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetString()
        {
            _context.SetParameter("val", "A string from .NET");

            _context.Run("val == 'A string from .NET'").Should().BeOfType<bool>().Which.Should().BeTrue();
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

            _context.Run("val == true").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        [TestMethod]
        public void SetDateTime()
        {
            _context.SetParameter("val", new DateTime(2010,10,10));

            _context.Run("val.getUTCFullYear() == 2010").Should().BeOfType<bool>().Which.Should().BeTrue();
            _context.Run("val.getMonth() == 9").Should().BeOfType<bool>().Which.Should().BeTrue();
            _context.Run("val.getDate() == 10").Should().BeOfType<bool>().Which.Should().BeTrue();
        }        
        
        [TestMethod]
        public void SetObject()
        {
            _context.SetParameter("val", new ConvertToJavascriptTests());

            _context.Run("val.ToString() == 'Noesis.Javascript.Tests.ConvertToJavascriptTests'").Should().BeOfType<bool>().Which.Should().BeTrue();
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
    }
}
