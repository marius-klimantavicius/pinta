using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineString : PintaCodeLineCode
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.String; } }

        public PintaCodeString String { get; private set; }

        public PintaCodeLineString(PintaCode code, PintaCodeString value)
            : base(code)
        {
            String = value;
        }

        public override string ToString()
        {
            return GetCodeString() + " '" + String.Value + "'";
        }
    }
}
