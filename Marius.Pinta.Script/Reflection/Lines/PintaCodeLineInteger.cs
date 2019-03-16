using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineInteger : PintaCodeLineCode
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.Integer; } }

        public int Value { get; private set; }

        public PintaCodeLineInteger(PintaCode code, int value)
            : base(code)
        {
            Value = value;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", GetCodeString(), Value);
        }
    }
}
