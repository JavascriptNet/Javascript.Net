using System;
using System.Reflection;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class MultipleAppDomainsTest
    {
#if NETFRAMEWORK
        private void ConstructContextInNewDomain()
        {
            var domainSetup = new AppDomainSetup { ApplicationBase = AppDomain.CurrentDomain.BaseDirectory };
            var domain = AppDomain.CreateDomain(typeof(MultipleAppDomainsTest).FullName, null, domainSetup);
            var javascriptNetAssembly = domain.Load(typeof(JavascriptContext).Assembly.FullName);
            domain.CreateInstance(typeof(JavascriptContext).Assembly.FullName, typeof(JavascriptContext).FullName);
        }

        [TestMethod]
        public void ConstructionContextInTwoDifferentAppDomainTests()
        {
            ConstructContextInNewDomain();
            ConstructContextInNewDomain();
        }
#else
        [TestMethod]
        public void ConstructionContextInTwoDifferentAppDomainTests()
        {
            // AppDomain.CreateDomain is not supported in .NET Core/.NET 8
            // This test is only applicable to .NET Framework
            Assert.Inconclusive("AppDomain.CreateDomain is not supported in .NET Core/.NET 8");
        }
#endif
    }
}
