using System;
using System.Collections.Generic;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;
using FluentAssertions;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Noesis.Javascript.Tests
{
    [TestClass]
    public class DebuggerTests
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
        public void CanCreateAndDisposeDebugger()
        {
            using var debugger = new JavascriptDebugger(_context, false);
            debugger.IsConnected.Should().BeTrue();
        }

        [TestMethod]
        public void DisposingDebuggerBeforeContextIsClean()
        {
            var debugger = new JavascriptDebugger(_context, false);
            debugger.Dispose();
            debugger.IsConnected.Should().BeFalse();

            // Context should still work after debugger is disposed.
            _context.Run("1 + 1").Should().Be(2);
        }

        [TestMethod]
        public void DisposingContextBeforeDebuggerIsClean()
        {
            var debugger = new JavascriptDebugger(_context, false);
            // Dispose context first - it should notify the debugger.
            _context.Dispose();
            // Debugger should now report not connected (context gone).
            debugger.IsConnected.Should().BeFalse();
            // Disposing the debugger afterwards should not throw.
            Action dispose = () => debugger.Dispose();
            dispose.Should().NotThrow();
        }

        [TestMethod]
        public void ReceivesResponseToDebuggerEnable()
        {
            using var debugger = new JavascriptDebugger(_context, false);

            var messages = new List<string>();
            debugger.MessageReceived += (_, e) => messages.Add(e.Message);

            // Queue Debugger.enable before Run(); it is dispatched at the start of Run().
            debugger.SendCommand("{\"id\":1,\"method\":\"Debugger.enable\"}");
            _context.Run("1 + 1");

            // Should have received a response with matching id.
            messages.Should().Contain(m => m.Contains("\"id\":1"));
        }

        [TestMethod]
        public async Task WaitsForDebugger()
        {
            using var debugger = new JavascriptDebugger(_context, true);
            var messages = new List<string>();
            debugger.MessageReceived += (_, e) => messages.Add(e.Message);

            var task = Task.Run(() => _context.Run("1 + 1;"));
            Thread.Sleep(100);
            debugger.IsPaused.Should().BeTrue("debugger should be paused waiting for runIfWaitingForDebugger");
            task.IsCompleted.Should().BeFalse("script should be waiting for debugger to attach");

            debugger.SendCommand("{\"id\":1,\"method\":\"Debugger.enable\"}");
            debugger.SendCommand("{\"id\":2,\"method\":\"Runtime.runIfWaitingForDebugger\"}");

            await task;

            debugger.IsPaused.Should().BeFalse("debugger should NOT be paused, run has completed");
        }

        [TestMethod]
        public void PausesAtDebuggerStatement()
        {
            using var debugger = new JavascriptDebugger(_context, false);

            bool pausedEventReceived = false;
            debugger.MessageReceived += (_, e) =>
            {
                var json = JsonDocument.Parse(e.Message);
                if (json.RootElement.TryGetProperty("method", out var method) &&
                    method.GetString() == "Debugger.paused")
                {
                    pausedEventReceived = true;
                    // Send resume to unblock execution.
                    debugger.SendCommand("{\"id\":2,\"method\":\"Debugger.resume\"}");
                }
            };

            debugger.SendCommand("{\"id\":1,\"method\":\"Debugger.enable\"}");
            _context.Run("debugger; 1 + 1;");

            pausedEventReceived.Should().BeTrue("a 'debugger' statement should pause execution");
        }

        [TestMethod]
        public void PausedEventContainsCallFrames()
        {
            using var debugger = new JavascriptDebugger(_context, false);

            string? pausedJson = null;
            debugger.MessageReceived += (_, e) =>
            {
                var json = JsonDocument.Parse(e.Message);
                if (json.RootElement.TryGetProperty("method", out var method) &&
                    method.GetString() == "Debugger.paused")
                {
                    pausedJson = e.Message;
                    debugger.SendCommand("{\"id\":2,\"method\":\"Debugger.resume\"}");
                }
            };

            debugger.SendCommand("{\"id\":1,\"method\":\"Debugger.enable\"}");
            _context.Run("debugger;", "test-script.js");

            pausedJson.Should().NotBeNull();
            var doc = JsonDocument.Parse(pausedJson!);
            doc.RootElement
               .GetProperty("params")
               .GetProperty("callFrames")
               .GetArrayLength()
               .Should().BeGreaterThan(0);
        }

        [TestMethod]
        public void ResumeAfterPauseAllowsScriptToComplete()
        {
            using var debugger = new JavascriptDebugger(_context, false);

            int resumeCount = 0;
            debugger.MessageReceived += (_, e) =>
            {
                var json = JsonDocument.Parse(e.Message);
                if (json.RootElement.TryGetProperty("method", out var method) &&
                    method.GetString() == "Debugger.paused")
                {
                    resumeCount++;
                    debugger.SendCommand($"{{\"id\":{resumeCount + 10},\"method\":\"Debugger.resume\"}}");
                }
            };

            debugger.SendCommand("{\"id\":1,\"method\":\"Debugger.enable\"}");

            // Script has two debugger statements; both should pause and resume.
            var result = _context.Run("debugger; debugger; 42;");

            resumeCount.Should().Be(2);
            result.Should().Be(42);
        }

        [TestMethod]
        public void SendCommandFromBackgroundThreadWhilePaused()
        {
            using var debugger = new JavascriptDebugger(_context, false);

            bool pausedEventReceived = false;
            debugger.MessageReceived += (_, e) =>
            {
                var json = JsonDocument.Parse(e.Message);
                if (json.RootElement.TryGetProperty("method", out var method) &&
                    method.GetString() == "Debugger.paused")
                {
                    pausedEventReceived = true;
                    // Send resume from a background thread.
                    Task.Run(() =>
                    {
                        Thread.Sleep(10); // Small delay to test async path.
                        debugger.SendCommand("{\"id\":2,\"method\":\"Debugger.resume\"}");
                    });
                }
            };

            debugger.SendCommand("{\"id\":1,\"method\":\"Debugger.enable\"}");
            _context.Run("debugger;");

            pausedEventReceived.Should().BeTrue();
        }

        [TestMethod]
        public async Task DisposingWhilePausedDoesNotDeadlock()
        {
            // Regression test: disposing the debugger from a non-V8 thread while
            // V8 is blocked in runMessageLoopOnPause previously deadlocked because
            // the destructor tried to acquire the V8 lock before signalling the
            // condition variable that unblocks the V8 thread.
            var debugger = new JavascriptDebugger(_context, false);

            // RunContinuationsAsynchronously prevents the await continuation from
            // running inline on the V8 thread (which fires this event), ensuring
            // Dispose() is always called from a separate thread.
            var pausedTcs = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
            debugger.MessageReceived += (_, e) =>
            {
                var json = JsonDocument.Parse(e.Message);
                if (json.RootElement.TryGetProperty("method", out var method) &&
                    method.GetString() == "Debugger.paused")
                {
                    pausedTcs.TrySetResult();
                }
            };

            debugger.SendCommand("{\"id\":1,\"method\":\"Debugger.enable\"}");

            // Run script on a background thread; it will pause at the debugger statement.
            var runTask = Task.Run(() => _context.Run("debugger; 1 + 1;"));

            // Wait until V8 has sent Debugger.paused, then give it a moment to
            // enter runMessageLoopOnPause (notification fires just before the call).
            await pausedTcs.Task.WaitAsync(TimeSpan.FromSeconds(5));
            Thread.Sleep(50);
            debugger.IsPaused.Should().BeTrue();

            // Dispose from this thread while V8 is paused — must not deadlock.
            debugger.Dispose();

            // The background Run() should complete now that the pause was broken.
            await runTask.WaitAsync(TimeSpan.FromSeconds(5));
            debugger.IsConnected.Should().BeFalse();
        }

        [TestMethod]
        public void SendCommandThrowsWhenDisposed()
        {
            var debugger = new JavascriptDebugger(_context, false);
            debugger.Dispose();

            Action send = () => debugger.SendCommand("{\"id\":1,\"method\":\"Debugger.enable\"}");
            send.Should().Throw<ObjectDisposedException>();
        }

        [TestMethod]
        public void MultipleRunsWithSameDebugger()
        {
            using var debugger = new JavascriptDebugger(_context, false);

            int pauseCount = 0;
            debugger.MessageReceived += (_, e) =>
            {
                var json = JsonDocument.Parse(e.Message);
                if (json.RootElement.TryGetProperty("method", out var method) &&
                    method.GetString() == "Debugger.paused")
                {
                    pauseCount++;
                    debugger.SendCommand($"{{\"id\":{pauseCount + 10},\"method\":\"Debugger.resume\"}}");
                }
            };

            debugger.SendCommand("{\"id\":1,\"method\":\"Debugger.enable\"}");

            _context.Run("debugger;");
            _context.Run("debugger;");

            pauseCount.Should().Be(2, "each Run() should see the debugger pause");
        }
    }
}
