using System;
using System.Collections.Generic;
using System.Linq;
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
            using (JavascriptContext _context = new JavascriptContext()) {
                int[] ints = new[] { 1, 2, 3 };
                _context.SetParameter("bozo", new Bozo(ints));
                try {
                    object res = _context.Run("bozo[7];");
                } catch (Exception ex) {
                    string s = (string)ex.Data["V8StackTrace"];
                    Console.WriteLine(s);
                }
                //Console.WriteLine(ints[1]);
            }
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
