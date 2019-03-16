using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaParameterCodeLine : PintaCodeLine
    {
        public PintaFunctionParameter Parameter { get; private set; }

        public PintaParameterCodeLine(PintaCode code, PintaFunctionParameter parameter)
            : base(code)
        {
            Parameter = parameter;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", GetCodeString(), Parameter.DebugName);
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
