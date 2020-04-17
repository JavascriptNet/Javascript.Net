using Microsoft.VisualStudio.TestTools.UnitTesting;
using FluentAssertions;
using System.Linq;
using System.Collections.Generic;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class AccessToStackTraceTest
    {
        private class StracktraceExporter
        {
            public JavascriptContext context { get; set; }

            public List<JavascriptStackFrame> frames(int depth)
            {
                return context.GetCurrentStack(depth);
            }
        }

        [TestMethod]
        public void TestSingleFrame()
        {
            JavascriptContext context = new JavascriptContext();
            context.SetParameter("obj", new StracktraceExporter { context = context });
            var frames = (object[])context.Run("obj.frames(1);", "Single Frame");

            var frame = (JavascriptStackFrame)frames.Single();
            frame.ScriptName.Should().Be("Single Frame");
            frame.ScriptNameOrSourceURL.Should().Be("Single Frame");
            frame.FunctionName.Should().BeNull();
            frame.IsConstructor.Should().Be(false);
            frame.IsEval.Should().Be(false);
            frame.IsWasm.Should().Be(false);
            frame.LineNumber.Should().Be(1);
            frame.Column.Should().Be(5);
        }

        [TestMethod]
        public void TestUnnamedFrame()
        {
            JavascriptContext context = new JavascriptContext();
            context.SetParameter("obj", new StracktraceExporter { context = context });
            context.Run("obj.frames(1)[0].ScriptName;").Should().Be(null);
        }

        [TestMethod]
        public void TestNestedFrame()
        {
            JavascriptContext context = new JavascriptContext();
            context.SetParameter("obj", new StracktraceExporter { context = context });
            context.Run("function func(depth, frame) {return obj.frames(depth)[frame];}", "func");

            context.Run("func(1, 0).ScriptName;", "bar").Should().Be("func");

            context.Run("func(2, 0).ScriptName;", "baz").Should().Be("func");
            context.Run("func(2, 1).ScriptName;", "baz").Should().Be("baz");
        }
    }
}
