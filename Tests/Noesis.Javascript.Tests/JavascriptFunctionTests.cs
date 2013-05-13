using NUnit.Framework;

namespace Noesis.Javascript.Tests
{
    [TestFixture]
    public class JavascriptFunctionTests
    {
        [Test]
        public void GetFunctionFromJsContext()
        {            
            JavascriptContext context = new JavascriptContext();
            context.Run("a = function(a,b) {return a+b;}");
            
            JavascriptFunction funcObj = context.GetParameter("a") as JavascriptFunction;

            Assert.That(funcObj, Is.Not.Null);

            object result = funcObj.Call(1, 2);

            Assert.That(result, Is.EqualTo(3));            
        }
    }
}
