using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaFunctionVariableCodeLine : PintaCodeLine
    {
        public PintaFunctionVariable Variable { get; private set; }

        public PintaFunctionVariableCodeLine(PintaCode code, PintaFunctionVariable variable)
            : base(code)
        {
            Variable = variable;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", GetCodeString(), Variable.DebugName);
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
