using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Marius.Pinta.Script.Reflection.Lines
{
    public class PintaCodeLineLabelMark : PintaCodeLine
    {
        public override PintaCodeLineType Type { get { return PintaCodeLineType.LabelMark; } }

        public PintaCodeLabel Label { get; private set; }

        public PintaCodeLineLabelMark(PintaCodeLabel label)
        {
            Label = label;
        }

        public override string ToString()
        {
            return string.Format("@{0}:", Label.Id);
        }
    }
}
