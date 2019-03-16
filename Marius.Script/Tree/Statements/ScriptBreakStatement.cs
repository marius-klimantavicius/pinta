using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptBreakStatement: ScriptStatement
    {
        public ScriptIdentifier Label { get; set; }

        public ScriptBreakStatement()
        {
        }

        public ScriptBreakStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptBreakStatement(ScriptIdentifier label)
        {
            Label = label;
        }

        public ScriptBreakStatement(ScriptIdentifier label, ScriptSourceSpan location)
            : base(location)
        {
            Label = label;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            writer.Write("break");
            if (Label != null)
            {
                writer.Write(" ");
                Label.WriteDebugView(writer);
            }
            writer.WriteLine(";");
        }
#endif
    }
}
