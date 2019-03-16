using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection
{
    public class PintaCodeClosure
    {
        public uint ClosureIndex { get; private set; }
        public PintaCodeLocal Local { get; private set; }
        public PintaCodeParameter Parameter { get; private set; }

        public PintaCodeClosure(uint closureIndex, PintaCodeLocal local)
        {
            ClosureIndex = closureIndex;
            Local = local;
        }

        public PintaCodeClosure(uint closureIndex, PintaCodeParameter parameter)
        {
            ClosureIndex = closureIndex;
            Parameter = parameter;
        }
    }
}
