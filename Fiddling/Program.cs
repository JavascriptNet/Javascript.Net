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
        public class Product
        {
            public Product(decimal price)
            {
                Price = price;
            }
            public decimal Price { get; set; }
            public void DoSomething() { }
            public void DoSomethingElse() { }
            public IEnumerable<decimal> GetTaxes() => new List<decimal> { 0.01m, 0.02m };
            public decimal GetSalesTax(JavascriptFunction callback) => Convert.ToDecimal(callback.Call(Price));
            public override string ToString() => Price.ToString();
        }

        // ...

        static void Main(string[] args)
        {
            using (JavascriptContext context = new JavascriptContext())
            {
                try
                {
                    // breakpoint here
                    context.SetConstructor<Product>("Product", (Func<decimal, Product>)(price => new Product(price)));
                    context.SetParameter("globalProduct", new Product(2));
                    var result = context.Run($@"
{{
    const importantProduct = new Product(3);
    let sum = 0;
    for (let i = 0; i < 200_000; i++) {{

        // Commit 1 - creating managed objects from JS
        const product = new Product(Math.random());

        // Commit 2 - calling methods on managed objects
        product.DoSomething();
        product.DoSomething();
        product.DoSomethingElse();

        // Commits 3 and 4 - using iterators
        for (const tax of product.GetTaxes())
        {{
            sum += tax;
        }}

        // Commit 5 - using JS callbacks in managed code without disposing them explicitly
        sum += product.GetSalesTax(p => p * 0.19);
        // End of scenarios

        sum += product.Price;
    }}
    [sum, importantProduct.Price, globalProduct.Price].toString();
}}
");
                    Console.WriteLine(result);
                    Console.WriteLine(context.GetParameter("globalProduct"));
                    // breakpoint here - pre dispose of the context
                }
                catch (Exception ex)
                {
                    var s = (string)ex.Data["V8StackTrace"]!;
                    Console.WriteLine(s);
                }
            }
            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();
            // breakpoint here - after dispose of the context (the garbage collection was only triggered to compare the object count in the memory dump)
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
            get { throw new ApplicationException("bozo"); }
            set { a.SetValue(value, i); }
        }
    }
}
