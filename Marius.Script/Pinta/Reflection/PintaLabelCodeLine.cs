using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Pinta.Reflection
{
    public class PintaLabelCodeLine : PintaCodeLine
    {
        public PintaLabel Label { get; private set; }

        public PintaLabelCodeLine(PintaCode code, PintaLabel label)
            : base(code)
        {
            Label = label;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", GetCodeString(), Label.Id);
        }

        public override void Accept(IPintaNodeVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
