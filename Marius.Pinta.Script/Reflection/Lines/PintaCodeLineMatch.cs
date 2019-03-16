using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineMatch : PintaCodeLineCode
    {
        public override PintaCodeLineType Type
        {
            get { return PintaCodeLineType.Match; }
        }

        public PintaCodeMatchFlags Flags { get; private set; }

        public PintaCodeLineMatch(PintaCodeMatchFlags flags) : base(PintaCode.Match)
        {
            Flags = flags;
        }

        public override string ToString()
        {
            var sb = new StringBuilder();
            sb.Append("match.");

            if (Flags == PintaCodeMatchFlags.Simple)
                sb.Append("simple");
            else if (Flags == PintaCodeMatchFlags.PrimaryAccountNumber)
                sb.Append("pan");
            else
                sb.Append("?");

            return sb.ToString();
        }
    }
}
