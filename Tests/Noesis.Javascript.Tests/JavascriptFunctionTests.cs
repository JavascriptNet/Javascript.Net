﻿using System;
using System.Collections.Generic;
using System.Linq;
using FluentAssertions;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class JavascriptFunctionTests
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
        public void GetFunctionExpressionFromJsContext()
        {
            _context.Run("a = function(a, b) { return a + b; }");
            
            JavascriptFunction funcObj = _context.GetParameter("a") as JavascriptFunction;
            funcObj.Should().NotBeNull();
            funcObj.Call(1, 2).Should().BeOfType<int>().Which.Should().Be(3);
        }

        [TestMethod]
        public void GetNamedFunctionFromJsContext()
        {
            _context.Run("function test(a, b) { return a + b; }");
            
            JavascriptFunction funcObj = _context.GetParameter("test") as JavascriptFunction;
            funcObj.Should().NotBeNull();
            funcObj.Call(1, 2).Should().BeOfType<int>().Which.Should().Be(3);
        }

        [TestMethod]
        public void GetArrowFunctionExpressionFromJsContext()
        {
            _context.Run("a = (a, b) => a + b");
            
            JavascriptFunction funcObj = _context.GetParameter("a") as JavascriptFunction;
            funcObj.Should().NotBeNull();
            funcObj.Call(1, 2).Should().BeOfType<int>().Which.Should().Be(3);
        }

        [TestMethod]
        public void PassFunctionToMethodInManagedObjectAndUseItToFilterAList()
        {
            _context.SetParameter("collection", new CollectionWrapper());
            
            var result = _context.Run("collection.Filter(x => x % 2 === 0)") as IEnumerable<int>;
            result.Should().NotBeNull();
            result.Should().BeEquivalentTo(2, 4);
        }

        [TestMethod]
        public void ExceptionsAreHandledAndWrappedInAJavascriptExceptionObject()
        {
            var function = _context.Run("() => { throw new Error('test'); }") as JavascriptFunction;
            function.Should().NotBeNull();
            Action action = () => function.Call();
            action.ShouldThrowExactly<JavascriptException>().WithMessage("Error: test");
        }
    }

    class CollectionWrapper
    {
        private IEnumerable<int> numbers = new List<int> { 1, 2, 3, 4, 5 };

        public IEnumerable<int> Filter(JavascriptFunction predicate)
        {
            return numbers.Where(x => (bool) predicate.Call(x));
        }
    }
}
