using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineParameter : PintaCodeLineCode
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.Parameter; } }

        public PintaCodeParameter Parameter { get; private set; }

        public PintaCodeLineParameter(PintaCode code, PintaCodeParameter parameter)
            : base(code)
        {
            Parameter = parameter;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", GetCodeString(), Parameter.Id);
        }
    }
}
