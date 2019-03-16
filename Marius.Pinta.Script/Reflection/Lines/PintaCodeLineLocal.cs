using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineLocal : PintaCodeLineCode
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.Local; } }

        public PintaCodeLocal Local { get; private set; }

        public PintaCodeLineLocal(PintaCode code, PintaCodeLocal local)
            : base(code)
        {
            Local = local;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", GetCodeString(), Local.Id);
        }
    }
}
