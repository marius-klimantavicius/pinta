using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaTemporaryFunctionVariable : IDisposable
    {
        private PintaFunctionBuilder _function;

        public PintaFunctionVariable Variable { get; private set; }

        public PintaTemporaryFunctionVariable(PintaFunctionBuilder function)
        {
            _function = function;
            Variable = function.DefineVariable("$$temp");
        }

        public void Dispose()
        {
            _function.Free(this);
        }
    }
}
