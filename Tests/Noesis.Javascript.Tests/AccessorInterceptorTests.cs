using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;
using System;
using System.Dynamic;
using System.Collections.Generic;

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
			[JSEnumerable]
            public string MyProperty { get; set; }
			
			public string PropNonEnumerable { get; set; }
		}

        [TestMethod]
        public void AccessingByNameAPropertyInManagedObject()
        {
            _context.SetParameter("myObject", new ClassWithProperty { MyProperty = "This is the string return by \"MyProperty\"" });

            _context.Run("myObject.MyProperty == 'This is the string return by \"MyProperty\"'").Should().BeOfType<bool>().Which.Should().BeTrue();
        }

		[TestMethod]
		public void ObjectKeys_NoPropertyEnum_EmptyArray()
		{
			_context.SetParameter("obj", new { });

			var result = _context.Run("Object.keys(obj);");
			result.Should().BeOfType<object[]>().Which.Should().HaveCount(0);
		}

		[TestMethod]
		public void ObjectKeys_SinglePropertyEnum_OnePropertyName()
		{
			_context.SetParameter("myObject", new ClassWithProperty { MyProperty = "" });

			var result = _context.Run("Object.keys(myObject);");
			result.Should().BeOfType<object[]>().Which.Should().HaveCount(1);
			((object[]) result)[0].Should().Be("MyProperty");
		}


		[TestMethod]
		public void ObjectKeys_MDN_StringObj_ThreePropertyNames()
		{
			//> Object.keys("foo") => TypeError: "foo" is not an object		// ES5 code
			//> Object.keys("foo") => ["0", "1", "2"]						// ES2015 code
			_context.SetParameter("obj", "foo");

			var result = _context.Run("Object.keys(obj);");
			
			result.Should().BeOfType<object[]>().Which.Should().HaveCount(3);
			((object[])result)[0].Should().Be("0");
			((object[])result)[1].Should().Be("1");
			((object[])result)[2].Should().Be("2");
		}

		[TestMethod]
		public void ObjectKeys_MDN_ArrayThreePropertiesEnum_ThreePropertyNames()
		{
			//var arr = ['a', 'b', 'c'];
			//console.log(Object.keys(arr)); // console: ['0', '1', '2']
			_context.SetParameter("arr", new string[]{ "a","b","c" });
			
			var result = _context.Run("Object.keys(arr);");
			result.Should().BeOfType<object[]>().Which.Should().HaveCount(3);
			((object[])result)[0].Should().Be("0");
			((object[])result)[1].Should().Be("1");
			((object[])result)[2].Should().Be("2");
		}
		class ClassWithKeyValueAccess : Dictionary<string, object>
		{
			[JSEnumerable]
			public string MyProperty { get; set; }

			public string PropNonEnumerable { get; set; }
		}
		[TestMethod]
		[Ignore("Array like object construction not possible.")]
		public void ObjectKeys_MDN_ArrayLikeObjectEnum_ThreePropertyNames()
		{
			// array like object
			//var obj = { 0: 'a', 1: 'b', 2: 'c' };
			//console.log(Object.keys(obj)); // console: ['0', '1', '2']

			var obj = new ExpandoObject() as IDictionary<string, object>;
			obj.Add("0", "a");
			obj.Add("1", "b");
			obj.Add("2", "c");

			_context.SetParameter("obj", obj);

			var result = _context.Run("Object.keys(obj);");
			result.Should().BeOfType<object[]>().Which.Should().HaveCount(3);
			((object[])result)[0].Should().Be("0");
			((object[])result)[1].Should().Be("1");
			((object[])result)[2].Should().Be("2");
		}

		[TestMethod]
		[Ignore("Array like object construction not possible.")]
		public void ObjectKeys_MDN_ArrayLikeObjectWithRandomKeyOrdering_ThreeAsPropertyNames()
		{
			// array like object with random key ordering
			//var an_obj = { 100: 'a', 2: 'b', 7: 'c' };
			//console.log(Object.keys(an_obj)); // console: ['2', '7', '100']

			var obj = new ExpandoObject() as IDictionary<string, Object>;
			obj.Add("100", "a");
			obj.Add("2", "b");
			obj.Add("7", "c");

			_context.SetParameter("obj", obj);

			var result = _context.Run("Object.keys(obj);");
			result.Should().BeOfType<object[]>().Which.Should().HaveCount(3);
			((object[])result)[0].Should().Be("2");
			((object[])result)[1].Should().Be("7");
			((object[])result)[2].Should().Be("100");
		}
		
		class ClassWithFunctionProperty
		{
			[JSEnumerable]
			public void TestFunc() { }

			public void TestFuncNonEnumerable() { }
		}

		[TestMethod]
		public void ObjectKeys_FunctionAsProperty_EmptyArray()
		{
			_context.SetParameter("myObject", new ClassWithFunctionProperty() );

			var result = _context.Run("Object.keys(myObject);");
			result.Should().BeOfType<object[]>().Which.Should().HaveCount(0);
		}
		
		[TestMethod]
		public void ObjectKeys_FunctionAsParam_EmptyArray()
		{
			Func<double> fooFunc = () => 42;
			_context.SetParameter("myObject", fooFunc);

			var result = _context.Run("Object.keys(myObject);");
			result.Should().BeOfType<object[]>().Which.Should().HaveCount(0);
		}
		
		[TestMethod]
		public void ForInLoop_ObjectWithoutProperties_EmptyArray()
		{
			_context.SetParameter("myObject", new {  });

			var result = _context.Run(@"var result = [];
for(var prop in myObject) {
result.push(prop);
}
result;");
			result.Should().BeOfType<object[]>().Which.Should().HaveCount(0);
		}

		[TestMethod]
		public void ForInLoop_ObjectProperties_OnePropertyName()
		{
			_context.SetParameter("myObject", new ClassWithProperty() { });

			var result = _context.Run(@"var result = [];
for(var prop in myObject) {
result.push(prop);
}
result;");
			result.Should().BeOfType<object[]>().Which.Should().HaveCount(1);
			((object[])result)[0].Should().Be("MyProperty");
		}

		abstract class ClassAsSuperClass
		{
			[JSEnumerable]
			public string MySuperClassProperty { get; set; }

			public string MySuperClassPropertyNonEnumerable { get; set; }
		}

		class ClassAsSubClass : ClassAsSuperClass
		{
			[JSEnumerable]
			public string MySubClassProperty { get; set; }

			public string MySubClassPropertyNonEnumerable { get; set; }
		}

		[TestMethod]
		public void ObjectKeys_OnlyOwnPropertiesInInheritance_OneName()
		{
			_context.SetParameter("myObject", new ClassAsSubClass());
	
			var result = _context.Run("Object.keys(myObject);");
			result.Should().BeOfType<object[]>().Which.Should().HaveCount(1);
			((object[])result)[0].Should().Be("MySubClassProperty");
		}


		[TestMethod]
		public void ForInLoop_ObjectInheritanceProperties_OnlySubClassPropertyName()
		{
			_context.SetParameter("myObject", new { MyProperty = "" });

			var result = _context.Run(@"var result = [];
//for(var prop in myObject) {
//result.push(prop);
//}
//result;");
//			result.Should().BeOfType<object[]>().Which.Should().HaveCount(1);
//			((object[])result)[0].Should().Be("MyProperty");
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
    }
}