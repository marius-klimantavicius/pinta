using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Marius.Pinta.Script.Reflection.Lines;

namespace Marius.Pinta.Script.Reflection
{
    public abstract class PintaCodeLine
    {
        public abstract PintaCodeLineType Type { get; }

        public override abstract string ToString();

        [DebuggerStepThrough]
        public T As<T>() where T: PintaCodeLine
        {
            return (T)this;
        }
    }
}
