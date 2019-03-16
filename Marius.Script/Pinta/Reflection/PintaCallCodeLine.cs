using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaCallCodeLine : PintaCodeLine
    {
        public PintaFunctionBuilder Function { get; private set; }
        public uint ArgumentsLength { get; private set; }

        public PintaCallCodeLine(PintaFunctionBuilder function, uint argumentsLength)
            : base(PintaCode.Call)
        {
            Function = function;
            ArgumentsLength = argumentsLength;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}, {2}", GetCodeString(), Function.Name, ArgumentsLength);
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
