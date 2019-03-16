using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Linq.Expressions;

namespace Marius.Script.Tree
{
    public class ScriptLabelStatement: ScriptStatement
    {
        public ScriptIdentifier Label { get; set; }
        public ScriptStatement Statement { get; set; }

        public ScriptLabelStatement()
        {
        }

        public ScriptLabelStatement(ScriptIdentifier label, ScriptStatement statement)
        {
            Label = label;
            Statement = statement;
        }

        public ScriptLabelStatement(ScriptSourceSpan location)
            : base(location)
        {
        }

        public ScriptLabelStatement(ScriptIdentifier label, ScriptStatement statement, ScriptSourceSpan location)
            : base(location)
        {
            Label = label;
            Statement = statement;
        }

        public override void Accept(IScriptVisitor visitor)
        {
            visitor.Visit(this);
        }

#if DEBUG
        public override void WriteDebugView(System.CodeDom.Compiler.IndentedTextWriter writer)
        {
            Label.WriteDebugView(writer);
            writer.WriteLine(":");

            Statement.WriteDebugView(writer);
        }
#endif
    }
}
