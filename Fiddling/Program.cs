using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Remoting.Contexts;
using System.Text;
using Noesis.Javascript;

namespace Fiddling
{
    /// <summary>
    /// This projects exists to try things out.  Also because I could not work out how
    /// to get the nunit tests to stop at break points.
    /// </summary>
    class Program
    {
        static void Main(string[] args)
        {
            JavascriptContext.SetFatalErrorHandler(FatalErrorHandler);
            using (JavascriptContext _context = new JavascriptContext()) {
                int[] ints = new[] { 1, 2, 3 };
                _context.SetParameter("bozo", new Bozo(ints));
                try {
                    //_context.Run("a=[]; while(true) a.push(0)");
                    //_context.Run("function f() { f(); } f()");
					while(true) {
						Console.Write("> ");
						var input = Console.ReadLine();
						if (input == "quit")
							break;
						var result = _context.Run(input);
						Console.WriteLine("Result is: `{0}`", result);
					}
                } catch (Exception ex) {
                    string s = (string)ex.Data["V8StackTrace"];
                    Console.WriteLine(s);
                }
                //Console.WriteLine(ints[1]);
            }
			Console.WriteLine("Press any key to end");
	        Console.ReadKey();
        }

        static void FatalErrorHandler(string a, string b)
        {
            Console.WriteLine(a);
            Console.WriteLine(b);
        }
    }

    class Bozo
    {
        Array a;
        internal Bozo(Array a) { this.a = a; }
        public object this[int i]
        {
            get { throw new ApplicationException("bozo");  return a.GetValue(i); }
            set { a.SetValue(value, i); }
        }
    }
}
