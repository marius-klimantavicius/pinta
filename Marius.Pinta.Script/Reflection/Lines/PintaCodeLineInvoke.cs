using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineInvoke : PintaCodeLineCode
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.Invoke; } }

        public uint ArgumentsCount { get; private set; }

        public PintaCodeLineInvoke(PintaCode code, uint argumentsCount)
            : base(code)
        {
            ArgumentsCount = argumentsCount;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", GetCodeString(), ArgumentsCount);
        }
    }
}
