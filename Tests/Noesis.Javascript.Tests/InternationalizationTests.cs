using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using FluentAssertions;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class InternationalizationTests
    {
        [TestMethod]
        public void BasicButNotFullIcuAvailable()
        {
            using (var context = new JavascriptContext()) {
                // Confirm that internationalization support works.
                // (I can find no way to disable this, so you must distribute icudtl.dat.)
                context.Run(@"
const january = new Date(9e8);
const spanish = new Intl.DateTimeFormat('es', { month: 'long' });
spanish.format(january) === 'enero';").Should().BeOfType<bool>().Which.Should().BeTrue("it should be able to convert months into foreign languages");
            }
        }
    }
}
