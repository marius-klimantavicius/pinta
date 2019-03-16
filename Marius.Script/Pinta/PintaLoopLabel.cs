using Marius.Script.Pinta.Reflection;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Marius.Script.Pinta
{
    public class PintaLoopLabel
    {
        public PintaLabel BreakLabel { get; private set; }
        public PintaLabel ContinueLabel { get; private set; }

        public PintaLoopLabel(PintaLabel breakLabel, PintaLabel continueLabel)
        {
            BreakLabel = breakLabel;
            ContinueLabel = continueLabel;
        }
    }
}
