using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;
using System.Collections.Generic;
using System.Linq;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class DateTest
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

        [TestMethod]
        public void SetDateTimeUtc()
        {
            _context.SetParameter("val", new DateTime(2010, 10, 10, 0, 0, 0, DateTimeKind.Utc));

            _context.Run("val.getUTCFullYear()").Should().BeOfType<int>().Which.Should().Be(2010);
            _context.Run("val.getUTCMonth()").Should().BeOfType<int>().Which.Should().Be(9);
            _context.Run("val.getUTCDate()").Should().BeOfType<int>().Which.Should().Be(10);
            _context.Run("val.getUTCHours()").Should().BeOfType<int>().Which.Should().Be(0);
            _context.Run("val.getUTCMinutes()").Should().BeOfType<int>().Which.Should().Be(0);
            _context.Run("val.getUTCSeconds()").Should().BeOfType<int>().Which.Should().Be(0);
        }

        [TestMethod]
        public void SetDateTimeLocal()
        {
            _context.SetParameter("val", new DateTime(2010, 10, 10, 0, 0, 0, DateTimeKind.Local));

            _context.Run("val.getFullYear()").Should().BeOfType<int>().Which.Should().Be(2010);
            _context.Run("val.getMonth()").Should().BeOfType<int>().Which.Should().Be(9);
            _context.Run("val.getDate()").Should().BeOfType<int>().Which.Should().Be(10);
            _context.Run("val.getHours()").Should().BeOfType<int>().Which.Should().Be(0);
            _context.Run("val.getMinutes()").Should().BeOfType<int>().Which.Should().Be(0);
            _context.Run("val.getSeconds()").Should().BeOfType<int>().Which.Should().Be(0);
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

        [TestMethod]
        public void SetDateTimeUtc_DateWhereTimezoneDatabaseIsImportant()
        {
            _context.SetParameter("val", new DateTime(1978, 6, 15, 0, 0, 0, DateTimeKind.Utc));

            _context.Run("val.getUTCFullYear()").Should().BeOfType<int>().Which.Should().Be(1978);
            _context.Run("val.getUTCMonth()").Should().BeOfType<int>().Which.Should().Be(5);
            _context.Run("val.getUTCDate()").Should().BeOfType<int>().Which.Should().Be(15);
            _context.Run("val.getUTCHours()").Should().BeOfType<int>().Which.Should().Be(0);
            _context.Run("val.getUTCMinutes()").Should().BeOfType<int>().Which.Should().Be(0);
            _context.Run("val.getUTCSeconds()").Should().BeOfType<int>().Which.Should().Be(0);
        }

        [TestMethod]
        public void SetDateTimeLocal_DateWhereTimezoneDatabaseIsImportant()
        {
            _context.SetParameter("val", new DateTime(1978, 6, 15, 0, 0, 0, DateTimeKind.Local));

            _context.Run("val.getFullYear()").Should().BeOfType<int>().Which.Should().Be(1978);
            _context.Run("val.getMonth()").Should().BeOfType<int>().Which.Should().Be(5);
            _context.Run("val.getDate()").Should().BeOfType<int>().Which.Should().Be(15);
            _context.Run("val.getHours()").Should().BeOfType<int>().Which.Should().Be(0);
            _context.Run("val.getMinutes()").Should().BeOfType<int>().Which.Should().Be(0);
            _context.Run("val.getSeconds()").Should().BeOfType<int>().Which.Should().Be(0);
        }

        //[TestMethod]
        public void SetAndReadDateTimeUtc_DateWhereTimezoneDatabaseIsImportant()
        {
            var dateTime = new DateTime(1978, 6, 15, 0, 0, 0, DateTimeKind.Utc);
            _context.SetParameter("val", dateTime);

            var dateFromV8 = (DateTime) _context.Run("val");
            dateFromV8.ToUniversalTime().Should().Be(dateTime); // this cannot work without an external dependency like NodaTime
        }

        //[TestMethod]
        public void SetAndReadDateTimeUtc_EdgeCaseOfAMonthWith30Days()
        {
            var dateTime = new DateTime(1967, 4, 1, 0, 0, 0, DateTimeKind.Utc);
            _context.SetParameter("val", dateTime);

            var dateFromV8 = (DateTime) _context.Run("val");
            dateFromV8.ToUniversalTime().Should().Be(dateTime); // this cannot work without an external dependency like NodaTime
        }

        //[TestMethod]
        public void SetAndReadDateTimeUtc_LeapYear()
        {
            var dateTime = new DateTime(1972, 2, 29, 0, 0, 0, DateTimeKind.Utc);
            _context.SetParameter("val", dateTime);

            var dateFromV8 = (DateTime) _context.Run("val");
            dateFromV8.ToUniversalTime().Should().Be(dateTime); // this cannot work without an external dependency like NodaTime
        }

        [TestMethod]
        public void SetAndReadDateTimeLocal_DateWhereTimezoneDatabaseIsImportant()
        {
            var dateTime = new DateTime(1978, 6, 15, 0, 0, 0, DateTimeKind.Local);
            _context.SetParameter("val", dateTime);

            _context.Run("val").Should().Be(dateTime);
        }

        [TestMethod]
        public void SetAndReadDateTimeLocal_EdgeCaseOfAMonthWith30Days()
        {
            var dateTime = new DateTime(1967, 4, 1, 0, 0, 0, DateTimeKind.Local);
            _context.SetParameter("val", dateTime);

            _context.Run("val").Should().Be(dateTime);
        }

        [TestMethod]
        public void SetAndReadDateTimeLocal_LeapYear()
        {
            var dateTime = new DateTime(1972, 2, 29, 0, 0, 0, DateTimeKind.Local);
            _context.SetParameter("val", dateTime);

            _context.Run("val").Should().Be(dateTime);
        }

        [TestMethod]
        public void SetAndReadDateTimeUnspecified_DateWhereTimezoneDatabaseIsImportant()
        {
            var dateTime = new DateTime(1978, 6, 15);
            _context.SetParameter("val", dateTime);

            _context.Run("val").Should().Be(dateTime);
        }

        [TestMethod]
        public void SetAndReadDateTimeUnspecified_EdgeCaseOfAMonthWith30Days()
        {
            var dateTime = new DateTime(1967, 4, 1);
            _context.SetParameter("val", dateTime);

            _context.Run("val").Should().Be(dateTime);
        }

        [TestMethod]
        public void SetAndReadDateTimeUnspecified_LeapYear()
        {
            var dateTime = new DateTime(1972, 2, 29);
            _context.SetParameter("val", dateTime);

            _context.Run("val").Should().Be(dateTime);
        }

        [TestMethod]
        public void CreateFixedDateInJavaScript_DateWhereTimezoneDatabaseIsImportant()
        {
            DateTime dateAsReportedByV8 = (DateTime) _context.Run("new Date(1978, 5, 15)");
            dateAsReportedByV8.Should().Be(new DateTime(1978, 6, 15));
        }

        [TestMethod]
        public void SetAndReadDateTimeLocal_DateRange()
        {
            var startDate = new DateTime(1900, 1, 1, 0, 0, 0, DateTimeKind.Local);
            var endDate = new DateTime(2100, 1, 1, 0, 0, 0, DateTimeKind.Local);
            var totalDays = (endDate - startDate).TotalDays;

            var errorDates = new List<string>();

            for (var i = 0; i < totalDays; i++)
            {
                var date = startDate.AddDays(i);
                _context.SetParameter("val", date);
                var result = (DateTime) _context.Run("val");
                if (result != date)
                    errorDates.Add($"Expected {date}, but got {result}");
            }

            if (errorDates.Any())
                Assert.Fail(string.Join("\n", errorDates));
        }
    }
}
