using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;
using System;

namespace Noesis.Javascript.Tests
{
    /// <summary>
    /// Tests for calling .NET methods from JavaScript contexts
    /// where Holder()->InternalFieldCount() == 0 (such as 'with' statements and Proxy contexts).
    /// </summary>
    [TestClass]
    public class MethodCachingWithProxyTests
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

        public class TestObject
        {
            public string TestValue { get; set; } = "original";

            public string GetTestValue()
            {
                return TestValue;
            }

            public int Add(int a, int b)
            {
                return a + b;
            }

            public string Echo(string message)
            {
                return "Echo: " + message;
            }
        }

        [TestMethod]
        public void MethodCalledDirectly_ShouldWork()
        {
            var testObj = new TestObject { TestValue = "direct call" };
            _context.SetParameter("obj", testObj);

            var result = _context.Run("obj.GetTestValue()");
            
            result.Should().Be("direct call");
        }

        [TestMethod]
        public void CachedMethodCalledFromWithStatement_ShouldWork()
        {
            var testObj = new TestObject { TestValue = "with statement" };
            _context.SetParameter("obj", testObj);

            // Cache the method by calling it first
            _context.Run("var cachedMethod = obj.GetTestValue;");
            
            // Now call the cached method from within a 'with' statement
            var result = _context.Run(@"
                (function() {
                    with(new Proxy({}, {})) {
                        return cachedMethod();
                    }
                })()
            ");
            
            result.Should().Be("with statement");
        }

        [TestMethod]
        public void CachedMethodWithParametersCalledFromWithStatement_ShouldWork()
        {
            var testObj = new TestObject();
            _context.SetParameter("obj", testObj);

            // Cache the method by calling it first
            _context.Run("var cachedAdd = obj.Add;");
            
            // Now call the cached method from within a 'with' statement with a Proxy
            var result = _context.Run(@"
                (function() {
                    with(new Proxy({}, {})) {
                        return cachedAdd(10, 32);
                    }
                })()
            ");
            
            result.Should().Be(42);
        }

        [TestMethod]
        public void CachedMethodCalledFromProxyContext_ShouldWork()
        {
            var testObj = new TestObject { TestValue = "proxy context" };
            _context.SetParameter("obj", testObj);

            // Cache the method
            _context.Run("var cachedEcho = obj.Echo;");
            
            // Call it through a Proxy get trap
            var result = _context.Run(@"
                var handler = {
                    get: function(target, prop) {
                        if (prop === 'cachedMethod') {
                            return cachedEcho;
                        }
                        return target[prop];
                    }
                };
                var proxy = new Proxy({}, handler);
                proxy.cachedMethod('Hello from proxy');
            ");
            
            result.Should().Be("Echo: Hello from proxy");
        }

        [TestMethod]
        public void MethodCalledMultipleTimesFromDifferentContexts_ShouldWork()
        {
            var testObj = new TestObject { TestValue = "multiple contexts" };
            _context.SetParameter("obj", testObj);

            var result = _context.Run(@"
                // Direct call
                var result1 = obj.GetTestValue();
                
                // Cache and call normally
                var cached = obj.GetTestValue;
                var result2 = cached();
                
                // Call from within 'with' statement
                var result3;
                with(new Proxy({}, {})) {
                    result3 = cached();
                }
                
                // All should return the same value
                result1 + ',' + result2 + ',' + result3;
            ");
            
            result.Should().Be("multiple contexts,multiple contexts,multiple contexts");
        }

        [TestMethod]
        public void MethodsCalledDirectlyFromDifferentObjects_ShouldWork()
        {
            var obj1 = new TestObject { TestValue = "object1" };
            var obj2 = new TestObject { TestValue = "object2" };
            _context.SetParameter("obj1", obj1);
            _context.SetParameter("obj2", obj2);

            // The fix ensures that methods called directly (obj.method()) work correctly
            // even when called from within 'with' or Proxy contexts
            var result = _context.Run(@"
                // Call methods directly (not cached in JS variables)
                var result1 = obj1.GetTestValue();
                var result2 = obj2.GetTestValue();
                
                result1 + ',' + result2;
            ");
            
            result.Should().Be("object1,object2");
            
            // Also verify that when called from within a 'with' statement, they still work
            var result2 = _context.Run(@"
                var result1, result2;
                with(new Proxy({}, {})) {
                    result1 = obj1.GetTestValue();
                    result2 = obj2.GetTestValue();
                }
                
                result1 + ',' + result2;
            ");
            
            result2.Should().Be("object1,object2");
        }

        [TestMethod]
        public void MethodCalledWithCallOrApply_ShouldWork()
        {
            var testObj = new TestObject { TestValue = "call/apply test" };
            _context.SetParameter("obj", testObj);

            var result = _context.Run(@"
                var cached = obj.Echo;
                
                // These should work because the method wrapper is embedded in function data
                var result1 = cached.call(null, 'test1');
                var result2 = cached.apply(null, ['test2']);
                
                result1 + '|' + result2;
            ");
            
            result.Should().Be("Echo: test1|Echo: test2");
        }

        [TestMethod]
        public void ComplexScenario_CachedFunctionInWithProxyAndIsolatedGlobals_ShouldWork()
        {
            // This is the exact scenario from the user's original error
            var testObj = new TestObject { TestValue = "complex scenario" };
            _context.SetParameter("Q", testObj);

            var result = _context.Run(@"
                var isolate_globals = { 
                    someGlobal: 'test' 
                };
                
                var cached_function = function() { 
                    with(new Proxy({}, isolate_globals)) { 
                        return Q.GetTestValue();
                    } 
                };
                
                cached_function();
            ");
            
            result.Should().Be("complex scenario");
        }
    }
}
