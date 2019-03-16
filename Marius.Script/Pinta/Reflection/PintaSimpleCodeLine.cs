using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaSimpleCodeLine : PintaCodeLine
    {
        public PintaSimpleCodeLine(PintaCode code)
            : base(code)
        {
        }

        public override string ToString()
        {
            return GetCodeString();
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
