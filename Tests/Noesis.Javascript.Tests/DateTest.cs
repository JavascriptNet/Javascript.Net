using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class DateTest
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
        public void SetDateTime()
        {
            _context.SetParameter("val", new DateTime(2010, 10, 10, 0, 0, 0, DateTimeKind.Utc));

            _context.Run("val.getUTCFullYear()").Should().BeOfType<int>().Which.Should().Be(2010);
            _context.Run("val.getUTCMonth()").Should().BeOfType<int>().Which.Should().Be(9);
            _context.Run("val.getUTCDate()").Should().BeOfType<int>().Which.Should().Be(10);
        }

        [TestMethod]
        public void SetAndReadDateTimeUtc()
        {
            var dateTime = new DateTime(2010, 10, 10, 0, 0, 0, DateTimeKind.Utc);
            _context.SetParameter("val", dateTime);

            var dateFromV8 = (DateTime) _context.Run("val");
            dateFromV8.ToUniversalTime().Should().Be(dateTime);
        }

        [TestMethod]
        public void SetAndReadDateTimeLocal()
        {
            var dateTime = new DateTime(2010, 10, 10, 0, 0, 0, DateTimeKind.Local);
            _context.SetParameter("val", dateTime);

            _context.Run("val").Should().Be(dateTime);
        }

        [TestMethod]
        public void SetAndReadDateTimeUnspecified()
        {
            var dateTime = new DateTime(2010, 10, 10);
            _context.SetParameter("val", dateTime);

            _context.Run("val").Should().Be(dateTime);
        }

        [TestMethod]
        public void CreateCurrentDateInJavaScript()
        {
            DateTime currentTimeAsReportedByV8 = (DateTime)_context.Run("new Date()");
            (currentTimeAsReportedByV8 - DateTime.Now).TotalSeconds.Should().BeLessThan(1, "Dates should be mostly equal");
        }

        [TestMethod]
        public void CreateFixedDateInJavaScript()
        {
            DateTime dateAsReportedByV8 = (DateTime)_context.Run("new Date(2010, 9, 10)");
            dateAsReportedByV8.Should().Be(new DateTime(2010, 10, 10));
        }
    }
}
