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
                _context.SetParameter("myArray", new Bozo(ints));
                object res = _context.Run("var a = myArray[1]; myArray[1] = 17; a");
                Console.WriteLine(ints[1]);
            }
        }
    }

    class Bozo
    {
        Array a;
        internal Bozo(Array a) { this.a = a; }
        object this[int i]
        {
            get { return a.GetValue(i); }
            set { a.SetValue(value, i); }
        }
    }
}
