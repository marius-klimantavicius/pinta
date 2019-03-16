using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineCallInternal : PintaCodeLineCode
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.CallInternal; } }

        public uint FunctionId { get; private set; }
        public uint ArgumentsCount { get; private set; }

        public PintaCodeLineCallInternal(uint id, uint argumentsCount)
            : base(PintaCode.CallInternal)
        {
            FunctionId = id;
            ArgumentsCount = argumentsCount;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}, {2}", GetCodeString(), FunctionId, ArgumentsCount);
        }
    }
}
