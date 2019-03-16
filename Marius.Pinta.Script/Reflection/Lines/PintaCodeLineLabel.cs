using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineLabel : PintaCodeLineCode
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.Label; } }

        public PintaCodeLabel Label { get; private set; }

        public int Offset { get; set; }
        public int Padding { get; set; }

        public PintaCodeLineLabel(PintaCode code, PintaCodeLabel label)
            : base(code)
        {
            Label = label;
        }

        public override string ToString()
        {
            return string.Format("{0} @{1}", GetCodeString(), Label.Id);
        }
    }
}
