using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Marius.Pinta.Script.Reflection;

namespace Marius.Pinta.Script.Code
{
    public class PintaFunctionClosure
    {
        public PintaCodeClosure Closure { get; set; }

        public PintaFunctionClosure(PintaCodeClosure closure)
        {
            Closure = closure;
        }
    }
}
