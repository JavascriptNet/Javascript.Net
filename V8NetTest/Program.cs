using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Noesis.Javascript;

namespace JSUnitTest
{
    class Foo
    {
        public string doSomething()
        {
            return "did something";
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine(JavascriptContext.V8Version);

            JavascriptContext context = new JavascriptContext();
            context.SetParameter("$obj", new Foo());
            var result = context.Run(@"var x = $obj.doSomething(); x");

            Console.WriteLine((string)result);

            Console.ReadLine();
        }
    }
}
