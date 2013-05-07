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
        public void GetFunctionFromJsContext()
        {
            _context.Run("a = function(a,b) {return a+b;}");
            
            JavascriptFunction funcObj = _context.GetParameter("a") as JavascriptFunction;
            funcObj.Should().NotBeNull();
            funcObj.Call(1, 2).Should().BeOfType<int>().Which.Should().Be(3);
        }
    }
}
