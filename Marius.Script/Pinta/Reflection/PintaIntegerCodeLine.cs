using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaIntegerCodeLine : PintaCodeLine
    {
        public int Value { get; private set; }

        public PintaIntegerCodeLine(PintaCode code, int value)
            : base(code)
        {
            Value = value;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", GetCodeString(), Value);
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
