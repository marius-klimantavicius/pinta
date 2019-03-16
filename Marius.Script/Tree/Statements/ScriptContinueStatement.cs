using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptContinueStatement: ScriptStatement
    {
        public ScriptIdentifier Label { get; set; }

        public ScriptContinueStatement()
        {
        }

        public ScriptContinueStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptContinueStatement(ScriptIdentifier label)
        {
            Label = label;
        }

        public ScriptContinueStatement(ScriptIdentifier label, ScriptSourceSpan location)
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
            writer.Write("continue");
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
