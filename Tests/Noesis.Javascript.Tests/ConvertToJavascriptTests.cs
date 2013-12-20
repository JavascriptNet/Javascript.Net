using System;
using System.Collections.Generic;
using NUnit.Framework;

namespace Noesis.Javascript.Tests
{
    [TestFixture]
    public class ConvertToJavascriptTests
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

        [Test]
        public void SetFloat()
        {
            _context.SetParameter("val", 125.25f);

            Assert.That(_context.Run("val == 125.25"), Is.True);
        }
        
        [Test]
        public void SetDouble()
        {
            _context.SetParameter("val", 125.25);

            Assert.That(_context.Run("val == 125.25"), Is.True);
        }

        [Test]
        public void SetInteger()
        {
            _context.SetParameter("val", 600);

            Assert.That(_context.Run("val == 600"), Is.True);
        }

        [Test]
        public void SetSignedByte()
        {
            _context.SetParameter("val", (sbyte)65);

            Assert.That(_context.Run("val == 65"), Is.True);
        }     
        
        [Test]
        public void SetShort()
        {
            _context.SetParameter("val", (short)600);

            Assert.That(_context.Run("val == 600"), Is.True);
        }
        
        [Test]
        public void SetLong()
        {
            _context.SetParameter("val", (long)60012312321);

            Assert.That(_context.Run("val == 60012312321"), Is.True);
        }

        [Test]
        public void SetUnsignedInteger()
        {
            _context.SetParameter("val", uint.MaxValue);

            Assert.That(_context.Run("val == 4294967295"), Is.True);
        }

        [Test]
        public void SetByte()
        {
            _context.SetParameter("val", (byte)255);

            Assert.That(_context.Run("val == 255"), Is.True);
        }

        [Test]
        public void SetUnsignedShort()
        {
            _context.SetParameter("val", ushort.MaxValue);

            Assert.That(_context.Run("val == 65535"), Is.True);
        }

        [Test]
        public void SetUnsignedLong()
        {
            _context.SetParameter("val", ulong.MaxValue);

            Assert.That(_context.Run("val == 18446744073709551615"), Is.True);
        }

        [Test]
        public void SetString()
        {
            _context.SetParameter("val", "A string from .NET");

            Assert.That(_context.Run("val == 'A string from .NET'"), Is.True);
        }
        
        [Test]
        public void SetArray()
        {
            _context.SetParameter("val", new[]{1,2,3});

            Assert.That(_context.Run("val[0] == 1 && val[1] == 2 && val[2] == 3"), Is.True);
        }

        [Test]
        public void SetBoolean()
        {
            _context.SetParameter("val", true);

            Assert.That(_context.Run("val == true"), Is.True);
        }

        [Test]
        public void SetDateTime()
        {
            _context.SetParameter("val", new DateTime(2010,10,10));

            Assert.That(_context.Run("val.getUTCFullYear() == 2010"), Is.True);
            Assert.That(_context.Run("val.getMonth() == 9"), Is.True);
            Assert.That(_context.Run("val.getDate() == 10"), Is.True);
        }        
        
        [Test]
        public void SetObject()
        {
            _context.SetParameter("val", new ConvertToJavascriptTests());

            Assert.That(_context.Run("val.ToString() == 'Noesis.Javascript.Tests.ConvertToJavascriptTests'"), Is.True);
        }

        [Test]
        public void SetList()
        {
            List<string> myList = new List<string>() { "Foo", "Bar", "FooBar" };
            _context.SetParameter("myList", myList);

            Assert.That(_context.Run("(myList[0] == 'Foo') && (myList[1] == 'Bar') && (myList[2] == 'FooBar')"), Is.True);
        }    
        
        [Test]
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

            Assert.That(_context.Run("(myDictionaryByKeyInteger['2'] == 8888855) && (myDictionaryByKeyInteger['200'] == 9191955) && (myDictionaryByKeyInteger['40000'] == 1236555)"), Is.True);
        }


        
        [Test]
        public void EnumerateDictionary()
        {
            Dictionary<string, int> myDictionaryByKeyString = new Dictionary<string, int>()
                {
                    {"A", 12},
                    {"C", 34},
                    {"E", 56}
                };
            _context.SetParameter("myDictionaryByKeyString", myDictionaryByKeyString);

            Assert.That(_context.Run(@" var counter = 0;
                                        for (var key in myDictionaryByKeyString) {
                                            if (12 == myDictionaryByKeyString[key])
                                                counter++;
                                            if (34 == myDictionaryByKeyString[key])
                                                counter++;
                                            if (56 == myDictionaryByKeyString[key])
                                                counter++;
                                        }
                                        counter == 3
                                        "), Is.True);
        }

        [Test]
        public void SetDelegate()
        {
            _context.SetParameter("delegate", new Func<string,string>((s) => s.ToUpper()));

            Assert.That(_context.Run("delegate('Noesis') == 'NOESIS'"), Is.True);
        }

        [Test]
        public void SetDelegateWithArray()
        {
            _context.SetParameter("delegate", new Func<string[], string>((a) => String.Join(" ", a)));

            Assert.That(_context.Run("delegate(['Big', 'dog']) == 'Big dog'"), Is.True);
        }

        [Test]
        public void SetEnum()
        {
            _context.SetParameter("val", UriKind.Absolute);

            Assert.That(_context.Run("val == 'Absolute'"), Is.True);
        }

        [Test]
        public void SetChar()
        {
            _context.SetParameter("val", 'B');

            Assert.That(_context.Run("val == 'B'"), Is.True);
        }        
    }
}
