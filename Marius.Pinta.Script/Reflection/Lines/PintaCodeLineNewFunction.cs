using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineNewFunction : PintaCodeLineCode
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.NewFunction; } }

        public PintaCodeFunction Function { get; private set; }

        public PintaCodeLineNewFunction(PintaCodeFunction function)
            : base(PintaCode.NewFunction)
        {
            Function = function;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", GetCodeString(), Function.Name);
        }
    }
}
