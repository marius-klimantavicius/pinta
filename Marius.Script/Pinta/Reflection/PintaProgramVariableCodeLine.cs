using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaProgramVariableCodeLine : PintaCodeLine
    {
        public PintaProgramVariable Variable { get; private set; }

        public PintaProgramVariableCodeLine(PintaCode code, PintaProgramVariable variable)
            : base(code)
        {
            Variable = variable;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", GetCodeString(), Variable.Name);
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
