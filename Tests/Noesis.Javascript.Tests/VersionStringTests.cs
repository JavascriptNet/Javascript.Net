using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class VersionStringTests
    {
        [TestMethod]
        public void RetrieveV8Version()
        {
            JavascriptContext.V8Version.Should().NotBeEmpty();
        }
    }
}
