using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;
using System;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class AccessorInterceptorTests
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
        public void AccessAnElementInAManagedArray()
        {
            int[] myArray = new int[] { 151515, 666, 2555, 888, 99 };
            _context.SetParameter("myArray", myArray);

           _context.Run("myArray[2] == 2555").Should().BeOfType<bool>().Which.Should().BeTrue();
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

        [TestMethod]
        public void AccessingByIndexAPropertyInAManagedObject()
        {
            _context.SetParameter("myObject", new ClassWithIndexer { Value = "Value"});

            _context.Run("myObject[99] == 'Value 99'").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        class ClassWithProperty
        {
            public string MyProperty { get; set; }
        }

        [TestMethod]
        public void AccessingByNameAPropertyInManagedObject()
        {
            _context.SetParameter("myObject", new ClassWithProperty { MyProperty = "This is the string return by \"MyProperty\"" });

            _context.Run("myObject.MyProperty == 'This is the string return by \"MyProperty\"'").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

        class ClassWithDecimalProperty
        {
            public decimal D { get; set; }
        }

        [TestMethod]
        public void AccessingByNameADecimalPropertyInManagedObject()
        {
            var myObject = new ClassWithDecimalProperty { D = 42 };
            _context.SetParameter("myObject", myObject);

            _context.Run("myObject.D = 43; myObject.D").Should().BeOfType<int>().Which.Should().Be(43);
            myObject.D.Should().Be(43);
        }

        [TestMethod]
        public void GracefullyHandlesAttemptsToAccessByIndexerWhenIndexerDoesntExist()
        {
            _context.SetParameter("myObject", new ClassWithProperty());

            _context.Run("myObject[20] === undefined").Should().BeOfType<bool>().Which.Should().BeTrue(); ;
        }

        [TestMethod]
        public void SetValueByIndexerInManagedObject()
        {
            var classWithIndexer = new ClassWithIndexer();
            _context.SetParameter("myObject", classWithIndexer);

            _context.Run("myObject[20] = 'The value is now set'");

            classWithIndexer.Value.Should().Be("The value is now set");
            classWithIndexer.Index.Should().Be(20);
        }

        [TestMethod]
        public void SetPropertyByNameInManagedObject()
        {
            var classWithProperty = new ClassWithProperty();
            _context.SetParameter("myObject", classWithProperty);

            _context.Run("myObject.MyProperty = 'hello'");

            classWithProperty.MyProperty.Should().Be("hello");
        }

        [TestMethod]
        public void SettingUnknownPropertiesIsAllowed()
        {
            _context.SetParameter("myObject", new ClassWithProperty());

            _context.Run("myObject.UnknownProperty = 77");

            _context.Run("myObject.UnknownProperty").Should().Be(77);
        }

        [TestMethod]
        public void SettingUnknownPropertiesIsDisallowedIfRejectUnknownPropertiesIsSet()
        {
            _context.SetParameter("myObject", new ClassWithProperty(), SetParameterOptions.RejectUnknownProperties);

            Action action = () => _context.Run("myObject.UnknownProperty = 77");
            action.ShouldThrowExactly<JavascriptException>();
        }
        
        [TestMethod]
        public void GettingUnknownPropertiesIsDisallowedIfRejectUnknownPropertiesIsSet()
        {
            _context.SetParameter("myObject", new ClassWithProperty(), SetParameterOptions.RejectUnknownProperties);

            Action action = () => _context.Run("myObject.UnknownProperty");
            action.ShouldThrowExactly<JavascriptException>().Which.Message.Should().StartWith("Unknown member:");
        }

        class ClassForTypeCoercion
        {
            public bool BooleanValue { get; set; }
            public UriKind EnumeratedValue { get; set; }
        }

        [TestMethod]
        public void TypeCoercionToBoolean()
        {
            var my_object = new ClassForTypeCoercion();
            _context.SetParameter("my_object", my_object);
            _context.Run("my_object.BooleanValue = true");
            my_object.BooleanValue.Should().BeTrue();
        }

        [TestMethod]
        public void TypeCoercionStringToEnum()
        {
            var my_object = new ClassForTypeCoercion();
            _context.SetParameter("my_object", my_object);
            _context.Run("my_object.EnumeratedValue = 'Absolute'");
            my_object.EnumeratedValue.Should().Be(UriKind.Absolute);
        }

        [TestMethod]
        public void TypeCoercionNumberToEnum()
        {
            var my_object = new ClassForTypeCoercion();
            _context.SetParameter("my_object", my_object);
            _context.Run("my_object.EnumeratedValue = 1.0");
            my_object.EnumeratedValue.Should().Be(UriKind.Absolute);
        }
    }
}