using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineCall : PintaCodeLineCode
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.Call; } }

        public PintaCodeFunction Function { get; private set; }
        public uint ArgumentsCount { get; private set; }

        public PintaCodeLineCall(PintaCodeFunction function, uint argumentsCount)
            : base(PintaCode.Call)
        {
            Function = function;
            ArgumentsCount = argumentsCount;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}, {2}", GetCodeString(), Function.Name, ArgumentsCount);
        }
    }
}
