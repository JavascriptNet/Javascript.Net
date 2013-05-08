using NUnit.Framework;

namespace Noesis.Javascript.Tests
{
    [TestFixture]
    public class VersionStringTests
    {
        [Test]
        public void RetrieveV8Version()
        {
            Assert.That(JavascriptContext.V8Version, Is.Not.Empty);
        }
    }
}
