using System;
using System.Collections.Generic;
using NUnit.Framework;

namespace Noesis.Javascript.Tests
{
    [TestFixture]
    public class ConvertFromJavascriptTests
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
        public void ReadFloat()
        {
            _context.Run("var myFloat = 10.0125");
            
            Assert.That(_context.GetParameter("myFloat"), Is.InstanceOf<double>().And.EqualTo(10.0125));
        }

        [Test]
        public void ReadInteger()
        {
            _context.Run("var myInteger = 1000");

            Assert.That(_context.GetParameter("myInteger"), Is.InstanceOf<int>().And.EqualTo(1000));
        }

        [Test]
        public void ReadString()
        {
            _context.Run("var myString = 'This is the string from Javascript'");

            Assert.That(_context.GetParameter("myString"), Is.EqualTo("This is the string from Javascript"));
        }

        [Test]
        public void ReadArray()
        {
            _context.Run("var myArray = [11,22,33]");

            Array jsArray = (Array)_context.GetParameter("myArray");

            Assert.That(jsArray, Is.EquivalentTo(new[] {11, 22, 33}));
        }

        [Test]
        public void ReadBooleanFalse()
        {
            _context.Run("var myBool = false");

            Assert.That(_context.GetParameter("myBool"), Is.False);
        }

        [Test]
        public void ReadBooleanTrue()
        {
            _context.Run("var myBool = true");

            Assert.That(_context.GetParameter("myBool"), Is.True);
        }

        [Test]
        public void ReadDate()
        {
            _context.Run("var myDate = new Date(2010,9,10)");

            Assert.That(_context.GetParameter("myDate"), Is.EqualTo(new DateTime(2010, 10, 10)));
        }

        [Test]
        public void ReadObject()
        {
            _context.Run(@"var myObject = new Object();
                           myObject.foo = 'new property';
                           myObject.bar = 123456;");

            Dictionary<string, Object> jsObject = (Dictionary<string, Object>)_context.GetParameter("myObject");

            Assert.That(jsObject["foo"], Is.EqualTo("new property"));
            Assert.That(jsObject["bar"], Is.EqualTo(123456));
        }

        [Test]
        public void ReadExternalObject()
        {
            _context.SetParameter("myExternal", new ConvertFromJavascriptTests());
            Assert.That(_context.GetParameter("myExternal"), Is.InstanceOf<ConvertFromJavascriptTests>());
        }

        [Test]
        public void ReadDoubleByteChar()
        {
            _context.SetParameter("UniString", "呵呵呵呵呵");
            Assert.That(_context.GetParameter("UniString"), Is.EqualTo("呵呵呵呵呵"));
        }
    }
}
