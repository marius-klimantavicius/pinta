using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaCallInternalCodeLine : PintaCodeLine
    {
        public uint InternalFunctionToken { get; private set; }
        public uint ArgumentsLength { get; private set; }

        public PintaCallInternalCodeLine(uint internalFunctionToken, uint argumentsLength)
            : base(PintaCode.CallInternal)
        {
            InternalFunctionToken = internalFunctionToken;
            ArgumentsLength = argumentsLength;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}, {2}", GetCodeString(), InternalFunctionToken, ArgumentsLength);
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
