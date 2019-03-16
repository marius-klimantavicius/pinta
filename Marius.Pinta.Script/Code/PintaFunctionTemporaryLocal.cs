using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Reflection;

namespace Marius.Pinta.Script.Code
{
    public class PintaFunctionTemporaryLocal : IDisposable
    {
        private Stack<PintaFunctionTemporaryLocal> _temporary;

        public PintaCodeLocal Local { get; set; }

        public PintaFunctionTemporaryLocal(Stack<PintaFunctionTemporaryLocal> temporary, PintaCodeLocal local)
        {
            _temporary = temporary;
            Local = local;
        }

        public void Dispose()
        {
            _temporary.Push(this);
        }
    }
}
