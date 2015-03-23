using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WixGen
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length != 2)
            {
                Console.WriteLine("Usage: WixGen configfile wixfile");
                return;
            }

            WixCreator wixCreator = new WixCreator();
            wixCreator.CreateWix(args[0], args[1]);
        }
    }
}
