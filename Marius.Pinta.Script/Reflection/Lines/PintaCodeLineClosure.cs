using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineClosure : PintaCodeLineCode
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.Closure; } }

        public PintaCodeClosure Closure { get; private set; }

        public PintaCodeLineClosure(PintaCode code, PintaCodeClosure closure)
            : base(code)
        {
            Closure = closure;
        }

        public override string ToString()
        {
            if (Closure.Parameter != null)
                return string.Format("{0} {1}, {2}", GetCodeString(), 2 * Closure.ClosureIndex, Closure.Parameter.Id);
            if (Closure.Local != null)
                return string.Format("{0} {1}, {2}", GetCodeString(), 2 * Closure.ClosureIndex + 1, Closure.Local.Id);

            return string.Format("{0} {1}, ?", GetCodeString(), Closure.ClosureIndex);
        }
    }
}
