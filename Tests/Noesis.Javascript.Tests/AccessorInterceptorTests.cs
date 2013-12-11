using NUnit.Framework;

namespace Noesis.Javascript.Tests
{
    [TestFixture]
    public class AccessorInterceptorTests
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
        public void AccessAnElementInAManagedArray()
        {
            int[] myArray = new int[] { 151515, 666, 2555, 888, 99 };
            _context.SetParameter("myArray", myArray);

            Assert.That(_context.Run("myArray[2] == 2555"), Is.True);
        }

        class ClassWithIndexer
        {
            public int Index { get; set; }
            public string Value { get; set; }

            public string this[int iIndex]
            {
                get { return (Value + " " + iIndex); }
                set { 
                    Value = value;
                    Index = iIndex;
                }
            }
        }

        [Test]
        public void AccessingByIndexAPropertyInAManagedObject()
        {
            _context.SetParameter("myObject", new ClassWithIndexer { Value = "Value"});

            Assert.That(_context.Run("myObject[99] == 'Value 99'"), Is.True);
        }

        class ClassWithProperty
        {
            public string MyProperty { get; set; }
        }

        [Test]
        public void AccessingByNameAPropertyInManagedObject()
        {
            _context.SetParameter("myObject", new ClassWithProperty { MyProperty = "This is the string return by \"MyProperty\"" });

            Assert.That(_context.Run("myObject.MyProperty == 'This is the string return by \"MyProperty\"'"), Is.True);
        }

        [Test]
        public void GracefullyHandlesAttemptsToAccessByIndexerWhenIndexerDoesntExist()
        {
            _context.SetParameter("myObject", new ClassWithProperty());

            Assert.That(_context.Run("myObject[20] === undefined"), Is.True);
        }

        [Test]
        public void SetValueByIndexerInManagedObject()
        {
            var classWithIndexer = new ClassWithIndexer();
            _context.SetParameter("myObject", classWithIndexer);

            _context.Run("myObject[20] = 'The value is now set'");

            Assert.That(classWithIndexer.Value, Is.EqualTo("The value is now set"));
            Assert.That(classWithIndexer.Index, Is.EqualTo(20));
        }

        [Test]
        public void SetPropertyByNameInManagedObject()
        {
            var classWithProperty = new ClassWithProperty();
            _context.SetParameter("myObject", classWithProperty);

            _context.Run("myObject.MyProperty = 'hello'");

            Assert.That(classWithProperty.MyProperty, Is.EqualTo("hello"));
        }

        [Test]
        public void SettingUnknownPropertiesIsAllowed()
        {
            _context.SetParameter("myObject", new ClassWithProperty());

            _context.Run("myObject.UnknownProperty = 77");

            Assert.That(_context.Run("myObject.UnknownProperty"), Is.EqualTo(77));
        }

        [Test]
        public void SettingUnknownPropertiesIsDisallowedIfRejectUnknownPropertiesIsSet()
        {
            _context.SetParameter("myObject", new ClassWithProperty(), SetParameterOptions.RejectUnknownProperties);

            Assert.Throws<JavascriptException>(() => _context.Run("myObject.UnknownProperty = 77"));
        }
        
        [Test]
        public void GettingUnknownPropertiesIsDisallowedIfRejectUnknownPropertiesIsSet()
        {
            _context.SetParameter("myObject", new ClassWithProperty(), SetParameterOptions.RejectUnknownProperties);

            Assert.Throws<JavascriptException>(() => _context.Run("myObject.UnknownProperty")).Message.StartsWith("Unknown member:");
        }
    }
}