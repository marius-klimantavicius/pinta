using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaStringCodeLine : PintaCodeLine
    {
        public PintaProgramString Value { get; private set; }

        public PintaStringCodeLine(PintaCode code, PintaProgramString value)
            : base(code)
        {
            Value = value;
        }

        public override string ToString()
        {
            return GetCodeString() + " '" + Value.Value + "'";
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
