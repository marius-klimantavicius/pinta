using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using QUT.Gppg;

namespace Marius.Script.Tree
{
    public class ScriptSourceSpan: IMerge<ScriptSourceSpan>
    {
        public ScriptSourceLocation Start { get; private set; }
        public ScriptSourceLocation End { get; private set; }

        public ScriptSourceSpan()
        {
        }

        public ScriptSourceSpan(ScriptSourceLocation start, ScriptSourceLocation end)
        {
            Start = start;
            End = end;
        }

        public ScriptSourceSpan Merge(ScriptSourceSpan last)
        {
            return new ScriptSourceSpan(Start, last.End);
        }

        public override string ToString()
        {
            return string.Format("({0}, {1})", Start.Line, Start.Column);
        }
    }
}
