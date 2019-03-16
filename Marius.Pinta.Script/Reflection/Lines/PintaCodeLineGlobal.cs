using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineGlobal : PintaCodeLineCode
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.Global; } }

        public PintaCodeGlobal Global { get; private set; }

        public PintaCodeLineGlobal(PintaCode code, PintaCodeGlobal global)
            : base(code)
        {
            Global = global;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", GetCodeString(), Global.Name.Value);
        }
    }
}
